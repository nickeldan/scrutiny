#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "internal.h"
#include "monkeypatch.h"

static void
paramFree(void *item)
{
    scrTestParam *param = item;

    free(param->name);
}

static bool
groupSetup(const scrGroup *group, const scrOptions *options, int error_fd, void **group_ctx)
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
groupDo(const scrGroup *group, const scrOptions *options, int error_fd, int pipe_fd)
{
    void *group_ctx;
    scrStats stats_obj = {0};
    scrTestParam *param;

    if (!groupSetup(group, options, error_fd, &group_ctx)) {
        return SCR_TEST_CODE_ERROR;
    }

    LINKED_LIST_ITERATE(&group->params, param)
    {
        int result;

#ifdef SCR_MONKEYPATCH
        result = testRun(param, options->flags & SCR_RUN_FLAG_VERBOSE, &group->patch_goals);
#else
        result = testRun(param, options->flags & SCR_RUN_FLAG_VERBOSE);
#endif

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
    scrTestParam *param;

    param = linkedListNodeNew(sizeof(*param));
    if (!param) {
        exit(1);
    }
    param->test_fn = test_fn;
    param->timeout = timeout;
    param->flags = flags;
    param->name = strdup(name);
    if (!param->name) {
        exit(1);
    }

    linkedListAppend(&group->params, &param->node);
    group->num_tests++;
}

bool
scrGroupPatchFunction(scrGroup *group, const char *func_name, void *new_func)
{
#ifdef SCR_MONKEYPATCH
    scrPatchGoal *goal;
    scrPatchGoal *ptr;

    LINKED_LIST_ITERATE(&group->patch_goals, ptr)
    {
        if (strcmp(ptr->func_name, func_name) == 0) {
            fprintf(stderr, "A patch has already been registered for %s\n", func_name);
            return false;
        }
    }

    goal = linkedListNodeNew(sizeof(*goal));
    goal->func_ptr = new_func;
    goal->func_name = strdup(func_name);
    if (!goal->func_name) {
        exit(1);
    }
    linkedListInit(&goal->got_entries);

    if (!findFunction(func_name, &goal->got_entries)) {
        fprintf(stderr, "%s not found\n", func_name);
        patchGoalFree(goal);
        return false;
    }

    linkedListAppend(&group->patch_goals, &goal->node);

    return true;
#else  // SCR_MONKEYPATCH
    (void)group;
    (void)func_name;
    (void)new_func;
    fprintf(stderr, "Monkeypatching is not available\n");
    return false;
#endif
}

void
groupFree(void *item)
{
    scrGroup *group = item;

    linkedListFree(&group->params, paramFree);
#ifdef SCR_MONKEYPATCH
    linkedListFree(&group->patch_goals, patchGoalFree);
#endif
}
