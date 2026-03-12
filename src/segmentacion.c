#include <stdlib.h>
#include "segmentacion.h"

segment_table_t* init_segment_table() {
    segment_table_t *table = malloc(sizeof(segment_table_t));
    table->num_segments = config.segments;
    table->segments = malloc(sizeof(segment_entry_t) * config.segments);

    for (int i = 0; i < config.segments; i++) {
        table->segments[i].base = 10000 * (i + 1); 
        
        if (config.seg_limits != NULL) {
            table->segments[i].limit = config.seg_limits[i];
        } else {
            table->segments[i].limit = 4096;
        }
    }
    return table;
}

void free_segment_table(segment_table_t *table) {
    if (table) {
        free(table->segments);
        free(table);
    }
}

/**
 * Realiza la traducción de una dirección en el esquema de segmentación.
 * Valida de forma estricta que la longitud del offset sea menor al límite.
 */
bool translate_segment(segment_table_t *table, virtual_addr_t va, uint64_t *pa) {
    if (va.id >= (uint64_t)table->num_segments) return false; 

    segment_entry_t entry = table->segments[va.id];
    
    if (va.offset >= entry.limit) {
        return false;
    }

    *pa = entry.base + va.offset;
    return true;
}