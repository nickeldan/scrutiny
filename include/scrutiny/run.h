#ifndef SCRUTINY_RUN_H
#define SCRUTINY_RUN_H

typedef struct scrRunner scrRunner;
typedef struct scrGroup scrGroup;

typedef void (*scrTestFn)(void *);
typedef void *(*scrCtxCreateFn)(void *);
typedef void (*scrCtxCleanupFn)(void *);

scrRunner *
scrRunnerCreate(void);

void
scrRunnerDestroy(scrRunner *runner);

scrGroup *
scrGroupCreate(scrRunner *runner, scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn);

void
scrGroupAddTest(scrGroup *group, const char *name, scrTestFn test_fn, unsigned int timeout);

void
scrRunnerRun(scrRunner *runner, void *global_ctx);

#endif  // SCRUTINY_RUN_H
