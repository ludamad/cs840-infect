#include <cmath>
#include <set>

#include "libs/perf_timer.h"

#include "state.h"

using namespace std;

void State::connect(entity_id A, entity_id B, double transmission_prob) {
	PERF_TIMER();
	if (A == B) {
		return;
	}
	get(A).connect(B, transmission_prob);
}

void State::generate_graph() {
	int size = entities.size();
	int rows = sqrt(size);
	ASSERT(rows*rows == size, "Logic error");
	vector<double> interests(size, 0);
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < rows; x++) {
			interests[y*rows+x] = rng.rand_real_not0();
		}
	}
#define FOR_ID(x, y, A) \
	for (int y = 0; y < rows; y++) \
		for (int x = 0, A = y * rows; x < rows; x++, A++)

	printf("CONNECTING ENTITIES\n");
	if (true) {
		MilestoneRep rep;
		FOR_ID(x, y, A) {
			rep.report("Connected %d entities");
			// How far away to effect?
			double SxD = 3, SyD = 3;
			for (int i = 0; i < 10; i++) {
				SxD += rng.rand_real_with01();
				SyD += rng.rand_real_with01();
				if (rng.random_chance(0.5)) {
					break;
				}
			}
			int Sx = (int)SxD, Sy = (int)SyD;
			double popularity = rng.rand_real_not0();
			int n_squares = (1+2*Sx) * (1+ 2 * Sy) - 1;
			for (int sy = -Sy; sy <= Sy; sy++) {
				for (int sx = -Sx; sx <= Sx; sx++) {
					int nx = (x+rows+sx) % rows, ny = (y+rows+sy)%rows;
					// This ensures everyone is connected to their neighbours, once it has completed.
					if (sx != 0 || sy != 0) {
						entity_id id = ny*rows+nx;
						connect(A, id, popularity / n_squares / 30);
					}
				}
			}
		}
//		FOR_ID(x, y, A) {
//			set<entity_id> used;
//			double popularity = 1.0;
//			double connections = 0;
//			if (rng.random_chance(0.001)) {
//				connections = 10;
//			}
//			for (int i = 0; i < connections; i++) {
//				int B = rng.rand_int(S.size);
//				if (A != B && used.find(B) == used.end()) {
//					connect(A, B, popularity * interests[B]);
//					used.insert(B);
//				}
//			}
//		}
	}

	if (false) {
		for (int y = 0; y < rows; y++) {
			for (int x = 0; x < rows; x++) {
				set<entity_id> used;
				entity_id A = y*rows+x; // Connect from this entity

				double popularity = 0.0075;
				double connections = 10;
				if (rng.random_chance(0.01)) {
					popularity = 0.005;
					connections = 100;
				}

				for (int i = 0; i < connections; i++) {
		//				int sx = rng.rand_int(-10, 11) + rng.rand_int(-10, 11) + rng.rand_int(-10, 11);
		//				int sy = rng.rand_int(-10, 11) + rng.rand_int(-10, 11) + rng.rand_int(-10, 11);
					int sx = rng.rand_int(-10, 11);
					int sy = rng.rand_int(-10, 11);
					int nx = (x+sx+rows) % rows, ny = (y+sy+rows)%rows;
					if (sx != 0 || sy != 0) {
						int id = ny*rows+nx;
						if (used.find(id) == used.end()) {
							connect(A, id, popularity * interests[id]);///max(abs(sx), abs(sy)));
							used.insert(id);
						}
					}
				}
		}
		}
	}

	printf("PREPROCESSING ENTITIES\n");
	MilestoneRep rep;
	for (int i = 0; i < entities.size(); i++) {
		rep.report("Preprocessed %d entities");
		PERF_TIMER2("walker method preprocess");
		entities[i].preprocess();
	}
}

void State::init(const Config& C) {
	PERF_TIMER();
	// Make our infection structure aware of the maximum amount of nodes:
	active_infections.init(C.size);
	rng.init_genrand(C.seed);
	entities.resize(C.size);
	time_elapsed = 0;
	n_steps = 0, n_infections = 0;
	halflife = C.halflife;
	time_interval_overage = 0;
}

// Carefully picked to form a PDF
// DECAY_MIN_INTERVAL: Essentially a dynamic sampling frequency
// Smaller means more accuracy but slower time progression.
static const double HALFLIFE = 60;
static double DECAY_MIN_INTERVAL = 0.01005033585350145 * HALFLIFE;
static double DECAY_MULTIPLIER = 0.99;
static double TIME_SUM_CORRECTION = 1.0/1.0005000833332613;
static double C1 = DECAY_MIN_INTERVAL * TIME_SUM_CORRECTION;
static double C2 = 1.0 / C1;

// Inverse operation of current_timestep below. Used in below constants calculation.
static double inv_current_timestep(double dt) {
	return (1 / dt) * C1;
}
static double DECAY_MIN_INTERVAL_WEIGHT = inv_current_timestep(DECAY_MIN_INTERVAL);

double State::current_timestep() {
	return 1 / active_infections.total_weight() * C2;
}

// After down-adjusting the timestep, is the resulting step have no change to the graph?
static bool test_if_null_step(MTwist& rng, double weight, double* delta_time) {
	*delta_time = DECAY_MIN_INTERVAL;
	double r = rng.rand_real_not0() * DECAY_MIN_INTERVAL_WEIGHT;
	return (r > weight);
}

void State::step() {
	PERF_TIMER();

	entity_id infected_id; // declared here to satisfy 'goto' constraints

	// TODO: Revise this comment
	// We employ the rejection method throughout for simplicity.
	// If a generated event fails to meet a constraint, we can simply restart the whole process.
	// This allows simpler weight calculations as we can use weights oblivious to the constraint.

	double delta_time = current_timestep();
	if (delta_time > DECAY_MIN_INTERVAL) {
		// Will down-adjust delta time to be DECAY_MIN_INTERVAL
		// To compensate, check if the adjusted action should be result in nothing happening.
		if (test_if_null_step(rng, active_infections.total_weight(), &delta_time)) {
			goto afterinfection;
		}
	}

	// Find the event that occurs (an infection)
	// NOTE: It may *seem* like we should employ the rejection method here,
	// but that would invalidate our timestep logic.
	// Since the timestep is overestimated with respect to invalid infections,
	// we simply do nothing but step time if a valid infection does not occur.
	infected_id = generate_potential_infection();
	try_infection(infected_id);

	afterinfection:
	n_steps++;
	// Pass time:
	time_interval_overage += delta_time;
	while (time_interval_overage > DECAY_MIN_INTERVAL) {
		active_infections.scale(DECAY_MULTIPLIER);
		time_interval_overage -= DECAY_MIN_INTERVAL;
	}
	time_elapsed += delta_time;
}
bool State::try_infection(entity_id infected_id) {
        Entity& e = entities[infected_id];
	if (e.infected) {
		return false;
	}
	if (on_infect_func) { PERF_TIMER2("on_infect callback"); on_infect_func(infected_id); }
//	printf("INFECTING (%d) -> (%d)\n", this->last_infector, infected_id);
	e.infected = true;
	active_infections.insert(infected_id, e.transmission_prob_total);
	n_infections++;
	// We have found a valid action
	return true;
}

void State::infect_n_random(int n) {
	// Uses rejection method implicitly:
	while (n > 0) {
		entity_id id = rng.rand_int(size());
		if (try_infection(id)) {
			n--;
		}
	}
}

entity_id State::generate_potential_infection() {
	PERF_TIMER();
	entity_id infector_id = active_infections.random_select(rng);
	this->last_infector = infector_id;
//	printf("Infector = %d\n", infector_id);
//	fflush(stdout);
	Entity& e = entities.at(infector_id);
	return e.pick_influence(rng);
}

void State::fast_reset(Config& S) {
    active_infections = Config::InfectionSet(S.size);
    time_elapsed = 0;
    n_steps = 0, n_infections = 0;
    halflife = S.halflife;
    time_interval_overage = 0;
    for (Entity& e : entities) {
    	e.infected = false;
    }
}
