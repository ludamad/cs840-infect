#include <cmath>
#include <set>
#include <iostream>

#include "libs/perf_timer.h"

#include "state_alt.h"

using namespace std;

void StateAlt::init(const Config& C) {
	PERF_TIMER();
	// Make our infection structure aware of the maximum amount of nodes:
	rng.init_genrand(C.seed);
	time_elapsed = 0;
	n_steps = 0, n_infections = 0;
	mean = (1.0 / C.halflife);
}

void StateAlt::queue_infection(entity_id id) {
	EntityAlt& e = get(id);
	if (e.infected) {
		return;
	}
	// Generate time of infection according to exponential distribution:
	double t = time_elapsed + rng.expovariate(1);
	if (e.event_handle.node_) {
		if (t < (*e.event_handle).time) {
			event_queue.decrease(e.event_handle, {t, id});
		}
	} else {
		e.event_handle = event_queue.push({t, id});
	}
}

void StateAlt::process_infection(entity_id id, double time) {
	EntityAlt& e = get(id);
	e.infected = true;
	if (on_infect_func != NULL) {
		on_infect_func(id);
	}
	for (int i = 0; i < e.node.size(); i++) {
		double p = e.node[i].prob;
		if (rng.rand_real_not0() < p) {
			queue_infection(e.node[i].node);
		}
	}
}

void StateAlt::infect_n_random(int n) {
	// Uses rejection method implicitly:
	while (n > 0) {
		entity_id id = rng.rand_int(size());
		if (!get(id).infected) {
			process_infection(id, 0);
			n--;
		}
	}
}

void StateAlt::step() {
	ASSERT(!event_queue.empty(), "Can't step!");
	InfectionEvent event = event_queue.top();
	ASSERT(event.time >= time_elapsed, "Time cannot go backwards!");
	event_queue.pop();
	time_elapsed = event.time;
	process_infection(event.infected, event.time);
}

void StateAlt::fast_reset(Config& S) {
	for (auto& entity : entities) {
		entity.infected = false;
		entity.event_handle = EventHandle();
	}
	time_elapsed = 0;
	event_queue.clear();
}
