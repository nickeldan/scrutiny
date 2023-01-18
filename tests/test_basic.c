#include <stdio.h>
#include <unistd.h>

#include <scrutiny/scrutiny.h>

#include "common.h"

static void
do_nothing(void)
{
}

static void
integers_equal(void)
{
    int x = 5, y = 5;

    SCR_ASSERT_EQ(x, y);
}

static void
integers_not_equal(void)
{
    int x = 5, y = 6;

    SCR_ASSERT_NEQ(x, y);
}

static void
integers_less_than(void)
{
    int x = 5, y = 6;

    SCR_ASSERT_LT(x, y);
}

static void
integers_less_than_or_equal(void)
{
    int x = 5, y = 5, z = 6;

    SCR_ASSERT_LE(x, y);
    SCR_ASSERT_LE(x, z);
}

static void
integers_greater_than(void)
{
    int x = 6, y = 5;

    SCR_ASSERT_GT(x, y);
}

static void
integers_greater_than_or_equal(void)
{
    int x = 6, y = 6, z = 5;

    SCR_ASSERT_GE(x, y);
    SCR_ASSERT_GE(x, z);
}

static void
unsigned_integers_equal(void)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_EQ(x, y);
}

static void
unsigned_integers_not_equal(void)
{
    unsigned int x = 5, y = 6;

    SCR_ASSERT_UNSIGNED_NEQ(x, y);
}

static void
unsigned_integers_less_than(void)
{
    unsigned int x = 5, y = 6;

    SCR_ASSERT_UNSIGNED_LT(x, y);
}

static void
unsigned_integers_less_than_or_equal(void)
{
    unsigned int x = 5, y = 5, z = 6;

    SCR_ASSERT_UNSIGNED_LE(x, y);
    SCR_ASSERT_UNSIGNED_LE(x, z);
}

static void
unsigned_integers_greater_than(void)
{
    unsigned int x = 6, y = 5;

    SCR_ASSERT_UNSIGNED_GT(x, y);
}

static void
unsigned_integers_greater_than_or_equal(void)
{
    unsigned int x = 6, y = 6, z = 5;

    SCR_ASSERT_UNSIGNED_GE(x, y);
    SCR_ASSERT_UNSIGNED_GE(x, z);
}

static void
floats_equal(void)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_EQ(x, y);
}

static void
floats_not_equal(void)
{
    float x = 5.0, y = 6.0;

    SCR_ASSERT_FLOAT_NEQ(x, y);
}

static void
floats_less_than(void)
{
    float x = 5.0, y = 6.0;

    SCR_ASSERT_FLOAT_LT(x, y);
}

static void
floats_less_than_or_equal(void)
{
    float x = 5.0, y = 5.0, z = 6.0;

    SCR_ASSERT_FLOAT_LE(x, y);
    SCR_ASSERT_FLOAT_LE(x, z);
}

static void
floats_greater_than(void)
{
    float x = 6.0, y = 5.0;

    SCR_ASSERT_FLOAT_GT(x, y);
}

static void
floats_greater_than_or_equal(void)
{
    float x = 6.0, y = 6.0, z = 5.0;

    SCR_ASSERT_FLOAT_GE(x, y);
    SCR_ASSERT_FLOAT_GE(x, z);
}

static void
pointers_equal(void)
{
    int x = 0;
    int *p1 = &x, *p2 = &x;

    SCR_ASSERT_PTR_EQ(p1, p2);
}

static void
pointers_not_equal(void)
{
    int x = 0, y = 0;
    int *p1 = &x, *p2 = &y;

    SCR_ASSERT_PTR_NEQ(p1, p2);
}

static void
strings_equal(void)
{
    const char *x = "foo", *y = "foo";

    SCR_ASSERT_STR_EQ(x, y);
}

static void
strings_not_equal(void)
{
    const char *x = "foo", *y = "bar";

    SCR_ASSERT_STR_NEQ(x, y);
}

static void
chars_equal(void)
{
    char x = 'a', y = 'a';

    SCR_ASSERT_CHAR_EQ(x, y);
}

static void
chars_not_equal(void)
{
    char x = 'a', y = 'b';

    SCR_ASSERT_CHAR_NEQ(x, y);
}

static void
buffers_equal(void)
{
    unsigned char buffer[] = {'h', 'e', 'l', 'p'};
    const char *word = "hello";

    SCR_ASSERT_MEM_EQ(buffer, word, 3);
}

static void
fail_integers_equal(void)
{
    int x = 5, y = 6;

    SCR_ASSERT_EQ(x, y);
}

static void
fail_integers_not_equal(void)
{
    int x = 5, y = 5;

    SCR_ASSERT_NEQ(x, y);
}

static void
fail_integers_less_than(void)
{
    int x = 5, y = 5;

    SCR_ASSERT_LT(x, y);
}

static void
fail_integers_less_than_or_equal(void)
{
    int x = 5, y = 4;

    SCR_ASSERT_LE(x, y);
}

static void
fail_integers_greater_than(void)
{
    int x = 5, y = 5;

    SCR_ASSERT_GT(x, y);
}

static void
fail_integers_greater_than_or_equal(void)
{
    int x = 5, y = 6;

    SCR_ASSERT_GE(x, y);
}

static void
fail_unsigned_integers_equal(void)
{
    unsigned int x = 5, y = 6;

    SCR_ASSERT_UNSIGNED_EQ(x, y);
}

static void
fail_unsigned_integers_not_equal(void)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_NEQ(x, y);
}

static void
fail_unsigned_integers_less_than(void)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_LT(x, y);
}

static void
fail_unsigned_integers_less_than_or_equal(void)
{
    unsigned int x = 5, y = 4;

    SCR_ASSERT_UNSIGNED_LE(x, y);
}

static void
fail_unsigned_integers_greater_than(void)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_GT(x, y);
}

static void
fail_unsigned_integers_greater_than_or_equal(void)
{
    unsigned int x = 5, y = 6;

    SCR_ASSERT_UNSIGNED_GE(x, y);
}

static void
fail_floats_equal(void)
{
    float x = 5.0, y = 6.0;

    SCR_ASSERT_FLOAT_EQ(x, y);
}

static void
fail_floats_not_equal(void)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_NEQ(x, y);
}

static void
fail_floats_less_than(void)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_LT(x, y);
}

static void
fail_floats_less_than_or_equal(void)
{
    float x = 6.0, y = 5.0;

    SCR_ASSERT_FLOAT_LE(x, y);
}

static void
fail_floats_greater_than(void)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_GT(x, y);
}

static void
fail_floats_greater_than_or_equal(void)
{
    float x = 5.0, y = 6.0;

    SCR_ASSERT_FLOAT_GE(x, y);
}

static void
fail_pointers_equal(void)
{
    int x = 0, y = 0;
    int *p1 = &x, *p2 = &y;

    SCR_ASSERT_PTR_EQ(p1, p2);
}

static void
fail_pointers_not_equal(void)
{
    int x = 0;
    int *p1 = &x, *p2 = &x;

    SCR_ASSERT_PTR_NEQ(p1, p2);
}

static void
fail_strings_equal(void)
{
    const char *x = "foo", *y = "bar";

    SCR_ASSERT_STR_EQ(x, y);
}

static void
fail_strings_not_equal(void)
{
    const char *x = "foo", *y = "foo";

    SCR_ASSERT_STR_NEQ(x, y);
}

static void
fail_chars_equal(void)
{
    char x = 'a', y = 'b';

    SCR_ASSERT_CHAR_EQ(x, y);
}

static void
fail_chars_not_equal(void)
{
    char x = 'a', y = 'a';

    SCR_ASSERT_CHAR_NEQ(x, y);
}

static void
fail_buffers_equal(void)
{
    const char *word1 = "help", *word2 = "hello";

    SCR_ASSERT_MEM_EQ(word1, word2, 5);
}

static void
fail_error_message(void)
{
    SCR_ERROR("This is an error message.");
}

static void
fail_with_output(void)
{
    printf("Here's some stdout\n");
    fprintf(stderr, "Here's some stderr\n");
    SCR_ERROR("Intentionally failing");
}

static void
fail_timeout(void)
{
    sleep(2);
}

static void
error_segfault(void)
{
    *(unsigned char *)SCR_GROUP_CTX() = 0;
}

static void
skip_me(void)
{
    SCR_TEST_SKIP();
}

static void
xfail_basic(void)
{
    SCR_ASSERT_EQ(5, 6);
}

static void
xpass_basic(void)
{
}

int
main(int argc, char **argv)
{
    unsigned int num_pass = 0, num_fail = 0, num_error = 0, num_skip = 0;
    scrRunner *runner;
    scrGroup *group;
    scrStats stats;
    (void)argc;

    printf("\nRunning %s\n\n", argv[0]);

    runner = scrRunnerCreate();
    group = scrGroupCreate(runner, NULL, NULL);

    ADD_PASS(do_nothing);
    ADD_PASS(integers_equal);
    ADD_PASS(integers_not_equal);
    ADD_PASS(integers_less_than);
    ADD_PASS(integers_less_than_or_equal);
    ADD_PASS(integers_greater_than);
    ADD_PASS(integers_greater_than_or_equal);
    ADD_PASS(unsigned_integers_equal);
    ADD_PASS(unsigned_integers_not_equal);
    ADD_PASS(unsigned_integers_less_than);
    ADD_PASS(unsigned_integers_less_than_or_equal);
    ADD_PASS(unsigned_integers_greater_than);
    ADD_PASS(unsigned_integers_greater_than_or_equal);
    ADD_PASS(floats_equal);
    ADD_PASS(floats_not_equal);
    ADD_PASS(floats_less_than);
    ADD_PASS(floats_less_than_or_equal);
    ADD_PASS(floats_greater_than);
    ADD_PASS(floats_greater_than_or_equal);
    ADD_PASS(pointers_equal);
    ADD_PASS(pointers_not_equal);
    ADD_PASS(strings_equal);
    ADD_PASS(strings_not_equal);
    ADD_PASS(chars_equal);
    ADD_PASS(chars_not_equal);
    ADD_PASS(buffers_equal);
    ADD_FAIL(fail_integers_equal);
    ADD_FAIL(fail_integers_not_equal);
    ADD_FAIL(fail_integers_less_than);
    ADD_FAIL(fail_integers_less_than_or_equal);
    ADD_FAIL(fail_integers_greater_than);
    ADD_FAIL(fail_integers_greater_than_or_equal);
    ADD_FAIL(fail_unsigned_integers_equal);
    ADD_FAIL(fail_unsigned_integers_not_equal);
    ADD_FAIL(fail_unsigned_integers_less_than);
    ADD_FAIL(fail_unsigned_integers_less_than_or_equal);
    ADD_FAIL(fail_unsigned_integers_greater_than);
    ADD_FAIL(fail_unsigned_integers_greater_than_or_equal);
    ADD_FAIL(fail_floats_equal);
    ADD_FAIL(fail_floats_not_equal);
    ADD_FAIL(fail_floats_less_than);
    ADD_FAIL(fail_floats_less_than_or_equal);
    ADD_FAIL(fail_floats_greater_than);
    ADD_FAIL(fail_floats_greater_than_or_equal);
    ADD_FAIL(fail_pointers_equal);
    ADD_FAIL(fail_pointers_not_equal);
    ADD_FAIL(fail_strings_equal);
    ADD_FAIL(fail_strings_not_equal);
    ADD_FAIL(fail_chars_equal);
    ADD_FAIL(fail_chars_not_equal);
    ADD_FAIL(fail_buffers_equal);
    ADD_FAIL(fail_error_message);
    ADD_FAIL(fail_with_output);
    ADD_TIMEOUT(fail_timeout, 1);
    ADD_ERROR(error_segfault);
    ADD_SKIP(skip_me);
    ADD_XFAIL(xfail_basic);
    ADD_XPASS(xpass_basic);

    scrRunnerRun(runner, NULL, &stats);
    scrRunnerDestroy(runner);

    return (stats.num_passed != num_pass || stats.num_skipped != num_skip || stats.num_failed != num_fail ||
            stats.num_errored != num_error);
}
