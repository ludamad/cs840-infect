#ifndef CUSTOMASSERT_H_
#define CUSTOMASSERT_H_

#include <cstdio>
#include "Timer.h"

// Checks that are always on:
#define ASSERT(expr, msg) \
	if (!(expr)) { \
		printf("FAILED CHECK: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
		throw msg; \
	}

// Checks that are turned off in release mode:
#ifdef NDEBUG
#define DEBUG_CHECK
#else
#define DEBUG_CHECK ASSERT
#endif

struct MilestoneRep {
	Timer timer;
	int actual = 0;
	int last = 1;
	void report(const char* str) {
		actual++;
		if (actual == last) {
			printf(str, actual);
			printf(" (%.9gms)\n", timer.get_microseconds()/1000.0);
			last *= 2;
		}
	}
};

#endif /* CUSTOMASSERT_H_ */
