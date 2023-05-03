#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <scrutiny/scrutiny.h>

#ifdef SCR_MONKEYPATCH

static void *
group_setup(void *global_ctx)
{
    (void)global_ctx;
    return (void *)(intptr_t)getpid();
}

static pid_t
fake_getppid(void)
{
    return 0;
}

static void
test_fake_getppid(void)
{
    SCR_ASSERT_EQ(getppid(), 0);
}

static void
test_true_getppid(void)
{
    pid_t answer;
    pid_t (*true_getppid)(void);

    answer = (intptr_t)SCR_GROUP_CTX();
    true_getppid = scrPatchedFunction("getppid");
    SCR_ASSERT_PTR_NEQ(true_getppid, NULL);
    SCR_ASSERT_EQ(true_getppid(), answer);
}

static void
test_nonpatched_function(void)
{
    SCR_ASSERT_PTR_EQ(scrPatchedFunction("malloc"), NULL);
}

int
main(int argc, char **argv)
{
    scrGroup *group;

    (void)argc;

    printf("\nRunning %s\n\n", argv[0]);

    group = scrGroupCreate(group_setup, NULL);

    if (!scrGroupPatchFunction(group, "getppid", fake_getppid)) {
        return 1;
    }

    scrGroupAddTest(group, "Fake getppid", test_fake_getppid, 0, 0);
    scrGroupAddTest(group, "Get true getppid", test_true_getppid, 0, 0);
    scrGroupAddTest(group, "Nonpatched function", test_nonpatched_function, 0, 0);

    return scrRun(NULL, NULL);
}

#else  // SCR_MONKEYPATCH

int
main()
{
    return 0;
}

#endif