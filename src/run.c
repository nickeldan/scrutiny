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

static int
testDo(int stdout_fd, int stderr_fd, int log_fd, const scrTestParam *param)
{
    int stdin_fd, local_errno;
    bool check;
    sigset_t set;

    setLogFd(log_fd);

    stdin_fd = open("/dev/null", O_RDONLY);
    if (stdin_fd < 0) {
        perror("open");
        goto error;
    }
    check = (dup2(stdin_fd, STDIN_FILENO) >= 0);
    local_errno = errno;
    close(stdin_fd);
    if (!check) {
        fprintf(stderr, "dup2: %s\n", strerror(local_errno));
        goto error;
    }

    check = (dup2(stdout_fd, STDOUT_FILENO) >= 0 && dup2(stderr_fd, STDERR_FILENO) >= 0);
    local_errno = errno;
    close(stdout_fd);
    close(stderr_fd);
    if (!check) {
        fprintf(stderr, "dup2: %s\n", strerror(local_errno));
        return SCR_TEST_CODE_ERROR;
    }

    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);
    if (param->timeout > 0) {
        alarm(param->timeout);
    }

    param->test_fn();
    return SCR_TEST_CODE_OK;

error:
    close(stdout_fd);
    close(stderr_fd);
    return SCR_TEST_CODE_ERROR;
}

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

static void
showTestResult(const char *test_name, scrTestCode result, bool show_color)
{
    printf("Test result: (%s): ", test_name);
    switch (result) {
    case SCR_TEST_CODE_OK:
        printf("%sPASSED%s\n", show_color ? GREEN : "", show_color ? RESET_COLOR : "");
        break;

    case SCR_TEST_CODE_SKIP:
        printf("%sSKIPPED%s\n", show_color ? YELLOW : "", show_color ? RESET_COLOR : "");
        break;

    case SCR_TEST_CODE_FAIL:
        printf("%sFAILED%s\n", show_color ? RED : "", show_color ? RESET_COLOR : "");
        break;

    default: printf("%sERROR%s\n", show_color ? RED : "", show_color ? RESET_COLOR : ""); break;
    }
}

static scrTestCode
testSummarize(const char *test_name, int stdout_fd, int stderr_fd, int log_fd, pid_t child, bool show_color)
{
    scrTestCode ret;
    int status;
    bool show_output = false;
    struct timespec spec = {.tv_nsec = 10000000};  // 1/100 of a second

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

    if (WIFSIGNALED(status)) {
        printf("Test result (%s): %sERROR%s: Terminated by signal (%i)\n", test_name, show_color ? RED : "",
               show_color ? RESET_COLOR : "", WTERMSIG(status));
        ret = SCR_TEST_CODE_ERROR;
        show_output = true;
    }
    else {
        ret = WEXITSTATUS(status);
        if (ret != SCR_TEST_CODE_OK && ret != SCR_TEST_CODE_SKIP) {
            show_output = true;
        }
        showTestResult(test_name, ret, show_color);
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

static scrTestCode
testRun(const scrTestParam *param, bool show_color)
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

    case 0: exit(testDo(stdout_fd, stderr_fd, log_fd, param));
    default: break;
    }

    ret = testSummarize(param->name, stdout_fd, stderr_fd, log_fd, child, show_color);

done:
    close(stdout_fd);
    close(stderr_fd);
    close(log_fd);
    return ret;
}

static void
groupRun(const scrGroup *group, void *global_ctx, scrStats *stats, bool show_color)
{
    pid_t child;
    int fds[2];
    scrStats stats_obj = {0};
    scrTestParam *param;

    if (pipe(fds) != 0) {
        perror("pipe");
        exit(SCR_TEST_CODE_ERROR);
    }

    child = fork();
    if (child < 0) {
        perror("fork");
        exit(SCR_TEST_CODE_ERROR);
    }

    if (child == 0) {
        void *group_ctx;
        sigset_t set;

        close(fds[0]);

        sigfillset(&set);
        sigprocmask(SIG_SETMASK, &set, NULL);

        setToTty(show_color);

        if (group->create_fn) {
            group_ctx = group->create_fn(global_ctx);
        }
        else {
            group_ctx = global_ctx;
        }

        setGroupCtx(group_ctx);

        GEAR_FOR_EACH(&group->params, param)
        {
            switch (testRun(param, show_color)) {
            case SCR_TEST_CODE_OK: stats_obj.num_passed++; break;

            case SCR_TEST_CODE_SKIP: stats_obj.num_skipped++; break;

            case SCR_TEST_CODE_FAIL: stats_obj.num_failed++; break;

            default: stats_obj.num_errored++; break;
            }
        }

        if (group->cleanup_fn) {
            group->cleanup_fn(group_ctx);
        }

        if (write(fds[1], &stats_obj, sizeof(stats_obj)) != (ssize_t)sizeof(stats_obj)) {
            exit(SCR_TEST_CODE_ERROR);
        }

        exit(SCR_TEST_CODE_OK);
    }
    else {
        int status;

        close(fds[1]);

        while (waitpid(child, &status, 0) < 0) {}

        if (WIFSIGNALED(status)) {
            fprintf(stderr, "Group runner was terminated by a signal: %i\n", WTERMSIG(status));
            stats->num_errored += group->params.length;
            GEAR_FOR_EACH(&group->params, param)
            {
                showTestResult(param->name, SCR_TEST_CODE_ERROR, show_color);
            }
        }
        else if (WEXITSTATUS(status) == SCR_TEST_CODE_SKIP) {
            stats->num_skipped += group->params.length;
            GEAR_FOR_EACH(&group->params, param)
            {
                showTestResult(param->name, SCR_TEST_CODE_SKIP, show_color);
            }
        }
        else if (WEXITSTATUS(status) != SCR_TEST_CODE_OK) {
            fprintf(stderr, "Group runner exited with an error\n");
            stats->num_errored += group->params.length;
            GEAR_FOR_EACH(&group->params, param)
            {
                showTestResult(param->name, SCR_TEST_CODE_ERROR, show_color);
            }
        }
        else {
            ssize_t transmitted;

            transmitted = read(fds[0], &stats_obj, sizeof(stats_obj));
            if (transmitted < (ssize_t)sizeof(stats_obj)) {
                if (transmitted < 0) {
                    perror("read");
                }
                else {
                    fprintf(stderr, "Failed to communicate with group runner\n");
                }
                stats->num_errored = group->params.length;
                GEAR_FOR_EACH(&group->params, param)
                {
                    showTestResult(param->name, SCR_TEST_CODE_ERROR, show_color);
                }
            }
            else {
                stats->num_passed += stats_obj.num_passed;
                stats->num_skipped += stats_obj.num_skipped;
                stats->num_failed += stats_obj.num_failed;
                stats->num_errored += stats_obj.num_errored;
            }
        }

        close(fds[0]);
    }
}

scrRunner *
scrRunnerCreate(void)
{
    static bool initialized = false;
    scrRunner *runner;

    if (!initialized) {
        int kill_signals[] = {SIGHUP, SIGQUIT, SIGTERM, SIGINT};
        struct sigaction action = {.sa_handler = signalHandler};

        sigfillset(&action.sa_mask);
        for (unsigned int k = 0; k < ARRAY_LENGTH(kill_signals); k++) {
            sigaction(kill_signals[k], &action, NULL);
        }

        initialized = true;
    }

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
    bool show_color;
    scrGroup *group;
    scrStats stats_obj;

    if (!stats) {
        stats = &stats_obj;
    }
    memset(stats, 0, sizeof(*stats));

    show_color = isatty(STDOUT_FILENO);

    GEAR_FOR_EACH(&runner->groups, group)
    {
        groupRun(group, global_ctx, stats, show_color);
    }

    printf("\n\nTests run: %u\n",
           stats->num_passed + stats->num_failed + stats->num_errored + stats->num_skipped);
    printf("Passed: %u\n", stats->num_passed);
    printf("Skipped: %u\n", stats->num_skipped);
    printf("Failed: %u\n", stats->num_failed);
    printf("Errored: %u\n", stats->num_errored);
}
