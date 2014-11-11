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

struct OutputData {
	vector<bool> wasInfected;
	vector<pair<entity_id, size_t>> newlyInfected;
	int keep_alive = 4000;
	bool SDL_INITIALIZED = false;
	double scale;
	int rows;
	State* network;
} O;

static void output_init(Config& config, State& network) {
	O.scale = config.window_size / double(config.sqrt_size);
	O.rows = config.sqrt_size;
	O.wasInfected.resize(network.size(), false);
	O.network = &network;
	sdl_init(O.scale * config.sqrt_size, O.scale * config.sqrt_size, config.sqrt_size, config.sqrt_size);
}

static void on_infect(entity_id id) {
	int x = id % O.rows,y = id/O.rows;
	sdl_fill_pixel(x, y, COL_RED1);
	O.wasInfected[id] = true;
	O.newlyInfected.push_back({id, O.network->n_steps});
}

static void output_network(Config& config, string label, State& network) {
	if (!config.visualize) {
		return;
	}
	PERF_TIMER();

	sdl_predraw();
	sdl_copybuffer();
	stringstream ss(label);
	char buff[500];
	int y = 30;
	while (!ss.eof()) {
		ss.getline(buff, 500);
		sdl_draw_text(buff, 30, y);
		y += 30;
	}
	sdl_postdraw(config.saved_image_base_path);
	auto copy = O.newlyInfected;
	O.newlyInfected.clear();
	for (auto& pair : copy) {
		int id = pair.first;
		if (pair.second > network.n_steps - O.keep_alive) {
			O.newlyInfected.push_back(pair);
		} else {
			sdl_fill_pixel(id % O.rows, id / O.rows, COL_ORANGE);
		}
	}
}


static int scan_flag(string flag, int argn, const char** argv) {
	for (int i = 1; i < argn; i++) {
		if (argv[i] == flag) {
			return i;
		}
	}
	return argn;
}

struct CmdLineParser {
	string write_filename, read_filename, saved_image_base_path;
	DataReader* reader = NULL;
	DataWriter* writer = NULL;
	int sqrt_size = -1;// -1 if not set here
	int seed = -1;
	bool visualize = true;
	~CmdLineParser() {
		delete reader;
		delete writer;
	}
	CmdLineParser(int argn, const char** argv) {
		int w_loc = scan_flag("-w", argn, argv);
		int r_loc = scan_flag("-r", argn, argv);
		int i_loc = scan_flag("-i", argn, argv);
		visualize = (scan_flag("-0", argn, argv) == argn);
		int s_loc = scan_flag("-seed", argn, argv);
		if (i_loc + 1 < argn) {
			saved_image_base_path = argv[i_loc + 1];
		}
		if (s_loc + 1 < argn) {
			stringstream(argv[s_loc+1]) >> seed;
		}
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
		config.saved_image_base_path = saved_image_base_path;
		config.seed = seed == -1 ? config.seed : seed;
		config.sqrt_size = sqrt_size == -1 ? config.sqrt_size : sqrt_size;
		config.size = config.sqrt_size * config.sqrt_size;
		config.visualize = visualize;
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
	PERF_TIMER();
	output_network(config, "Initial Conditions", network);
    if (config.delay) {
    	sdl_delay(100);
    }
    MilestoneRep rep;
    bool finished = false;
    while (!finished) {
//		sdl_delay(10);
		Timer timer;
		double last_time = network.time_elapsed;
		// Draw after every millisecond of simulation:
		while (last_time + 10000 > network.time_elapsed) {
			if (network.time_elapsed >= config.min_time && network.active_infections.total_weight() <= config.max_weight) {
				finished = true;
				break; // Done
			}
			network.step();
			rep.report("Simulated step %d");
		}
		stringstream ss("Simulation ");
	    ss.imbue(std::locale(""));
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
    CmdLineParser cmd(argn, argv);
    Config config(seed, Config::DEFAULT_SQRT_SIZE);
	State state;
	// Create the network according to passed settings
	if (!cmd.init_state(config, state)) {
		// We are just writing the graph and exiting
		return 0;
	}

	PERF_UNIT("Network Simulation Stats");
	for (int i = 0; i < 10; i++) {
		printf("SIMULATION TRIAL (%d/%d)\n", i+1, 10);
		if (cmd.visualize) {
			output_init(config, state);
			state.on_infect_func = on_infect;
		}
		printf("Infecting 20000 random\n");
		state.infect_n_random(20000);
		output_network(config, "Initial Conditions", state);
		run(config, state);
		printf("Simulation complete!\n");
		state.fast_reset(config);
	}
	return 0;
}
