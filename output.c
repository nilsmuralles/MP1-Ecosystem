#include <stdio.h>
#include <stdio.h>
#include "ecosystem.h"

#define RESULTS_HTML_FILENAME "results.html"

// Códigos ANSI para diferenciar especies en la terminal
#define ANSI_RESET   "\x1b[0m"
#define ANSI_PLANT   "\x1b[32m"  // verde
#define ANSI_HERB    "\x1b[33m"  // amarillo
#define ANSI_CARN    "\x1b[31m"  // rojo
#define ANSI_EMPTY   "\x1b[90m"  // gris

// Colores equivalentes en HTML
#define HTML_PLANT_COLOR "#2e7d32"
#define HTML_HERB_COLOR  "#f9a825"
#define HTML_CARN_COLOR  "#c62828"
#define HTML_EMPTY_COLOR "#9e9e9e"

static char cell_char(CellType t) {
    switch (t) {
        case PLANT:     return 'P';
        case HERBIVORE: return 'H';
        case CARNIVORE: return 'C';
        default:         return '.';
    }
}
 
static const char *ansi_color_for(CellType t) {
    switch (t) {
        case PLANT:     return ANSI_PLANT;
        case HERBIVORE: return ANSI_HERB;
        case CARNIVORE: return ANSI_CARN;
        default:         return ANSI_EMPTY;
    }
}

static void print_grid_distribution(FILE *stream) {
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            fprintf(stream, "%c ", cell_char(grid[r][c].type));
        }
        fprintf(stream, "\n");
    }
}

#if defined(_WIN32) || defined(__MINGW32__)
#include <windows.h>
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode)) {
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
}
#endif
static void print_grid_distribution_console(void) {
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            CellType t = grid[r][c].type;
            printf("%s%c%s ", ansi_color_for(t), cell_char(t), ANSI_RESET);
        }
        printf("\n");
    }
}

void print_ecosystem_state(int tick, PopulationCount pop) {
#if defined(_WIN32) || defined(__MINGW32__)
    static int ansi_ready = 0;
    if (!ansi_ready) {
        enable_ansi_on_windows();
        ansi_ready = 1;
    }
#endif
 
    printf("Tick %d - Plantas: %d, Herbivoros: %d, Carnivoros: %d\n",
           tick, pop.plants, pop.herbivores, pop.carnivores);
 
    if (tick == 1 || tick % CONSOLE_SNAPSHOT_INTERVAL == 0) {
        printf("Distribucion:\n");
        print_grid_distribution_console();
        printf("\n");
    }
}
 
// Escribe el <head> y estilos una sola vez, en el primer tick
static void write_html_header(FILE *f) {
    fprintf(f,
        "<!DOCTYPE html>\n<html lang=\"es\">\n<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<title>Resultados - Simulacion de Ecosistema</title>\n"
        "<style>\n"
        "  body { background:#111; color:#eee; font-family: monospace; }\n"
        "  h2 { color:#fff; border-top:1px solid #444; padding-top:10px; }\n"
        "  .grid { line-height:1.3; font-size:16px; letter-spacing:2px; }\n"
        "  .plant { color:%s; font-weight:bold; }\n"
        "  .herb  { color:%s; font-weight:bold; }\n"
        "  .carn  { color:%s; font-weight:bold; }\n"
        "  .empty { color:%s; }\n"
        "  .counts { margin-bottom:6px; }\n"
        "</style>\n</head>\n<body>\n"
        "<h1>Simulacion de Ecosistema con OpenMP - Resultados</h1>\n"
        "<p>P: Plantas (verde), H: Herbivoros (amarillo), C: Carnivoros (rojo)</p>\n",
        HTML_PLANT_COLOR, HTML_HERB_COLOR, HTML_CARN_COLOR, HTML_EMPTY_COLOR);
}
 
static const char *html_class_for(CellType t) {
    switch (t) {
        case PLANT:     return "plant";
        case HERBIVORE: return "herb";
        case CARNIVORE: return "carn";
        default:         return "empty";
    }
}
 
static void append_tick_to_html(int tick, PopulationCount pop) {
    // El header solo se escribe una vez al inicio de la simulación
    FILE *f = fopen(RESULTS_HTML_FILENAME, (tick == 1) ? "w" : "a");
    if (!f) return;
 
    if (tick == 1) {
        write_html_header(f);
    }
 
    fprintf(f, "<h2>Tick %d</h2>\n", tick);
    fprintf(f, "<div class=\"counts\">Plantas: %d | Herbivoros: %d | Carnivoros: %d</div>\n",
            pop.plants, pop.herbivores, pop.carnivores);
    fprintf(f, "<div class=\"grid\">\n");
    for (int r = 0; r < GRID_SIZE; r++) {
        for (int c = 0; c < GRID_SIZE; c++) {
            CellType t = grid[r][c].type;
            fprintf(f, "<span class=\"%s\">%c</span>", html_class_for(t), cell_char(t));
        }
        fprintf(f, "<br>\n");
    }
    fprintf(f, "</div>\n");
 
    fclose(f);
}
 
// Guarda en el archivo de resultados el estado COMPLETO del ecosistema en cada tick.
void save_results_to_file(const char *filename, int tick, PopulationCount pop) {
    FILE *f = fopen(filename, "a");
    if (f) {
        fprintf(f, "Tick %d\n", tick);
        fprintf(f, "Plantas: %d\n", pop.plants);
        fprintf(f, "Herbivoros: %d\n", pop.herbivores);
        fprintf(f, "Carnivoros: %d\n", pop.carnivores);
        fprintf(f, "Distribucion:\n");
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                fprintf(f, "%c ", cell_char(grid[r][c].type));
            }
            fprintf(f, "\n");
        }
        fprintf(f, "\n");
        fclose(f);
    }
 
    append_tick_to_html(tick, pop);
}