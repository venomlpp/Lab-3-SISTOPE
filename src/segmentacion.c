#include <stdlib.h>
#include "segmentacion.h"

segment_table_t* init_segment_table() {
    segment_table_t *table = malloc(sizeof(segment_table_t));
    table->num_segments = config.segments;
    table->segments = malloc(sizeof(segment_entry_t) * config.segments);

    // Asignamos una base simulada (ej. 10000, 20000...) y los límites
    for (int i = 0; i < config.segments; i++) {
        table->segments[i].base = 10000 * (i + 1); 
        
        // Si el usuario pasó una lista de límites, la usamos. Si no, default 4096.
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

// Retorna true si fue exitoso (y guarda la dir física en pa), false si hay segfault
bool translate_segment(segment_table_t *table, virtual_addr_t va, uint64_t *pa) {
    // Seguridad: Que el segmento exista
    if (va.id >= (uint64_t)table->num_segments) return false; 

    segment_entry_t entry = table->segments[va.id];
    
    // Validación obligatoria: offset < limit
    if (va.offset >= entry.limit) {
        return false; // ¡Segfault!
    }

    // PA = base + offset
    *pa = entry.base + va.offset;
    return true;
}