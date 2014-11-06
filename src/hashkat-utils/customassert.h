/*
 * customassert.h
 *
 *  Created on: Nov 6, 2014
 *      Author: adomurad
 */

#ifndef CUSTOMASSERT_H_
#define CUSTOMASSERT_H_

// Checks that are always on:
#define ASSERT(expr, msg) \
if (!(expr)) { \
printf("FAILED CHECK: %s (%s:%d)\n", msg, __FILE__, __LINE__); \
throw msg; \
}
// Checks that are turned off in release mode:
#define DEBUG_CHECK ASSERT

#endif /* CUSTOMASSERT_H_ */
