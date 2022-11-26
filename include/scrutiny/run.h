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
scrRunnerCreate(void);

void
scrRunnerDestroy(scrRunner *runner);

scrGroup *
scrGroupCreate(scrRunner *runner, scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn);

void
scrGroupAddTest(scrGroup *group, const char *name, scrTestFn test_fn, unsigned int timeout);

void
scrRunnerRun(scrRunner *runner, void *global_ctx, scrStats *stats);

#endif  // SCRUTINY_RUN_H
