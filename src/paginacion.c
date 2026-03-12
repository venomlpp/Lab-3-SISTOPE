#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <time.h>
#include "paginacion.h"
#include "frame_allocator.h"
#include "tlb.h"

// Referencias a los arreglos globales (definidos en simulator.c)
extern page_table_t **global_pts;
extern tlb_t **global_tlbs;

page_table_t* init_page_table() {
    page_table_t *pt = malloc(sizeof(page_table_t));
    pt->num_pages = config.pages;
    pt->entries = calloc(config.pages, sizeof(page_table_entry_t));
    return pt;
}

void free_page_table(page_table_t *pt) {
    if (pt) {
        free(pt->entries);
        free(pt);
    }
}

bool translate_page(int thread_id, page_table_t *pt, virtual_addr_t va, uint64_t *pa) {
    if (va.id >= (uint64_t)pt->num_pages) return false;

    if (!pt->entries[va.id].valid) {
        // PAGE FAULT: Simular latencia de ir a disco (2 milisegundos)
        struct timespec req = {0, 2000000}; 
        nanosleep(&req, NULL);

        int evicted_thread = -1;
        uint64_t evicted_vpn = 0;
        
        // Asignar frame (esto podría expulsar la página de alguien más por FIFO)
        int frame = allocate_frame(thread_id, va.id, &evicted_thread, &evicted_vpn);
        
        if (frame != -1) {
            // Si se expulsó a alguien, invalidar su Tabla de Páginas y su TLB
            if (evicted_thread != -1) {
                global_pts[evicted_thread]->entries[evicted_vpn].valid = false;
                tlb_invalidate(global_tlbs[evicted_thread], evicted_vpn);
            }

            pt->entries[va.id].frame_number = frame;
            pt->entries[va.id].valid = true;
        } else {
            return false; // Out of memory total (No debería pasar gracias al Eviction)
        }
    }

    *pa = (pt->entries[va.id].frame_number * config.page_size) + va.offset;
    return true;
}