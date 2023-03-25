/**
 * @file run.h
 * @brief Defines the runner functionality.
 */

#ifndef SCRUTINY_RUN_H
#define SCRUTINY_RUN_H

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
 * @brief Options to pass to scrRunnerRun.
 */
typedef struct scrOptions {
    void *global_ctx;   /**< The global context for the tests. */
    unsigned int flags; /**< Bitwise-or-combined flags. */
} scrOptions;

/**
 * @brief Holds the test results.
 */
typedef struct scrStats {
    unsigned int num_passed;  /**< The number of tests which passed. */
    unsigned int num_skipped; /**< The number of tests which were skipped. */
    unsigned int num_failed;  /**< The number of tests which failed. */
    unsigned int
        num_errored; /**< The number of tests which encountered an error (i.e., some terminating signal). */
} scrStats;

/**
 * @brief Indicates that a test is expected to fail.
 */
#define SCR_TEST_FLAG_XFAIL 0x00000001

/**
 * @brief Instructs the runner to stop as soon as a test fails.
 */
#define SCR_RUN_FLAG_FAIL_FAST 0x00000001

/**
 * @brief Creates a new test group.
 *
 * @param create_fn     If not NULL, then a function that will create the group context from the global
 * context.
 * @param cleanup_fn    If not NULL, then a function that will be called with the group context after the
 * tests have finished.
 *
 * @return  A group handle.
 */
scrGroup *
scrGroupCreate(scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn) SCR_MALLOC;

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

/**
 * @brief Runs all of tests.
 *
 * @param options       A pointer to options to use.  If NULL, default options will be used.
 * @param[out] stats    If not NULL, then will be populated with the run's statistics.
 *
 * @return              0 if all of the tests either passed or were skipped and 1 otherwise.
 */
int
scrRun(const scrOptions *options, scrStats *stats);

#endif  // SCRUTINY_RUN_H
