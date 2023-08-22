#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "internal.h"
#include "monkeypatch.h"

static gear groups;
static const int kill_signals[] = {SIGHUP, SIGQUIT, SIGTERM, SIGINT};

static void
signalHandler(int signum)
{
    (void)signum;
    kill(0, SIGTERM);
    while (wait(NULL) > 0) {}
    _exit(1);
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
receiveStats(int pipe_fd, const scrGroup group, scrStats *stats)
{
    bool were_failures;
    ssize_t transmitted;
    scrStats stats_obj;
    scrTest *test;

    transmitted = read(pipe_fd, &stats_obj, sizeof(stats_obj));
    if (transmitted < (ssize_t)sizeof(stats_obj)) {
        if (transmitted < 0) {
            perror("read");
        }
        else {
            fprintf(stderr, "Failed to communicate with group runner\n");
        }
        stats->num_errored += group->tests.length;
        GEAR_FOR_EACH(&group->tests, test)
        {
            showTestResult(test, SCR_TEST_CODE_ERROR);
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
groupRun(const scrGroup group, const scrOptions *options, scrStats *stats)
{
    bool were_failures;
    int status, exit_code;
    pid_t child;
    int fds[2], error_fds[2];
    scrTest *test;

    if (group->tests.length == 0) {
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
        _exit(groupDo(group, options, error_fds[1], fds[1]));
    default: break;
    }

    close(fds[1]);
    close(error_fds[1]);

    while (waitpid(child, &status, 0) < 0) {}

    exit_code = WEXITSTATUS(status);

    if (WIFSIGNALED(status)) {
        were_failures = true;
        fprintf(stderr, "Group runner was terminated by a signal: %i\n", WTERMSIG(status));
        stats->num_errored += group->tests.length;
        GEAR_FOR_EACH(&group->tests, test)
        {
            showTestResult(test, SCR_TEST_CODE_ERROR);
        }
    }
    else if (exit_code == SCR_TEST_CODE_SKIP) {
        were_failures = false;
        stats->num_skipped += group->tests.length;
        GEAR_FOR_EACH(&group->tests, test)
        {
            showTestResult(test, SCR_TEST_CODE_SKIP);
        }
    }
    else if (exit_code != SCR_TEST_CODE_OK) {
        were_failures = true;
        if (exit_code == SCR_TEST_CODE_FAIL) {
            stats->num_failed += group->tests.length;
        }
        else {
            fprintf(stderr, "Group runner exited with an error\n");
            stats->num_errored += group->tests.length;
        }
        GEAR_FOR_EACH(&group->tests, test)
        {
            showTestResult(test, exit_code);
        }
        dumpFd(error_fds[0], false);
    }
    else {
        were_failures = receiveStats(fds[0], group, stats);
    }

    close(fds[0]);
    close(error_fds[0]);

    return !(were_failures && (options->flags & SCR_RF_FAIL_FAST));
}

static void
freeResources(void)
{
    scrGroup group;

    GEAR_FOR_EACH(&groups, group)
    {
        groupFree(group);
    }
    gearReset(&groups);
}

scrGroup
scrGroupCreate(scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn)
{
    struct scrGroupStruct group = {.create_fn = create_fn, .cleanup_fn = cleanup_fn};

    if (groups.item_size == 0) {
        struct sigaction action = {.sa_handler = signalHandler};

        gearInit(&groups, sizeof(group));
        atexit(freeResources);

        sigfillset(&action.sa_mask);
        for (unsigned int k = 0; k < ARRAY_LENGTH(kill_signals); k++) {
            sigaction(kill_signals[k], &action, NULL);
        }
    }

    gearInit(&group.tests, sizeof(scrTest));
    gearSetExpansion(&group.tests, 5, 10);

#ifdef SCR_MONKEYPATCH
    gearInit(&group.patch_goals, sizeof(scrPatchGoal));
#endif

    if (gearAppend(&groups, &group) != GEAR_RET_OK) {
        exit(1);
    }

    return GEAR_GET_ITEM(&groups, groups.length - 1);
}

int
scrRun(const scrOptions *options, scrStats *stats)
{
    scrStats stats_obj;
    const scrOptions options_obj = {0};
    scrGroup group;

    if (!stats) {
        stats = &stats_obj;
    }
    memset(stats, 0, sizeof(*stats));

    printf("Scrutiny %s\n\n", SCRUTINY_VERSION);

    if (groups.item_size == 0) {
        goto show_summary;
    }

    if (!options) {
        options = &options_obj;
    }

    show_color = isatty(STDOUT_FILENO);

    GEAR_FOR_EACH(&groups, group)
    {
        if (!groupRun(group, options, stats)) {
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
