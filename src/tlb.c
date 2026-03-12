#include <stdlib.h>
#include "tlb.h"
#include "simulator.h"

tlb_t* init_tlb() {
    tlb_t *tlb = malloc(sizeof(tlb_t));
    tlb->size = config.tlb_size;
    tlb->entries = calloc(config.tlb_size, sizeof(tlb_entry_t));
    tlb->fifo_ptr = 0;
    tlb->hits = 0;
    tlb->misses = 0;
    return tlb;
}

int tlb_lookup(tlb_t *tlb, uint64_t vpn, bool is_write) {
    for (int i = 0; i < tlb->size; i++) {
        if (tlb->entries[i].valid && tlb->entries[i].vpn == vpn) {
            tlb->hits++;
            // Si es un acierto y además es escritura, ensuciamos la página en caché
            if (is_write) tlb->entries[i].dirty = true;
            return tlb->entries[i].frame_number;
        }
    }
    tlb->misses++;
    return -1;
}

void tlb_insert(tlb_t *tlb, uint64_t vpn, int frame_number, bool is_write) {
    if (tlb->size == 0) return; // Evitamos división por cero
    tlb->entries[tlb->fifo_ptr].vpn = vpn;
    tlb->entries[tlb->fifo_ptr].frame_number = frame_number;
    tlb->entries[tlb->fifo_ptr].valid = true;
    tlb->entries[tlb->fifo_ptr].dirty = is_write;
    
    tlb->fifo_ptr = (tlb->fifo_ptr + 1) % tlb->size;
}

bool tlb_invalidate(tlb_t *tlb, uint64_t vpn) {
    for (int i = 0; i < tlb->size; i++) {
        if (tlb->entries[i].valid && tlb->entries[i].vpn == vpn) {
            tlb->entries[i].valid = false;
            return tlb->entries[i].dirty; // Retornamos si estaba sucia para avisar a la Page Table
        }
    }
    return false;
}

void free_tlb(tlb_t *tlb) {
    if (tlb) {
        if (tlb->entries) {
            free(tlb->entries);
        }
        free(tlb);
    }
}