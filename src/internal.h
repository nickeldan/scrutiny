#pragma once

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

typedef struct scrTest {
    scrTestFn *test_fn;
    char *name;
    scrTestOptions options;
#ifdef SCR_MONKEYPATCH
    gear *patch_goals;
#endif
} scrTest;

#ifdef SCR_MONKEYPATCH

typedef struct scrPatchGoal {
    void *func_ptr;
    gear got_entries;
} scrPatchGoal;

#endif

typedef struct scrGroupStruct {
    scrCtxCreateFn *create_fn;
    scrCtxCleanupFn *cleanup_fn;
    gear tests;
#ifdef SCR_MONKEYPATCH
    gear patch_goals;
#endif
} scrGroupStruct;

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
groupDo(const scrGroupStruct *group, const scrOptions *options, int error_fd, int pipe_fd);

void
groupFree(scrGroupStruct *group);

void
showTestResult(const scrTest *test, scrTestCode result);

scrTestCode
testRun(const scrTest *test, bool verbose);

void
setGroupCtx(void *ctx);

void
setLogFd(int fd);

void
waitForProcess(pid_t pid, unsigned int timeout, int *status, bool *timed_out);

extern gear groups;
extern bool show_color;
