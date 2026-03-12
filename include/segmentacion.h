#ifndef SEGMENTACION_H
#define SEGMENTACION_H

#include <stdint.h>
#include <stdbool.h>
#include "simulator.h"

// Estructura de la entrada de segmento
typedef struct {
    uint64_t base;  // Dirección física base
    uint64_t limit; // Tamaño máximo del segmento
} segment_entry_t;

// Estructura de la tabla de segmentos
typedef struct {
    segment_entry_t *segments;
    int num_segments;
} segment_table_t;

// Prototipos
segment_table_t* init_segment_table();
void free_segment_table(segment_table_t *table);
bool translate_segment(segment_table_t *table, virtual_addr_t va, uint64_t *pa);

#endif // SEGMENTACION_H