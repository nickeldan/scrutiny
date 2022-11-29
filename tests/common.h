#ifndef SCRUTINY_TESTS_COMMON_H
#define SCRUTINY_TESTS_COMMON_H

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
#define ADD_ERROR(test)                                          \
    do {                                                         \
        scrGroupAddTest(group, "error_" #test, error_##test, 0); \
        num_error++;                                             \
    } while (0)
#define ADD_TIMEOUT(test, timeout)                    \
    do {                                              \
        scrGroupAddTest(group, #test, test, timeout); \
        num_error++;                                  \
    } while (0)
#define ADD_SKIP(test)                          \
    do {                                        \
        scrGroupAddTest(group, #test, test, 0); \
        num_skip++;                             \
    } while (0)

#endif  // SCRUTINY_TESTS_COMMON_H
