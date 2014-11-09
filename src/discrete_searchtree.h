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
    DSTNode(entity_id e, double w) {
        entity = e;
        weight = w;
    }
    ~DSTNode() {
        delete lchild;
        delete rchild;
    }
    DSTNode* get_left() { return lchild; }
    DSTNode* get_right() { return rchild; }
    void set_left(DSTNode* node) {
    	double wl = w(lchild);
        weight += w(node) - wl;
        rchild = node;
    }
    void set_right(DSTNode* node) {
		double wr = w(rchild);
		weight += w(node) - wr;
		lchild = node;
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
    	double lw = w(root->lchild), lr = w(root->rchild), cw = child->weight;
    	double cW = lw + lr + cw;

    	if (lw < lr)  {
    		if (lw + cw >= lr) {
    			root->lchild = insert(root->lchild, child);
    			root->weight += child->weight;
    		} else if (rw < cW) {
    			child->lchild = insert(root->lchild, root);
    			child->weight = child->lchild->weight;
    			root = child;
    		}
    	} else {
    		if (lw + cw >= lr) {
				root->lchild = insert(root->lchild, child);
				root->weight += child->weight;
			} else if (rw < cW) {
    			child->set_right(insert(root->rchild, root));
    			child->weight = child->rchild->weight;
				root = child;
			}
    	}
    	return root;
    }
    DSTNode* insert(DSTNode* child) {
    	DSTNode* root = this;
    	double oldw = weight;
    	double lw = w(lchild), lr = w(rchild);
    	if (lw < lr) {
    		if (oldw < lw + lr + w) {
    			root->lchild = insert(lchild, root);
    			root = child;
    		}

    	} else {

    	}
    	return root;
    }
    double sole_weight() {
        return weight - w(lchild) - w(rchild);
    }
    void assert_relation() {
    	double wsum = 0;
    	if (lchild) {
    		wsum += w(lchild);
    		lchild->assert_relation();
    	}
    	if (rchild) {
    		wsum += w(rchild);
    		rchild->assert_relation();
    	}
    	ASSERT(weight >= wsum, "Relation broken!");
    }
private:
    static double w(DSTNode* node) {
        return node == NULL ? 0 : node->weight;
    }
    DSTNode* lchild = NULL;
    DSTNode* rchild = NULL;
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
        auto* N = new DSTNode(entity, weight);
        if (root == NULL) {
        	root = N;
        	return;
        }
        root->assert_relation();
        auto* nodeL = node->get_left(), *nodeR = node->get_right();
        double leftw = nodeL == NULL ? 0 : nodeL->weight;
        double rightw = nodeR == NULL ? 0 : nodeR->weight;
        // If we add this weight to our left subtree, will it continue to meet our constraint?
        if (weight + leftw < rightw) {
            N->set_left(nodeL);
            node->set_left(NULL);
        } else {
            N->set_right(nodeR);
            node->set_right(NULL);
        }
        root = N;
        root->assert_relation();
    }

    entity_id random_select(MTwist& rng) {
        ASSERT(root != NULL && total_weight() > 0.0, "Can't do random select with 0 weight!");
        return root->random_select(rng.rand_real_not1() * total_weight());
    }

    static void scale(DSTNode* node, double multiplier) {
        if (node == NULL) {
            return;
        }
        node->weight *= multiplier;
        scale(node->get_left(), multiplier);
        scale(node->get_right(), multiplier);
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
