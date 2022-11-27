#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include <gear/gear.h>

#include <scrutiny/run.h>
#include <scrutiny/test.h>

#include "internal.h"

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))
#endif

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
    scrStats stats;
};

static void
signalHandler(int signum)
{
    (void)signum;
    kill(-1, SIGTERM);
    while (waitpid(-1, NULL, 0) > 0) {}
}

static bool
caughtSignal(void)
{
    sigset_t set;

    sigpending(&set);
    return sigismember(&set, SIGTERM);
}

#ifdef __GNUC__
static void
testDo(int stdout_fd, int stderr_fd, int log_fd, const scrTestParam *param, void *group_ctx)
    __attribute__((noreturn));
#endif

static void
testDo(int stdout_fd, int stderr_fd, int log_fd, const scrTestParam *param, void *group_ctx)
{
    int ret = SCR_TEST_CODE_ERROR, stdin_fd;
    sigset_t set;

    setParams(group_ctx, log_fd);

    stdin_fd = open("/dev/null", O_RDONLY);
    if (stdin_fd < 0 || dup2(stdin_fd, STDIN_FILENO) < 0) {
        goto done;
    }
    close(stdin_fd);

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

    param->test_fn();
    ret = SCR_TEST_CODE_OK;

done:
    exit(ret);
}

#define GREEN       "\x1b[0;32m"
#define YELLOW      "\x1b[0;33m"
#define RED         "\x1b[0;31m"
#define RESET_COLOR "\x1b[0m"

static bool
dumpFd(int fd, bool show_color)
{
    bool printed = false;
    ssize_t transmitted;
    char buffer[1024];

    lseek(fd, 0, SEEK_SET);

    while ((transmitted = read(fd, buffer, sizeof(buffer))) > 0) {
        if (write(STDOUT_FILENO, buffer, transmitted) < 0) {}
        printed = true;
    }

    if (show_color) {
        if (write(STDOUT_FILENO, RESET_COLOR, sizeof(RESET_COLOR) - 1) < 0) {}
    }

    return printed;
}

static scrTestCode
testSummarize(const char *test_name, int stdout_fd, int stderr_fd, int log_fd, pid_t child)
{
    scrTestCode ret;
    int status;
    bool show_output = false, show_color;
    struct timespec spec = {.tv_nsec = 10000000};  // 1/100 of a second

    show_color = isatty(STDOUT_FILENO);

    while (1) {
        if (caughtSignal()) {
            kill(child, SIGKILL);
            waitpid(child, NULL, 0);
            exit(0);
        }

        if (waitpid(child, &status, WNOHANG) == child) {
            break;
        }

        nanosleep(&spec, NULL);
    }

    printf("Test result (%s): ", test_name);

    if (WIFSIGNALED(status)) {
        printf("%sERROR%s: Terminated by signal (%i)\n", show_color ? RED : "",
               show_color ? RESET_COLOR : "", WTERMSIG(status));
        ret = SCR_TEST_CODE_ERROR;
        show_output = true;
    }
    else {
        ret = WEXITSTATUS(status);
        switch (ret) {
        case SCR_TEST_CODE_OK:
            printf("%sPASSED%s\n", show_color ? GREEN : "", show_color ? RESET_COLOR : "");
            break;

        case SCR_TEST_CODE_SKIP:
            printf("%sSKIPPED%s\n", show_color ? YELLOW : "", show_color ? RESET_COLOR : "");
            break;

        case SCR_TEST_CODE_FAIL:
            printf("%sFAILED%s\n", show_color ? RED : "", show_color ? RESET_COLOR : "");
            show_output = true;
            break;

        default:
            ret = SCR_TEST_CODE_ERROR;
            /* FALLTHROUGH */
        case SCR_TEST_CODE_ERROR:
            printf("%sERROR%s\n", show_color ? RED : "", show_color ? RESET_COLOR : "");
            show_output = true;
            break;
        }
    }

    if (show_output) {
        fflush(stdout);
        dumpFd(log_fd, show_color);
        printf("\n");

        printf("-------- stdout --------\n");
        fflush(stdout);
        if (dumpFd(stdout_fd, show_color)) {
            printf("\n");
        }
        printf("------------------------\n\n");

        printf("-------- stderr --------\n");
        fflush(stdout);
        if (dumpFd(stderr_fd, show_color)) {
            printf("\n");
        }
        printf("------------------------\n\n");
    }

    return ret;
}

#undef GREEN
#undef YELLOW
#undef RED
#undef RESET_COLOR

static scrTestCode
testRun(const scrTestParam *param, void *group_ctx)
{
    int stdout_fd, stderr_fd = -1, log_fd = -1;
    scrTestCode ret = SCR_TEST_CODE_ERROR;
    pid_t child;
    char stdout_template[] = "/tmp/scrutiny_out_XXXXXX", stderr_template[] = "/tmp/scrutiny_err_XXXXXX",
         log_template[] = "/tmp/scrutiny_log_XXXXXX";

    stdout_fd = mkstemp(stdout_template);
    if (stdout_fd < 0) {
        perror("mkstemp");
        return SCR_TEST_CODE_ERROR;
    }
    unlink(stdout_template);
    stderr_fd = mkstemp(stderr_template);
    if (stderr_fd < 0) {
        perror("mkstemp");
        goto done;
    }
    unlink(stderr_template);

    log_fd = mkstemp(log_template);
    if (log_fd < 0) {
        perror("mkstemp");
        goto done;
    }
    unlink(log_template);

    child = fork();
    switch (child) {
    case -1: perror("fork"); goto done;

    case 0: testDo(stdout_fd, stderr_fd, log_fd, param, group_ctx);

    default: break;
    }

    ret = testSummarize(param->name, stdout_fd, stderr_fd, log_fd, child);

done:
    close(stdout_fd);
    close(stderr_fd);
    close(log_fd);
    return ret;
}

static void
groupRun(scrRunner *runner, const scrGroup *group, void *global_ctx)
{
    pid_t child;
    int fds[2];
    uint32_t stats[4];

    if (pipe(fds) != 0) {
        perror("pipe");
        exit(1);
    }

    child = fork();
    if (child < 0) {
        perror("fork");
        exit(1);
    }

    if (child == 0) {
        void *group_ctx;
        sigset_t set;
        scrTestParam *param;

        close(fds[0]);

        sigfillset(&set);
        sigprocmask(SIG_SETMASK, &set, NULL);

        if (group->create_fn) {
            group_ctx = group->create_fn(global_ctx);
        }
        else {
            group_ctx = NULL;
        }

        memset(stats, 0, sizeof(stats));

        GEAR_FOR_EACH(&group->params, param)
        {
            switch (testRun(param, group_ctx)) {
            case SCR_TEST_CODE_OK: stats[0]++; break;

            case SCR_TEST_CODE_SKIP: stats[1]++; break;

            case SCR_TEST_CODE_FAIL: stats[2]++; break;

            default: stats[3]++; break;
            }
        }

        if (group->cleanup_fn) {
            group->cleanup_fn(group_ctx);
        }

        if (write(fds[1], stats, sizeof(stats)) != (ssize_t)sizeof(stats)) {
            exit(1);
        }

        exit(0);
    }
    else {
        int status;

        close(fds[1]);

        while (waitpid(child, &status, 0) < 0) {}

        if (WIFSIGNALED(status)) {
            fprintf(stderr, "Group runner was terminated by a signal: %i\n", WTERMSIG(status));
            runner->stats.num_errored = group->params.length;
        }
        else if (WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Group runner exited with an error\n");
            runner->stats.num_errored = group->params.length;
        }
        else {
            ssize_t transmitted;

            transmitted = read(fds[0], &stats, sizeof(stats));
            if (transmitted < (ssize_t)sizeof(stats)) {
                if (transmitted < 0) {
                    perror("read");
                }
                else {
                    fprintf(stderr, "Failed to communicate with group runner\n");
                }
                runner->stats.num_errored = group->params.length;
            }
            else {
                runner->stats.num_passed = stats[0];
                runner->stats.num_skipped = stats[1];
                runner->stats.num_failed = stats[2];
                runner->stats.num_errored = stats[3];
            }
        }

        close(fds[0]);
    }
}

void
scrInit(void)
{
    int kill_signals[] = {SIGHUP, SIGQUIT, SIGTERM, SIGINT};
    struct sigaction action = {.sa_handler = signalHandler};

    sigfillset(&action.sa_mask);
    for (unsigned int k = 0; k < ARRAY_LENGTH(kill_signals); k++) {
        sigaction(kill_signals[k], &action, NULL);
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
    gearInit(&runner->groups, sizeof(scrGroup));

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
        gearReset(&group->params);
    }

    gearReset(&runner->groups);
    free(runner);
}

scrGroup *
scrGroupCreate(scrRunner *runner, scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn)
{
    scrGroup group = {.create_fn = create_fn, .cleanup_fn = cleanup_fn};

    gearInit(&group.params, sizeof(scrTestParam));
    gearSetExpansion(&group.params, 5, 10);
    if (gearAppend(&runner->groups, &group) != GEAR_RET_OK) {
        exit(1);
    }

    return GEAR_GET_ITEM(&runner->groups, runner->groups.length - 1);
}

void
scrGroupAddTest(scrGroup *group, char *name, scrTestFn test_fn, unsigned int timeout)
{
    scrTestParam param = {.test_fn = test_fn, .timeout = timeout};

    param.name = strdup(name);
    if (!param.name) {
        exit(1);
    }

    if (gearAppend(&group->params, &param) != GEAR_RET_OK) {
        exit(1);
    }
}

void
scrRunnerRun(scrRunner *runner, void *global_ctx, scrStats *stats)
{
    scrGroup *group;

    runner->stats = (scrStats){0};

    GEAR_FOR_EACH(&runner->groups, group)
    {
        groupRun(runner, group, global_ctx);
    }

    printf("\n\nTests run: %u\n", runner->stats.num_passed + runner->stats.num_failed +
                                      runner->stats.num_errored + runner->stats.num_skipped);
    printf("Passed: %u\n", runner->stats.num_passed);
    printf("Skipped: %u\n", runner->stats.num_skipped);
    printf("Failed: %u\n", runner->stats.num_failed);
    printf("Errored: %u\n", runner->stats.num_errored);

    if (stats) {
        memcpy(stats, &runner->stats, sizeof(*stats));
    }
}
