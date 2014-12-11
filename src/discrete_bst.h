
#ifndef DISCRETE_BST_H_
#define DISCRETE_BST_H_

#include <cstdlib>
#include <algorithm>

#include "libs/mtwist.h"

template <class K, class Num, class V>
struct DBstNode {
    K key;
    Num weight;
    V value;
    DBstNode* left = NULL;
    DBstNode* right = NULL;
    DBstNode(const K& k, const Num& initial_weight) : key(k), weight(initial_weight) {
    }
    ~DBstNode() {
        delete left, delete right;
    }
    void update_child(DBstNode** child, DBstNode* new_value) {
        if (*child) {
            weight -= (*child)->weight;
        }
        if (new_value) {
            weight += new_value->weight;
        }
        *child = new_value;
    }
    DBstNode* rot_left() {
        DBstNode* K1 = left;
        update_child(&left, K1->right);
        K1->update_child(&K1->right, this);
        return K1; /* New root */
    }

    DBstNode* rot_right() {
        DBstNode* K2 = right;
        update_child(&right, K2->left);
        K2->update_child(&K2->left, this);
        return K2; /* New root */
    }

    static Num w(DBstNode* n) {
        return n ? n->weight : Num(0);
    }

    DBstNode* try_rotate() {
        // Calculate, using our weighting heuristic, which rotation (if any) to do.
        // We want to minimize our heuristic value (hval).
        Num wt = w(this), wl = w(left), wr = w(right);
        Num hval = std::max(wl, wr);
        if (left) {
            Num lval = std::max(w(left->left), wt - wl + w(left->right));
            if (lval < hval) {
                return rot_left();
            }
        } else if (right) {
            Num rval = std::max(w(right->right), wt - wr + w(right->left));
            if (rval < hval) {
                return rot_right();
            }
        }
        return this;
    }

    static DBstNode* find(DBstNode** root, const K& k, const Num& delta_weight) {
        if (!*root) {
            *root = new DBstNode(k, delta_weight);
            return *root;
        }
        (*root)->weight += delta_weight;
        if ((*root)->key == k) {
            return *root;
        }
        if ((*root)->key < k) {
            DBstNode* result = find(&(*root)->left, k, delta_weight);
            *root = (*root)->try_rotate();
            return result;
        }
        DBstNode* result = find(&(*root)->right, k, delta_weight);
        *root = (*root)->try_rotate();
        return result;
    }

    DBstNode* weighted_select(Num num) {
        Num wl = w(left), wr = w(right);
        if (num < wl) {
            return left->weighted_select(num);
        }
        num -= wl;
        if (num < wr) {
            return right->weighted_select(num);
        }
        return this;
    }
};

template <class K, class W, class V>
struct Treap {
    DBstNode<K,W,V>* root = NULL;
    ~Treap() {
        delete root;
    }

    DBstNode<K,W,V>* find(const K& k, const W& delta_weight) {
        return DBstNode<K,W,V>::find(&root, k, delta_weight);
    }
};

// Guarantees O(log N) operations very trivially
// where N is the _maximum_ size of the universe.
struct DiscreteBST : public Treap<int, double, int> {
    DiscreteBST(int __unused = 0) {
    }
	void init(int n) {
		*this = DiscreteBST();
	}

	void insert(int i, double delta_weight) {
	    find(i, delta_weight);
	}

	int random_select(MTwist& rng) {
		double r = rng.rand_real_not1() * total_weight();
		return root->weighted_select(r)->key;
	}

	void scale(double multiplier) {
//		decay_factor /= multiplier;
//		if (decay_factor > 1.0e100) {
//			for (auto& node : nodes) {
//				node.total_weight /= decay_factor;
//			}
//			decay_factor = 1.0;
//		}
	}
	double total_weight() const {
	    return root ? root->weight : 0;
	}
};

#endif /* DISCRETE_BST_H_ */
