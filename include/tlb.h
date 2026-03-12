#ifndef TLB_H
#define TLB_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t vpn;
    int frame_number;
    bool valid;
} tlb_entry_t;

typedef struct {
    tlb_entry_t *entries;
    int size;
    int fifo_ptr; // Puntero para la política de reemplazo FIFO
    int hits;
    int misses;
} tlb_t;

tlb_t* init_tlb();
void free_tlb(tlb_t *tlb);
int tlb_lookup(tlb_t *tlb, uint64_t vpn);
void tlb_insert(tlb_t *tlb, uint64_t vpn, int frame_number);
void tlb_invalidate(tlb_t *tlb, uint64_t vpn);

#endif // TLB_H