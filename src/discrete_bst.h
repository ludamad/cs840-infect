
#ifndef DISCRETE_BST_H_
#define DISCRETE_BST_H_

#include <cstdlib>
#include <algorithm>

#include "libs/mtwist.h"
#include "discrete_common.h"

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

    template <class Functor>
    void forall(Functor f) {
    	f(this);
    	if (left) left->forall(f);
    	if (right) right->forall(f);
    }
    void scale(Num d) {
    	weight *= d;
    	if (left) left->scale(d);
    	if (right) right->scale(d);
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
struct DBST {
	typedef DBstNode<K,W,V> Node;
    Node* root = NULL;
    ~DBST() {
        delete root;
    }

    Node* find(const K& k, const W& delta_weight) {
        return Node::find(&root, k, delta_weight);
    }
};

// Guarantees O(log N) operations very trivially
// where N is the _maximum_ size of the universe.
struct DiscreteBST : public DBST<int, floatT, int> {
    DiscreteBST(int __unused = 0) {
    }
	void init(int n) {
		*this = DiscreteBST();
	}

	void insert(int i, floatT delta_weight) {
		PERF_TIMER();
	    find(i, delta_weight / decay_factor);
	}

	int random_select(MTwist& rng) {
		PERF_TIMER();
		floatT r = rng.rand_real_not1() * total_weight();
		return root->weighted_select(r)->key;
	}

	void scale(floatT multiplier) {
		decay_factor *= multiplier;
		if (decay_factor < FLOAT_EXP_BOTTOM) {
			if (root) {
				root->scale(decay_factor);
			}
			decay_factor = 1.0;
		}
	}
	floatT total_weight() const {
	    return root ? root->weight * decay_factor : 0;
	}
	floatT decay_factor = 1.0;
};

#endif /* DISCRETE_BST_H_ */
