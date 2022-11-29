#ifndef SCRUTINY_INTERNAL_H
#define SCRUTINY_INTERNAL_H

#include <stdbool.h>

#define GREEN       "\x1b[0;32m"
#define YELLOW      "\x1b[0;33m"
#define RED         "\033[31m"
#define RESET_COLOR "\x1b[0m"

void
setGroupCtx(void *ctx)
#ifdef __GNUC__
    __attribute__((visibility("hidden")))
#endif
    ;

void
setToTty(bool to_tty)
#ifdef __GNUC__
    __attribute__((visibility("hidden")))
#endif
    ;

void
setLogFd(int fd)
#ifdef __GNUC__
    __attribute__((visibility("hidden")))
#endif
    ;

#endif  // SCRUTINY_INTERNAL_H
