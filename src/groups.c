#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "internal.h"
#include "monkeypatch.h"

static bool
groupSetup(const scrGroup group, const scrOptions *options, int error_fd, void **group_ctx)
{
    sigset_t set;

    if (dup2(error_fd, STDERR_FILENO) < 0) {
        perror("dup2");
        return false;
    }

    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);

    if (group->create_fn) {
        setLogFd(error_fd);
        *group_ctx = group->create_fn(options->global_ctx);
    }
    else {
        *group_ctx = options->global_ctx;
    }
    setGroupCtx(*group_ctx);

    return true;
}

int
groupDo(const scrGroup group, const scrOptions *options, int error_fd, int pipe_fd)
{
    void *group_ctx;
    scrStats stats_obj = {0};
    scrTest *test;

    if (!groupSetup(group, options, error_fd, &group_ctx)) {
        return SCR_TEST_CODE_ERROR;
    }

    GEAR_FOR_EACH(&group->tests, test)
    {
        int result;

#ifdef SCR_MONKEYPATCH
        result = testRun(test, options->flags & SCR_RF_VERBOSE, &group->patch_goals);
#else
        result = testRun(test, options->flags & SCR_RF_VERBOSE);
#endif

        switch (result) {
        case SCR_TEST_CODE_OK: stats_obj.num_passed++; break;
        case SCR_TEST_CODE_SKIP: stats_obj.num_skipped++; break;
        case SCR_TEST_CODE_FAIL: stats_obj.num_failed++; break;
        default: stats_obj.num_errored++; break;
        }

        if ((options->flags) & SCR_RF_FAIL_FAST && result != SCR_TEST_CODE_OK &&
            result != SCR_TEST_CODE_SKIP) {
            break;
        }
    }

    if (group->cleanup_fn) {
        group->cleanup_fn(group_ctx);
    }

    if (write(pipe_fd, &stats_obj, sizeof(stats_obj)) != (ssize_t)sizeof(stats_obj)) {
        return SCR_TEST_CODE_ERROR;
    }

    return SCR_TEST_CODE_OK;
}

void
scrGroupAddTest(scrGroup group, const char *name, scrTestFn test_fn, const scrTestOptions *options)
{
    scrTest test = {.test_fn = test_fn};

    if (options) {
        memcpy(&test.options, options, sizeof(*options));
    }

    test.name = strdup(name);
    if (!test.name) {
        exit(1);
    }

    if (gearAppend(&group->tests, &test) != GEAR_RET_OK) {
        exit(1);
    }
}

bool
scrGroupPatchFunction(scrGroup group, const char *func_name, const char *file_substring, void *new_func)
{
#ifdef SCR_MONKEYPATCH
    scrPatchGoal goal = {.func_ptr = new_func};

    gearInit(&goal.got_entries, sizeof(void *));
    if (!findFunction(func_name, file_substring, &goal.got_entries)) {
        fprintf(stderr, "%s not found\n", func_name);
        return false;
    }

    if (gearAppend(&group->patch_goals, &goal) != GEAR_RET_OK) {
        exit(1);
    }

    return true;
#else  // SCR_MONKEYPATCH
    (void)group;
    (void)func_name;
    (void)file_substring;
    (void)new_func;
    fprintf(stderr, "Monkeypatching is not available\n");
    return false;
#endif
}

void
groupFree(scrGroup group)
{
    scrTest *test;
#ifdef SCR_MONKEYPATCH
    scrPatchGoal *goal;
#endif

    GEAR_FOR_EACH(&group->tests, test)
    {
        free(test->name);
    }
    gearReset(&group->tests);

#ifdef SCR_MONKEYPATCH
    GEAR_FOR_EACH(&group->patch_goals, goal)
    {
        gearReset(&goal->got_entries);
    }
    gearReset(&group->patch_goals);
#endif
}
