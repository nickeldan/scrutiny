/**
 * @file test.h
 * @brief Defines the test macros.
 */

#ifndef SCRUTINY_TEST_H
#define SCRUTINY_TEST_H

#include <stddef.h>
#include <stdint.h>

#include "definitions.h"

void *
scrGroupCtx(void) SCR_PURE;

/**
 * @brief Returns the group context in the form of a void*.
 */
#define SCR_GROUP_CTX() scrGroupCtx()

void
scrTestSkip(void) SCR_NORETURN;
/**
 * @brief Skips the current test.
 */
#define SCR_TEST_SKIP() scrTestSkip()

#define SCR_CONTEXT_DECL   const char *file_name, const char *function_name, unsigned int line_no
#define SCR_CONTEXT_PARAMS __FILE__, __func__, __LINE__

void
scrLog(SCR_CONTEXT_DECL, const char *format, ...) SCR_PRINTF(4);
/**
 * @brief Logs a message.
 */
#define SCR_LOG(...) scrLog(SCR_CONTEXT_PARAMS, __VA_ARGS__)

void
scrFail(SCR_CONTEXT_DECL, const char *format, ...) SCR_PRINTF(4) SCR_NORETURN;
/**
 * @brief Fails the current test with a message.
 */
#define SCR_FAIL(...) scrFail(SCR_CONTEXT_PARAMS, __VA_ARGS__)

/**
 * @brief Asserts that an expression is true and fails the test if it isn't.
 */
#define SCR_ASSERT(expr)                             \
    do {                                             \
        if (!(expr)) {                               \
            SCR_FAIL("Assertion failed: %s", #expr); \
        }                                            \
    } while (0)

#define SCR_ASSERT_FUNC(func, type) \
    void scrAssert##func(SCR_CONTEXT_DECL, type value1, const char *expr1, type value2, const char *expr2)
#define SCR_ASSERT_MACRO(func, expr1, expr2) scrAssert##func(SCR_CONTEXT_PARAMS, expr1, #expr1, expr2, #expr2)

SCR_ASSERT_FUNC(Eq, intmax_t);
/**
 * @brief Asserts that two integers are equal.
 */
#define SCR_ASSERT_EQ(expr1, expr2) SCR_ASSERT_MACRO(Eq, expr1, expr2)

SCR_ASSERT_FUNC(Neq, intmax_t);
/**
 * @brief Asserts that two integers are not equal.
 */
#define SCR_ASSERT_NEQ(expr1, expr2) SCR_ASSERT_MACRO(Neq, expr1, expr2)

SCR_ASSERT_FUNC(Lt, intmax_t);
/**
 * @brief Asserts that one integer is less than another.
 */
#define SCR_ASSERT_LT(expr1, expr2) SCR_ASSERT_MACRO(Lt, expr1, expr2)

SCR_ASSERT_FUNC(Le, intmax_t);
/**
 * @brief Asserts that one integer is less than or equal to another.
 */
#define SCR_ASSERT_LE(expr1, expr2) SCR_ASSERT_MACRO(Le, expr1, expr2)

SCR_ASSERT_FUNC(Gt, intmax_t);
/**
 * @brief Asserts that one integer is greater than another.
 */
#define SCR_ASSERT_GT(expr1, expr2) SCR_ASSERT_MACRO(Gt, expr1, expr2)

SCR_ASSERT_FUNC(Ge, intmax_t);
/**
 * @brief Asserts that one integer is greater than or equal to another.
 */
#define SCR_ASSERT_GE(expr1, expr2) SCR_ASSERT_MACRO(Ge, expr1, expr2)

SCR_ASSERT_FUNC(EqUnsigned, uintmax_t);
/**
 * @brief Asserts that two unsigned integers are equal.
 */
#define SCR_ASSERT_UNSIGNED_EQ(expr1, expr2) SCR_ASSERT_MACRO(EqUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(NeqUnsigned, uintmax_t);
/**
 * @brief Asserts that two unsigned integers are not equal.
 */
#define SCR_ASSERT_UNSIGNED_NEQ(expr1, expr2) SCR_ASSERT_MACRO(NeqUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(LtUnsigned, uintmax_t);
/**
 * @brief Asserts that one unsigned integer is less than another.
 */
#define SCR_ASSERT_UNSIGNED_LT(expr1, expr2) SCR_ASSERT_MACRO(LtUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(LeUnsigned, uintmax_t);
/**
 * @brief Asserts that one unsigned integer is less than or equal to another.
 */
#define SCR_ASSERT_UNSIGNED_LE(expr1, expr2) SCR_ASSERT_MACRO(LeUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(GtUnsigned, uintmax_t);
/**
 * @brief Asserts that one unsigned integer is greater than another.
 */
#define SCR_ASSERT_UNSIGNED_GT(expr1, expr2) SCR_ASSERT_MACRO(GtUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(GeUnsigned, uintmax_t);
/**
 * @brief Asserts that one unsigned integer is greater than or equal to another.
 */
#define SCR_ASSERT_UNSIGNED_GE(expr1, expr2) SCR_ASSERT_MACRO(GeUnsigned, expr1, expr2)

SCR_ASSERT_FUNC(EqFloat, long double);
/**
 * @brief Asserts that two floating-point values are equal.
 */
#define SCR_ASSERT_FLOAT_EQ(expr1, expr2) SCR_ASSERT_MACRO(EqFloat, expr1, expr2)

SCR_ASSERT_FUNC(NeqFloat, long double);
/**
 * @brief Asserts that two floating-point values are not equal.
 */
#define SCR_ASSERT_FLOAT_NEQ(expr1, expr2) SCR_ASSERT_MACRO(NeqFloat, expr1, expr2)

SCR_ASSERT_FUNC(LtFloat, long double);
/**
 * @brief Asserts that one floating-point value is less than another.
 */
#define SCR_ASSERT_FLOAT_LT(expr1, expr2) SCR_ASSERT_MACRO(LtFloat, expr1, expr2)

SCR_ASSERT_FUNC(LeFloat, long double);
/**
 * @brief Asserts that one floating-point value is less than or equal to another.
 */
#define SCR_ASSERT_FLOAT_LE(expr1, expr2) SCR_ASSERT_MACRO(LeFloat, expr1, expr2)

SCR_ASSERT_FUNC(GtFloat, long double);
/**
 * @brief Asserts that one floating-point value is greater than another.
 */
#define SCR_ASSERT_FLOAT_GT(expr1, expr2) SCR_ASSERT_MACRO(GtFloat, expr1, expr2)

SCR_ASSERT_FUNC(GeFloat, long double);
/**
 * @brief Asserts that one floating-point value is greater than or equal to another.
 */
#define SCR_ASSERT_FLOAT_GE(expr1, expr2) SCR_ASSERT_MACRO(GeFloat, expr1, expr2)

SCR_ASSERT_FUNC(PtrEq, const void *);
/**
 * @brief Asserts that two pointers are equal.
 */
#define SCR_ASSERT_PTR_EQ(expr1, expr2) SCR_ASSERT_MACRO(PtrEq, expr1, expr2)

SCR_ASSERT_FUNC(PtrNeq, const void *);
/**
 * @brief Asserts that two pointers are not equal.
 */
#define SCR_ASSERT_PTR_NEQ(expr1, expr2) SCR_ASSERT_MACRO(PtrNeq, expr1, expr2)

SCR_ASSERT_FUNC(StrEq, const char *);
/**
 * @brief Asserts that two strings are equal.
 */
#define SCR_ASSERT_STR_EQ(expr1, expr2) SCR_ASSERT_MACRO(StrEq, expr1, expr2)

SCR_ASSERT_FUNC(StrNeq, const char *);
/**
 * @brief Asserts that two strings are not equal.
 */
#define SCR_ASSERT_STR_NEQ(expr1, expr2) SCR_ASSERT_MACRO(StrNeq, expr1, expr2)

SCR_ASSERT_FUNC(StrBeginsWith, const char *);
/**
 * @brief Asserts that one string starts with another.
 */
#define SCR_ASSERT_STR_BEGINS_WITH(expr1, expr2) SCR_ASSERT_MACRO(StrBeginsWith, expr1, expr2)

SCR_ASSERT_FUNC(StrNBeginsWith, const char *);
/**
 * @brief Asserts that one string doesn't start with another.
 */
#define SCR_ASSERT_STR_NBEGINS_WITH(expr1, expr2) SCR_ASSERT_MACRO(StrNBeginsWith, expr1, expr2)

size_t
scrAssertStrContains(SCR_CONTEXT_DECL, const char *value1, const char *expr1, const char *value2,
                     const char *expr2);
/**
 * @brief Asserts that one string contains another.
 *
 * @return  If successful, the offset in the first string where the second string begins.
 */
#define SCR_ASSERT_STR_CONTAINS(expr1, expr2) \
    scrAssertStrContains(SCR_CONTEXT_PARAMS, expr1, #expr1, expr2, #expr2)

SCR_ASSERT_FUNC(StrNContains, const char *);
/**
 * @brief Asserts that one string doesn't contain another.
 */
#define SCR_ASSERT_STR_NCONTAINS(expr1, expr2) SCR_ASSERT_MACRO(StrNContains, expr1, expr2)

SCR_ASSERT_FUNC(CharEq, char);
/**
 * @brief Asserts that two characters are equal.
 */
#define SCR_ASSERT_CHAR_EQ(expr1, expr2) SCR_ASSERT_MACRO(CharEq, expr1, expr2)

SCR_ASSERT_FUNC(CharNeq, char);
/**
 * @brief Asserts that two characters are not equal.
 */
#define SCR_ASSERT_CHAR_NEQ(expr1, expr2) SCR_ASSERT_MACRO(CharNeq, expr1, expr2)

void
scrAssertMemEq(SCR_CONTEXT_DECL, const void *ptr1, const char *expr1, const void *ptr2, const char *expr2,
               size_t size);
/**
 * @brief Asserts that two memory regions are equal.
 */
#define SCR_ASSERT_MEM_EQ(expr1, expr2, size) \
    scrAssertMemEq(SCR_CONTEXT_PARAMS, expr1, #expr1, expr2, #expr2, size)

#endif  // SCRUTINY_TEST_H
