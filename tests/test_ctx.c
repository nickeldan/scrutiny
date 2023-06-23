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
    SCR_FAIL("Intentionally failing in group setup");
}

static void
use_global_ctx(void)
{
    SCR_ASSERT_PTR_EQ(scrGroupCtx(), &global_num);
}

static void
read_from_pipe1(void)
{
    int fd = *(int *)scrGroupCtx();
    unsigned char buffer[2];

    SCR_ASSERT_EQ(read(fd, buffer, sizeof(buffer)), 2);
    SCR_ASSERT_EQ(buffer[0], 'a');
    SCR_ASSERT_EQ(buffer[1], 'b');
}

static void
read_from_pipe2(void)
{
    int fd = *(int *)scrGroupCtx();
    unsigned char buffer[2];

    SCR_ASSERT_EQ(read(fd, buffer, sizeof(buffer)), 2);
    SCR_ASSERT_EQ(buffer[0], 'c');
    SCR_ASSERT_EQ(buffer[1], 'd');
}

static void
skip_me1(void)
{
}

static void
skip_me2(void)
{
}

static void
setup_fail1(void)
{
}

static void
setup_fail2(void)
{
}

int
main(int argc, char **argv)
{
    unsigned int num_pass = 0, num_skip = 0, num_fail = 0, num_error = 0;
    scrGroup *group;
    scrOptions options = {.global_ctx = &global_num};
    scrStats stats;
    (void)argc;

    printf("\nRunning %s\n\n", argv[0]);

    group = scrGroupCreate(NULL, NULL);
    ADD_PASS(use_global_ctx);

    group = scrGroupCreate(setup_func, cleanup_func);
    ADD_PASS(read_from_pipe1);
    ADD_PASS(read_from_pipe2);

    group = scrGroupCreate(setup_skip, NULL);
    ADD_SKIP(skip_me1);
    ADD_SKIP(skip_me2);

    group = scrGroupCreate(bad_setup, NULL);
    ADD_FAIL(setup_fail1);
    ADD_FAIL(setup_fail2);

    scrRun(&options, &stats);

    return (stats.num_passed != num_pass || stats.num_skipped != num_skip || stats.num_failed != num_fail ||
            stats.num_errored != num_error);
}
