#ifndef ECOSYSTEM_H
#define ECOSYSTEM_H

#include <stdbool.h>

#define GRID_SIZE 20
#define MAX_TICKS 50

#define PLANT_REPRO_PROB 0.30
#define HERBIVORE_REPRO_ENERGY 5
#define CARNIVORE_REPRO_ENERGY 8
#define HERBIVORE_STARVE_TICKS 3
#define CARNIVORE_STARVE_TICKS 5

#define INITIAL_PLANTS 150
#define INITIAL_HERBIVORES 40
#define INITIAL_CARNIVORES 15

typedef enum {
  EMPTY = 0,
  PLANT,
  HERBIVORE,
  CARNIVORE
} CellType;

typedef struct {
  CellType type;
  int energy;
  int ticks_without_food;
  int ticks_alive;
} Cell;

extern Cell grid[GRID_SIZE][GRID_SIZE];
extern Cell next_grid[GRID_SIZE][GRID_SIZE];

typedef struct {
  int plants;
  int herbivores;
  int carnivores;
} PopulationCount;

void init_ecosystem(void);
void run_simulation(int num_ticks, int num_threads);
void swap_grids(void);
PopulationCount count_population(void);

bool try_place_in_next_grid(int row, int col, Cell organism);

void update_plant(int row, int col);
void update_herbivore(int row, int col);

void update_carnivore(int row, int col);
void print_ecosystem_state(int tick, PopulationCount pop);
void save_results_to_file(const char *filename, int tick, PopulationCount pop);

#endif
