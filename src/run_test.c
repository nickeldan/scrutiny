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

struct testFds {
    int stdout_fd;
    int stderr_fd;
    int log_fd;
};

#ifdef SCR_MONKEYPATCH

#include <sys/ptrace.h>

static bool
applyPatches(pid_t child, const gear *patch_goals, int *status)
{
    scrPatchGoal *goal;

    while (waitpid(child, status, 0) < 0) {}

    if (!WIFSTOPPED(*status)) {
        return false;
    }

    GEAR_FOR_EACH(patch_goals, goal)
    {
        void **got_entry;

        GEAR_FOR_EACH(&goal->got_entries, got_entry)
        {
            if (ptrace(PTRACE_POKEDATA, child, *got_entry, goal->func_ptr) == -1) {
                perror("ptrace (POKEDATA)");
                kill(child, SIGKILL);
                ptrace(PTRACE_DETACH, child, NULL, NULL);
                while (waitpid(child, status, 0) < 0 && !(WIFEXITED(*status) || WIFSIGNALED(*status))) {}
                return false;
            }
        }
    }

    ptrace(PTRACE_DETACH, child, NULL, NULL);
    return true;
}

#endif  // SCR_MONKEYPATCH

static int
testDo(const struct testFds *fds, const scrTestParam *param)
{
    int stdin_fd, local_errno;
    bool check;
    sigset_t set;

    setLogFd(fds->log_fd);

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
        goto error;
    }

    check = (dup2(fds->stdout_fd, STDOUT_FILENO) >= 0 && dup2(fds->stderr_fd, STDERR_FILENO) >= 0);
    local_errno = errno;
    close(fds->stdout_fd);
    close(fds->stderr_fd);
    if (!check) {
        fprintf(stderr, "dup2: %s\n", strerror(local_errno));
        return SCR_TEST_CODE_ERROR;
    }

    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);

#ifdef SCR_MONKEYPATCH
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
        perror("ptrace (TRACEME)");
        return SCR_TEST_CODE_ERROR;
    }
    raise(SIGSTOP);
#endif

    param->test_fn();
    return SCR_TEST_CODE_OK;

error:
    close(fds->stdout_fd);
    close(fds->stderr_fd);
    return SCR_TEST_CODE_ERROR;
}

static bool
hasData(int fd)
{
    return lseek(fd, 0, SEEK_END) > 0;
}

static void
showTestOutput(const struct testFds *fds, bool show_color)
{
    bool some_output = false;

    if (hasData(fds->log_fd)) {
        dumpFd(fds->log_fd, show_color);
        some_output = true;
    }

    if (hasData(fds->stdout_fd)) {
        printf("\n-------- stdout --------\n");
        dumpFd(fds->stdout_fd, show_color);
        printf("\n------------------------\n");
        some_output = true;
    }

    if (hasData(fds->stderr_fd)) {
        printf("\n-------- stderr --------\n");
        dumpFd(fds->stderr_fd, show_color);
        printf("\n------------------------\n");
        some_output = true;
    }

    if (some_output) {
        printf("\n");
    }
}

static scrTestCode
summarizeTest(const scrTestParam *param, const struct testFds *fds, pid_t child, const int *status_ptr,
              bool verbose, bool show_color)
{
    scrTestCode ret;
    int status;
    bool timed_out, show_output = true;

    if (status_ptr) {
        status = *status_ptr;
    }
    else {
        waitForProcess(child, param->timeout, &status, &timed_out);
    }

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

    if (show_output || verbose) {
        showTestOutput(fds, show_color);
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
#ifdef SCR_MONKEYPATCH
testRun(const scrTestParam *param, bool verbose, bool show_color, const gear *patch_goals)
#else
testRun(const scrTestParam *param, bool verbose, bool show_color)
#endif
{
#ifdef SCR_MONKEYPATCH
    int status;
#endif
    int *status_ptr = NULL;
    scrTestCode ret = SCR_TEST_CODE_ERROR;
    pid_t child;
    struct testFds fds = {.log_fd = -1};
    char stdout_template[] = TEMPLATE(out), stderr_template[] = TEMPLATE(err), log_template[] = TEMPLATE(log);
#undef TMP_PREFIX
#undef TEMPLATE

    fds.stdout_fd = makeTempFile(stdout_template);
    if (fds.stdout_fd < 0) {
        return SCR_TEST_CODE_ERROR;
    }
    fds.stderr_fd = makeTempFile(stderr_template);
    if (fds.stderr_fd < 0) {
        goto done;
    }
    fds.log_fd = makeTempFile(log_template);
    if (fds.log_fd < 0) {
        goto done;
    }

    child = cleanFork();
    switch (child) {
    case -1: perror("fork"); goto done;
    case 0: exit(testDo(&fds, param));
    default: break;
    }

#ifdef SCR_MONKEYPATCH
    if (!applyPatches(child, patch_goals, &status)) {
        status_ptr = &status;
    }
#endif

    ret = summarizeTest(param, &fds, child, status_ptr, verbose, show_color);

done:
    close(fds.stdout_fd);
    close(fds.stderr_fd);
    close(fds.log_fd);
    return ret;
}
