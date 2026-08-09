#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>
#include "ecosystem.h"

Cell grid[GRID_SIZE][GRID_SIZE];
Cell next_grid[GRID_SIZE][GRID_SIZE];

static void place_random(CellType type, int count, int energy) {
    int placed = 0;
    while (placed < count) {
        int r = rand() % GRID_SIZE;
        int c = rand() % GRID_SIZE;
        if (grid[r][c].type == EMPTY) {
            grid[r][c] = (Cell){type, energy, 0, 0};
            placed++;
        }
    }
}

void init_ecosystem(void) {
    srand((unsigned int)time(NULL));

    for (int r = 0; r < GRID_SIZE; r++)
        for (int c = 0; c < GRID_SIZE; c++)
            grid[r][c] = (Cell){EMPTY, 0, 0, 0};

    place_random(PLANT, INITIAL_PLANTS, 0);
    place_random(HERBIVORE, INITIAL_HERBIVORES, 3);
    place_random(CARNIVORE, INITIAL_CARNIVORES, 5);
}

bool try_place_in_next_grid(int row, int col, Cell organism) {
    bool placed = false;
    #pragma omp critical(next_grid_write)
    {
        if (next_grid[row][col].type == EMPTY) {
            next_grid[row][col] = organism;
            placed = true;
        }
    }
    return placed;
}

void swap_grids(void) {
    memcpy(grid, next_grid, sizeof(grid));
}

PopulationCount count_population(void) {
    int plants = 0, herbivores = 0, carnivores = 0;

    #pragma omp parallel for collapse(2) reduction(+:plants,herbivores,carnivores)
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            switch (grid[r][c].type) {
                case PLANT:     plants++;     break;
                case HERBIVORE: herbivores++; break;
                case CARNIVORE: carnivores++; break;
                default: break;
            }
        }
    }

    return (PopulationCount){plants, herbivores, carnivores};
}

void run_simulation(int num_ticks, int num_threads) {
    for (int tick = 1; tick <= num_ticks; tick++) {
        double start = omp_get_wtime();

        #pragma omp parallel for collapse(2) num_threads(num_threads)
        for (int r = 0; r < GRID_SIZE; r++)
            for (int c = 0; c < GRID_SIZE; c++)
                next_grid[r][c] = (Cell){EMPTY, 0, 0, 0};

        #pragma omp parallel for collapse(2) schedule(dynamic) num_threads(num_threads)
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                switch (grid[r][c].type) {
                    case PLANT: update_plant(r, c); break;
                    case HERBIVORE: update_herbivore(r, c); break;
                    case CARNIVORE: update_carnivore(r, c); break;
                    default: break;
                }
            }
        }

        double elapsed = omp_get_wtime() - start;

        swap_grids();

        PopulationCount pop = count_population();
        print_ecosystem_state(tick, pop);
        save_results_to_file("results.txt", tick, pop);
        printf("[tick %d] %d hilos, %.6f s\n", tick, num_threads, elapsed);
    }
}
