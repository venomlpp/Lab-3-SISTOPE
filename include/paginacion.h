#ifndef PAGINACION_H
#define PAGINACION_H

#include <stdint.h>
#include <stdbool.h>
#include "simulator.h"
#include "workloads.h"

typedef struct {
    int frame_number;
    bool valid;
    bool dirty;
} page_table_entry_t;

typedef struct {
    page_table_entry_t *entries;
    int num_pages;
} page_table_t;

page_table_t* init_page_table();
void free_page_table(page_table_t *pt);
// Nueva firma que recibe el ID del hilo
bool translate_page(int thread_id, page_table_t *pt, virtual_addr_t va, uint64_t *pa);

#endif // PAGINACION_H