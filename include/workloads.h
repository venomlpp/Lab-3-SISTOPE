#ifndef WORKLOADS_H
#define WORKLOADS_H

#include <stdint.h>

// Representa una dirección virtual dividida (Sirve para Seg y Page)
typedef struct {
    uint64_t id;      // Representa el seg_id o el VPN
    uint64_t offset;  // El desplazamiento
} virtual_addr_t;

// Prototipo de generación de direcciones
virtual_addr_t generate_address(unsigned int *seedp);

#endif // WORKLOADS_H