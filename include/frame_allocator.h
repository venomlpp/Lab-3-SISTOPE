#ifndef FRAME_ALLOCATOR_H
#define FRAME_ALLOCATOR_H

#include <stdbool.h>

void init_frame_allocator();
void free_frame_allocator();

// Intenta asignar un frame libre. Retorna el número de frame o -1 si no hay memoria.
int allocate_frame(); 

#endif // FRAME_ALLOCATOR_H