#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#include "internal.h"

#ifndef ARRAY_LENGTH
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

static const int kill_signals[] = {SIGHUP, SIGQUIT, SIGTERM, SIGINT};

static void
killAndExit(pid_t child)
{
    kill(child, SIGKILL);
    while (waitpid(child, NULL, 0) < 0) {}
    exit(1);
}

const int *
getKillSignals(unsigned int *count)
{
    *count = ARRAY_LENGTH(kill_signals);
    return kill_signals;
}

pid_t
cleanFork(void)
{
    fflush(stdout);
    return fork();
}

void
dumpFd(int fd, bool show_color)
{
    ssize_t transmitted;
    char buffer[1024];

    fflush(stdout);
    lseek(fd, 0, SEEK_SET);

    while ((transmitted = read(fd, buffer, sizeof(buffer))) > 0) {
        if (write(STDOUT_FILENO, buffer, transmitted) < 0) {}
    }

    if (show_color && write(STDOUT_FILENO, RESET_COLOR, sizeof(RESET_COLOR) - 1) < 0) {}
}

void
showTestResult(const scrTestParam *param, scrTestCode result, bool show_color)
{
    bool xfail = (param->flags & SCR_TEST_FLAG_XFAIL);

    printf("Test result (%s): ", param->name);
    switch (result) {
    case SCR_TEST_CODE_OK:
        printf("%s%s%s\n", show_color ? GREEN : "", xfail ? "XFAILED" : "PASSED",
               show_color ? RESET_COLOR : "");
        break;

    case SCR_TEST_CODE_SKIP:
        printf("%sSKIPPED%s\n", show_color ? YELLOW : "", show_color ? RESET_COLOR : "");
        break;

    case SCR_TEST_CODE_FAIL:
        printf("%s%s%s\n", show_color ? RED : "", xfail ? "XPASSED" : "FAILED",
               show_color ? RESET_COLOR : "");
        break;

    default: printf("%sERROR%s\n", show_color ? RED : "", show_color ? RESET_COLOR : ""); break;
    }
}

#if defined(__linux__)

#include <errno.h>
#include <poll.h>
#include <string.h>
#include <sys/signalfd.h>
#include <unistd.h>

static void
clearSignal(int signum)
{
    int dummy;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, signum);
    sigwait(&set, &dummy);
}

static void
clearSignals(void)
{
    const int signals[] = {SIGCHLD, SIGALRM};
    sigset_t set;

    sigpending(&set);
    for (unsigned int k = 0; k < ARRAY_LENGTH(signals); k++) {
        if (sigismember(&set, signals[k])) {
            clearSignal(signals[k]);
        }
    }
}

void
waitForProcess(pid_t child, unsigned int timeout, int *status, bool *timed_out)
{
    ssize_t num_read;
    sigset_t set;
    struct pollfd poller = {.events = POLLIN};
    struct signalfd_siginfo info[3];

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGCHLD);
    if (timeout > 0) {
        sigaddset(&set, SIGALRM);
    }
    poller.fd = signalfd(-1, &set, 0);
    if (poller.fd < 0) {
        perror("signalfd");
        goto error;
    }

    if (timeout > 0) {
        alarm(timeout);
    }

    while (poll(&poller, 1, -1) < 0) {
        if (errno != EINTR) {
            perror("poll");
            goto error;
        }
    }

    num_read = read(poller.fd, info, sizeof(info));
    if (num_read < 0) {
        perror("read");
        goto error;
    }
    num_read /= sizeof(info[0]);

    for (int k = 0; k < num_read; k++) {
        if (info[k].ssi_signo == SIGTERM) {
            goto error;
        }
    }

    for (int k = 0; k < num_read; k++) {
        if (info[k].ssi_signo == SIGCHLD) {
            *timed_out = false;
            goto get_status;
        }
    }

    *timed_out = true;

get_status:
    while (waitpid(child, status, 0) != child) {}
    clearSignals();
    return;

error:
    killAndExit(child);
}

#else  // no pidfd_open

#include <time.h>

static bool
caughtSignal(void)
{
    sigset_t set;

    sigpending(&set);
    return sigismember(&set, SIGTERM);
}

void
waitForProcess(pid_t child, unsigned int timeout, int *status, bool *timed_out)
{
    struct timespec start_time, lapse = {.tv_nsec = 10000000};

    *timed_out = false;
    if (timeout > 0) {
        clock_gettime(CLOCK_MOOTONIC_COARSE, &start_time);
    }

    while (1) {
        if (caughtSignal()) {
            killAndExit(child);
        }

        if (waitpid(child, status, WNOHANG) == child) {
            return;
        }

        nanosleep(&spec, NULL);
        if (timeout > 0 && !*timed_out) {
            struct timespec now;

            clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
            if (now.tv_sec - start_time.tv_sec >= timeout) {
                *timed_out = true;
                kill(child, SIGKILL);
            }
        }
    }
}

#endif
