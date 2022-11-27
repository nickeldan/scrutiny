#ifndef SCRUTINY_INTERNAL_H
#define SCRUTINY_INTERNAL_H

void
setParams(void *ctx, int fd)
#ifdef __GNUC__
    __attribute__((visibility("hidden")))
#endif
    ;

#endif  // SCRUTINY_INTERNAL_H
