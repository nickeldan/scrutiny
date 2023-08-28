#include <ctype.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include "internal.h"

bool show_color;

static void
replaceNonPrintable(char *buffer, size_t size)
{
    for (size_t k = 0; k < size; k++) {
        char c = buffer[k];

        if (!isprint(c) && c != '\n' && c != '\t' && c != '\r') {
            buffer[k] = '.';
        }
    }
}

static void
killAndExit(pid_t child)
{
    kill(child, SIGKILL);
    while (waitpid(child, NULL, 0) < 0) {}
    _exit(1);
}

pid_t
cleanFork(void)
{
    fflush(stdout);
    fflush(stderr);
    return fork();
}

void
dumpFd(int fd, bool printable_only)
{
    ssize_t transmitted;
    char buffer[1024];

    fflush(stdout);
    lseek(fd, 0, SEEK_SET);

    while ((transmitted = read(fd, buffer, sizeof(buffer))) > 0) {
        if (printable_only) {
            replaceNonPrintable(buffer, transmitted);
        }
        if (write(STDOUT_FILENO, buffer, transmitted) < 0) {}
    }
}

void
showTestResult(const scrTest *test, scrTestCode result)
{
    bool xfail = (test->options.flags & SCR_TF_XFAIL);

    printf("Test result (%s): ", test->name);
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
#include <sys/timerfd.h>

void
waitForProcess(pid_t child, unsigned int timeout, int *status, bool *timed_out)
{
    unsigned int num_pollers;
    struct pollfd pollers[3] = {{.events = POLLIN}, {.events = POLLIN}};
    sigset_t set;

    *timed_out = false;

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

    if (timeout > 0) {
        struct itimerspec timer = {.it_value = {.tv_sec = timeout}};

        num_pollers = 3;
        pollers[2].events = POLLIN;
        pollers[2].fd = timerfd_create(CLOCK_MONOTONIC, 0);
        if (pollers[2].fd < 0) {
            perror("timerfd_create");
            goto error;
        }

        if (timerfd_settime(pollers[2].fd, 0, &timer, NULL) != 0) {
            perror("timerfd_settime");
            goto error;
        }
    }
    else {
        num_pollers = 2;
    }

    while (poll(pollers, num_pollers, -1) < 0) {
        int local_errno = errno;

        if (local_errno != EINTR) {
            fprintf(stderr, "poll: %s\n", strerror(local_errno));
            goto error;
        }
    }

    for (unsigned int k = 0; k < num_pollers; k++) {
        close(pollers[k].fd);
    }

    if (pollers[1].revents & POLLIN) {
        goto error;
    }
    if (num_pollers == 3 && pollers[2].revents & POLLIN) {
        *timed_out = true;
        kill(child, SIGKILL);
    }

    while (waitpid(child, status, 0) < 0) {}
    return;

error:
    killAndExit(child);
}

#else  // SYS_pidfd_open

#include <time.h>

#ifdef CLOCK_MONOTONIC_COARSE
#define SCR_CLOCK_TYPE CLOCK_MONOTONIC_COARSE
#else
#define SCR_CLOCK_TYPE CLOCK_MONOTONIC
#endif

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
