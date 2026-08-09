#include "ecosystem.h"

// TODO(Persona 2): reglas reales de reproducción, consumo y movimiento.
void update_plant(int row, int col) {
    next_grid[row][col] = grid[row][col];
}

// TODO(Persona 2): reglas reales de reproducción, consumo y movimiento.
void update_herbivore(int row, int col) {
    next_grid[row][col] = grid[row][col];
}
