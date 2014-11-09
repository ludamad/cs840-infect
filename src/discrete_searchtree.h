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
    DSTNode* lchild = NULL;
    DSTNode* rchild = NULL;

    DSTNode(entity_id e, double w) {
        entity = e;
        weight = w;
    }
    ~DSTNode() {
        delete lchild;
        delete rchild;
    }

    entity_id random_select(double r) {
        DEBUG_CHECK(r > 0 && r < weight, "Bad random value!");
        double wl = w(lchild), wr = w(rchild);
        if (r < wl) {
            return lchild->random_select(r);
        }
        r -= wl;
        if (r < wr) {
            return rchild->random_select(r);
        }
        // if lchild == NULL and rchild == NULL always will return entity
        return entity;
    }
    static DSTNode* insert(DSTNode* root, DSTNode* child) {
    	if (root == NULL) {
    		return child;
    	}
    	double rw = root->weight;
    	double ll = w(root->lchild), lr = w(root->rchild), cw = child->weight;
    	double cW = ll + lr + cw;

    	if (ll < lr)  {
    		if (rw > cW || ll + cw >= lr) {
    			root->lchild = insert(root->lchild, child);
    			root->weight += cw;
    		} else {
    			child->lchild = root;
				child->weight += rw;
				root = child;
    		}
    	} else {
    		if (rw > cW || lr + cw >= ll) {
				root->rchild = insert(root->rchild, child);
				root->weight += cw;
			} else {
    			child->rchild = root;
    			child->weight += rw;
				root = child;
			}
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
        // max_size not used currently
    }
    ~DiscreteSearchTree() {
        delete root;
    }
    void init(int max_size = 0) {
        // max_size not used currently
        delete root;
        root = NULL;
    }

    void insert(entity_id entity, double weight) {
    	PERF_TIMER();
    	root = DSTNode::insert(root, new DSTNode(entity, weight));
//    	root->assert_relation();
    }

    entity_id random_select(MTwist& rng) {
    	PERF_TIMER();
        ASSERT(root != NULL && total_weight() > 0.0, "Can't do random select with 0 weight!");
        return root->random_select(rng.rand_real_not1() * total_weight());
    }

    static void scale(DSTNode* node, double multiplier) {
        if (node == NULL) {
            return;
        }
        node->weight *= multiplier;
        scale(node->lchild, multiplier);
        scale(node->rchild, multiplier);
    }

    void scale(double multiplier) {
        decay_factor /= multiplier;
        if (decay_factor > 1.0e100) {
            scale(root, decay_factor);
            decay_factor = 1.0;
        }
    }
    double total_weight() {
        return root == NULL ? 0 : root->weight;
    }
private:
    double decay_factor = 1.0;
    DSTNode* root = NULL;
};


#endif
