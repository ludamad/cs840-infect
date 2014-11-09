#include <cstdio>

#include <UnitTest++.h>

#include "discrete_fixedtree.h"
#include "discrete_searchtree.h"

#include "network.h"

#include "libs/StatCalc.h"

const int TEST_SIZE = 1024;
const int TEST_SAMPLES = 1000;
const int INCREMENT = TEST_SIZE / 100;
const double SCALE_FACTOR = (TEST_SIZE) / double(TEST_SAMPLES) / 2.0;

static int permutei(int i) {
	return (i * 257 + 1) % TEST_SIZE;
}

// This test trivially passes.
// Its output is merely empirical evidence that this walker method implementation is correct.
TEST(walker_method_empirical) {
	PERF_UNIT("walkermethod");
	time_t seed;
	time(&seed);
	MTwist rng(seed);
	// Range of discrete distribution
	const int N = TEST_SIZE;
	// N * M = number of samples taken
	const int M = TEST_SAMPLES;

	// Test the average results of applying our distribution algorithm for entity connection relationships.
	std::vector<entity_id> influence_set;
	Entity e;
	{ PERF_TIMER2("walker_method_preprocess");
	int j = TEST_SIZE/2;
	for (int i = 0; i < N; i++) {
		e.connect(j, (double)j);
		j = permutei(j);
	}
	e.preprocess(); }

	printf("Picking %d numbers\n", N *M);
	std::vector<int> pick_count(N, 0);
	for (int i = 0; i < N * M; i++) {
		int p;
		{ PERF_TIMER2("walker_method_pick") ; p = e.pick_influence(rng); }
		pick_count[p]++;
	}

	ASSERT(pick_count[0] == 0, "'0' should not be picked!");
	// for i>0, normalize and output:
	for (int i = 1; i < N; i += INCREMENT) {
		printf("For %d: %.4f\n", i, pick_count[i] * SCALE_FACTOR / double(i));
	}
	StatCalc stats;
	for (int i = 1; i < N; i++) {
		stats.add_element((N/M) * pick_count[i] * SCALE_FACTOR / double(i));
	}
	stats.print_summary();
}

// This test mostly trivially passes.
// Its output is merely empirical evidence that data structure implementation is correct.
template <typename T>
static void test_discrete_choice_structure() {
	time_t seed;
	time(&seed);
	MTwist rng(seed);
	// Range of discrete distribution
	const int N = TEST_SIZE;
	// N * M = number of samples taken
	const int M = TEST_SAMPLES;

	// Test the average results of applying our distribution algorithm for entity connection relationships.
	T tree(N*10);
	int j = 17;
	for (int i = 0; i < N; i++) {
		tree.insert(j, j);
		j = permutei(j);
	}

	printf("Picking %d numbers\n", N *M);
	std::vector<int> pick_count(N, 0);
	for (int i = 0; i < N * M; i++) {
		int p = tree.random_select(rng);
		pick_count.at(p)++;
	}

	ASSERT(pick_count[0] == 0, "'0' should not be picked!");
	// for i>0, normalize and output:
	for (int i = 1; i < N; i += INCREMENT) {
		printf("For %d: %.4f\n", i,pick_count[i] * SCALE_FACTOR / double(i));
	}
	StatCalc stats;
	for (int i = 1; i < N; i++) {
		stats.add_element(pick_count[i] * SCALE_FACTOR / double(i));
	}
	stats.print_summary();
}

TEST(discrete_fixedtree_empirical) {
	PERF_UNIT("fixedtree");
	test_discrete_choice_structure<DiscreteFixedTree>();
}

TEST(discrete_splaytree_empirical) {
	PERF_UNIT("splaytree");
	test_discrete_choice_structure<DiscreteSearchTree>();
}
