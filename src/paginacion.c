#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <time.h>
#include "paginacion.h"
#include "frame_allocator.h"
#include "tlb.h"

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

/**
 * Traduce una dirección virtual a física utilizando la tabla de páginas.
 * Maneja los fallos de página simulando latencia de disco y gestionando la asignación
 * de marcos de memoria mediante el allocador global.
 */
bool translate_page(int thread_id, page_table_t *pt, virtual_addr_t va, uint64_t *pa) {
    if (va.id >= (uint64_t)pt->num_pages) return false;

    if (!pt->entries[va.id].valid) {
        struct timespec req = {0, 2000000}; 
        nanosleep(&req, NULL);

        int evicted_thread = -1;
        uint64_t evicted_vpn = 0;
        
        int frame = allocate_frame(thread_id, va.id, &evicted_thread, &evicted_vpn);
        
        if (frame != -1) {
            if (evicted_thread != -1) {
                if (config.use_dirty_pages && global_pts[evicted_thread]->entries[evicted_vpn].dirty) {
                    nanosleep(&req, NULL); 
                }

                global_pts[evicted_thread]->entries[evicted_vpn].valid = false;
                global_pts[evicted_thread]->entries[evicted_vpn].dirty = false;
                tlb_invalidate(global_tlbs[evicted_thread], evicted_vpn);
            }

            pt->entries[va.id].frame_number = frame;
            pt->entries[va.id].valid = true;
            
            if (config.use_dirty_pages) {
                pt->entries[va.id].dirty = va.is_write;
            }

        } else {
            return false;
        }
    } else {
        if (config.use_dirty_pages && va.is_write) {
            pt->entries[va.id].dirty = true;
        }
    }

    *pa = (pt->entries[va.id].frame_number * config.page_size) + va.offset;
    return true;
}