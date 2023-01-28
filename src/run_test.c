#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "internal.h"

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
hasData(int fd)
{
    return lseek(fd, 0, SEEK_END) > 0;
}

static scrTestCode
testSummarize(const scrTestParam *param, int stdout_fd, int stderr_fd, int log_fd, pid_t child,
              bool show_color)
{
    scrTestCode ret;
    int status;
    bool show_output = false;

    waitForProcess(child, &status);

    if (WIFSIGNALED(status)) {
        int signum = WTERMSIG(status);

        printf("Test result (%s): %s%s%s: ", param->name, show_color ? RED : "",
               (signum == SIGALRM) ? "FAIL" : "ERROR", show_color ? RESET_COLOR : "");
        if (signum == SIGALRM) {
            printf("Timed out\n");
            ret = SCR_TEST_CODE_FAIL;
        }
        else {
            printf("Terminated by signal (%i): %s\n", signum, strsignal(signum));
            ret = SCR_TEST_CODE_ERROR;
        }
        show_output = true;
    }
    else {
        ret = WEXITSTATUS(status);
        if (param->flags & SCR_TEST_FLAG_XFAIL) {
            if (ret == SCR_TEST_CODE_OK) {
                ret = SCR_TEST_CODE_FAIL;
            }
            else if (ret == SCR_TEST_CODE_FAIL) {
                ret = SCR_TEST_CODE_OK;
            }
        }
        if (ret != SCR_TEST_CODE_OK && ret != SCR_TEST_CODE_SKIP) {
            show_output = true;
        }
        showTestResult(param, ret, show_color);
    }

    if (show_output) {
        bool some_output = false;

        if (hasData(log_fd)) {
            dumpFd(log_fd, show_color);
            some_output = true;
        }

        if (hasData(stdout_fd)) {
            printf("\n-------- stdout --------\n");
            dumpFd(stdout_fd, show_color);
            printf("\n------------------------\n");
            some_output = true;
        }

        if (hasData(stderr_fd)) {
            printf("\n-------- stderr --------\n");
            dumpFd(stderr_fd, show_color);
            printf("\n------------------------\n");
            some_output = true;
        }

        if (some_output) {
            printf("\n");
        }
    }

    return ret;
}

#if defined(ANDROID) || defined(__ANDROID__)
#define TMP_PREFIX "/data/local"
#else
#define TMP_PREFIX
#endif
#define TEMPLATE(fmt) TMP_PREFIX "/tmp/scrutiny_" #fmt "_XXXXXX"

scrTestCode
testRun(const scrTestParam *param, bool show_color)
{
    int stdout_fd, stderr_fd = -1, log_fd = -1;
    scrTestCode ret = SCR_TEST_CODE_ERROR;
    pid_t child;
    char stdout_template[] = TEMPLATE(out), stderr_template[] = TEMPLATE(err), log_template[] = TEMPLATE(log);
#undef TMP_PREFIX
#undef TEMPLATE

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

    child = cleanFork();
    switch (child) {
    case -1: perror("fork"); goto done;

    case 0: exit(testDo(stdout_fd, stderr_fd, log_fd, param));
    default: break;
    }

    ret = testSummarize(param, stdout_fd, stderr_fd, log_fd, child, show_color);

done:
    close(stdout_fd);
    close(stderr_fd);
    close(log_fd);
    return ret;
}