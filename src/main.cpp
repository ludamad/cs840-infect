#include <cstdio>
#include <string>
#include <sstream>
#include <vector>

#include "libs/unittest.h"

#include "sdl.h"
#include "network.h"

using namespace std;

/*****************************************************************************
 * Test driver
 *****************************************************************************/

void output_network(string label, int dim, State& network) {

	static vector<bool> wasInfected;
	static bool SDL_INITIALIZED = false;

	static unsigned int COL_BLACK = 0;
	static unsigned int COL_WHITE = 0xFFFFFFFF;
	static unsigned int COL_GREY = 0xFF666699;
	static unsigned int COL_RED1 = 0xFFFF0000;
	static unsigned int COL_RED2 = 0xFF990000;
	static unsigned int COL_ORANGE = 0xFFFF9900;

	double scale = 900 / dim;
	if (!SDL_INITIALIZED) {
		wasInfected.resize(network.size(), false);
		sdl_init(scale * dim, scale * dim);
		SDL_INITIALIZED = true;
	}
	sdl_predraw();
	int i = 0;
	for (int y = 0; y < dim; y++) {
		for (int x = 0; x < dim; x++) {
			auto& e = network.entities.at(i);
			unsigned int colour = COL_WHITE;
			if (e.infected) {
				colour = COL_ORANGE;
				if (!wasInfected[i]) {
					colour = COL_RED1;
					wasInfected[i] = true;
				}
			}
			sdl_fill_rect(floor(x * scale), floor(y * scale), ceil(scale), ceil(scale), colour);
//			sdl_fill_rect(x * scale, y * scale, scale, scale, COL_BLACK);
//			sdl_fill_rect(x * scale, y * scale, scale - 1, scale - 1, colour);
			i++;
		}
	}
	stringstream ss(label);
	char buff[500];
	int y = 30;
	while (!ss.eof()) {
		ss.getline(buff, 500);
		sdl_draw_text(buff, 30, y);
		y += 30;
	}
	sdl_postdraw();
}

int main(int argn, char** args) {
    run_unittests();
    int dim = 900, n_start = 10000;
    time_t seed;
    time(&seed);
    Settings settings(dim*dim, 1, seed, 0.01);
    State network;
    // Create the network according to passed settings
    network.init(settings);
    restart:
	network.fast_reset(settings);
    network.infect_n_random(100);
	output_network("Initial Conditions", dim, network);
    sdl_delay(100);
    double min_time = 600, max_weight = 1e-32;
    int step = 0;
    int step_milestone = 1;
    while (true) {
		sdl_delay(0);
    	if (network.time_elapsed >= min_time && network.active_infections.total_weight() <= max_weight) {
    		break; // Done
    	}
		double last_time = network.time_elapsed;
		// Draw after every millisecond of simulation:
		while (network.time_elapsed < last_time + 1000) {
			network.step();
			if (step == step_milestone) {
				printf("Simulated step %d, time %.2f\n", step, network.time_elapsed);
				step_milestone *= 2;
				break;
			}
			step++;
		}
		stringstream ss("Simulation ");
		ss << "W = " << network.active_infections.total_weight() << endl;
		ss << "sim-time =" << network.time_elapsed << "s" << endl;
		ss << "step: " << network.n_steps << endl;
		ss << "infections: " << network.n_infections;
		output_network(ss.str(), dim, network);
    }
    while (true) {
		output_network("Simulation Complete", dim, network);
		sdl_delay(100);
		goto restart;
    }
	return 0;
}
