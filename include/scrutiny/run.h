/**
 * @file run.h
 * @brief Defines the runner and group functionality.
 */

#ifndef SCRUTINY_RUN_H
#define SCRUTINY_RUN_H

/**
 * @brief An opaque structure which runs all of the tests.
 */
typedef struct scrRunner scrRunner;

/**
 * @brief An opaque structure encapsulating each testing group.
 */
typedef struct scrGroup scrGroup;

/**
 * @brief Holds the test results.
 */
typedef struct scrStats {
    unsigned int num_passed;   /// The number of tests which passed.
    unsigned int num_skipped;  /// The number of tests which were skipped.
    unsigned int num_failed;   /// The number of tests which failed.
    unsigned int
        num_errored;  /// The number of tests which encountered an error (i.e., some terminating signal).
} scrStats;

/**
 * @brief The signature for a test function.
 */
typedef void (*scrTestFn)(void);

/**
 * @brief The signature for a group context creation function.
 */
typedef void *(*scrCtxCreateFn)(void *);

/**
 * @brief The signature for a group context cleanup function.
 */
typedef void (*scrCtxCleanupFn)(void *);

/**
 * @brief Indicates that a test is expected to fail.
 */
#define SCR_TEST_FLAG_XFAIL 0x00000001

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
scrRunnerCreate(void)
#ifdef __GNUC__
    __attribute__((malloc))
#endif
    ;

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
 * @param flags         Zero or more flags bitwise-or combined.
 * @param global_ctx    The global context to use for the run.
 * @param[out] stats    If not NULL, then will be populated with the run's statistics.
 *
 * @return              0 if all of the tests either passed or were skipped and 1 otherwise.
 */
int
scrRunnerRun(scrRunner *runner, unsigned int flags, void *global_ctx, scrStats *stats)
#ifdef __GNUC__
    __attribute__((nonnull(1)))
#endif
    ;

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
scrGroupCreate(scrRunner *runner, scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn)
#ifdef __GNUC__
    __attribute__((malloc)) __attribute__((nonnull(1)))
#endif
    ;

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
                unsigned int flags)
#ifdef __GNUC__
    __attribute__((nonnull(1, 2, 3)))
#endif
    ;

#endif  // SCRUTINY_RUN_H
