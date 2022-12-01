#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <scrutiny/scrutiny.h>

#include "common.h"

static int global_num;

static void *
setup_func(void *global_ctx)
{
    int *fds;

    SCR_ASSERT_PTR_EQ(global_ctx, &global_num);

    fds = malloc(sizeof(int) * 2);
    SCR_ASSERT_PTR_NEQ(fds, NULL);
    SCR_ASSERT_EQ(pipe(fds), 0);
    SCR_ASSERT_EQ(write(fds[1], "abcd", 4), 4);

    return fds;
}

static void
cleanup_func(void *group_ctx)
{
    int *fds = group_ctx;

    SCR_ASSERT_EQ(close(fds[0]), 0);
    SCR_ASSERT_EQ(close(fds[1]), 0);
    free(fds);
}

static void *
setup_skip(void *global_ctx)
{
    (void)global_ctx;
    SCR_TEST_SKIP();
}

static void *
bad_setup(void *global_ctx)
{
    (void)global_ctx;
    SCR_ERROR("Failing in group setup.");
}

static SCR_TEST_FN(use_global_ctx)
{
    SCR_ASSERT_PTR_EQ(SCR_GROUP_CTX(), &global_num);
}

static SCR_TEST_FN(read_from_pipe1)
{
    int fd = *(int *)SCR_GROUP_CTX();
    unsigned char buffer[2];

    SCR_ASSERT_EQ(read(fd, buffer, sizeof(buffer)), 2);
    SCR_ASSERT_EQ(buffer[0], 'a');
    SCR_ASSERT_EQ(buffer[1], 'b');
}

static SCR_TEST_FN(read_from_pipe2)
{
    int fd = *(int *)SCR_GROUP_CTX();
    unsigned char buffer[2];

    SCR_ASSERT_EQ(read(fd, buffer, sizeof(buffer)), 2);
    SCR_ASSERT_EQ(buffer[0], 'c');
    SCR_ASSERT_EQ(buffer[1], 'd');
}

static SCR_TEST_FN(skip_me1)
{
}

static SCR_TEST_FN(skip_me2)
{
}

static SCR_TEST_FN(error_setup_fail1)
{
}

static SCR_TEST_FN(error_setup_fail2)
{
}

int
main(int argc, char **argv)
{
    unsigned int num_pass = 0, num_skip = 0, num_error = 0;
    scrRunner *runner;
    scrGroup *group;
    scrStats stats;
    (void)argc;

    printf("\nRunning %s\n\n", argv[0]);

    runner = scrRunnerCreate();

    group = scrGroupCreate(runner, NULL, NULL);
    ADD_PASS(use_global_ctx);

    group = scrGroupCreate(runner, setup_func, cleanup_func);
    ADD_PASS(read_from_pipe1);
    ADD_PASS(read_from_pipe2);

    group = scrGroupCreate(runner, setup_skip, NULL);
    ADD_SKIP(skip_me1);
    ADD_SKIP(skip_me2);

    group = scrGroupCreate(runner, bad_setup, NULL);
    ADD_ERROR(error_setup_fail1);
    ADD_ERROR(error_setup_fail2);

    scrRunnerRun(runner, &global_num, &stats);
    scrRunnerDestroy(runner);

    return (stats.num_passed != num_pass || stats.num_skipped != num_skip || stats.num_failed != 0 ||
            stats.num_errored != num_error);
}
