/**
 * @file run.h
 * @brief Defines the runner functionality.
 */

#ifndef SCRUTINY_RUN_H
#define SCRUTINY_RUN_H

#include "definitions.h"
#include "groups.h"

/**
 * @brief An opaque structure which runs all of the tests.
 */
typedef struct scrRunner scrRunner;

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
 * @brief Instructs the runner to stop as soon as a test fails.
 */
#define SCR_RUN_FLAG_FAIL_FAST 0x00000001

/**
 * @brief Creates a runner.
 *
 * @return  A runner handle.
 */
scrRunner *
scrRunnerCreate(void) SCR_MALLOC;

/**
 * @brief Releases a runner's resources.
 *
 * @param runner    The runner handle.
 */
void
scrRunnerDestroy(scrRunner *runner);

/**
 * @brief Runs all of tests.
 *
 * @param runner        The runner handle.
 * @param options       A pointer to options to use.  If NULL, default options will be used.
 * @param[out] stats    If not NULL, then will be populated with the run's statistics.
 *
 * @return              0 if all of the tests either passed or were skipped and 1 otherwise.
 */
int
scrRunnerRun(scrRunner *runner, const scrOptions *options, scrStats *stats) SCR_NONNULL(1);

/**
 * @brief Creates a new test group.
 *
 * @param runner        The runner handle.
 * @param create_fn     If not NULL, then a function that will create the group context from the global
 * context.
 * @param cleanup_fn    If not NULL, then a function that will be called with the group context after the
 * tests have finished.
 *
 * @return  A group handle.
 */
scrGroup *
scrGroupCreate(scrRunner *runner, scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn) SCR_MALLOC
    SCR_NONNULL(1);

#endif  // SCRUTINY_RUN_H
