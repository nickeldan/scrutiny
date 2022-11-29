#ifndef SCRUTINY_TEST_H
#define SCRUTINY_TEST_H

#include <stdint.h>
#include <stdlib.h>

typedef enum scrTestCode {
    SCR_TEST_CODE_OK = 0,
    SCR_TEST_CODE_FAIL,
    SCR_TEST_CODE_ERROR,
    SCR_TEST_CODE_SKIP,
} scrTestCode;

#define SCR_TEST_FN(name) void name(void)

void *
scrGroupCtx(void)
#ifdef __GNUC__
    __attribute__((pure))
#endif
    ;
#define SCR_GROUP_CTX() scrGroupCtx()

#define SCR_TEST_SKIP() exit(SCR_TEST_CODE_SKIP);

void
scrLog(const char *format, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 1, 2)))
#endif
    ;
#define SCR_LOG(...) scrLog(__VA_ARGS__)

#define SCR_CONTEXT_DECL   const char *file_name, const char *function_name, unsigned int line_no
#define SCR_CONTEXT_PARAMS __FILE__, __func__, __LINE__

void
scrError(SCR_CONTEXT_DECL, const char *format, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 4, 5))) __attribute__((noreturn))
#endif
    ;
#define SCR_ERROR(...) scrError(SCR_CONTEXT_PARAMS, __VA_ARGS__)

#define SCR_ASSERT(expr)                              \
    do {                                              \
        if (!(expr)) {                                \
            SCR_ERROR("Assertion failed: %s", #expr); \
        }                                             \
    } while (0)

#define SCR_ASSERT_FUNC(func, type) \
    void scrAssert##func(SCR_CONTEXT_DECL, type value1, const char *expr1, type value2, const char *expr2)
#define SCR_ASSERT_MACRO(func, expr1, expr2) \
    scrAssert##func(SCR_CONTEXT_PARAMS, expr1, #expr1, expr2, #expr2)

SCR_ASSERT_FUNC(Eq, intmax_t);
#define SCR_ASSERT_EQ(expr1, expr2) SCR_ASSERT_MACRO(Eq, expr1, expr2)

SCR_ASSERT_FUNC(Neq, intmax_t);
#define SCR_ASSERT_NEQ(expr1, expr2) SCR_ASSERT_MACRO(Neq, expr1, expr2)

SCR_ASSERT_FUNC(Lt, intmax_t);
#define SCR_ASSERT_LT(expr1, expr2) SCR_ASSERT_MACRO(Lt, expr1, expr2)

SCR_ASSERT_FUNC(Le, intmax_t);
#define SCR_ASSERT_LE(expr1, expr2) SCR_ASSERT_MACRO(Le, expr1, expr2)

SCR_ASSERT_FUNC(Gt, intmax_t);
#define SCR_ASSERT_GT(expr1, expr2) SCR_ASSERT_MACRO(Gt, expr1, expr2)

SCR_ASSERT_FUNC(Ge, intmax_t);
#define SCR_ASSERT_GE(expr1, expr2) SCR_ASSERT_MACRO(Ge, expr1, expr2)

SCR_ASSERT_FUNC(EqUnsigned, uintmax_t);
#define SCR_ASSERT_UNSIGNED_EQ(expr1, expr2) SCR_ASSERT_MACRO(EqUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(NeqUnsigned, uintmax_t);
#define SCR_ASSERT_UNSIGNED_NEQ(expr1, expr2) SCR_ASSERT_MACRO(NeqUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(LtUnsigned, uintmax_t);
#define SCR_ASSERT_UNSIGNED_LT(expr1, expr2) SCR_ASSERT_MACRO(LtUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(LeUnsigned, uintmax_t);
#define SCR_ASSERT_UNSIGNED_LE(expr1, expr2) SCR_ASSERT_MACRO(LeUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(GtUnsigned, uintmax_t);
#define SCR_ASSERT_UNSIGNED_GT(expr1, expr2) SCR_ASSERT_MACRO(GtUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(GeUnsigned, uintmax_t);
#define SCR_ASSERT_UNSIGNED_GE(expr1, expr2) SCR_ASSERT_MACRO(GeUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(EqFloat, long double);
#define SCR_ASSERT_FLOAT_EQ(expr1, expr2) SCR_ASSERT_MACRO(EqFloat, expr1, expr2)

SCR_ASSERT_FUNC(NeqFloat, long double);
#define SCR_ASSERT_FLOAT_NEQ(expr1, expr2) SCR_ASSERT_MACRO(NeqFloat, expr1, expr2)

SCR_ASSERT_FUNC(LtFloat, long double);
#define SCR_ASSERT_FLOAT_LT(expr1, expr2) SCR_ASSERT_MACRO(LtFloat, expr1, expr2)

SCR_ASSERT_FUNC(LeFloat, long double);
#define SCR_ASSERT_FLOAT_LE(expr1, expr2) SCR_ASSERT_MACRO(LeFloat, expr1, expr2)

SCR_ASSERT_FUNC(GtFloat, long double);
#define SCR_ASSERT_FLOAT_GT(expr1, expr2) SCR_ASSERT_MACRO(GtFloat, expr1, expr2)

SCR_ASSERT_FUNC(GeFloat, long double);
#define SCR_ASSERT_FLOAT_GE(expr1, expr2) SCR_ASSERT_MACRO(GeFloat, expr1, expr2)

SCR_ASSERT_FUNC(PtrEq, const void *);
#define SCR_ASSERT_PTR_EQ(expr1, expr2) SCR_ASSERT_MACRO(PtrEq, expr1, expr2)

SCR_ASSERT_FUNC(PtrNeq, const void *);
#define SCR_ASSERT_PTR_NEQ(expr1, expr2) SCR_ASSERT_MACRO(PtrNeq, expr1, expr2)

#endif  // SCRUTINY_TEST_H
