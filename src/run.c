#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "internal.h"

struct scrRunner {
    gear groups;
};

static void
signalHandler(int signum)
{
    (void)signum;
    kill(-1, SIGTERM);
    while (waitpid(-1, NULL, 0) > 0) {}
    exit(1);
}

static bool
receiveStats(int pipe_fd, const scrGroup *group, scrStats *stats, bool show_color)
{
    bool were_failures = true;
    ssize_t transmitted;
    scrStats stats_obj;
    scrTestParam *param;

    transmitted = read(pipe_fd, &stats_obj, sizeof(stats_obj));
    if (transmitted < (ssize_t)sizeof(stats_obj)) {
        if (transmitted < 0) {
            printf("read: %s\n", strerror(errno));
        }
        else {
            printf("Failed to communicate with group runner\n");
        }
        stats->num_errored = group->params.length;
        GEAR_FOR_EACH(&group->params, param)
        {
            showTestResult(param, SCR_TEST_CODE_ERROR, show_color);
        }
    }
    else {
        stats->num_passed += stats_obj.num_passed;
        stats->num_skipped += stats_obj.num_skipped;
        stats->num_failed += stats_obj.num_failed;
        stats->num_errored += stats_obj.num_errored;

        were_failures = (stats_obj.num_failed > 0 || stats_obj.num_errored > 0);
    }

    return were_failures;
}

static bool
groupRun(const scrGroup *group, const scrOptions *options, scrStats *stats, bool show_color)
{
    bool were_failures;
    int status, exit_code;
    pid_t child;
    int fds[2], error_fds[2];
    scrTestParam *param;

    if (pipe(fds) != 0 || pipe(error_fds) != 0) {
        printf("pipe: %s\n", strerror(errno));
        exit(1);
    }

    child = cleanFork();
    switch (child) {
    case -1: printf("fork: %s\n", strerror(errno)); exit(1);
    case 0:
        close(fds[0]);
        close(error_fds[0]);
        exit(groupDo(group, options, show_color, error_fds[1], fds[1]));
    default: break;
    }

    close(fds[1]);
    close(error_fds[1]);

    while (waitpid(child, &status, 0) < 0) {}

    exit_code = WEXITSTATUS(status);

    if (WIFSIGNALED(status)) {
        were_failures = true;
        printf("Group runner was terminated by a signal: %i\n", WTERMSIG(status));
        stats->num_errored += group->params.length;
        GEAR_FOR_EACH(&group->params, param)
        {
            showTestResult(param, SCR_TEST_CODE_ERROR, show_color);
        }
    }
    else if (exit_code == SCR_TEST_CODE_SKIP) {
        were_failures = false;
        stats->num_skipped += group->params.length;
        GEAR_FOR_EACH(&group->params, param)
        {
            showTestResult(param, SCR_TEST_CODE_SKIP, show_color);
        }
    }
    else if (exit_code != SCR_TEST_CODE_OK) {
        were_failures = true;
        if (exit_code == SCR_TEST_CODE_FAIL) {
            stats->num_failed += group->params.length;
        }
        else {
            printf("Group runner exited with an error\n");
            stats->num_errored += group->params.length;
        }
        GEAR_FOR_EACH(&group->params, param)
        {
            showTestResult(param, exit_code, show_color);
        }
        dumpFd(error_fds[0], show_color);
    }
    else {
        were_failures = receiveStats(fds[0], group, stats, show_color);
    }

    close(fds[0]);
    close(error_fds[0]);

    return !(were_failures && (options->flags & SCR_RUN_FLAG_FAIL_FAST));
}

scrRunner *
scrRunnerCreate(void)
{
    static bool initialized = false;
    scrRunner *runner;

    if (!initialized) {
        unsigned int num_signals;
        const int *kill_signals;
        struct sigaction action = {.sa_handler = signalHandler};

        kill_signals = getKillSignals(&num_signals);

        sigfillset(&action.sa_mask);
        for (unsigned int k = 0; k < num_signals; k++) {
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

int
scrRunnerRun(scrRunner *runner, const scrOptions *options, scrStats *stats)
{
    bool show_color;
    scrGroup *group;
    const scrOptions options_obj = {0};
    scrStats stats_obj;

    if (!options) {
        options = &options_obj;
    }

    if (!stats) {
        stats = &stats_obj;
    }
    memset(stats, 0, sizeof(*stats));

    show_color = isatty(STDOUT_FILENO);

    printf("Scrutiny version " SCRUTINY_VERSION "\n\n");

    GEAR_FOR_EACH(&runner->groups, group)
    {
        if (!groupRun(group, options, stats, show_color)) {
            break;
        }
    }

    printf("\n\nTests run: %u\n",
           stats->num_passed + stats->num_failed + stats->num_errored + stats->num_skipped);
    printf("Passed: %u\n", stats->num_passed);
    printf("Skipped: %u\n", stats->num_skipped);
    printf("Failed: %u\n", stats->num_failed);
    printf("Errored: %u\n", stats->num_errored);

    return (stats->num_failed > 0 || stats->num_errored > 0);
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
