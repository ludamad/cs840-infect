#ifndef DISCRETE_BUCKETTREE_H_
#define DISCRETE_BUCKETTREE_H_

#include "discrete_bst.h"
#include <vector>

struct Slot {
	entity_id entity;
	double prob;
};

struct Bucket {
	std::vector<Slot> slots;

	void insert(entity_id e, double p) {
		slots.push_back({e, p});
	}

	entity_id random_select(MTwist& rng) {
		while (true) {
			int i = rng.rand_int(slots.size());
			if (rng.random_chance(slots[i].prob)) {
				return slots[i].entity;
			}
		}
		return 0; // Unreachable
	}
};

struct DiscreteBucketTree : DBST<int, floatT, Bucket> {

	DiscreteBucketTree(int __unused = 0) {
		// Compatibility
	}
	void init(int n) {
		*this = DiscreteBucketTree();
	}

	void insert(entity_id e, floatT weight) {

		PERF_TIMER();
		int slot;
//		double expect = total_weight() + weight;
		weight /= decay_factor;
//	    printf("TOTAL WEIGHT BEFORE %f -> %f\n", (float)total_weight(), (float)expect);
		// We obtain a number in [0.5,1]. We use this as the
		// probability that the node is selected from the bucket.
		double prob = frexp(weight, &slot);
	    Node* node = find(slot, weight);
	    node->value.insert(e, weight);
//	    printf("TOTAL WEIGHT AFTER %f is it %f\n", (float)total_weight(), (float)expect);
	}

	entity_id random_select(MTwist& rng) {
		PERF_TIMER();
		floatT r = rng.rand_real_not1() * total_weight();
		Bucket& bucket = root->weighted_select(r)->value;
		return bucket.random_select(rng);
	}

	void scale(floatT multiplier) {
		decay_factor *= multiplier;
		if (decay_factor < FLOAT_EXP_BOTTOM) {
			if (root) {
				root->forall([](Node* node) {
					node->key -= FLOAT_EXP_SHIFT;
				  });
			}
			// Pull the decay factor significantly closer to 1
			decay_factor /= FLOAT_EXP_BOTTOM;
		}
	}
	floatT total_weight() const {
	    return root ? root->weight * decay_factor : 0;
	}
	floatT decay_factor = 1.0;
};



#endif /* DISCRETE_BUCKETTREE_H_ */
