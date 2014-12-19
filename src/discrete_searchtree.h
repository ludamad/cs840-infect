#ifndef __DISCRETE_SEARCHTREE_H__
#define __DISCRETE_SEARCHTREE_H__

#include <algorithm>
#include <iostream>
#include <cstdlib>

#include "discrete_common.h"

// Only used in DiscreteSplayTree
struct DSTNode {
    double weight;
    entity_id entity;
    int lchild = -1;
    int rchild = -1;
    DSTNode(entity_id e = -1, double w = -1) {
        entity = e;
        weight = w;
    }

    void assert_relation() {
//    	ASSERT(this != lchild, "Loop!");
//    	ASSERT(this != rchild, "Loop!");
    	double wsum = 0;
    	if (lchild) {
//    		wsum += w(lchild);
//    		lchild->assert_relation();
    	}
    	if (rchild) {
//    		wsum += w(rchild);
//    		rchild->assert_relation();
    	}
//    	ASSERT(fabs(weight - wsum - original) < 0.001, "Relation 1 broken!");
//    	ASSERT(fabs(weight - expected) < 0.001, "Relation 2 broken!");
    }
};

struct DiscreteSearchTree {
    DiscreteSearchTree(int max_size = 0) {
    	buffer.resize(max_size);
    }
    void init(int max_size = 0) {
    	*this = DiscreteSearchTree(max_size);
    }

    void insert(entity_id entity, double weight) {
    	PERF_TIMER();
    	DSTNode* node = &buffer[last_used++];
    	node->entity = entity;
    	node->weight = weight * decay_factor;
    	root_id = insert(root_id, node);
//    	root->assert_relation();
    }

    entity_id random_select(MTwist& rng) {
    	PERF_TIMER();
        ASSERT(!nil(root_id) && total_weight() > 0.0, "Can't do random select with 0 weight!");
        return random_select(to_node(root_id), rng.rand_real_not1() * to_node(root_id)->weight);
    }

    void downscale(int id, double decay) {
        if (nil(id)) {
            return;
        }
        auto* node = to_node(id);
        node->weight /= decay;
        downscale(node->lchild, decay);
        downscale(node->rchild, decay);
    }

    void scale(double multiplier) {
        decay_factor /= multiplier;
        if (decay_factor > 1.0e100) {
        	PERF_TIMER2("SearchTree: full scale");
            downscale(root_id, decay_factor);
            decay_factor = 1.0;
        }
    }
     entity_id random_select(DSTNode* N, double r) {
        DEBUG_CHECK(r > 0 && r < N->weight, "Bad random value!");
        double wl = w(N->lchild);
        if (r < wl) {
            return random_select(to_node(N->lchild), r);
        }
        r -= wl;
        double wr = w(N->rchild);
        if (r < wr) {
        	return random_select(to_node(N->rchild), r);
        }
        // if lchild is empty and rchild is empty always will return entity
        return N->entity;
    }
    int insert(int id, DSTNode* child) {
    	if (id == -1) {
    		return to_id(child);
    	}
    	auto* node = to_node(id);
    	ASSERT(nil(child->lchild) && nil(child->rchild), "Nontrivial insert!");
    	double lW = w(node->lchild), rW = w(node->rchild);
    	double ww = node->weight - lW - rW;
    	if (child->weight > ww) {
    		// Swap who-is-who:
    		std::swap(node->lchild, child->lchild);
    		std::swap(node->rchild, child->rchild);
    		child->weight += node->weight;
    		node->weight = ww;
    		std::swap(node, child);
    		id = to_id(node);
    	} else {
    		node->weight += child->weight;
    	}
    	if (lW > rW) {
    		node->rchild = insert(node->rchild, child);
    	} else {
    		node->lchild = insert(node->lchild, child);
    	}
    	return id;
    }

    double total_weight() const {
        return nil(root_id) ? 0 : to_node(root_id)->weight / decay_factor;
    }
private:
	static bool nil(int a) {
		return a == -1;
	}
	int to_id(DSTNode* node) {
		if (!node) {
			return -1;
		}
		return (node - &buffer[0]);
	}
	DSTNode* to_node(int id) const {
		return (DSTNode*)&buffer[id];
	}

    double w(int id) {
        return id == -1 ? 0 : to_node(id)->weight;
    }

    double decay_factor = 1.0;
    std::vector<DSTNode> buffer;
    int last_used = 0;
    int root_id = -1;
};


#endif
