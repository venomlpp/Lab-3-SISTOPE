#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdint.h>
#include <stdbool.h>

typedef enum { MODE_NONE, MODE_SEG, MODE_PAGE } sim_mode_t;
typedef enum { WORKLOAD_UNIFORM, WORKLOAD_80_20 } workload_t;

// Representa una dirección virtual dividida (Sirve para Seg y Page)
typedef struct {
    uint64_t id;      // Representa el seg_id o el VPN
    uint64_t offset;  // El desplazamiento
} virtual_addr_t;

// Estructura global de configuración
typedef struct {
    sim_mode_t mode;
    int threads;
    int ops_per_thread;
    workload_t workload;
    int seed;
    bool unsafe;
    bool stats;
    
    int segments;
    int *seg_limits; 
    
    int pages;
    int frames;
    int page_size;
    int tlb_size;
    char tlb_policy[16];
    char evict_policy[16];
} sim_config_t;

extern sim_config_t config;

// Prototipo de generación de direcciones (implementado en workloads.c)
virtual_addr_t generate_address(unsigned int *seedp);

#endif // SIMULATOR_H