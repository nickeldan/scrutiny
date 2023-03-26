#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "internal.h"

#ifdef CLOCK_MONOTONIC_COARSE
#define SCR_CLOCK_TYPE CLOCK_MONOTONIC_COARSE
#else
#define SCR_CLOCK_TYPE CLOCK_MONOTONIC
#endif

static void
killAndExit(pid_t child)
{
    kill(child, SIGKILL);
    while (waitpid(child, NULL, 0) < 0) {}
    exit(1);
}

pid_t
cleanFork(void)
{
    fflush(stdout);
    fflush(stderr);
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

#ifdef SYS_pidfd_open

#include <errno.h>
#include <poll.h>
#include <string.h>
#include <sys/signalfd.h>

void
waitForProcess(pid_t child, unsigned int timeout, int *status, bool *timed_out)
{
    struct pollfd pollers[2] = {{.events = POLLIN}, {.events = POLLIN}};
    sigset_t set;
    struct timespec start_time;

    *timed_out = false;

    if (timeout > 0) {
        clock_gettime(SCR_CLOCK_TYPE, &start_time);
    }

    pollers[0].fd = syscall(SYS_pidfd_open, child, 0);
    if (pollers[0].fd < 0) {
        perror("pidfd_open");
        goto error;
    }

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    pollers[1].fd = signalfd(-1, &set, 0);
    if (pollers[1].fd < 0) {
        perror("signalfd");
        goto error;
    }

    while (1) {
        int remaining = -1, res;

        if (timeout > 0 && !*timed_out) {
            time_t elapsed;
            struct timespec now;

            clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
            elapsed = now.tv_sec - start_time.tv_sec;
            if (elapsed >= timeout) {
                *timed_out = true;
                kill(child, SIGKILL);
            }
            else {
                remaining = timeout - elapsed;
            }
        }

        res = poll(pollers, 2, remaining * 1000);
        if (res > 0) {
            break;
        }
        else if (res < 0) {
            int local_errno = errno;

            if (local_errno != EINTR) {
                printf("poll: %s\n", strerror(local_errno));
                goto error;
            }
        }
    }

    for (int k = 0; k < 2; k++) {
        close(pollers[k].fd);
    }

    if (pollers[1].revents & POLLIN) {
        goto error;
    }

    while (waitpid(child, status, 0) < 0) {}
    return;

error:
    killAndExit(child);
}

#else  // SYS_pidfd_open

#define ONE_TENTH_SECOND 10000000

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
    struct timespec start_time, lapse = {.tv_nsec = ONE_TENTH_SECOND};

    *timed_out = false;
    if (timeout > 0) {
        clock_gettime(SCR_CLOCK_TYPE, &start_time);
    }

    while (1) {
        if (caughtSignal()) {
            killAndExit(child);
        }

        if (waitpid(child, status, WNOHANG) == child) {
            return;
        }

        nanosleep(&lapse, NULL);
        if (timeout > 0 && !*timed_out) {
            struct timespec now;

            clock_gettime(SCR_CLOCK_TYPE, &now);
            if (now.tv_sec - start_time.tv_sec >= timeout) {
                *timed_out = true;
                kill(child, SIGKILL);
            }
        }
    }
}

#endif  // SYS_pidfd_open
