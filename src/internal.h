#ifndef SCRUTINY_INTERNAL_H
#define SCRUTINY_INTERNAL_H

#include <stdbool.h>
#include <sys/types.h>

#include <gear/gear.h>

#include <scrutiny/scrutiny.h>

typedef struct scrTestParam {
    scrTestFn test_fn;
    char *name;
    unsigned int timeout;
    unsigned int flags;
} scrTestParam;

struct scrGroup {
    scrCtxCreateFn create_fn;
    scrCtxCleanupFn cleanup_fn;
    gear params;
};

#define GREEN       "\x1b[0;32m"
#define YELLOW      "\x1b[0;33m"
#define RED         "\x1b[0;31m"
#define RESET_COLOR "\x1b[0m"

const int *
getKillSignals(unsigned int *count);

pid_t
cleanFork(void);

void
dumpFd(int fd, bool show_color);

int
groupDo(const scrGroup *group, const scrOptions *options, bool show_color, int error_fd, int pipe_fd);

void
showTestResult(const scrTestParam *param, scrTestCode result, bool show_color);

scrTestCode
testRun(const scrTestParam *param, bool show_color);

void
setGroupCtx(void *ctx);

void
setShowColor(bool should_show_color);

void
setLogFd(int fd);

void
waitForProcess(pid_t pid, int *status);

#endif  // SCRUTINY_INTERNAL_H
