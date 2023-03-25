#include <stdio.h>

#include <scrutiny/scrutiny.h>

static void
gonna_fail(void)
{
    SCR_FAIL("Failing");
}

static void
never_gonna_run(void)
{
}

int
main(int argc, char **argv)
{
    scrGroup *group;
    scrOptions options = {.flags = SCR_RUN_FLAG_FAIL_FAST};
    scrStats stats;
    (void)argc;

    printf("\nRunning %s\n\n", argv[0]);

    group = scrGroupCreate(NULL, NULL);
    scrGroupAddTest(group, "gonna_fail", gonna_fail, 0, 0);
    scrGroupAddTest(group, "never_gonna_run", never_gonna_run, 0, 0);

    scrRun(&options, &stats);

    return !(stats.num_passed == 0 && stats.num_skipped == 0 && stats.num_failed == 1 &&
             stats.num_errored == 0);
}
