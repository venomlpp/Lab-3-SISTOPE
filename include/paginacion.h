#ifndef PAGINACION_H
#define PAGINACION_H

#include <stdint.h>
#include <stdbool.h>
#include "simulator.h"
#include "workloads.h"

// Entrada de la tabla de páginas (PTE)
typedef struct {
    int frame_number;
    bool valid; // true si está en memoria (frame asignado)
} page_table_entry_t;

// Tabla de páginas por hilo
typedef struct {
    page_table_entry_t *entries;
    int num_pages;
} page_table_t;

page_table_t* init_page_table();
void free_page_table(page_table_t *pt);
bool translate_page(page_table_t *pt, virtual_addr_t va, uint64_t *pa);

#endif // PAGINACION_H