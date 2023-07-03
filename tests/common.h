#ifndef SCRUTINY_TESTS_COMMON_H
#define SCRUTINY_TESTS_COMMON_H

#define ADD_PASS(test)                             \
    do {                                           \
        scrGroupAddTest(group, #test, test, NULL); \
        num_pass++;                                \
    } while (0)
#define ADD_XFAIL(test)                                      \
    do {                                                     \
        scrGroupAddTest(group, #test, test, &xfail_options); \
        num_pass++;                                          \
    } while (0)
#define ADD_FAIL(test)                             \
    do {                                           \
        scrGroupAddTest(group, #test, test, NULL); \
        num_fail++;                                \
    } while (0)
#define ADD_XPASS(test)                                      \
    do {                                                     \
        scrGroupAddTest(group, #test, test, &xfail_options); \
        num_fail++;                                          \
    } while (0)
#define ADD_ERROR(test)                            \
    do {                                           \
        scrGroupAddTest(group, #test, test, NULL); \
        num_error++;                               \
    } while (0)
#define ADD_TIMEOUT(test)                                      \
    do {                                                       \
        scrGroupAddTest(group, #test, test, &timeout_options); \
        num_fail++;                                            \
    } while (0)
#define ADD_SKIP(test)                             \
    do {                                           \
        scrGroupAddTest(group, #test, test, NULL); \
        num_skip++;                                \
    } while (0)

#endif  // SCRUTINY_TESTS_COMMON_H
