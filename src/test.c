#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <scrutiny/test.h>

#include "internal.h"

static void *group_ctx;
static int log_fd;
static bool show_color;

#define LOG_STRING(str)                                 \
    do {                                                \
        if (write(log_fd, str, sizeof(str) - 1) < 0) {} \
    } while (0)

//                        Assertion failed:
#define ERROR_NEW_LINE "\t                  "

void
setGroupCtx(void *ctx)
{
    group_ctx = ctx;
}

void
setShowColor(bool should_show_color)
{
    show_color = should_show_color;
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

    LOG_STRING("[INFO] ");

    va_start(args, format);
    vdprintf(log_fd, format, args);
    va_end(args);

    LOG_STRING("\n");
}

void
scrFail(SCR_CONTEXT_DECL, const char *format, ...)
{
    va_list args;

    if (show_color) {
        LOG_STRING(RED);
    }

    dprintf(log_fd, "[ERROR] On line %u of %s in %s:\n\t", line_no, function_name, file_name);

    va_start(args, format);
    vdprintf(log_fd, format, args);
    va_end(args);

    LOG_STRING("\n");

    if (show_color) {
        LOG_STRING(RESET_COLOR);
    }

    exit(SCR_TEST_CODE_FAIL);
}

SCR_ASSERT_FUNC(Eq, intmax_t)
{
    if (value1 != value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s == %s\n%s%ji == %ji", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(Neq, intmax_t)
{
    if (value1 == value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s != %s\n%s%ji != %ji", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(Lt, intmax_t)
{
    if (value1 >= value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s < %s\n%s%ji < %ji", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(Le, intmax_t)
{
    if (value1 > value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s <= %s\n%s%ji <= %ji", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(Gt, intmax_t)
{
    if (value1 <= value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s > %s\n%s%ji > %ji", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(Ge, intmax_t)
{
    if (value1 < value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s >= %s\n%s%ji >= %ji", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(EqUnsigned, uintmax_t)
{
    if (value1 != value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s == %s\n%s%ju == %ju", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(NeqUnsigned, uintmax_t)
{
    if (value1 == value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s != %s\n%s%ju != %ju", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(LtUnsigned, uintmax_t)
{
    if (value1 >= value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s < %s\n%s%ju < %ju", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(LeUnsigned, uintmax_t)
{
    if (value1 > value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s <= %s\n%s%ju <= %ju", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(GtUnsigned, uintmax_t)
{
    if (value1 <= value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s > %s\n%s%ju > %ju", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(GeUnsigned, uintmax_t)
{
    if (value1 < value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s >= %s\n%s%ju >= %ju", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(EqFloat, long double)
{
    if (value1 != value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s == %s\n%s%Lg == %Lg", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(NeqFloat, long double)
{
    if (value1 == value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s != %s\n%s%Lg != %Lg", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(LtFloat, long double)
{
    if (value1 >= value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s < %s\n%s%Lg < %Lg", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(LeFloat, long double)
{
    if (value1 > value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s <= %s\n%s%Lg <= %Lg", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(GtFloat, long double)
{
    if (value1 <= value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s > %s\n%s%Lg > %Lg", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(GeFloat, long double)
{
    if (value1 < value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s >= %s\n%s%Lg >= %Lg", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(PtrEq, const void *)
{
    if (value1 != value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s == %s\n%s%p == %p", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(PtrNeq, const void *)
{
    if (value1 == value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s != %s\n%s%p != %p", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(StrEq, const char *)
{
    if (strcmp(value1, value2) != 0) {
        scrFail(file_name, function_name, line_no,
                "Assertion failed: strcmp(%s, %s) == 0\n%sstrcmp(\"%s\", \"%s\") == 0", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(StrNeq, const char *)
{
    if (strcmp(value1, value2) == 0) {
        scrFail(file_name, function_name, line_no,
                "Assertion failed: strcmp(%s, %s) != 0\n%sstrcmp(\"%s\", \"%s\") != 0", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(StrBeginsWith, const char *)
{
    size_t len1, len2;

    len1 = strlen(value1);
    len2 = strlen(value2);

    if (len2 > len1 || memcmp(value1, value2, len2) != 0) {
        scrFail(file_name, function_name, line_no,
                "Assertion failed: %s starts with %s\n%s\"%s\" starts with \"%s\"", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(StrNBeginsWith, const char *)
{
    size_t len1, len2;

    len1 = strlen(value1);
    len2 = strlen(value2);

    if (len2 <= len1 && memcmp(value1, value2, len2) == 0) {
        scrFail(file_name, function_name, line_no,
                "Assertion failed: %s doesn't start with %s\n%s\"%s\" doesn't start with \"%s\"", expr1,
                expr2, ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(CharEq, char)
{
    if (value1 != value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s == %s\n%s'%c' == '%c'", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(CharNeq, char)
{
    if (value1 == value2) {
        scrFail(file_name, function_name, line_no, "Assertion failed: %s != %s\n%s'%c' != '%c'", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

void
scrAssertMemEq(SCR_CONTEXT_DECL, const void *ptr1, const char *expr1, const void *ptr2, const char *expr2,
               size_t size)
{
    const unsigned char *buffer1 = ptr1, *buffer2 = ptr2;

    for (size_t k = 0; k < size; k++) {
        if (buffer1[k] != buffer2[k]) {
            scrFail(file_name, function_name, line_no,
                    "Assertion failed: memcmp(%s, %s, %zu) == 0\n%sAt index %zu, %u != %u", expr1, expr2,
                    size, ERROR_NEW_LINE, k, buffer1[k], buffer2[k]);
        }
    }
}
