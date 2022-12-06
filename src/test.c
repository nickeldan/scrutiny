#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <scrutiny/test.h>

#include "internal.h"

static void *group_ctx;
static int log_fd;
static bool log_to_tty;

void
setGroupCtx(void *ctx)
{
    group_ctx = ctx;
}

void
setToTty(bool to_tty)
{
    log_to_tty = to_tty;
}

void
setLogFd(int fd)
{
    log_fd = fd;
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
        dprintf(log_fd, RESET_COLOR);
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

SCR_ASSERT_FUNC(StrEq, const char *)
{
    if (strcmp(value1, value2) != 0) {
        scrError(file_name, function_name, line_no,
                 "Assertion failed: strcmp(%s, %s) == 0\n\tstrcmp(\"%s\", \"%s\") == 0", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(StrNeq, const char *)
{
    if (strcmp(value1, value2) == 0) {
        scrError(file_name, function_name, line_no,
                 "Assertion failed: strcmp(%s, %s) != 0\n\tstrcmp(\"%s\", \"%s\") != 0", expr1, expr2,
                 value1, value2);
    }
}

SCR_ASSERT_FUNC(CharEq, char)
{
    if (value1 != value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s == %s\n\t'%c' == '%c'", expr1,
                 expr2, value1, value2);
    }
}

SCR_ASSERT_FUNC(CharNeq, char)
{
    if (value1 == value2) {
        scrError(file_name, function_name, line_no, "Assertion failed: %s != %s\n\t'%c' != '%c'", expr1,
                 expr2, value1, value2);
    }
}
