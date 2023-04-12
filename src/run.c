#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "internal.h"

static gear groups;
static const int kill_signals[] = {SIGHUP, SIGQUIT, SIGTERM, SIGINT};

static void
signalHandler(int signum)
{
    (void)signum;
    kill(0, SIGTERM);
    while (wait(NULL) > 0) {}
    exit(1);
}

static void
removeSignalHandler(void)
{
    struct sigaction action = {.sa_handler = SIG_DFL};

    for (unsigned int k = 0; k < ARRAY_LENGTH(kill_signals); k++) {
        sigaction(kill_signals[k], &action, NULL);
    }
}

static bool
receiveStats(int pipe_fd, const scrGroup *group, scrStats *stats, bool show_color)
{
    bool were_failures;
    ssize_t transmitted;
    scrStats stats_obj;
    scrTestParam *param;

    transmitted = read(pipe_fd, &stats_obj, sizeof(stats_obj));
    if (transmitted < (ssize_t)sizeof(stats_obj)) {
        if (transmitted < 0) {
            perror("read");
        }
        else {
            printf("Failed to communicate with group runner\n");
        }
        stats->num_errored += group->params.length;
        GEAR_FOR_EACH(&group->params, param)
        {
            showTestResult(param, SCR_TEST_CODE_ERROR, show_color);
        }

        were_failures = true;
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

    if (group->params.length == 0) {
        return true;
    }

    if (pipe(fds) != 0 || pipe(error_fds) != 0) {
        perror("pipe");
        exit(1);
    }

    child = cleanFork();
    switch (child) {
    case -1: perror("fork"); exit(1);
    case 0:
        close(fds[0]);
        close(error_fds[0]);
        removeSignalHandler();
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

static void
freeResources(void)
{
    scrGroup *group;

    GEAR_FOR_EACH(&groups, group)
    {
        groupFree(group);
    }
    gearReset(&groups);
}

scrGroup *
scrGroupCreate(scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn)
{
    scrGroup group = {.create_fn = create_fn, .cleanup_fn = cleanup_fn};

    if (groups.item_size == 0) {
        struct sigaction action = {.sa_handler = signalHandler};

        gearInit(&groups, sizeof(scrGroup));
        atexit(freeResources);

        sigfillset(&action.sa_mask);
        for (unsigned int k = 0; k < ARRAY_LENGTH(kill_signals); k++) {
            sigaction(kill_signals[k], &action, NULL);
        }

        if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
            perror("dup2");
            exit(1);
        }
    }

    gearInit(&group.params, sizeof(scrTestParam));
    gearSetExpansion(&group.params, 5, 10);
    if (gearAppend(&groups, &group) != GEAR_RET_OK) {
        exit(1);
    }

    return GEAR_GET_ITEM(&groups, groups.length - 1);
}

int
scrRun(const scrOptions *options, scrStats *stats)
{
    bool show_color;
    scrStats stats_obj;
    const scrOptions options_obj = {0};
    scrGroup *group;

    if (!stats) {
        stats = &stats_obj;
    }
    memset(stats, 0, sizeof(*stats));

    printf("Scrutiny version %s\n\n", SCRUTINY_VERSION);

    if (groups.item_size == 0) {
        goto show_summary;
    }

    if (!options) {
        options = &options_obj;
    }

    show_color = isatty(STDOUT_FILENO);

    GEAR_FOR_EACH(&groups, group)
    {
        if (!groupRun(group, options, stats, show_color)) {
            break;
        }
    }

show_summary:
    printf("\n\nTests run: %u\n",
           stats->num_passed + stats->num_failed + stats->num_errored + stats->num_skipped);
    printf("Passed: %u\n", stats->num_passed);
    printf("Skipped: %u\n", stats->num_skipped);
    printf("Failed: %u\n", stats->num_failed);
    printf("Errored: %u\n", stats->num_errored);

    return (stats->num_failed > 0 || stats->num_errored > 0);
}
