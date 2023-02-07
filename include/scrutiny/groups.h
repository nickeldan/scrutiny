/**
 * @file groups.h
 * @brief Defines the group functionality.
 */

#ifndef SCRUTINY_GROUPS_H
#define SCRUTINY_GROUPS_H

#include "definitions.h"

/**
 * @brief An opaque structure encapsulating each testing group.
 */
typedef struct scrGroup scrGroup;

/**
 * @brief The signature for a group context creation function.
 */
typedef void *
scrCtxCreateFn(void *);

/**
 * @brief The signature for a group context cleanup function.
 */
typedef void
scrCtxCleanupFn(void *);

/**
 * @brief The signature for a test function.
 */
typedef void
scrTestFn(void);

/**
 * @brief Indicates that a test is expected to fail.
 */
#define SCR_TEST_FLAG_XFAIL 0x00000001

/**
 * @brief Adds a test to a group.
 *
 * @param group     The group handle.
 * @param name      The name of the test.
 * @param test_fn   The test function.
 * @param timeout   If positive, then the number of seconds the test has to finish.
 * @param flags     Zero or more flags bitwise-or combined.
 */
void
scrGroupAddTest(scrGroup *group, const char *name, scrTestFn test_fn, unsigned int timeout,
                unsigned int flags) SCR_NONNULL(1, 2, 3);

#endif  // SCRUTINY_GROUPS_H
