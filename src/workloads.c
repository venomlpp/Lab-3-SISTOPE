#include <stdlib.h>
#include "simulator.h"

static uint64_t get_rand_range(unsigned int *seedp, uint64_t max) {
    if (max == 0) return 0;
    return rand_r(seedp) % max;
}

virtual_addr_t generate_address(unsigned int *seedp) {
    virtual_addr_t addr = {0, 0};
    uint64_t max_id = (config.mode == MODE_SEG) ? config.segments : config.pages;
    
    bool use_hotspot = false;
    if (config.workload == WORKLOAD_80_20) {
        use_hotspot = (get_rand_range(seedp, 100) < 80); 
    }

    if (config.workload == WORKLOAD_UNIFORM || !use_hotspot) {
        addr.id = get_rand_range(seedp, max_id);
    } else {
        uint64_t hotspot_limit = (max_id * 20) / 100;
        if (hotspot_limit == 0) hotspot_limit = 1;
        addr.id = get_rand_range(seedp, hotspot_limit);
    }

    if (config.mode == MODE_SEG) {
        // Encontrar el límite máximo entre todos los segmentos
        uint64_t max_limit = 4096; // Límite por defecto
        if (config.seg_limits != NULL) {
            max_limit = config.seg_limits[0];
            for (int i = 1; i < config.segments; i++) {
                if ((uint64_t)config.seg_limits[i] > max_limit) {
                    max_limit = config.seg_limits[i];
                }
            }
        }
        // Generamos el offset basado en el límite MÁS GRANDE posible
        addr.offset = get_rand_range(seedp, max_limit);
    } else {
        addr.offset = get_rand_range(seedp, config.page_size);
    }
    return addr;
}