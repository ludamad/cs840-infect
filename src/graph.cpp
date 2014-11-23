#include "graph.h"
#include "libs/mtwist.h"

using namespace std;

static void connect(Node& n1, int id2, double weight) {
	n1.push_back({weight, id2});
}

static void connect(Graph& g, int id1, int id2, double weight) {
	connect(g[id1], id2, weight);
}

Graph generate_graph(Config& config) {
	MTwist rng(config.seed);
	int rows = config.sqrt_size, size = config.size;

	Graph g(size);

	ASSERT(rows*rows == size, "Logic error");
	vector<double> interests(size, 0);
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < rows; x++) {
			interests[y*rows+x] = rng.rand_real_not0();
		}
	}

	printf("CONNECTING ENTITIES\n");
	#define FOR_ID(x, y, A) \
		for (int y = 0; y < rows; y++) \
			for (int x = 0, A = y * rows; x < rows; x++, A++)

	MilestoneRep rep;
	FOR_ID(x, y, A) {
		rep.report("Connected %d entities");
		// How far away to effect?
		double SxD = 3, SyD = 3;
		for (int i = 0; i < 10; i++) {
			SxD += rng.rand_real_with01();
			SyD += rng.rand_real_with01();
			if (rng.random_chance(0.5)) {
				break;
			}
		}
		int Sx = (int) SxD, Sy = (int) SyD;
		double popularity = rng.rand_real_not0();
		int n_squares = (1 + 2 * Sx) * (1 + 2 * Sy) - 1;
		for (int sy = -Sy; sy <= Sy; sy++) {
			for (int sx = -Sx; sx <= Sx; sx++) {
				int nx = (x + rows + sx) % rows, ny = (y + rows + sy) % rows;
				// This ensures everyone is connected to their neighbours, once it has completed.
				if (sx != 0 || sy != 0) {
					entity_id id = ny * rows + nx;
					connect(g, A, id, popularity / n_squares / 30);
				}
			}
		}
	}

	return g;
}