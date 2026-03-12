#ifndef FRAME_ALLOCATOR_H
#define FRAME_ALLOCATOR_H

#include <stdbool.h>
#include <stdint.h>

void init_frame_allocator();
void free_frame_allocator();

// Intenta asignar un frame libre. Retorna el número de frame o -1 si no hay memoria.
int allocate_frame(int thread_id, uint64_t vpn, int *evicted_thread, uint64_t *evicted_vpn); 

#endif // FRAME_ALLOCATOR_H