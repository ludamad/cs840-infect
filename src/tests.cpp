#include <cstdio>

#include <UnitTest++.h>

#include "discrete_fixedtree.h"
#include "discrete_searchtree.h"

#include "network.h"

// This test trivially passes.
// Its output is merely empirical evidence that this walker method implementation is correct.
TEST(walker_method_empirical) {
	time_t seed;
	time(&seed);
	MTwist rng(seed);
	// Range of discrete distribution
	const int N = 1000;
	// N * M = number of samples taken
	const int M = 1000;

	// Test the average results of applying our distribution algorithm for entity connection relationships.
	std::vector<entity_id> influence_set;
	Entity e;
	for (int i = 0; i < N; i++) {
		e.connect(i, (double)i);
	}
	e.preprocess();

	printf("Picking %d numbers\n", N *M);
	std::vector<int> pick_count(N, 0);
	for (int i = 0; i < N * M; i++) {
		int p = e.pick_influence(rng);
		pick_count[p]++;
	}

	ASSERT(pick_count[0] == 0, "'0' should not be picked!");
	// for i>0, normalize and output:
	for (int i = 1; i < N; i += 25) {
		printf("For %d: %.4f\n", i, pick_count[i] / 2 / double(i));
	}

}

// This test mostly trivially passes.
// Its output is merely empirical evidence that data structure implementation is correct.
template <typename T>
static void test_discrete_choice_structure() {
	time_t seed;
	time(&seed);
	MTwist rng(seed);
	// Range of discrete distribution
	const int N = 1000;
	// N * M = number of samples taken
	const int M = 1000;

	// Test the average results of applying our distribution algorithm for entity connection relationships.
	T tree(N);
	for (int i = 0; i < N; i++) {
		tree.insert(i, i);
	}

	printf("Picking %d numbers\n", N *M);
	std::vector<int> pick_count(N, 0);
	for (int i = 0; i < N * M; i++) {
		int p = tree.random_select(rng);
		pick_count.at(p)++;
	}

	ASSERT(pick_count[0] == 0, "'0' should not be picked!");
	// for i>0, normalize and output:
	for (int i = 1; i < N; i += 1) {
		printf("For %d: %.4f\n", i, pick_count[i] / 2 / double(i));
	}



}

TEST(discrete_fixedtree_empirical) {
	test_discrete_choice_structure<DiscreteFixedTree>();
}

TEST(discrete_splaytree_empirical) {
	test_discrete_choice_structure<DiscreteSearchTree>();
}
