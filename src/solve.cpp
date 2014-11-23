/*
 * There are two approaches:
 * Approach 1:
 * 		- preprocess weights to get O(1) neighbour selection
 *		- use random selection of active infections
 *		- find neighbour using O(1) selection
 *		- advance time
 * Approach 2:
 * 	    - For all initially active infections, do a weighted coin flip over neighbours to determine if the infection spreads.
 *	    - Pick the minimum time infection that was generated, and consider its neighbours.
 *	    - Repeat until set of infections is empty.
 */

#include "state.h"

struct Solver {



};

/**
 * Solve all the infections that will ever occur.
 */
int solve(State& state) {

}
