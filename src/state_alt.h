#ifndef STATE_ALT_H_
#define STATE_ALT_H_

#include <cmath>
#include <string>
#include <utility>

#include "boost/heap/binomial_heap.hpp"
#include "boost/heap/fibonacci_heap.hpp"
#include "boost/heap/priority_queue.hpp"

#include "libs/mtwist.h"

#include "config.h"
#include "discrete_fixedtree.h"
#include "graph.h"

/*
 * There are two approaches:
 * Approach 1:
 * 		- preprocess weights to get O(1) neighbour selection
 *		- use random selection of active infections
 *		- find neighbour using O(1) selection
 *		- advance time
 * Approach 2:
 * 	    - For all initially active infections, do a weighted coin flip over neighbours to determine if the infection spreads.
 *	    - Pick the minimum time infection that was generated, and consider its neighbours.
 *	    - Repeat until set of infections is empty.
 *
 * This is the second approach.
 * Unlike State, StateAlt uses Graph directly.
 */

// A not-yet processed infection event.
struct InfectionEvent {
	double time;
	entity_id infected;
	bool operator<(const InfectionEvent& o) const {
		return time > o.time;
	}
};

typedef boost::heap::fibonacci_heap<InfectionEvent> EventQueue;
typedef EventQueue::handle_type EventHandle;

struct EntityAlt {
	Node node;
	bool infected = false;
	EventHandle event_handle;
	READ_WRITE(rw) {
		rw << node;
	}
};

struct StateAlt {
	typedef void (*oninfectf)(int infected_Id);
    size_t size() {
    	return entities.size();
    }

	void init(const Config& C);
	void set_graph(Graph&& g) {
		entities.resize(g.size());
		for (int i = 0; i < g.size(); i++) {
			entities[i].node = std::move(g[i]);
		}
	}

	READ_WRITE(rw) {
		rw << mean;
		rw.visit_objs(entities);
	}
    void step();
    void queue_infection(entity_id id);
    void process_infection(entity_id id, double time);
    void infect_n_random(int n);

    EntityAlt& get(entity_id id) {
    	return entities.at(id);
    }
    double current_timestep() {
    	return time_elapsed;
    }
    void fast_reset(Config& C);

    bool finished(Config& C) const {
    	return event_queue.empty();
    }

    double total_weight() {
    	return 0;
    }
public:
    // Incremental drawing function:
    oninfectf on_infect_func = NULL;
    // The RNG:
    MTwist rng;
    // The graph:
    std::vector<EntityAlt> entities;
    double mean = -1, time_elapsed = 0;
    size_t n_steps = 0, n_infections = 0;
    // Events:
    EventQueue event_queue;
};

#endif /* STATE_ALT_H_ */
