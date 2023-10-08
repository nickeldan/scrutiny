/**
 * @file run.h
 * @brief Defines the runner functionality.
 */

#pragma once

#include <stdbool.h>
#include <sys/types.h>

#include "definitions.h"

/**
 * @brief A representation of a testing group.
 */
typedef size_t scrGroup;

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
 * @brief Options to pass to scrGroupAddTest.
 */
typedef struct scrTestOptions {
    unsigned int timeout; /**< If positive, the number of seconds to timeout the test. */
    unsigned int flags;   /**< Bitwise-or-combined flags. */
} scrTestOptions;

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
#define SCR_TF_XFAIL 0x00000001

/**
 * @brief Instructs the runner to stop as soon as a test fails.
 */
#define SCR_RF_FAIL_FAST 0x00000001
/**
 * @brief Displays output even when tests pass.
 */
#define SCR_RF_VERBOSE 0x00000002

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
scrGroup
scrGroupCreate(scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn) SCR_EXPORT;

/**
 * @brief Adds a test to a group.
 *
 * @param group     The group handle.
 * @param name      The name of the test.
 * @param test_fn   The test function.
 * @param options   A pointer to the options to use.  If NULL, default options will be used.
 */
void
scrGroupAddTest(scrGroup group, const char *name, scrTestFn test_fn, const scrTestOptions *options) SCR_EXPORT
    SCR_NONNULL(2, 3);

/**
 * @brief Enables monkeypatching of a function for all of a group's tests.
 *
 * @param group             The group handle.
 * @param func_name         The name of the function to patch.
 * @param file_substring    If not NULL, then only files containing this value as a substring will be patched.
 * @param new_func          The new function to use.
 *
 * @return              true if successful and false otherwise.  If monkeypatching was not enabled at compile
 * time, then this function will always return false.
 *
 * @note                Functions within libscrutiny.so cannot be monkeypatched.
 */
bool
scrGroupPatchFunction(scrGroup group, const char *func_name, const char *file_substring,
                      void *new_func) SCR_EXPORT SCR_NONNULL(2, 4);

/**
 * @brief Runs all of the tests.
 *
 * @param options       A pointer to options to use.  If NULL, default options will be used.
 * @param[out] stats    If not NULL, then will be populated with the run's statistics.
 *
 * @return              0 if all of the tests either passed or were skipped and 1 otherwise.
 */
int
scrRun(const scrOptions *options, scrStats *stats) SCR_EXPORT;
