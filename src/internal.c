#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <unistd.h>

#include "internal.h"

static const int kill_signals[] = {SIGHUP, SIGQUIT, SIGTERM, SIGINT};
#define NUM_SIGNALS (sizeof(kill_signals) / sizeof(int))

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
    *count = NUM_SIGNALS;
    return kill_signals;
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

#if defined(__linux__) && defined(SYS_pidfd_open)

#include <errno.h>
#include <poll.h>
#include <string.h>
#include <sys/signalfd.h>
#include <unistd.h>

void
waitForProcess(pid_t child, int *status)
{
    struct pollfd pollers[2] = {{.events = POLLIN}, {.events = POLLIN}};
    sigset_t set;

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

    while (poll(pollers, 2, -1) < 0) {
        int local_errno = errno;

        if (local_errno != EINTR) {
            fprintf(stderr, "poll: %s\n", strerror(local_errno));
            goto error;
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

#else  // no pidfd_open

#include <stdbool.h>
#include <time.h>

static bool
caughtSignal(void)
{
    sigset_t set;

    sigpending(&set);
    return sigismember(&set, SIGTERM);
}

void
waitForProcess(pid_t child, int *status)
{
    struct timespec spec = {.tv_nsec = 10000000};  // 1/100 of a second

    while (1) {
        if (caughtSignal()) {
            killAndExit(child);
        }

        if (waitpid(child, status, WNOHANG) == child) {
            return;
        }

        nanosleep(&spec, NULL);
    }
}

#endif
