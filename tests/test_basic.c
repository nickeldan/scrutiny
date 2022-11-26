#include <scrutiny/run.h>
#include <scrutiny/test.h>

static SCR_TEST_FN(do_nothing)
{
    (void)_test_ctx;
}

static SCR_TEST_FN(integers_equal)
{
    int x = 5, y = 5;

    SCR_ASSERT_EQ(x, y);
}

static SCR_TEST_FN(integers_not_equal)
{
    int x = 5, y = 6;

    SCR_ASSERT_NEQ(x, y);
}

static SCR_TEST_FN(integers_less_than)
{
    int x = 5, y = 6;

    SCR_ASSERT_LT(x, y);
}

static SCR_TEST_FN(integers_less_than_or_equal)
{
    int x = 5, y = 5, z = 6;

    SCR_ASSERT_LE(x, y);
    SCR_ASSERT_LE(x, z);
}

static SCR_TEST_FN(integers_greater_than)
{
    int x = 6, y = 5;

    SCR_ASSERT_GT(x, y);
}

static SCR_TEST_FN(integers_greater_than_or_equal)
{
    int x = 6, y = 6, z = 5;

    SCR_ASSERT_GE(x, y);
    SCR_ASSERT_GE(x, z);
}

static SCR_TEST_FN(unsigned_integers_equal)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_EQ(x, y);
}

static SCR_TEST_FN(unsigned_integers_not_equal)
{
    unsigned int x = 5, y = 6;

    SCR_ASSERT_UNSIGNED_NEQ(x, y);
}

static SCR_TEST_FN(unsigned_integers_less_than)
{
    unsigned int x = 5, y = 6;

    SCR_ASSERT_UNSIGNED_LT(x, y);
}

static SCR_TEST_FN(unsigned_integers_less_than_or_equal)
{
    unsigned int x = 5, y = 5, z = 6;

    SCR_ASSERT_UNSIGNED_LE(x, y);
    SCR_ASSERT_UNSIGNED_LE(x, z);
}

static SCR_TEST_FN(unsigned_integers_greater_than)
{
    unsigned int x = 6, y = 5;

    SCR_ASSERT_UNSIGNED_GT(x, y);
}

static SCR_TEST_FN(unsigned_integers_greater_than_or_equal)
{
    unsigned int x = 6, y = 6, z = 5;

    SCR_ASSERT_UNSIGNED_GE(x, y);
    SCR_ASSERT_UNSIGNED_GE(x, z);
}

static SCR_TEST_FN(floats_equal)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_EQ(x, y);
}

static SCR_TEST_FN(floats_not_equal)
{
    float x = 5.0, y = 6.0;

    SCR_ASSERT_FLOAT_NEQ(x, y);
}

static SCR_TEST_FN(floats_less_than)
{
    float x = 5.0, y = 6.0;

    SCR_ASSERT_FLOAT_LT(x, y);
}

static SCR_TEST_FN(floats_less_than_or_equal)
{
    float x = 5.0, y = 5.0, z = 6.0;

    SCR_ASSERT_FLOAT_LE(x, y);
    SCR_ASSERT_FLOAT_LE(x, z);
}

static SCR_TEST_FN(floats_greater_than)
{
    float x = 6.0, y = 5.0;

    SCR_ASSERT_FLOAT_GT(x, y);
}

static SCR_TEST_FN(floats_greater_than_or_equal)
{
    float x = 6.0, y = 6.0, z = 5.0;

    SCR_ASSERT_FLOAT_GE(x, y);
    SCR_ASSERT_FLOAT_GE(x, z);
}

static SCR_TEST_FN(pointers_equal)
{
    int x = 0;
    int *p1 = &x, *p2 = &x;

    SCR_ASSERT_PTR_EQ(p1, p2);
}

static SCR_TEST_FN(pointers_not_equal)
{
    int x = 0, y = 0;
    int *p1 = &x, *p2 = &y;

    SCR_ASSERT_PTR_NEQ(p1, p2);
}

static SCR_TEST_FN(fail_integers_equal)
{
    int x = 5, y = 6;

    SCR_ASSERT_EQ(x, y);
}

static SCR_TEST_FN(fail_integers_not_equal)
{
    int x = 5, y = 5;

    SCR_ASSERT_NEQ(x, y);
}

static SCR_TEST_FN(fail_integers_less_than)
{
    int x = 5, y = 5;

    SCR_ASSERT_LT(x, y);
}

static SCR_TEST_FN(fail_integers_less_than_or_equal)
{
    int x = 5, y = 4;

    SCR_ASSERT_LE(x, y);
}

static SCR_TEST_FN(fail_integers_greater_than)
{
    int x = 5, y = 5;

    SCR_ASSERT_GT(x, y);
}

static SCR_TEST_FN(fail_integers_greater_than_or_equal)
{
    int x = 5, y = 6;

    SCR_ASSERT_GE(x, y);
}

static SCR_TEST_FN(fail_unsigned_integers_equal)
{
    unsigned int x = 5, y = 6;

    SCR_ASSERT_UNSIGNED_EQ(x, y);
}

static SCR_TEST_FN(fail_unsigned_integers_not_equal)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_NEQ(x, y);
}

static SCR_TEST_FN(fail_unsigned_integers_less_than)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_LT(x, y);
}

static SCR_TEST_FN(fail_unsigned_integers_less_than_or_equal)
{
    unsigned int x = 5, y = 4;

    SCR_ASSERT_UNSIGNED_LE(x, y);
}

static SCR_TEST_FN(fail_unsigned_integers_greater_than)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_GT(x, y);
}

static SCR_TEST_FN(fail_unsigned_integers_greater_than_or_equal)
{
    unsigned int x = 5, y = 6;

    SCR_ASSERT_UNSIGNED_GE(x, y);
}

static SCR_TEST_FN(fail_floats_equal)
{
    float x = 5.0, y = 6.0;

    SCR_ASSERT_FLOAT_EQ(x, y);
}

static SCR_TEST_FN(fail_floats_not_equal)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_NEQ(x, y);
}

static SCR_TEST_FN(fail_floats_less_than)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_LT(x, y);
}

static SCR_TEST_FN(fail_floats_less_than_or_equal)
{
    float x = 6.0, y = 5.0;

    SCR_ASSERT_FLOAT_LE(x, y);
}

static SCR_TEST_FN(fail_floats_greater_than)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_GT(x, y);
}

static SCR_TEST_FN(fail_floats_greater_than_or_equal)
{
    float x = 5.0, y = 6.0;

    SCR_ASSERT_FLOAT_GE(x, y);
}

static SCR_TEST_FN(fail_pointers_equal)
{
    int x = 0, y = 0;
    int *p1 = &x, *p2 = &y;

    SCR_ASSERT_PTR_EQ(p1, p2);
}

static SCR_TEST_FN(fail_pointers_not_equal)
{
    int x = 0;
    int *p1 = &x, *p2 = &x;

    SCR_ASSERT_PTR_NEQ(p1, p2);
}

static SCR_TEST_FN(skip_me)
{
    (void)_test_ctx;
    SCR_TEST_SKIP();
}

int
main()
{
    unsigned int num_pass = 0, num_fail = 0, num_skip = 0;
    scrRunner *runner;
    scrGroup *group;
    scrStats stats;

    runner = scrRunnerCreate();
    group = scrGroupCreate(runner, NULL, NULL);

#define ADD_PASS(test)                          \
    do {                                        \
        scrGroupAddTest(group, #test, test, 0); \
        num_pass++;                             \
    } while (0)
#define ADD_FAIL(test)                                         \
    do {                                                       \
        scrGroupAddTest(group, "fail_" #test, fail_##test, 0); \
        num_fail++;                                            \
    } while (0)
#define ADD_SKIP(test)                          \
    do {                                        \
        scrGroupAddTest(group, #test, test, 0); \
        num_skip++;                             \
    } while (0)

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
    ADD_FAIL(integers_equal);
    ADD_FAIL(integers_not_equal);
    ADD_FAIL(integers_less_than);
    ADD_FAIL(integers_less_than_or_equal);
    ADD_FAIL(integers_greater_than);
    ADD_FAIL(integers_greater_than_or_equal);
    ADD_FAIL(unsigned_integers_equal);
    ADD_FAIL(unsigned_integers_not_equal);
    ADD_FAIL(unsigned_integers_less_than);
    ADD_FAIL(unsigned_integers_less_than_or_equal);
    ADD_FAIL(unsigned_integers_greater_than);
    ADD_FAIL(unsigned_integers_greater_than_or_equal);
    ADD_FAIL(floats_equal);
    ADD_FAIL(floats_not_equal);
    ADD_FAIL(floats_less_than);
    ADD_FAIL(floats_less_than_or_equal);
    ADD_FAIL(floats_greater_than);
    ADD_FAIL(floats_greater_than_or_equal);
    ADD_FAIL(pointers_equal);
    ADD_FAIL(pointers_not_equal);
    ADD_SKIP(skip_me);

#undef ADD_PASS
#undef ADD_FAIL
#undef ADD_SKIP

    scrRunnerRun(runner, NULL, &stats);
    scrRunnerDestroy(runner);

    return (stats.num_passed != num_pass || stats.num_skipped != num_skip || stats.num_failed != num_fail ||
            stats.num_errored != 0);
}
