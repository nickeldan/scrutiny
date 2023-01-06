#ifndef SCRUTINY_INTERNAL_H
#define SCRUTINY_INTERNAL_H

#include <stdbool.h>
#include <sys/types.h>

#define GREEN       "\x1b[0;32m"
#define YELLOW      "\x1b[0;33m"
#define RED         "\x1b[0;31m"
#define RESET_COLOR "\x1b[0m"

void
setGroupCtx(void *ctx);

void
setShowColor(bool should_show_color);

void
setLogFd(int fd);

void
waitForProcess(pid_t pid, int *status);

#endif  // SCRUTINY_INTERNAL_H
