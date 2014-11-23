#ifndef STATE_H_
#define STATE_H_

#include <cmath>
#include <string>

#include "libs/mtwist.h"

#include "config.h"
#include "discrete_fixedtree.h"
#include "graph.h"

/*****************************************************************************
 * An entity in the random generation simulation
 *****************************************************************************/

// A connection, preprocessed using the walker method
struct WalkerConnection {
	double choice_a_prob;
	entity_id choice_a, choice_b;
	entity_id pick(MTwist& rng) const {
		bool use_choice_a = (rng.rand_real_not1() < choice_a_prob);
		return use_choice_a ? choice_a : choice_b;
	}
};

// Code modified from github.com/ntamas/netctrl
struct WalkerConnections {
	std::vector<WalkerConnection> connections;
	double total_prob = 0;
	entity_id pick(MTwist& rng) const {
		size_t m = connections.size();
		return connections[rng.rand_int(m)].pick(rng);
	}
	void init(const Node& node) {
		// Initial values:
		size_t n = node.size();
    	connections.resize(n);
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
	        connections[S].choice_b = node[L].node;
	        connections[L].choice_a_prob -= (1 - connections[S].choice_a_prob);
	        if (connections[L].choice_a_prob < 1) {
	            shortI.push_back(L);
	            longI.pop_back();
	        }
	    }
	}
};

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

    /** Member variables: **/
	// Note: infection is idempotent.
	// Once an individual is infected and starts a contagion window, it can effectively be considered deleted from the network.
	bool infected = false;

	double total_probability() {
		return influence_set.total_prob;
	}
private:
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
    void fast_reset(Config& S);

    /** Member variables: **/
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
