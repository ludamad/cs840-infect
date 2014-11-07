#include <cmath>
#include "network.h"

using namespace std;

void State::connect(entity_id A, entity_id B, double transmission_prob) {
	get(A).connect(B, transmission_prob);

}

void State::biconnect(entity_id A, entity_id B, double transmission_prob) {
	if (A == B) {
		return;
	}
	connect(A, B, transmission_prob);
	connect(B, A, transmission_prob);
}

void State::init(const Settings& S) {
	// Make our infection structure aware of the maximum amount of nodes:
	active_infections.init(S.size);
	rng.init_genrand(S.seed);
	entities.resize(S.size);
	printf("Creating network of size %d\n", S.size);
	time_elapsed = 0;
	n_steps = 0, n_infections = 0;
	halflife = S.halflife;
	time_interval_overage = 0;

	int rows = sqrt(S.size);
	ASSERT(rows*rows == S.size, "Only dealing with square graphs for now!");
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < rows; x++) {
			int nx = (x+1) % rows, ny = (y+1)%rows;
			entity_id A = y*rows+x; // Connect from this entity
			// This ensures everyone is connected to their neighbours, once it has completed.
			biconnect(A, y*rows+nx, S.transmission_prob);
			biconnect(A, ny*rows+x, S.transmission_prob);
			biconnect(A, ny*rows+nx, S.transmission_prob);
		}
	}

//	// Create the connectivity graph:
//	for (Entity& e : entities) {
//		std::vector<entity_id> influence_set;
//		std::vector<double> transmission_probs;
//		for (entity_id e_id = 0; e_id < size(); e_id++) {
//			if (&e != &entities[e_id]) {
//				// For every node other than ourselves, connect with 'connectivity' probability
//				if (rng.rand_real_not1() < S.connectivity) {
//					influence_set.push_back(e_id);
//					transmission_probs.push_back(S.transmission_prob);
//				}
//			}
//		}
//		e = Entity(influence_set, transmission_probs);
//	}

	for (int i = 0; i < entities.size(); i++) {
		entities[i].preprocess();
	}
}

// Carefully picked to form a PDF
// DECAY_MIN_INTERVAL: Essentially a dynamic sampling frequency
// Smaller means more accuracy but slower time progression.
static double DECAY_MIN_INTERVAL = 0.01005033585350145;
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
//	printf("INFECTING (%d) -> (%d)\n", this->last_infector, infected_id);
	e.infected = true;
	active_infections.insert(infected_id, e.transmission_total);
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
	entity_id infector_id = active_infections.random_select(rng);
	this->last_infector = infector_id;
//	printf("Infector = %d\n", infector_id);
//	fflush(stdout);
	Entity& e = entities.at(infector_id);
	return e.pick_influence(rng);
}
