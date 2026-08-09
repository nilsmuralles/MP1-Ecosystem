#include <stdio.h>
#include "ecosystem.h"

// TODO(Persona 3): reglas reales de caza, reproducción y movimiento.
void update_carnivore(int row, int col) {
    next_grid[row][col] = grid[row][col];
}

void print_ecosystem_state(int tick, PopulationCount pop) {
    printf("Tick %d - Plantas: %d, Herbivoros: %d, Carnivoros: %d\n",
           tick, pop.plants, pop.herbivores, pop.carnivores);
}

void save_results_to_file(const char *filename, int tick, PopulationCount pop) {
    FILE *f = fopen(filename, "a");
    if (!f) return;
    fprintf(f, "Tick %d - Plantas: %d, Herbivoros: %d, Carnivoros: %d\n",
            tick, pop.plants, pop.herbivores, pop.carnivores);
    fclose(f);
}
