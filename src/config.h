/*
 * Alter the behaviour of the network here.
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "discrete_fixedtree.h"
#include "discrete_searchtree.h"
#include "discrete_bst.h"

struct Config {
	/** Change these! */

	/*****************************************************************************
	 * InfectionSet: The structure that holds the 'active contagions'.
	 * This structure provides efficient operations for storing
	 * exponential decaying weights.
	 *
	 * Must have the following operations:
	 * 	insert(i, weight)
	 *  decay(half-lives) -> Make all elements scale down by 2^(-half-lives)
	 *  random_select(rng)
	 *  total_weight() -> Has important meaning in the kmc simulation: time_step = 1/total_weight
	 *****************************************************************************/
#ifndef SEARCH_STRUCT
	typedef DiscreteFixedTree InfectionSet;
#else
	typedef SEARCH_STRUCT InfectionSet;
#endif
//	typedef DiscreteSearchTree InfectionSet;

	static const int DEFAULT_SQRT_SIZE = 900;
	// Actual size of network is sqrt_size * sqrt_size.
	// Simple restriction to allow for easy drawing.
	double halflife = 1;
	bool delay = true;
	// Simulation end conditions:
	double min_time = 600, max_weight = 1e-32;

	// Strictly for visualization purposes:
	size_t window_size = 900;

	/** Don't change these! */
	Config(int seed, int sqrt_size) :
			sqrt_size(sqrt_size), size(sqrt_size*sqrt_size), seed(seed) {
		size = sqrt_size * sqrt_size;
	}

	std::string saved_image_base_path;
	int sqrt_size;
	int size;
	bool visualize = false;
	int seed;
};

#endif
