#ifndef SCRUTINY_RUN_H
#define SCRUTINY_RUN_H

typedef struct scrRunner scrRunner;
typedef struct scrGroup scrGroup;

typedef void (*scrTestFn)(void *);
typedef void *(*scrCtxCreateFn)(void *);
typedef void (*scrCtxCleanupFn)(void *);

typedef struct scrStats {
    unsigned int num_passed;
    unsigned int num_skipped;
    unsigned int num_failed;
    unsigned int num_errored;
} scrStats;

scrRunner *
scrRunnerCreate(void)
#ifdef __GNUC__
    __attribute__((malloc))
#endif
    ;

void
scrRunnerDestroy(scrRunner *runner);

scrGroup *
scrGroupCreate(scrRunner *runner, scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn)
#ifdef __GNUC__
    __attribute__((malloc)) __attribute__((nonnull(1)))
#endif
    ;

void
scrGroupAddTest(scrGroup *group, char *name, scrTestFn test_fn, unsigned int timeout)
#ifdef __GNUC__
    __attribute__((nonnull(1, 2, 3)))
#endif
    ;

void
scrRunnerRun(scrRunner *runner, void *global_ctx, scrStats *stats)
#ifdef __GNUC__
    __attribute__((nonnull(1)))
#endif
    ;

#endif  // SCRUTINY_RUN_H
