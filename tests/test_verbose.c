#include <stdio.h>

#include <scrutiny/scrutiny.h>

static void
log_passing(void)
{
    SCR_LOG("This is a log message");
}

static void
log_skipping(void)
{
    SCR_LOG("This test will be skipped");
    SCR_TEST_SKIP();
}

static void
show_stdout_passing(void)
{
    printf("This is stdout\n");
}

static void
show_stderr_passing(void)
{
    fprintf(stderr, "This is stderr\n");
}

int
main(int argc, char **argv)
{
    scrGroup group;
    scrOptions options = {.flags = SCR_RF_VERBOSE};
    (void)argc;

    printf("\nRunning %s\n\n", argv[0]);

    group = scrGroupCreate(NULL, NULL);
    scrGroupAddTest(group, "Log passing", log_passing, NULL);
    scrGroupAddTest(group, "Log skipping", log_skipping, NULL);
    scrGroupAddTest(group, "Show stdout passing", show_stdout_passing, NULL);
    scrGroupAddTest(group, "Show stderr passing", show_stderr_passing, NULL);

    return scrRun(&options, NULL);
}
