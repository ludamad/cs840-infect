#include <cstdio>
#include <string>
#include <vector>

#include "hashkat-utils/mtwist.h"

using namespace std;

typedef int entity_id;

struct Entity {
    vector<entity_id> entities;
};

struct Settings {
    string graph_type = "uniform";
    string method = "bad";
    int seed = 0xBADF00D;
    size_t size = 10;
    double connectivity = 0.1;
};

struct Network {
	MTwist rng;
    vector<Entity> entities;
    size_t size() {
    	return entities.size();
    }
    void init(const Settings& settings);
};

void Network::init(const Settings& S) {
	rng.init_genrand(S.seed);
	entities = vector<Entity>(S.size);
	for (Entity& e : entities) {
		for (entity_id e_id = 0; e_id < size(); e_id++) {
			if (&e == &entities[e_id]) {
				// For every node other than ourselves, connect with 'connectivity' probability
				if (rng.rand_real_not1() < S.connectivity) {
					e.entities.push_back(e_id);
				}
			}
		}
	}
}

int main(int argn, char** args) {
	Settings settings;
	Network network;
	// Create the network according to passed settings
	network.init(settings);
    return 0;
}
