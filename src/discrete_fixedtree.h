#ifndef DISCRETE_FIXEDTREE_H_
#define DISCRETE_FIXEDTREE_H_

#include <cmath>
#include <vector>

#include "hashkat-utils/customassert.h"
#include "discrete_common.h"

const int DFTNotExists = -1;

struct DFTNode {
	double total_weight = 0;
	int parent_id = DFTNotExists;
	int left_id = DFTNotExists, right_id = DFTNotExists;
};

// Guarantees O(log N) operations very trivially
// where N is the _maximum_ size of the universe.
struct DiscreteFixedTree {
	DiscreteFixedTree() {
	}
	void init(int n) {
		// Easy thanks to vector value-copy semantics:
		*this = DiscreteFixedTree(n);
	}
	DiscreteFixedTree(int n) : size(n), nodes(n) {
		int root_id = _init_node(DFTNotExists, 0, n);
		ASSERT(root_id == size, "Root problem! Should be right after weights.");
	}

	void insert(int i, double weight) {
		weight *= decay_factor;
		DFTNode* N = &nodes[i];
		double delta_weight = (weight - N->total_weight);
		N->total_weight = weight;
		int pid = N->parent_id;
		while (pid != DFTNotExists) {
			N = &nodes[pid];
			// Readjust all the weights:
			N->total_weight = (nodes[N->left_id].total_weight + nodes[N->right_id].total_weight);
			pid = N->parent_id;
		}
	}

	int random_select(MTwist& rng) {
		DEBUG_CHECK(total_weight() > 0.0, "Can't do random select with 0 weight!");
		// Note: The root node is located at 'size'
		double r = rng.rand_real_not1() * nodes[size].total_weight;
		return random_select(r, size);
	}

	void scale(double multiplier) {
		decay_factor /= multiplier;
		if (decay_factor > 1.0e250) {
			for (auto& node : nodes) {
				node.total_weight /= decay_factor;
			}
			decay_factor = 1.0;
		}
	}
	double total_weight() {
		return nodes[size].total_weight / decay_factor;
	}
private:
	int random_select(double r, int node_id) {
		if (node_id < size) {
			return node_id;
		}
		DFTNode& node = nodes[node_id];
		double leftw = nodes[node.left_id].total_weight;
		if (r < leftw) {
			return random_select(r, node.left_id);
		}
		return random_select(r - leftw, node.right_id);
	}

	// Returns the node 'id'
	int _init_node(int parent, int i, int n) {
		DEBUG_CHECK(n > 0, "range covered should be at least 1!");
		int id = i; // Start of range, used if n == 1
		if (n > 1) {
			id = nodes.size();
			nodes.push_back(DFTNode());
			int half1 = n >> 1, half2 = (n+1) >> 1;
			nodes[id].left_id = _init_node(id, i, half1);
			nodes[id].right_id = _init_node(id, i + half1, half2);
		}
		// Else, for n == 1, we do not have children to consider
		nodes[id].parent_id = parent;
		return id;
	}
	double decay_factor = 1.0;
	size_t size = 0;
	std::vector<DFTNode> nodes;
};

#endif /* DISCRETE_FIXEDTREE_H_ */
