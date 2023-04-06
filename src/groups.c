#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "internal.h"

static bool
groupSetup(const scrGroup *group, const scrOptions *options, bool show_color, int error_fd, void **group_ctx)
{
    sigset_t set;

    if (dup2(error_fd, STDERR_FILENO) < 0) {
        perror("dup2");
        return false;
    }

    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);

    setShowColor(show_color);

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
groupDo(const scrGroup *group, const scrOptions *options, bool show_color, int error_fd, int pipe_fd)
{
    void *group_ctx;
    scrStats stats_obj = {0};
    scrTestParam *param;

    if (!groupSetup(group, options, show_color, error_fd, &group_ctx)) {
        return SCR_TEST_CODE_ERROR;
    }

    GEAR_FOR_EACH(&group->params, param)
    {
        int result;

        result = testRun(param, show_color);

        switch (result) {
        case SCR_TEST_CODE_OK: stats_obj.num_passed++; break;

        case SCR_TEST_CODE_SKIP: stats_obj.num_skipped++; break;

        case SCR_TEST_CODE_FAIL: stats_obj.num_failed++; break;

        default: stats_obj.num_errored++; break;
        }

        if ((options->flags) & SCR_RUN_FLAG_FAIL_FAST && result != SCR_TEST_CODE_OK &&
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
scrGroupAddTest(scrGroup *group, const char *name, scrTestFn test_fn, unsigned int timeout,
                unsigned int flags)
{
    scrTestParam param = {.test_fn = test_fn, .timeout = timeout, .flags = flags};

    param.name = strdup(name);
    if (!param.name) {
        exit(1);
    }

    if (gearAppend(&group->params, &param) != GEAR_RET_OK) {
        exit(1);
    }
}

void
groupFree(scrGroup *group)
{
    scrTestParam *param;

    GEAR_FOR_EACH(&group->params, param)
    {
        free(param->name);
    }
    gearReset(&group->params);
}
