#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdint.h>
#include <stdbool.h>

typedef enum { MODE_NONE, MODE_SEG, MODE_PAGE } sim_mode_t;
typedef enum { WORKLOAD_UNIFORM, WORKLOAD_80_20 } workload_t;

// Estructura global de configuración
typedef struct {
    sim_mode_t mode;
    int threads;
    int ops_per_thread;
    workload_t workload;
    int seed;
    bool unsafe;
    bool stats;
    
    // Específicos de Segmentación
    int segments;
    int *seg_limits; // Arreglo dinámico para los límites
    
    // Específicos de Paginación
    int pages;
    int frames;
    int page_size;
    int tlb_size;
    char tlb_policy[16];
    char evict_policy[16];
} sim_config_t;

// Variable global de configuración
extern sim_config_t config;

#endif // SIMULATOR_H