#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <scrutiny/test.h>

#include "internal.h"

#define RED   "\033[31m"
#define RESET "\033[0m"

static void *group_ctx;
static int log_fd;
static bool log_to_tty;

void
setParams(void *ctx, int fd)
{
    group_ctx = ctx;
    log_fd = fd;
    log_to_tty = isatty(STDOUT_FILENO);
}

void *
scrGroupCtx(void)
{
    return group_ctx;
}

void
scrLog(const char *format, ...)
{
    va_list args;

    dprintf(log_fd, "[INFO]  ");

    va_start(args, format);
    vdprintf(log_fd, format, args);
    va_end(args);

    dprintf(log_fd, "\n");
}

void
scrError(SCR_CONTEXT_DECL, const char *format, ...)
{
    va_list args;

    if (log_to_tty) {
        dprintf(log_fd, RED);
    }

    dprintf(log_fd, "[ERROR] On line %u of %s in %s:\n\t", line_no, function_name, file_name);

    va_start(args, format);
    vdprintf(log_fd, format, args);
    va_end(args);

    dprintf(log_fd, "\n");

    if (log_to_tty) {
        dprintf(log_fd, RESET);
    }

    exit(SCR_TEST_CODE_FAIL);
}

SCR_ASSERT_FUNC(Eq, intmax_t)
{
    if (value1 != value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s == %s\n\t%ji == %ji", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(Neq, intmax_t)
{
    if (value1 == value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s != %s\n\t%ji != %ji", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(Lt, intmax_t)
{
    if (value1 >= value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s < %s\n\t%ji < %ji", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(Le, intmax_t)
{
    if (value1 > value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s <= %s\n\t%ji <= %ji", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(Gt, intmax_t)
{
    if (value1 <= value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s > %s\n\t%ji > %ji", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(Ge, intmax_t)
{
    if (value1 < value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s >= %s\n\t%ji >= %ji", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(EqUnsigned, uintmax_t)
{
    if (value1 != value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s == %s\n\t%ju == %ju", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(NeqUnsigned, uintmax_t)
{
    if (value1 == value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s != %s\n\t%ju != %ju", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(LtUnsigned, uintmax_t)
{
    if (value1 >= value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s < %s\n\t%ju < %ju", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(LeUnsigned, uintmax_t)
{
    if (value1 > value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s <= %s\n\t%ju <= %ju", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(GtUnsigned, uintmax_t)
{
    if (value1 <= value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s > %s\n\t%ju > %ju", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(GeUnsigned, uintmax_t)
{
    if (value1 < value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s >= %s\n\t%ju >= %ju", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(EqFloat, long double)
{
    if (value1 != value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s == %s\n\t%Lg == %Lg", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(NeqFloat, long double)
{
    if (value1 == value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s != %s\n\t%Lg != %Lg", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(LtFloat, long double)
{
    if (value1 >= value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s < %s\n\t%Lg < %Lg", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(LeFloat, long double)
{
    if (value1 > value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s <= %s\n\t%Lg <= %Lg", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(GtFloat, long double)
{
    if (value1 <= value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s > %s\n\t%Lg > %Lg", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(GeFloat, long double)
{
    if (value1 < value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s >= %s\n\t%Lg >= %Lg", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(PtrEq, const void *)
{
    if (value1 != value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s == %s\n\t%p == %p", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(PtrNeq, const void *)
{
    if (value1 == value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s != %s\n\t%p != %p", expr1, expr2,
                 value1, value2);
    }
}
