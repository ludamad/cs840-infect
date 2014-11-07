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

	int scale = 640 / dim;
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
			// TODO: Simplify for large networks:
			sdl_fill_rect(x * scale, y * scale, scale, scale, COL_BLACK);
			sdl_fill_rect(x * scale, y * scale, scale - 1, scale - 1, colour);
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
    int dim = 100, n_start = 10;
    Settings settings(dim*dim, 1, 0xBADF00D);
    State network;
    // Create the network according to passed settings
    network.init(settings);
    for (int i = 0; i < n_start; i++) {
		sdl_delay(100);
		network.infect_n_random(1);
		stringstream ss;
		ss << i;
		output_network("Seeding Infection #" + ss.str(), dim, network);
    }
    while (true) {
		sdl_delay(100);
		network.step();
		stringstream ss("Simulation ");
		ss << "W = " << network.active_infections.total_weight() << endl;
		ss << ", sim-time =" << network.time_elapsed * 1000 << "ms" << endl;
		ss << "step: " << network.n_steps;
		ss << "infections: " << network.n_infections;
		output_network(ss.str(), dim, network);
    }
    return 0;
}
