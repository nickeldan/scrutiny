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

struct testParams {
    int stdout_fd;
    int stderr_fd;
    int log_fd;
#ifdef SCR_MONKEYPATCH
    unsigned int have_patches : 1;
#endif
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
testDo(const struct testParams *params, const scrTest *test)
{
    int stdin_fd, local_errno;
    bool check;
    sigset_t set;

    setLogFd(params->log_fd);

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

    check = (dup2(params->stdout_fd, STDOUT_FILENO) >= 0 && dup2(params->stderr_fd, STDERR_FILENO) >= 0);
    local_errno = errno;
    close(params->stdout_fd);
    close(params->stderr_fd);
    if (!check) {
        fprintf(stderr, "dup2: %s\n", strerror(local_errno));
        return SCR_TEST_CODE_ERROR;
    }

    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);

#ifdef SCR_MONKEYPATCH
    if (params->have_patches) {
        if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
            perror("ptrace (TRACEME)");
            return SCR_TEST_CODE_ERROR;
        }
        raise(SIGSTOP);
    }
#endif

    test->test_fn();
    return SCR_TEST_CODE_OK;

error:
    close(params->stdout_fd);
    close(params->stderr_fd);
    return SCR_TEST_CODE_ERROR;
}

static bool
hasData(int fd)
{
    return lseek(fd, 0, SEEK_END) > 0;
}

static void
showTestOutput(const struct testParams *fds)
{
    bool some_output = false;

    if (hasData(fds->log_fd)) {
        dumpFd(fds->log_fd, false);
        some_output = true;
    }

    if (hasData(fds->stdout_fd)) {
        printf("\n-------- stdout --------\n");
        dumpFd(fds->stdout_fd, true);
        printf("\n------------------------\n");
        some_output = true;
    }

    if (hasData(fds->stderr_fd)) {
        printf("\n-------- stderr --------\n");
        dumpFd(fds->stderr_fd, true);
        printf("\n------------------------\n");
        some_output = true;
    }

    if (some_output) {
        printf("\n");
    }
}

static scrTestCode
summarizeTest(const scrTest *test, const struct testParams *fds, pid_t child, const int *status_ptr,
              bool verbose)
{
    scrTestCode ret;
    int status;
    bool timed_out, show_output = true;

    if (status_ptr) {
        status = *status_ptr;
    }
    else {
        waitForProcess(child, test->options.timeout, &status, &timed_out);
    }

    if (timed_out) {
        printf("Test result (%s): %sFAIL%s: Timed out\n", test->name, show_color ? RED : "",
               show_color ? RESET_COLOR : "");
        ret = SCR_TEST_CODE_FAIL;
    }
    else if (WIFSIGNALED(status)) {
        int signum = WTERMSIG(status);

        printf("Test result (%s): %sERROR%s: Terminated by signal (%i): %s\n", test->name,
               show_color ? RED : "", show_color ? RESET_COLOR : "", signum, strsignal(signum));
        ret = SCR_TEST_CODE_ERROR;
    }
    else {
        ret = WEXITSTATUS(status);
        if (test->options.flags & SCR_TF_XFAIL) {
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
        showTestResult(test, ret);
    }

    if (show_output || verbose) {
        showTestOutput(fds);
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
testRun(const scrTest *test, bool verbose)
{
#ifdef SCR_MONKEYPATCH
    int status;
#endif
    int *status_ptr = NULL;
    scrTestCode ret = SCR_TEST_CODE_ERROR;
    pid_t child;
    struct testParams params = {.log_fd = -1};
    char stdout_template[] = TEMPLATE(out), stderr_template[] = TEMPLATE(err), log_template[] = TEMPLATE(log);
#undef TMP_PREFIX
#undef TEMPLATE

    params.stdout_fd = makeTempFile(stdout_template);
    if (params.stdout_fd < 0) {
        return SCR_TEST_CODE_ERROR;
    }
    params.stderr_fd = makeTempFile(stderr_template);
    if (params.stderr_fd < 0) {
        goto done;
    }
    params.log_fd = makeTempFile(log_template);
    if (params.log_fd < 0) {
        goto done;
    }

#ifdef SCR_MONKEYPATCH
    params.have_patches = (test->patch_goals->length > 0);
#endif

    child = cleanFork();
    switch (child) {
    case -1: perror("fork"); goto done;
    case 0: _exit(testDo(&params, test)); break;
    default: break;
    }

#ifdef SCR_MONKEYPATCH
    if (params.have_patches && !applyPatches(child, test->patch_goals, &status)) {
        status_ptr = &status;
    }
#endif

    ret = summarizeTest(test, &params, child, status_ptr, verbose);

done:
    close(params.stdout_fd);
    close(params.stderr_fd);
    close(params.log_fd);
    return ret;
}
