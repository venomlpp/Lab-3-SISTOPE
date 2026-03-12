#ifndef WORKLOADS_H
#define WORKLOADS_H

#include <stdint.h>
#include <stdbool.h> // <-- Necesario para bool

// Representa una dirección virtual dividida (Sirve para Seg y Page)
typedef struct {
    uint64_t id;      // Representa el seg_id o el VPN
    uint64_t offset;  // El desplazamiento
    bool is_write;    // NUEVO: Indica si la operación es una escritura
} virtual_addr_t;

virtual_addr_t generate_address(unsigned int *seedp);

#endif // WORKLOADS_H