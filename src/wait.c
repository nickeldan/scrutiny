#include <signal.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/wait.h>

#include "internal.h"

static void
killAndExit(pid_t child)
{
    kill(child, SIGKILL);
    while (waitpid(child, NULL, 0) < 0) {}
    exit(1);
}

#if defined(__linux__) && defined(SYS_pidfd_open)

#include <errno.h>
#include <poll.h>
#include <stdio.h>
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
