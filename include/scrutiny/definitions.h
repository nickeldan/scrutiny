#ifndef SCRUTINY_DEFINITIONS_H
#define SCRUTINY_DEFINITIONS_H

#ifdef __GNUC__

#define SCR_EXPORT       __attribute__((visibility("default")))
#define SCR_MALLOC       __attribute__((malloc))
#define SCR_NONNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#define SCR_PURE         __attribute__((pure))
#define SCR_PRINTF(pos)  __attribute__((format(printf, pos, pos + 1)))
#define SCR_NORETURN     __attribute__((noreturn))

#else

#define SCR_EXPORT
#define SCR_MALLOC
#define SCR_NONNULL(...)
#define SCR_PURE
#define SCR_PRINTF(pos)
#define SCR_NORETURN

#endif

#endif  // SCRUTINY_DEFINITIONS_H
