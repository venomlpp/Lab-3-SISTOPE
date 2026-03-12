#include <stdlib.h>
#include "paginacion.h"
#include "frame_allocator.h"

page_table_t* init_page_table() {
    page_table_t *pt = malloc(sizeof(page_table_t));
    pt->num_pages = config.pages;
    // calloc inicializa 'valid' en false para todas las entradas
    pt->entries = calloc(config.pages, sizeof(page_table_entry_t));
    return pt;
}

void free_page_table(page_table_t *pt) {
    if (pt) {
        free(pt->entries);
        free(pt);
    }
}

// Retorna true si logró traducir, false si ocurrió un fallo (ej. Out of Memory)
bool translate_page(page_table_t *pt, virtual_addr_t va, uint64_t *pa) {
    if (va.id >= (uint64_t)pt->num_pages) return false;

    // ¿La página no está en memoria física? (Page Fault básico)
    if (!pt->entries[va.id].valid) {
        int frame = allocate_frame();
        if (frame != -1) {
            pt->entries[va.id].frame_number = frame;
            pt->entries[va.id].valid = true;
        } else {
            // No hay frames libres (aún no tenemos eviction/reemplazo)
            return false; 
        }
    }

    // Traducción: PA = (Frame * PageSize) + Offset
    *pa = (pt->entries[va.id].frame_number * config.page_size) + va.offset;
    return true;
}