#ifndef NETWORK_H_
#define NETWORK_H_

#include <cmath>
#include <string>

#include "hashkat-utils/mtwist.h"
#include "github-ntamas-netctrl/walker_sampling.hpp"

#include "discrete_fixedtree.h"

/*****************************************************************************
 * InfectionSet: The structure that holds the 'active contagions'.
 * This structure provides efficient operations for storing
 * exponential decaying weights.
 *
 * Must have the following operations:
 * 	insert(i, weight)
 *  decay(half-lives) -> Make all elements scale down by 2^(-half-lives)
 *  random_select(rng)
 *  total_weight() -> Has important meaning in the kmc simulation: time_step = 1/total_weight
 *****************************************************************************/

typedef DiscreteFixedTree InfectionSet;

/*****************************************************************************
 *
 *****************************************************************************/

//struct Connection {
//	entity_id entity;
//	double transmission_probability;
//	Connection(entity_id entity, double transmission_probability) {
//		this->entity = entity;
//		this->transmission_probability = transmission_probability;
//	}
//};

struct Entity {
	entity_id pick_influence(MTwist& rng) {
		entity_id pick = weighted_picker.sample(rng);
		return influence_set.at(pick);
	}

	// Note: infection is idempotent.
	// Once an individual is infected and starts a contagion window, it can effectively be considered deleted from the network.
	bool infected = false;
    // The sum of all transmission probabilities:
    double transmission_total = 0;

    void connect(entity_id neighbour, double weight) {
    	transmission_probs.push_back(weight);
    	influence_set.push_back(neighbour);
    	transmission_total += weight;
    }

    // Must call before simulation!
    void preprocess() {
    	weighted_picker = WalkerSampling(transmission_probs.begin(), transmission_probs.end());
    	transmission_probs = std::vector<double>();
    }

private:
    std::vector<double> transmission_probs;
	std::vector<entity_id> influence_set;
    WalkerSampling weighted_picker;
};

struct Settings;

struct State {
    size_t size() {
    	return entities.size();
    }

    // Based on the passed settings, create a random initial state.
    // Right now, we just generate a directed graph (the network state) of some average connectivity and uniformly weighted connections
	void init(const Settings& S);

    void step();
    // Returns false if entity was already infected
    bool try_infection(entity_id infected_id);
    void infect_n_random(int n);
    // Generate an infection, possibly invalid
    entity_id generate_potential_infection();

    void connect(entity_id A, entity_id B, double transmission_prob);
    void biconnect(entity_id A, entity_id B, double transmission_prob);
    Entity& get(entity_id id) {
    	return entities.at(id);
    }
    double current_timestep();
    void fast_reset(Settings& S);
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
	InfectionSet active_infections;
};

struct Settings {
    Settings(size_t size, double halflife, int seed, double transmission_prob) {
    	this->size = size;
    	this->halflife = halflife;
    	this->seed = seed;
    	this->transmission_prob = transmission_prob;
    }
    size_t size;
    double halflife;
    int seed;

    std::string graph_type = "uniform";
    std::string method = "bad";
    double transmission_prob = 0.1;
    double connectivity = 0.1;
};

#endif /* NETWORK_H_ */
