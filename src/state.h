#ifndef STATE_H_
#define STATE_H_

#include <cmath>
#include <string>

#include "libs/mtwist.h"
#include "libs/github-ntamas-netctrl/walker_sampling.hpp"

#include "config.h"
#include "discrete_fixedtree.h"

/*****************************************************************************
 *
 *****************************************************************************/

struct Entity {
	entity_id pick_influence(MTwist& rng) {
		entity_id pick = weighted_picker.sample(rng);
		return influence_set.at(pick);
	}

	// Note: infection is idempotent.
	// Once an individual is infected and starts a contagion window, it can effectively be considered deleted from the network.
	bool infected = false;
    // The sum of all transmission probabilities:
    double transmission_prob_total = 0;

    void connect(entity_id neighbour, double weight) {
    	transmission_probs.push_back(weight);
    	influence_set.push_back(neighbour);
    	transmission_prob_total += weight;
    }

    // Must call before simulation!
    void preprocess() {
    	weighted_picker = WalkerSampling(transmission_probs.begin(), transmission_probs.end());
    	transmission_probs = std::vector<double>();
    }

    READ_WRITE(rw) {
    	PERF_TIMER();
    	rw << infected << transmission_prob_total;
    	rw << transmission_probs << influence_set;
    	weighted_picker.visit(rw);
    }

private:
    std::vector<double> transmission_probs;
	std::vector<entity_id> influence_set;
    WalkerSampling weighted_picker;
};

struct State {
    size_t size() {
    	return entities.size();
    }

    // Based on the passed settings, create a random initial state.
    // Right now, we just generate a directed graph (the network state) of some average connectivity and uniformly weighted connections
	void init(const Config& C);
	void generate_graph();

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

    void connect(entity_id A, entity_id B, double transmission_prob);
    Entity& get(entity_id id) {
    	return entities.at(id);
    }
    double current_timestep();
    void fast_reset(Config& S);
    double time_interval_overage = -1;
    double halflife = -1;
    // Hack to store extra result from generate_potential_infection:
    entity_id last_infector = -1;
    // Members:
    MTwist rng;
    size_t n_steps = 0;
    size_t n_infections = 0;
    std::vector<Entity> entities;
    double time_elapsed = 0;
	Config::InfectionSet active_infections;
};

#endif /* NETWORK_H_ */
