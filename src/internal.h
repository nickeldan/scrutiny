#ifndef SCRUTINY_INTERNAL_H
#define SCRUTINY_INTERNAL_H

#include <stdbool.h>

#define GREEN       "\x1b[0;32m"
#define YELLOW      "\x1b[0;33m"
#define RED         "\x1b[0;31m"
#define RESET_COLOR "\x1b[0m"

void
setGroupCtx(void *ctx);

void
setToTty(bool to_tty);

void
setLogFd(int fd);

#endif  // SCRUTINY_INTERNAL_H
