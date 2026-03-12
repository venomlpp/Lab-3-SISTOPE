#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

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
    bool use_dirty_pages; 
    
    int segments;
    int *seg_limits; 
    
    int pages;
    int frames;
    int page_size;
    int tlb_size;
    char tlb_policy[16];
    char evict_policy[16];
} sim_config_t;

// Estructura para métricas por hilo
typedef struct {
    int translations_ok;
    int segfaults;
} thread_stats_t;

extern sim_config_t config;

// Variables globales para métricas y sincronización
extern thread_stats_t *t_stats;
extern uint64_t global_translations_ok;
extern uint64_t global_segfaults;
extern pthread_mutex_t global_stats_mutex;

#endif // SIMULATOR_H