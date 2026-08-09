#include <stdlib.h>
#include <omp.h>
#include "ecosystem.h"

// Edad máxima de un herbívoro (muerte por vejez).
#define HERBIVORE_MAX_AGE 25
 
// Costo de energía al reproducirse (se resta al padre y el hijo nace con la energía inicial de un herbívoro recién nacido).
#define HERBIVORE_REPRO_COST 3
#define HERBIVORE_NEWBORN_ENERGY 3
 
// Desplazamientos para las 8 celdas vecinas (vecindad de Moore).
static const int DR[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
static const int DC[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
 
// Genera una semilla distinta por celda/hilo para poder usar rand_r() de forma segura dentro de las regiones paralelas.
static inline unsigned int cell_seed(int row, int col) {
    return (unsigned int)(row * GRID_SIZE + col + 1) * 2654435761u
           ^ (unsigned int)omp_get_thread_num();
}

// PLANTAS

void update_plant(int row, int col) {
    Cell p = grid[row][col];
    p.ticks_alive++;
 
    int empty_r[8], empty_c[8], empty_count = 0;
    int plant_neighbors = 0, total_neighbors = 0;
 
    for (int i = 0; i < 8; i++) {
        int nr = row + DR[i];
        int nc = col + DC[i];
        if (nr < 0 || nr >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE) continue;
 
        total_neighbors++;
        CellType t = grid[nr][nc].type;
        if (t == EMPTY) {
            empty_r[empty_count] = nr;
            empty_c[empty_count] = nc;
            empty_count++;
        } else if (t == PLANT) {
            plant_neighbors++;
        }
    }
 
    // Muerte por sobrepoblación: rodeada completamente por otras plantas.
    if (total_neighbors > 0 && plant_neighbors == total_neighbors) {
        return;
    }
 
    // La planta se queda en su propia celda
    next_grid[row][col] = p;
 
    // Intento de reproducción hacia una celda vecina vacía.
    if (empty_count > 0) {
        unsigned int seed = cell_seed(row, col);
        double roll = (double)rand_r(&seed) / ((double)RAND_MAX + 1.0);
        if (roll < PLANT_REPRO_PROB) {
            int idx = rand_r(&seed) % empty_count;
            Cell sprout = {PLANT, 0, 0, 0};
            try_place_in_next_grid(empty_r[idx], empty_c[idx], sprout);
        }
    }
}
 
// HERBÍVOROS

void update_herbivore(int row, int col) {
    Cell h = grid[row][col];
 
    // Muerte por inanición: ya acumuló demasiados ticks sin comer.
    if (h.ticks_without_food >= HERBIVORE_STARVE_TICKS) {
        return; // next_grid[row][col] queda EMPTY
    }
    // Muerte por vejez.
    if (h.ticks_alive >= HERBIVORE_MAX_AGE) {
        return;
    }
 
    h.ticks_alive++;
 
    int empty_r[8], empty_c[8], empty_count = 0;
    int food_r = -1, food_c = -1;
    int predator_nearby = 0;
 
    for (int i = 0; i < 8; i++) {
        int nr = row + DR[i];
        int nc = col + DC[i];
        if (nr < 0 || nr >= GRID_SIZE || nc < 0 || nc >= GRID_SIZE) continue;
 
        CellType t = grid[nr][nc].type;
        if (t == EMPTY) {
            empty_r[empty_count] = nr;
            empty_c[empty_count] = nc;
            empty_count++;
        } else if (t == PLANT && food_r == -1) {
            food_r = nr;
            food_c = nc;
        } else if (t == CARNIVORE) {
            predator_nearby = 1;
        }
    }
 
    unsigned int seed = cell_seed(row, col);
 
    // 1. Huir de un depredador adyacente, si hay a dónde escapar.
    if (predator_nearby && empty_count > 0) {
        int idx = rand_r(&seed) % empty_count;
        h.ticks_without_food++;
        if (try_place_in_next_grid(empty_r[idx], empty_c[idx], h)) {
            return;
        }
        next_grid[row][col] = h;
        return;
    }
 
    // 2. Comer una planta adyacente (movimiento hacia esa celda).
    if (food_r != -1) {
        Cell fed = h;
        fed.energy += 1;
        fed.ticks_without_food = 0;
        if (try_place_in_next_grid(food_r, food_c, fed)) {
            return;
        }
        h.ticks_without_food++;
        next_grid[row][col] = h;
        return;
    }

    h.ticks_without_food++;
 
    // 3- Reproducirse si tiene suficiente energía y hay espacio libre.
    if (h.energy >= HERBIVORE_REPRO_ENERGY && empty_count > 0) {
        int idx = rand_r(&seed) % empty_count;
        Cell offspring = {HERBIVORE, HERBIVORE_NEWBORN_ENERGY, 0, 0};
        if (try_place_in_next_grid(empty_r[idx], empty_c[idx], offspring)) {
            h.energy -= HERBIVORE_REPRO_COST;
        }
        next_grid[row][col] = h;
        return;
    }
 
    // 4. Deambular ocasionalmente en busca de comida.
    if (empty_count > 0 && (rand_r(&seed) % 100) < 30) {
        int idx = rand_r(&seed) % empty_count;
        if (try_place_in_next_grid(empty_r[idx], empty_c[idx], h)) {
            return;
        }
    }
 
    // 5. Por defecto, se queda en su celda.
    next_grid[row][col] = h;
}