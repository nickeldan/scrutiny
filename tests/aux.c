#include <sys/types.h>
#include <unistd.h>

pid_t
indirectGetPpid(void)
#ifdef __GNUC__
    __attribute__((visibility("default")))
#endif
    ;

pid_t
indirectGetPpid(void)
{
    return getppid();
}