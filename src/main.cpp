#include <cstdio>
#include <string>
#include <sstream>
#include <vector>

#include "libs/unittest.h"

#include "sdl.h"
#include "state.h"

using namespace std;

/*****************************************************************************
 * Test driver
 *****************************************************************************/

static void output_network(Config& config, string label, State& network) {
	PERF_TIMER();
	static vector<bool> wasInfected;
	static bool SDL_INITIALIZED = false;

	double scale = config.window_size / double(config.sqrt_size);
	if (!SDL_INITIALIZED) {
		wasInfected.resize(network.size(), false);
		sdl_init(scale * config.sqrt_size, scale * config.sqrt_size);
		SDL_INITIALIZED = true;
	}
	sdl_predraw();
	int i = 0;
	for (int y = 0; y < config.sqrt_size; y++) {
		for (int x = 0; x < config.sqrt_size; x++) {
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


static int scan_flag(string flag, int argn, const char** argv) {
	for (int i = 1; i < argn; i++) {
		if (argv[i] == flag) {
			return i;
		}
	}
	return argn;
}

struct IOOp {
	string write_filename, read_filename;
	DataReader* reader = NULL;
	DataWriter* writer = NULL;
	int sqrt_size = -1;// -1 if not set here
	~IOOp() {
		delete reader;
		delete writer;
	}
	IOOp(int argn, const char** argv) {
		int w_loc = scan_flag("-w", argn, argv);
		int r_loc = scan_flag("-r", argn, argv);
		if (w_loc + 2 < argn) {
			stringstream(argv[w_loc+1]) >> sqrt_size;
			write_filename = argv[w_loc+2];
			writer = new DataWriter(write_filename);
		} else if (w_loc + 1 < argn) {
			// Size wasn't explicit:
			write_filename = argv[w_loc+1];
			writer = new DataWriter(write_filename);
		} else if (r_loc + 1 < argn) {
			read_filename = argv[r_loc+1];
			reader = new DataReader(read_filename);
		} else if (argn > 1 && string(argv[1]).at(0) != '-') {
			stringstream(argv[1]) >> sqrt_size;
		}
		if (reader != NULL) {
			printf("Will be reading from '%s'\n", read_filename.c_str());
		}
		if (writer != NULL) {
			printf("Will be writing to '%s'\n", write_filename.c_str());
		}
	}
	bool init_state(Config& config, State& state) {
		PERF_UNIT("Initialization of Network");
		PERF_TIMER();
		bool do_simulation = true;
		if (reader != NULL) {
			printf("Loading from '%s'\n", read_filename.c_str());
			(*reader) << config.sqrt_size;
			config.size = config.sqrt_size * config.sqrt_size;
			printf("Creating network of size %d\n", config.size);
			state.init(config);
			state.visit(*reader);
			printf("Loaded from '%s': Graph of size %d by %d\n",
					read_filename.c_str(), config.sqrt_size, config.sqrt_size);
		} else {
			printf("Creating network of size %d\n", config.size);
			state.init(config);
			state.generate_graph();
			if (writer != NULL) {
				printf("Saving to '%s': Graph of size %d by %d\n",
						write_filename.c_str(), config.sqrt_size, config.sqrt_size);
				(*writer) << config.sqrt_size;
				state.visit(*writer);
				printf("Saved to '%s': Graph of size %d by %d\n",
						write_filename.c_str(), config.sqrt_size, config.sqrt_size);
				printf("Exiting. Run with ./run.sh -r '%s'\n", write_filename.c_str());
				do_simulation = false; // We are just writing & quitting
			}
			// No file IO? Just init and generate graph.
		}
		delete writer; writer = NULL;
		delete reader; reader = NULL;
		return do_simulation;
	}
};

static void run(Config& config, State& network) {
	PERF_UNIT("Network Simulation Stats");
	PERF_TIMER();
	output_network(config, "Initial Conditions", network);
    if (config.delay) {
    	sdl_delay(100);
    }
    MilestoneRep rep;
    while (true) {
		sdl_delay(0);
    	if (network.time_elapsed >= config.min_time && network.active_infections.total_weight() <= config.max_weight) {
    		break; // Done
    	}
		double last_time = network.time_elapsed;
		// Draw after every millisecond of simulation:
		while (network.time_elapsed < last_time + 1000) {
			network.step();
			rep.report("Simulated step %d");
		}
		stringstream ss("Simulation ");
		ss << "W = "         << network.active_infections.total_weight() << endl
		   << "sim-time ="   << network.time_elapsed << "s"              << endl
		   << "step: "       << network.n_steps                          << endl
		   << "infections: " << network.n_infections;
		output_network(config, ss.str(), network);
	}
    output_network(config, "Simulation Complete", network);
    if (config.delay) {
    	sdl_delay(1000);
    }
}

int main(int argn, const char** argv) {
	if (scan_flag("--test", argn, argv) != argn) {
		return run_unittests();
	}
    time_t seed;
    time(&seed);
    IOOp op(argn, argv);
    State state;
    Config config(seed, op.sqrt_size == -1 ? Config::DEFAULT_SQRT_SIZE : op.sqrt_size);

    // Create the network according to passed settings
    if (!op.init_state(config, state)) {
    	// We are just writing the graph and exiting
    	return 0;
    }
    state.infect_n_random(100);
	output_network(config, "Initial Conditions", state);
    run(config, state);
    printf("Simulation complete!\n");
	return 0;
}
