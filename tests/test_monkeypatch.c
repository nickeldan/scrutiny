#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <scrutiny/scrutiny.h>

#ifdef SCR_MONKEYPATCH

pid_t
indirectGetPpid(void);

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

    answer = (intptr_t)scrGroupCtx();
    true_getppid = scrPatchedFunction("getppid");
    SCR_ASSERT_PTR_NEQ(true_getppid, NULL);
    SCR_ASSERT_EQ(true_getppid(), answer);
}

static void
test_nonpatched_function(void)
{
    SCR_ASSERT_PTR_EQ(scrPatchedFunction("malloc"), NULL);
}

static void
test_selective_patch(void)
{
    pid_t answer;

    answer = (intptr_t)scrGroupCtx();
    SCR_ASSERT_EQ(getppid(), answer);
    SCR_ASSERT_EQ(indirectGetPpid(), 0);
}

int
main(int argc, char **argv)
{
    scrGroup group;

    (void)argc;

    printf("\nRunning %s\n\n", argv[0]);

    group = scrGroupCreate(group_setup, NULL);
    if (!scrGroupPatchFunction(group, "getppid", NULL, fake_getppid)) {
        return 1;
    }
    scrGroupAddTest(group, "Fake getppid", test_fake_getppid, NULL);
    scrGroupAddTest(group, "Get true getppid", test_true_getppid, NULL);
    scrGroupAddTest(group, "Nonpatched function", test_nonpatched_function, NULL);

    group = scrGroupCreate(group_setup, NULL);
    if (!scrGroupPatchFunction(group, "getppid", "libaux", fake_getppid)) {
        return 1;
    }
    scrGroupAddTest(group, "Selective patching", test_selective_patch, NULL);

    return scrRun(NULL, NULL);
}

#else  // SCR_MONKEYPATCH

int
main()
{
    return 0;
}

#endif
