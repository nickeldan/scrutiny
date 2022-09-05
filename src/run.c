#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <gear/gear.h>

#include <scrutiny/run.h>
#include <scrutiny/test.h>

typedef struct scrTestParam {
    scrTestFn test_fn;
    char *name;
    unsigned int timeout;
} scrTestParam;

struct scrGroup {
    scrCtxCreateFn create_fn;
    scrCtxCleanupFn cleanup_fn;
    gear params;
};

struct scrRunner {
    gear groups;
};

static void
testDo(int stdout_fd, int stderr_fd, int log_fd, const scrTestParam *param, void *group_ctx)
#ifdef __GNUC__
    __attribute__((noreturn))
#endif
{
    int ret = SCR_CODE_SETUP_FAIL, stdin_fd, flags;
    sigset_t set;
    scrTestCtx ctx = {.group_ctx = group_ctx, .log_fd = log_fd};

    stdin_fd = open("/dev/null", O_RDONLY);
    if (stdin_fd < 0 || dup2(stdin_fd, STDIN_FILENO) < 0) {
        goto done;
    }
    close(stdin_fd);

    ctx.log_to_tty = isatty(STDOUT_FILENO);

    if (dup2(stdout_fd, STDOUT_FILENO) < 0 || dup2(stderr_fd, STDERR_FILENO) < 0) {
        goto done;
    }
    close(stdout_fd);
    close(stderr_fd);

    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);

    if (param->timeout > 0) {
        alarm(param->timeout);
    }

    param->test_fn(&ctx);
    ret = SCR_TEST_CODE_OK;

done:
    exit(ret);
}

static int
testSummarize(const char *test_name, int stdout_fd, int stderr_fd, int log_fd, pid_t child)
{
    int status;

    while (waitpid(child, &status, 0) < 0) {}
}

static int
testRun(const scrTestParam *param, void *group_ctx)
{
    int ret = 1, stdout_fd, stderr_fd = -1, log_fd = -1, flags;
    pid_t child;
    char stdout_template[] = "/tmp/scrutiny_out_XXXXXX", stderr_template[] = "/tmp/scrutiny_err_XXXXXX",
         log_template[] = "/tmp/scrutiny_log_XXXXXX";

    stdout_fd = mkstemp(stdout_template);
    if (stdout_fd < 0) {
        return 1;
    }
    unlink(stdout_template);
    stderr_fd = mkstemp(stderr_template);
    if (stderr_fd < 0) {
        goto done;
    }
    unlink(stderr_template);

    log_fd = mkstemp(log_template);
    if (log_fd < 0) {
        goto done;
    }
    unlink(log_template);

    child = fork();
    switch (fork) {
    case -1: goto done;

    case 0: testDo(stdout_fd, stderr_fd, log_fd, param, group_ctx);

    default: break;
    }

    ret = testSummarize(test->name, stdout_fd, stderr_fd, log_fd, child);

done:
    close(stdout_fd);
    close(stderr_fd);
    close(log_fd);
    return ret;
}

static void
groupRun(const scrGroup *group, void *global_ctx)
{
    pid_t child;

    child = fork();
    if (child < 0) {
        exit(1);
    }

    if (child == 0) {
        pid_t grandchild;
        void *group_ctx;

        if (group->create_fn) {
            group_ctx = group->create_fn(global_ctx);
        }
        else {
            group_ctx = NULL;
        }

        grandchild = fork();
        if (grandchild < 0) {
            exit(1);
        }

        if (grandchild == 0) {
            if (dup2(stdout_fd, STDOUT_FILENO) < 0 || dup2(stderr_fd, STDERR_FILENO) < 0) {
                exit(SCR_TEST_CODE_SETUP_FAIL);
            }
            close(stdout_fd);
            close(stderr_fd);
        }
        else {
        }
    }
    else {
    }
}

scrRunner *
scrRunnerCreate(void)
{
    scrRunner *runner;

    runner = malloc(sizeof(*runner));
    if (!runner) {
        exit(1);
    }
    runner->groups = GEAR_INIT(scrGroup);

    return runner;
}

void
scrRunnerDestroy(scrRunner *runner)
{
    scrGroup *group;

    if (!runner) {
        return;
    }

    GEAR_FOR_EACH(&runner->groups, group)
    {
        scrTestParam *param;

        GEAR_FOR_EACH(&group->params, param)
        {
            free(param->name);
        }
        gearFree(&group->params);
    }

    gearFree(&runner->groups);
    free(runner);
}

scrGroup *
scrGroupCreate(scrRunner *runner, scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn)
{
    scrGroup group = {.create_fn = create_fn, .cleanup_fn = cleanup_fn, .params = GEAR_INIT(scrTestParam)};

    if (!runner) {
        return NULL;
    }

    if (gearAppend(&runner->groups, &group) != GEAR_RET_OK) {
        exit(1);
    }

    return GEAR_GET_ITEM(&runner->groups, runner->groups.length - 1);
}

void
scrGroupAddTest(scrGroup *group, const char *name, scrTestFn test_fn, scrCtxCreateFn create_fn,
                scrCtxCleanupFn cleanup_fn, unsigned int timeout)
{
    scrTestParam param = {
        .test_fn = test_fn, .create_fn = create_fn, .cleanup_fn = cleanup_fn, .timeout = timeout};

    if (!group || !name || !test_fn) {
        return;
    }

    param.name = strdup(name);
    if (!param.name) {
        exit(1);
    }

    if (gearAppend(&group->params, &param) != GEAR_RET_OK) {
        exit(1);
    }
}

void
scrRunnerRun(scrRunner *runner, void *global_ctx)
{
    scrGroup *group;

    GEAR_FOR_EACH(&runner->groups, group)
    {
        groupRun(group, global_ctx);
    }
}
