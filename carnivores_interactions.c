#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "ecosystem.h"

// Edad máxima de un carnívoro
#define CARNIVORE_MAX_AGE 20

// Costo de energía al reproducirse
#define CARNIVORE_REPRO_COST 4
#define CARNIVORE_NEWBORN_ENERGY 4

// Ganancia de energía al cazar un herbívoro
#define CARNIVORE_HUNT_ENERGY_GAIN 2

// Desplazamientos para las 8 celdas vecinas
static const int DR[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
static const int DC[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

// Semilla distinta por celda/hilo para usar rand_r()
static inline unsigned int cell_seed(int row, int col) {
    return (unsigned int)(row * GRID_SIZE + col + 1) * 2654435761u
           ^ (unsigned int)omp_get_thread_num();
}

// CARNÍVOROS
void update_carnivore(int row, int col) {
    Cell c = grid[row][col];

    // Muerte por inanición
    if (c.ticks_without_food >= CARNIVORE_STARVE_TICKS) {
        return;
    }

    // Muerte por vejez
    if (c.ticks_alive >= CARNIVORE_MAX_AGE) {
        return;
    }

    c.ticks_alive++;

    int empty_r[8], empty_c[8], empty_count = 0;
    int prey_r = -1, prey_c = -1;

    // Buscar celdas vecinas vacías y herbívoros adyacentes
    // para cazar
    for (int i = 0; i < 8; i++) {
        int nr = row + DR[i];
        int nc = col + DC[i];
        if (nr < 0 || nr >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE) continue;

        CellType t = grid[nr][nc].type;
        if (t == EMPTY) {
            empty_r[empty_count] = nr;
            empty_c[empty_count] = nc;
            empty_count++;
        } else if (t == HERBIVORE && prey_r == -1) {
            prey_r = nr;
            prey_c = nc;
        }
    }

    unsigned int seed = cell_seed(row, col);

    // Cazar, moverse hacia un herbívoro adyacente y consumirlo
    if (prey_r != -1) {
        Cell fed = c;
        fed.energy += CARNIVORE_HUNT_ENERGY_GAIN;
        fed.ticks_without_food = 0;

        if (try_place_in_next_grid(prey_r, prey_c, fed)) {
            return;
        }
        c.ticks_without_food++;
        next_grid[row][col] = c;
        return;
    }

    c.ticks_without_food++;

    // Reproducirse si tiene suficiente energía y hay espacio libre
    if (c.energy >= CARNIVORE_REPRO_ENERGY && empty_count > 0) {
        int idx = rand_r(&seed) % empty_count;
        Cell offspring = {CARNIVORE, CARNIVORE_NEWBORN_ENERGY, 0, 0};
        if (try_place_in_next_grid(empty_r[idx], empty_c[idx], offspring)) {
            c.energy -= CARNIVORE_REPRO_COST;
        }
        next_grid[row][col] = c;
        return;
    }

    // Deambular ocasionalmente en busca de presas
    if (empty_count > 0 && (rand_r(&seed) % 100) < 30) {
        int idx = rand_r(&seed) % empty_count;
        if (try_place_in_next_grid(empty_r[idx], empty_c[idx], c)) {
            return;
        }
    }

    // Por defecto se queda en su celda
    next_grid[row][col] = c;
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