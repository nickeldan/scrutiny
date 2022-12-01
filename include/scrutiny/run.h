#ifndef SCRUTINY_RUN_H
#define SCRUTINY_RUN_H

typedef struct scrRunner scrRunner;
typedef struct scrGroup scrGroup;

typedef struct scrStats {
    unsigned int num_passed;
    unsigned int num_skipped;
    unsigned int num_failed;
    unsigned int num_errored;
} scrStats;

typedef void (*scrTestFn)(void);
typedef void *(*scrCtxCreateFn)(void *);
typedef void (*scrCtxCleanupFn)(void *);

#define SCR_FLAG_XFAIL 0x00000001

scrRunner *
scrRunnerCreate(void)
#ifdef __GNUC__
    __attribute__((malloc))
#endif
    ;

void
scrRunnerDestroy(scrRunner *runner);

void
scrRunnerRun(scrRunner *runner, void *global_ctx, scrStats *stats)
#ifdef __GNUC__
    __attribute__((nonnull(1)))
#endif
    ;

scrGroup *
scrGroupCreate(scrRunner *runner, scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn)
#ifdef __GNUC__
    __attribute__((malloc)) __attribute__((nonnull(1)))
#endif
    ;

void
scrGroupAddTest(scrGroup *group, char *name, scrTestFn test_fn, unsigned int timeout, unsigned int flags)
#ifdef __GNUC__
    __attribute__((nonnull(1, 2, 3)))
#endif
    ;

#endif  // SCRUTINY_RUN_H
