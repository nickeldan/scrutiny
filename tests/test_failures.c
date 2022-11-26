#include <scrutiny/run.h>
#include <scrutiny/test.h>

static SCR_TEST_FN(integers_equal)
{
    int x = 5, y = 6;

    SCR_ASSERT_EQ(x, y);
}

static SCR_TEST_FN(integers_not_equal)
{
    int x = 5, y = 5;

    SCR_ASSERT_NEQ(x, y);
}

static SCR_TEST_FN(integers_less_than)
{
    int x = 5, y = 5;

    SCR_ASSERT_LT(x, y);
}

static SCR_TEST_FN(integers_less_than_or_equal)
{
    int x = 5, y = 4;

    SCR_ASSERT_LE(x, y);
}

static SCR_TEST_FN(integers_greater_than)
{
    int x = 5, y = 5;

    SCR_ASSERT_GT(x, y);
}

static SCR_TEST_FN(integers_greater_than_or_equal)
{
    int x = 5, y = 6;

    SCR_ASSERT_GE(x, y);
}

static SCR_TEST_FN(unsigned_integers_equal)
{
    unsigned int x = 5, y = 6;

    SCR_ASSERT_UNSIGNED_EQ(x, y);
}

static SCR_TEST_FN(unsigned_integers_not_equal)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_NEQ(x, y);
}

static SCR_TEST_FN(unsigned_integers_less_than)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_LT(x, y);
}

static SCR_TEST_FN(unsigned_integers_less_than_or_equal)
{
    unsigned int x = 5, y = 4;

    SCR_ASSERT_UNSIGNED_LE(x, y);
}

static SCR_TEST_FN(unsigned_integers_greater_than)
{
    unsigned int x = 5, y = 5;

    SCR_ASSERT_UNSIGNED_GT(x, y);
}

static SCR_TEST_FN(unsigned_integers_greater_than_or_equal)
{
    unsigned int x = 5, y = 6;

    SCR_ASSERT_UNSIGNED_GE(x, y);
}

static SCR_TEST_FN(floats_equal)
{
    float x = 5.0, y = 6.0;

    SCR_ASSERT_FLOAT_EQ(x, y);
}

static SCR_TEST_FN(floats_not_equal)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_NEQ(x, y);
}

static SCR_TEST_FN(floats_less_than)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_LT(x, y);
}

static SCR_TEST_FN(floats_less_than_or_equal)
{
    float x = 6.0, y = 5.0;

    SCR_ASSERT_FLOAT_LE(x, y);
}

static SCR_TEST_FN(floats_greater_than)
{
    float x = 5.0, y = 5.0;

    SCR_ASSERT_FLOAT_GT(x, y);
}

static SCR_TEST_FN(floats_greater_than_or_equal)
{
    float x = 5.0, y = 6.0;

    SCR_ASSERT_FLOAT_GE(x, y);
}

static SCR_TEST_FN(pointers_equal)
{
    int x = 0, y = 0;
    int *p1 = &x, *p2 = &y;

    SCR_ASSERT_PTR_EQ(p1, p2);
}

static SCR_TEST_FN(pointers_not_equal)
{
    int x = 0;
    int *p1 = &x, *p2 = &x;

    SCR_ASSERT_PTR_NEQ(p1, p2);
}

int
main()
{
    unsigned int num_tests = 0;
    scrRunner *runner;
    scrGroup *group;
    scrStats stats;

    runner = scrRunnerCreate();
    group = scrGroupCreate(runner, NULL, NULL);

#define ADD_TEST(test)                          \
    do {                                        \
        scrGroupAddTest(group, #test, test, 0); \
        num_tests++;                            \
    } while (0)
    ADD_TEST(integers_equal);
    ADD_TEST(integers_not_equal);
    ADD_TEST(integers_less_than);
    ADD_TEST(integers_less_than_or_equal);
    ADD_TEST(integers_greater_than);
    ADD_TEST(integers_greater_than_or_equal);
    ADD_TEST(unsigned_integers_equal);
    ADD_TEST(unsigned_integers_not_equal);
    ADD_TEST(unsigned_integers_less_than);
    ADD_TEST(unsigned_integers_less_than_or_equal);
    ADD_TEST(unsigned_integers_greater_than);
    ADD_TEST(unsigned_integers_greater_than_or_equal);
    ADD_TEST(floats_equal);
    ADD_TEST(floats_not_equal);
    ADD_TEST(floats_less_than);
    ADD_TEST(floats_less_than_or_equal);
    ADD_TEST(floats_greater_than);
    ADD_TEST(floats_greater_than_or_equal);
    ADD_TEST(pointers_equal);
    ADD_TEST(pointers_not_equal);
#undef ADD_TEST

    scrRunnerRun(runner, NULL, &stats);
    scrRunnerDestroy(runner);

    return (stats.num_passed != 0 || stats.num_skipped != 0 || stats.num_failed != num_tests ||
            stats.num_errored != 0);
}
