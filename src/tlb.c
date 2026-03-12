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

void free_tlb(tlb_t *tlb) {
    if (tlb) {
        free(tlb->entries);
        free(tlb);
    }
}

// Retorna el frame si hay hit, o -1 si hay miss
int tlb_lookup(tlb_t *tlb, uint64_t vpn) {
    for (int i = 0; i < tlb->size; i++) {
        if (tlb->entries[i].valid && tlb->entries[i].vpn == vpn) {
            tlb->hits++;
            return tlb->entries[i].frame_number;
        }
    }
    tlb->misses++;
    return -1;
}

// Inserta o reemplaza según FIFO circular
void tlb_insert(tlb_t *tlb, uint64_t vpn, int frame_number) {
    tlb->entries[tlb->fifo_ptr].vpn = vpn;
    tlb->entries[tlb->fifo_ptr].frame_number = frame_number;
    tlb->entries[tlb->fifo_ptr].valid = true;
    
    // Avanzar puntero FIFO
    tlb->fifo_ptr = (tlb->fifo_ptr + 1) % tlb->size;
}

// Para cuando el sistema global expulsa (evict) una página
void tlb_invalidate(tlb_t *tlb, uint64_t vpn) {
    for (int i = 0; i < tlb->size; i++) {
        if (tlb->entries[i].valid && tlb->entries[i].vpn == vpn) {
            tlb->entries[i].valid = false;
            break;
        }
    }
}