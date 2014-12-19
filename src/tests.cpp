#include <cstdio>
#include <vector>
#include <initializer_list>

#include <UnitTest++.h>

#include "discrete_bst.h"
#include "discrete_fixedtree.h"
#include "discrete_searchtree.h"
#include "discrete_buckettree.h"

#include "state.h"

#include "boost/heap/binomial_heap.hpp"
#include "boost/heap/fibonacci_heap.hpp"
#include "boost/heap/priority_queue.hpp"
#include "boost/heap/skew_heap.hpp"
#include "boost/heap/pairing_heap.hpp"

#include "libs/StatCalc.h"

const int TEST_SIZE = 256;
const int TEST_SAMPLES = 1000;
const int INCREMENT = TEST_SIZE / 25;
const double SCALE_FACTOR = (TEST_SIZE) / double(TEST_SAMPLES) / 2.0;

static int permutei(int i, int max) {
	return (i * 257 + 1) % max;
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
	Entity e;
	{ PERF_TIMER2("walker_method_preprocess");
	Node n;
	int j = N/2;
	for (int i = 0; i < N; i++) {
		n.push_back({(double)j, j});
		j = permutei(j, N);
	}
	e.init(n); }

	std::vector<int> pick_count(N, 0);
	for (int i = 0; i < N * M; i++) {
		int p = -1;
	    while (p == -1) {
	        PERF_TIMER2("walker_method_pick") ; p = e.pick_influence(rng);
	    }
		if (i % (e.influence_set.connections.size() /2) == 0) {
            for (auto& conn : e.influence_set.connections) {
                conn.was_chosen = false;
            }
		}
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
		j = permutei(j, N);
	}

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

TEST(discrete_searchtree_empirical) {
	PERF_UNIT("searchtree");
	test_discrete_choice_structure<DiscreteSearchTree>();
}

TEST(discrete_buckettree_empirical) {
	PERF_UNIT("buckettree");
	test_discrete_choice_structure<DiscreteBucketTree>();
}

TEST(discrete_bst_empirical) {
	PERF_UNIT("weighted bst");
	DiscreteBST bst;
	for (int i = 1; i < 100; i++) {
	    bst.insert(i,i);
	}
	for (int i = 1; i < 100; i++) {
	    CHECK(bst.find(i, 0)->key == i);
	}
	test_discrete_choice_structure<DiscreteBST>();
}

template <typename T>
static void measure_heap() {
	PERF_TIMER2("measure_heap");
	T tree;
	const int N = TEST_SIZE * TEST_SAMPLES;
	int j = N/2;
	for (int i = 0; i < N; i++) {
		tree.push(j);
		j = permutei(j, N);
		if ((j+i) % 52 == 0) {
			tree.pop();
		}
		if (tree.size() > 100) {
			while (!tree.empty()) {
				tree.pop();
			}
		}
	}
	while (!tree.empty()) {
		tree.pop();
	}
}

TEST(fibonacci_heap_perf) {
	PERF_UNIT("fibonacci_heap_perf");
	measure_heap<boost::heap::fibonacci_heap<double>>();
}

TEST(binomial_heap_perf) {
	PERF_UNIT("binomial_heap_perf");
	measure_heap<boost::heap::binomial_heap<double>>();
}

TEST(priority_queue_perf) {
	PERF_UNIT("priority_queue_perf");
	measure_heap<boost::heap::priority_queue<double>>();
}

TEST(pairing_heap_perf) {
	PERF_UNIT("pairing_heap_perf");
	measure_heap<boost::heap::pairing_heap<double>>();
}

TEST(skew_heap_perf) {
	PERF_UNIT("skew_heap_perf");
	measure_heap<boost::heap::skew_heap<double>>();
}
