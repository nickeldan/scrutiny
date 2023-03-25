Scrutiny
========

Scrutiny is a C testing framework for POSIX environments.

Testing basics
--------------

All of the functionality can be accessed by

```c
#include <scrutiny/scrutiny.h>
```

Every test must be part of a group.  You can create a group by

```c
scrGroup *group;

group = scrGroupCreate(NULL, NULL);
```

After that, you can define a test function:

```c
void my_test(void) {
    ...
}
```

and add it to a group:

```c
scrGroupAddTest(group, "Test name", my_test, 0, 0);
```

Once you have added all of the tests, you can run them by

```c
scrRun(NULL, NULL);
```

This function returns `0` if all of the tests pass (or are skipped) and `1` otherwise.  The function also summarizes the results in `stdout`.

You can pass a `scrStats*` to `scrRun`:

```c
scrStats stats;

scrRun(NULL, &stats);
```

where `scrStats` is defined as

```c
typedef struct scrStats {
    unsigned int num_passed;
    unsigned int num_skipped;
    unsigned int num_failed;
    unsigned int num_errored;
} scrStats;
```

Writing tests
-------------

Various macros are provided in order to test various conditions.  For example, for integers,

```c
void integer_test(void) {
    int x = 5, y = 5, z = 6;

    SCR_ASSERT_EQ(x, y); // equal
    SCR_ASSERT_NEQ(x, z); // not equal
    SCR_ASSERT_LE(x, y); // less than or equal to
    SCR_ASSERT_LT(x, z); // less than
    SCR_ASSERT_GE(z, x); // greater than or equal to
    SCR_ASSERT_GT(z, x); // greater than
}
```

All integer values are upgraded to `intmax_t`.  If you need to use `uintmax_t`, use the `UNSIGNED` macros like `SCR_ASSERT_UNSIGNED_LT`.  For floating-point values, use macros like `SCR_ASSERT_FLOAT_LT`.

Though `char` variables are also integer variables, you should use the `SCR_ASSERT_CHAR_EQ` and `SCR_ASSERT_CHAR_NEQ` macros to compare them.

You can compare pointers by

```c
void pointer_test(void) {
    void *p = NULL;
    int x;

    SCR_ASSERT_PTR_EQ(p, NULL);
    SCR_ASSERT_PTR_NEQ(&x, p);
}
```

You can compare strings (i.e., `char` arrays) by

```c
void string_test(void) {
    const char *word = "hello";

    SCR_ASSERT_STR_EQ(word, "hello");
    SCR_ASSERT_STR_NEQ(word, "goodbye");
    SCR_ASSERT_STR_BEGINS_WITH(word, "hel");
    SCR_ASSERT_STR_NBEGINS_WITH(word, "hellp");
}
```

Please note that you cannot use the string macros with `NULL` pointers.

You can test that two memory regions are equal (essentially, running `memcmp`) by

```c
void buffers_equal(void) {
    SCR_ASSERT_MEM_EQ("help", "hello", 3);
}
```

You can skip a test by

```c
void skip_me(void) {
    SCR_TEST_SKIP();
}
```

In addition, you can make general assertions by

```c
void my_test(void) {
    int x = 5, y = 5;

    SCR_ASSERT(x + y == 10);
}
```

You can fail a test without any assertion by

```c
void gonna_fail(void) {
    SCR_FAIL("Failing this test for reasons");
}
```

You can emit logging statements by

```c
void my_test(void) {
    int x = 5;

    SCR_LOG("x is %i", x);
}
```

Note that such statements will only be displayed if the test fails.

Test parameters
---------------

The signature of `scrGroupAddTest` is

```c
void
scrGroupAddTest(scrGroup *group, const char *name, scrTestFn test_fn, unsigned int timeout, unsigned int flags);
```

If `timeout` is positive, then the test will fail if not completed within that many seconds.

At the moment, the only valid value for `flags` other than `0` is `SCR_TEST_FLAG_XFAIL`.  If this value is passed, then success/failure will be inverted.  That is, the test will be expected to fail and a failure will be counted if the test passes.

Global/group context
--------------------

For each group of tests, there is a group context which is a `void*`.  It is accessible from the tests via

```c
void my_test(void) {
    void *ctx = SCR_GROUP_CTX();
}
```

The signature of `scrRun` is

```c
int
scrRun(const scrOptions *options, scrStats *stats);
```

where `scrOptions` is defined as

```c
typedef struct scrOptions {
    void *global_ctx;
    unsigned int flags;
} scrOptions;
```

If the `options` argument is `NULL`, then default values will be used (i.e., `NULL` and `0`).

By default, each group context is equal to the global context.  However, you can pass function pointers to `scrGroupCreate` which can set up and tear down a group context.  The signature of `scrGroupCreate` is

```c
scrGroup *
scrGroupCreate(scrCtxCreateFn create_fn, scrCtxCleanupFn cleanup_fn);
```

where

```c
typedef void *(*scrCtxCreateFn)(void *);
typedef void (*scrCtxCleanupFn)(void *);
```

If specified, then `create_fn` will be called with the global context as the argument.  The pointer returned will be the group context.

If specified, then `cleanup_fn` will be called with the group context (or the global context if `create_fn` was unspecified).

You can use the test macros in `create_fn`.  If any of the assertions fail, then all of the tests in that group will be counted as having failed.  You can also call `SCR_TEST_SKIP()` which will skip all of the group's tests.

Run flags
---------

At the moment, the only valid value for `flags` in `scrOptions` other than `0` is `SCR_RUN_FLAG_FAIL_FAST`.  This tells the framework to stop running tests as soon as any test either fails or encounters an error.

Building Scrutiny
-----------------

Scrutiny has a submodule so you'll need to add `--recurse-submodules` to your `git clone` invocation.

You can build and install Scrutiny by

```sh
make install
```

After that, you can link your test program to Scrutiny with `-lscrutiny`.

Scrutiny can be uninstalled by

```sh
make uninstall
```
