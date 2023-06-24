#ifndef SCRUTINY_INTERNAL_H
#define SCRUTINY_INTERNAL_H

#include <stdbool.h>
#include <sys/types.h>

#include <gear/gear.h>

#include <scrutiny/scrutiny.h>

typedef enum scrTestCode {
    SCR_TEST_CODE_OK = 0,
    SCR_TEST_CODE_FAIL,
    SCR_TEST_CODE_ERROR,
    SCR_TEST_CODE_SKIP,
} scrTestCode;

typedef struct scrTestParam {
    scrTestFn *test_fn;
    char *name;
    unsigned int timeout;
    unsigned int flags;
} scrTestParam;

#ifdef SCR_MONKEYPATCH

typedef struct scrPatchGoal {
    char *func_name;
    void *func_ptr;
    gear got_entries;
} scrPatchGoal;

#endif

struct scrGroup {
    scrCtxCreateFn *create_fn;
    scrCtxCleanupFn *cleanup_fn;
    gear params;
#ifdef SCR_MONKEYPATCH
    gear patch_goals;
#endif
};

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#define GREEN       "\x1b[0;32m"
#define YELLOW      "\x1b[0;33m"
#define RED         "\x1b[0;31m"
#define RESET_COLOR "\x1b[0m"

pid_t
cleanFork(void);

void
dumpFd(int fd, bool printable_only);

int
groupDo(const scrGroup *group, const scrOptions *options, int error_fd, int pipe_fd);

void
groupFree(scrGroup *group);

void
showTestResult(const scrTestParam *param, scrTestCode result);

#ifdef SCR_MONKEYPATCH
scrTestCode
testRun(const scrTestParam *param, bool verbose, const gear *patch_goals);
#else
scrTestCode
testRun(const scrTestParam *param, bool verbose);
#endif

void
setGroupCtx(void *ctx);

void
setLogFd(int fd);

void
waitForProcess(pid_t pid, unsigned int timeout, int *status, bool *timed_out);

extern bool show_color;

#endif  // SCRUTINY_INTERNAL_H
