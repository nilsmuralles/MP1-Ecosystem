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

// Fase 1 (caza): se ejecuta antes que update_carnivore/update_herbivore. Si
// el carnívoro logra reclamar un herbívoro adyacente, se coloca directamente
// en next_grid[prey_r][prey_c] y marca hunted[row][col] para que
// update_carnivore no lo vuelva a procesar en la fase 2.
bool try_hunt_herbivore(int row, int col) {
    Cell c = grid[row][col];

    if (c.ticks_without_food >= CARNIVORE_STARVE_TICKS) return false;
    if (c.ticks_alive >= CARNIVORE_MAX_AGE) return false;

    int prey_r = -1, prey_c = -1;

    for (int i = 0; i < 8; i++) {
        int nr = row + DR[i];
        int nc = col + DC[i];
        if (nr < 0 || nr >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE) continue;

        if (grid[nr][nc].type == HERBIVORE) {
            prey_r = nr;
            prey_c = nc;
            break;
        }
    }

    if (prey_r == -1) return false;

    Cell fed = c;
    fed.ticks_alive++;
    fed.energy += CARNIVORE_HUNT_ENERGY_GAIN;
    fed.ticks_without_food = 0;

    if (try_claim_prey(prey_r, prey_c, fed)) {
        hunted[row][col] = true;
        return true;
    }
    return false;
}

void update_carnivore(int row, int col) {
    // Ya resuelto en la fase de caza (cazó con éxito).
    if (hunted[row][col]) {
        return;
    }

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

    // Buscar celdas vecinas vacías (ya no busca presa, eso se resolvió
    // en la fase 1 con try_hunt_herbivore).
    for (int i = 0; i < 8; i++) {
        int nr = row + DR[i];
        int nc = col + DC[i];
        if (nr < 0 || nr >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE) continue;

        if (grid[nr][nc].type == EMPTY) {
            empty_r[empty_count] = nr;
            empty_c[empty_count] = nc;
            empty_count++;
        }
    }

    unsigned int seed = cell_seed(row, col);

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
