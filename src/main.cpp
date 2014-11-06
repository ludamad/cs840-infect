#include <cstdio>
#include <string>
#include <vector>

#include "hashkat-utils/mtwist.h"

using namespace std;

typedef int entity_id;

/*****************************************************************************
 *
 *****************************************************************************/

struct SelectDS {

};

/*****************************************************************************
 *
 *****************************************************************************/

struct Connection {
	entity_id entity;
	double transmission_probability;
	Connection(entity_id entity, double transmission_probability) {
		this->entity = entity;
		this->transmission_probability = transmission_probability;
	}
};

struct Entity {
    vector<Connection> influence_set;
};

struct Settings {
    string graph_type = "uniform";
    string method = "bad";
    int seed = 0xBADF00D;
    size_t size = 10;
    double transmission_prob = 0.1;
    double connectivity = 0.1;
};

struct State {
	MTwist rng;
    vector<Entity> entities;
    double time = 0;

    size_t size() {
    	return entities.size();
    }
    void init(const Settings& settings);
    void step();
};

/*****************************************************************************
 * Implementations
 *****************************************************************************/

void State::init(const Settings& S) {
	rng.init_genrand(S.seed);
	entities = vector<Entity>(S.size);
	time = 0;
	for (Entity& e : entities) {
		for (entity_id e_id = 0; e_id < size(); e_id++) {
			if (&e != &entities[e_id]) {
				// For every node other than ourselves, connect with 'connectivity' probability
				if (rng.rand_real_not1() < S.connectivity) {
					e.influence_set.push_back({e_id, S.transmission_prob});
				}
			}
		}
	}
}

/*****************************************************************************
 * Test driver
 *****************************************************************************/

int main(int argn, char** args) {
	Settings settings;
	State network;
	// Create the network according to passed settings
	network.init(settings);
    return 0;
}
