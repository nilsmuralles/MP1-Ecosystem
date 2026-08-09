#include <stdlib.h>
#include "ecosystem.h"

int main(int argc, char *argv[]) {
    int num_threads = (argc > 1) ? atoi(argv[1]) : 4;

    init_ecosystem();
    run_simulation(MAX_TICKS, num_threads);

    return 0;
}
