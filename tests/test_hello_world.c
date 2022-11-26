#include <scrutiny/run.h>
#include <scrutiny/test.h>

static SCR_TEST_FN(do_nothing)
{
    NOT_USING_CTX;
}

int
main()
{
    scrRunner *runner;
    scrGroup *group;
    scrStats stats;

    runner = scrRunnerCreate();
    group = scrGroupCreate(runner, NULL, NULL);
    scrGroupAddTest(group, "do_nothing", do_nothing, 0);

    scrRunnerRun(runner, NULL, &stats);
    scrRunnerDestroy(runner);

    return (stats.num_passed != 1 || stats.num_skipped != 0 || stats.num_failed != 0 ||
            stats.num_errored != 0);
}
