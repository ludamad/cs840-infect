#ifndef DISCRETE_COMMON_H_
#define DISCRETE_COMMON_H_

#include <cstdio>
#include "hashkat-utils/mtwist.h"

#include "libs/perf_timer.h"

// An effective scheme for 'decay' is to, instead of effecting all current elements
// we simply scale UP elements inserted from that point forth.
// To effectively store
struct weight_t {
	weight_t(double d) {
		val = frexp(d, &exponent);
	}
	weight_t& operator+=(const weight_t& w) {
		int delta_exp = exponent - w.exponent;
		int exp_overflow = 0;
		val = frexp(val + ldexp(w.val, delta_exp), &exp_overflow);
		exponent += exp_overflow;
		return *this;
	}
	weight_t operator+(const weight_t& w) {
		weight_t copy(*this);
		copy += w;
		return copy;
	}
	weight_t& operator*=(const weight_t& w) {
		int exp_overflow = 0;
		val = frexp(val * w.val, &exp_overflow);
		exponent += exp_overflow + w.exponent;
		return *this;
	}
	weight_t operator*(const weight_t& w) {
		weight_t copy(*this);
		copy *= w;
		return copy;
	}
	int exponent; // Note: can be negative, to allow for using 'frexp' directly
	double val; // val <= 1.00
};

typedef int entity_id;

#endif /* DISCRETE_COMMON_H_ */
