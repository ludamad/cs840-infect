#ifndef __DISCRETE_SEARCHTREE_H__
#define __DISCRETE_SEARCHTREE_H__

#include <algorithm>
#include <iostream>
#include <cstdlib>

#include "discrete_common.h"

const double MIN_WEIGHT_TO_KEEP = 1.0e-25;

// Only used in DiscreteSearchTree
struct DSTNode {
    double weight;
    entity_id entity;
    // Note: Not responsible for deleting children at all.
    DSTNode* lchild = NULL;
    DSTNode* rchild = NULL;
    DSTNode(entity_id e = -1, double w = -1) {
        entity = e;
        weight = w;
    }

    entity_id random_select(double r) {
        DEBUG_CHECK(r > 0 && r < weight, "Bad random value!");
        double wl = w(lchild);
        if (r < wl) {
            return lchild->random_select(r);
        }
        r -= wl;
        double wr = w(rchild);
        if (r < wr) {
            return rchild->random_select(r);
        }
        // if lchild == NULL and rchild == NULL always will return entity
        return entity;
    }
// How to delete?

    entity_id random_remove(double r) {
        DEBUG_CHECK(r > 0 && r < weight, "Bad random value!");
        double wl = w(lchild);
        if (r < wl) {
            return lchild->random_select(r);
        }
        r -= wl;
        double wr = w(rchild);
        if (r < wr) {
            return rchild->random_select(r);
        }
        // if lchild == NULL and rchild == NULL always will return entity
        // Delete the the entity here,
        return entity;
    }

    static DSTNode* insert(DSTNode* root, DSTNode* child) {
    	if (root == NULL) {
    		return child;
    	}
//    	ASSERT(child->lchild == NULL && child->rchild == NULL, "Nontrivial insert!");
    	double lW = w(root->lchild), rW = w(root->rchild);
    	double ww = root->weight - lW - rW;
    	if (child->weight > ww) {
    		// Swap who-is-who:
    		std::swap(root->lchild, child->lchild);
    		std::swap(root->rchild, child->rchild);
    		child->weight += root->weight - ww;
    		root->weight = ww;
    		std::swap(root, child);
    	} else {
    		root->weight += child->weight;
    	}
    	if (lW > rW) {
    		root->rchild = insert(root->rchild, child);
    	} else {
    		root->lchild = insert(root->lchild, child);
    	}
    	return root;
    }

    void assert_relation() {
    	ASSERT(this != lchild, "Loop!");
    	ASSERT(this != rchild, "Loop!");
    	double wsum = 0;
    	if (lchild) {
    		wsum += w(lchild);
    		lchild->assert_relation();
    	}
    	if (rchild) {
    		wsum += w(rchild);
    		rchild->assert_relation();
    	}
//    	ASSERT(fabs(weight - wsum - original) < 0.001, "Relation 1 broken!");
//    	ASSERT(fabs(weight - expected) < 0.001, "Relation 2 broken!");
    }

    static double w(DSTNode* node) {
        return node == NULL ? 0 : node->weight;
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
    	root = DSTNode::insert(root, node);
//    	root->assert_relation();
    }

    entity_id random_select(MTwist& rng) {
    	PERF_TIMER();
        ASSERT(root != NULL && total_weight() > 0.0, "Can't do random select with 0 weight!");
        double r = rng.rand_real_not1() * root->weight;
        return root->random_select(r);
    }

    entity_id random_remove(MTwist& rng) {
    	PERF_TIMER();
        ASSERT(root != NULL && total_weight() > 0.0, "Can't do random select with 0 weight!");
        double r = rng.rand_real_not1() * root->weight;
        return root->random_remove(&root, r);
    }

    // Downscale the node, possibly reorganizing the tree
    static void downscale(DSTNode* node, double decay) {
        if (node == NULL) {
            return;
        }
        downscale(node->lchild, decay);
        downscale(node->rchild, decay);
        node->weight /= decay;
    }

    void scale(double multiplier) {
        decay_factor /= multiplier;
        if (decay_factor > 1.0e100) {
        	double decay_inv = 1/decay_factor;
        	PERF_TIMER2("SearchTree: full scale");
//            downscale(root, decay_factor);
        	for (auto& node : buffer) {
        		node.weight *= decay_inv;
        	}
            decay_factor = 1.0;
        }
    }
    double total_weight() const {
        return root == NULL ? 0 : root->weight / decay_factor;
    }
private:
    double decay_factor = 1.0;
    std::vector<DSTNode> buffer;
    int last_used = 0;
    DSTNode* root = NULL;
};


#endif
