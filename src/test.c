#include <ctype.h>
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

static char *
displayChar(char c, char *dst)
{
    if (isprint(c)) {
        dst[0] = c;
    }
    else if (c == '\0' || c == '\t' || c == '\n' || c == '\r') {
        dst[0] = '\\';
        switch (c) {
        case '\0': dst[1] = '0'; break;
        case '\t': dst[1] = 't'; break;
        case '\n': dst[1] = 'n'; break;
        default: dst[1] = 'r'; break;
        }
    }
    else {
        sprintf(dst, "\\x%02x", (int)c);
    }

    return dst;
}

static const char *
getBaseFileName(const char *file_name)
{
    size_t idx;

    for (idx = strlen(file_name) - 1; idx > 0; idx--) {
        if (file_name[idx] == '/') {
            idx++;
            goto found_slash;
        }
    }

    if (file_name[0] == '/') {
        idx = 1;
    }

found_slash:
    return file_name + idx;
}

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
scrTestSkip(void)
{
    exit(SCR_TEST_CODE_SKIP);
}

void
scrLog(SCR_CONTEXT_DECL, const char *format, ...)
{
    va_list args;

    dprintf(log_fd, "[%sINFO%s] (%s:%s:%u) ", show_color ? GREEN : "", show_color ? RESET_COLOR : "",
            getBaseFileName(file_name), function_name, line_no);

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

    dprintf(log_fd, "[ERROR] On line %u of %s in %s:\n\t", line_no, function_name,
            getBaseFileName(file_name));

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

size_t
scrAssertStrContains(SCR_CONTEXT_DECL, const char *value1, const char *expr1, const char *value2,
                     const char *expr2)
{
    char *loc;

    loc = strstr(value1, value2);
    if (!loc) {
        scrFail(file_name, function_name, line_no,
                "Assertion failed: %s contains %s\n%s\"%s\" contains \"%s\"", expr1, expr2, ERROR_NEW_LINE,
                value1, value2);
    }
    return loc - value1;
}

SCR_ASSERT_FUNC(StrNContains, const char *)
{
    if (strstr(value1, value2)) {
        scrFail(file_name, function_name, line_no,
                "Assertion failed: %s doesn't contain %s\n%s\"%s\" doesn't contain \"%s\"", expr1, expr2,
                ERROR_NEW_LINE, value1, value2);
    }
}

SCR_ASSERT_FUNC(CharEq, char)
{
    if (value1 != value2) {
        char display1[5] = {0}, display2[5] = {0};

        scrFail(file_name, function_name, line_no, "Assertion failed: %s == %s\n%s'%s' == '%s'", expr1, expr2,
                ERROR_NEW_LINE, displayChar(value1, display1), displayChar(value2, display2));
    }
}

SCR_ASSERT_FUNC(CharNeq, char)
{
    if (value1 == value2) {
        char display1[5] = {0}, display2[5] = {0};

        scrFail(file_name, function_name, line_no, "Assertion failed: %s != %s\n%s'%s' != '%s'", expr1, expr2,
                ERROR_NEW_LINE, displayChar(value1, display1), displayChar(value2, display2));
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
                    "Assertion failed: memcmp(%s, %s, %zu) == 0\n%sAt index %zu, 0x%02x != 0x%02x", expr1,
                    expr2, size, ERROR_NEW_LINE, k, buffer1[k], buffer2[k]);
        }
    }
}
