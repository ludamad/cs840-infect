#ifndef STATE_H_
#define STATE_H_

#include <cmath>
#include <string>

#include "libs/mtwist.h"

#include "config.h"
#include "discrete_fixedtree.h"
#include "discrete_searchtree.h"
#include "graph.h"

/*****************************************************************************
 * An entity in the random generation simulation
 *****************************************************************************/


// Code modified from github.com/ntamas/netctrl
struct WalkerConnections {
	// Returns -1 on invalid event
	entity_id pick(MTwist& rng) {
		size_t m = connections.size();
		return connections[rng.rand_int(m)].pick(*this, rng);
	}
	void init(const Node& node) {
		// Initial values:
		size_t n = node.size();
    	connections.resize(n);
    	was_infected.resize(n);
	    std::vector<int> shortI, longI;
	    shortI.reserve(n), longI.reserve(n);
		total_prob = 0;
    	for (auto& edge : node) {
    		total_prob += edge.prob;
    	}

    	// Initialize with normalized probabilities, scaled by 'n':
    	for (int i = 0; i < n; i++) {
    		connections[i].choice_a_prob = node[i].prob / total_prob * n;
    		connections[i].choice_a = node[i].node;
    		connections[i].choice_b_index = -1;
    		connections[i].was_chosen = false;
    	}

	    // Initialize shortIndexes and longIndexes
    	for (int i = 0; i < n; i++) {
	        bool is_short = connections[i].choice_a_prob < 1;
	        (is_short ? shortI : longI).push_back(i);
	    }

	    // Prepare the tables (O(n) time)
	    while (!shortI.empty() && !longI.empty()) {
	        int S = shortI.back(), L = longI.back();
	        shortI.pop_back();
	        connections[S].choice_b_index = L;
	        connections[L].choice_a_prob -= (1 - connections[S].choice_a_prob);
	        if (connections[L].choice_a_prob < 1) {
	            shortI.push_back(L);
	            longI.pop_back();
	        }
	    }
	}
public:
	// A connection, preprocessed using the walker method
	struct Connection {
		double choice_a_prob;
		entity_id choice_a;
		int choice_b_index;
		bool was_chosen;
		// Return -1 if invalid event generated:
		entity_id pickA() {
			if (was_chosen) {
				return -1;
			}
			was_chosen = true;
			return choice_a;
		}
		entity_id pick(WalkerConnections& W, MTwist& rng) {
			bool use_choice_a = (rng.rand_real_not1() < choice_a_prob);
			return use_choice_a ? pickA() : W.connections[choice_b_index].pickA();
		}
	};
	std::vector<Connection> connections;
	std::vector<bool> was_infected;
	double total_prob = 0;
};
//
//struct DynamicConnections {
//	entity_id pick_influence(MTwist& rng) {
//		entity_id id = search_tree.random_select(rng);
//
//	}
//
//	DiscreteSearchTree search_tree;
//};

struct Entity {
	entity_id pick_influence(MTwist& rng) {
		return influence_set.pick(rng);
	}

    // Must call before simulation!
    void init(const Node& node) {
    	influence_set.init(node);
    }

    READ_WRITE(rw) {
    	PERF_TIMER();
    	rw << infected << influence_set.total_prob;
    	rw << influence_set.connections;
    }

	double total_probability() {
		return influence_set.total_prob;
	}

public:
	// Note: infection is idempotent.
	// Once an individual is infected and starts a contagion window, it can effectively be considered deleted from the network.
	bool infected = false;
    // The preprocessed influences:
	WalkerConnections influence_set;
};

struct State {
	typedef void (*oninfectf)(int infected_Id);
    size_t size() {
    	return entities.size();
    }

	void init(const Config& C);
	void set_graph(const Graph& graph);

	READ_WRITE(rw) {
		rw << time_interval_overage << halflife << last_infector;
		rw.visit_objs(entities);
	}
    void step();
    // Returns false if entity was already infected
    bool try_infection(entity_id infected_id);
    void infect_n_random(int n);
    // Generate an infection, possibly invalid
    entity_id generate_potential_infection();

    Entity& get(entity_id id) {
    	return entities.at(id);
    }
    double current_timestep();
    void fast_reset(Config& C);

    double total_weight() const {
    	return active_infections.total_weight();
    }

    bool finished(Config& C) const {
    	return (time_elapsed >= C.min_time && total_weight() <= C.max_weight);
    }

public:
    double time_interval_overage = -1;
    double halflife = -1;
    // Hack to store extra result from generate_potential_infection:
    entity_id last_infector = -1;
    // Members:
    MTwist rng;
    size_t n_steps = 0;
    size_t n_infections = 0;
    oninfectf on_infect_func = NULL;
    std::vector<Entity> entities;
    double time_elapsed = 0;
	Config::InfectionSet active_infections;
};

#endif /* NETWORK_H_ */
