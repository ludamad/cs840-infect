#ifndef CUSTOMASSERT_H_
#define CUSTOMASSERT_H_

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
#endif /* CUSTOMASSERT_H_ */
