#include <stdio.h>

#include <scrutiny/scrutiny.h>

int
main(int argc, char **argv)
{
    scrStats stats;
    (void)argc;

    printf("\nRunning %s\n\n", argv[0]);

    scrGroupCreate(NULL, NULL);

    scrRun(NULL, &stats);

    return (stats.num_passed != 0 || stats.num_skipped != 0 || stats.num_failed != 0 ||
            stats.num_errored != 0);
}
