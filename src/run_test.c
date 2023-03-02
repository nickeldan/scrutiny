#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
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
        perror("dup2");
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

static void
showTestOutput(int log_fd, int stdout_fd, int stderr_fd, bool show_color)
{
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

static scrTestCode
summarizeTest(const scrTestParam *param, int stdout_fd, int stderr_fd, int log_fd, pid_t child,
              bool show_color)
{
    scrTestCode ret;
    int status;
    bool timed_out, show_output = true;

    waitForProcess(child, param->timeout, &status, &timed_out);

    if (timed_out) {
        printf("Test result (%s): %sFAIL%s: Timed out\n", param->name, show_color ? RED : "",
               show_color ? RESET_COLOR : "");
        ret = SCR_TEST_CODE_FAIL;
    }
    else if (WIFSIGNALED(status)) {
        int signum = WTERMSIG(status);

        printf("Test result (%s): %sERROR%s: Terminated by signal (%i): %s\n", param->name,
               show_color ? RED : "", show_color ? RESET_COLOR : "", signum, strsignal(signum));
        ret = SCR_TEST_CODE_ERROR;
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
        if (ret == SCR_TEST_CODE_OK || ret == SCR_TEST_CODE_SKIP) {
            show_output = false;
        }
        showTestResult(param, ret, show_color);
    }

    if (show_output) {
        showTestOutput(log_fd, stdout_fd, stderr_fd, show_color);
    }

    return ret;
}

static int
makeTempFile(char *template)
{
    int fd;

    fd = mkstemp(template);
    if (fd < 0) {
        perror("mkstemp");
    }
    else {
        unlink(template);
    }
    return fd;
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
    int stdout_fd, stderr_fd, log_fd = -1;
    scrTestCode ret = SCR_TEST_CODE_ERROR;
    pid_t child;
    char stdout_template[] = TEMPLATE(out), stderr_template[] = TEMPLATE(err), log_template[] = TEMPLATE(log);
#undef TMP_PREFIX
#undef TEMPLATE

    stdout_fd = makeTempFile(stdout_template);
    if (stdout_fd < 0) {
        return SCR_TEST_CODE_ERROR;
    }
    stderr_fd = makeTempFile(stderr_template);
    if (stderr_fd < 0) {
        goto done;
    }
    log_fd = makeTempFile(log_template);
    if (log_fd < 0) {
        goto done;
    }

    child = cleanFork();
    switch (child) {
    case -1: perror("fork"); goto done;
    case 0: exit(testDo(stdout_fd, stderr_fd, log_fd, param));
    default: break;
    }

    ret = summarizeTest(param, stdout_fd, stderr_fd, log_fd, child, show_color);

done:
    close(stdout_fd);
    close(stderr_fd);
    close(log_fd);
    return ret;
}
