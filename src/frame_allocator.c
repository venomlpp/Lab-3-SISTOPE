#include <stdlib.h>
#include <pthread.h>
#include "frame_allocator.h"
#include "simulator.h"

// Estructura para saber a quién pertenece cada frame físico
typedef struct {
    int owner_thread_id;
    uint64_t vpn;
    bool is_free;
} physical_frame_t;

static physical_frame_t *frames;
static int total_frames;
static pthread_mutex_t allocator_mutex = PTHREAD_MUTEX_INITIALIZER;

// Cola FIFO para Eviction
static int *fifo_queue;
static int fifo_head = 0;
static int fifo_tail = 0;
static int fifo_count = 0;

void init_frame_allocator() {
    total_frames = config.frames;
    frames = malloc(sizeof(physical_frame_t) * total_frames);
    fifo_queue = malloc(sizeof(int) * total_frames);
    
    for (int i = 0; i < total_frames; i++) {
        frames[i].is_free = true;
    }
}

void free_frame_allocator() {
    if (frames) free(frames);
    if (fifo_queue) free(fifo_queue);
}

// Ahora recibe el ID del hilo y la página virtual que solicita el espacio
int allocate_frame(int thread_id, uint64_t vpn, int *evicted_thread, uint64_t *evicted_vpn) {
    int target_frame = -1;
    *evicted_thread = -1; // Por defecto no expulsamos a nadie
    
    if (!config.unsafe) pthread_mutex_lock(&allocator_mutex);
    
    // 1. Buscar si hay memoria libre
    if (fifo_count < total_frames) {
        for (int i = 0; i < total_frames; i++) {
            if (frames[i].is_free) {
                target_frame = i;
                frames[i].is_free = false;
                break;
            }
        }
    } 
    // 2. No hay memoria: Reemplazo (Eviction) por FIFO
    else {
        target_frame = fifo_queue[fifo_head]; // Tomamos el frame más antiguo
        
        // Registrar a quién vamos a "matar" (para que paginacion.c lo invalide)
        *evicted_thread = frames[target_frame].owner_thread_id;
        *evicted_vpn = frames[target_frame].vpn;
        
        fifo_head = (fifo_head + 1) % total_frames;
        fifo_count--;
    }

    // 3. Asignar al nuevo dueño y poner al final de la cola FIFO
    frames[target_frame].owner_thread_id = thread_id;
    frames[target_frame].vpn = vpn;
    
    fifo_queue[fifo_tail] = target_frame;
    fifo_tail = (fifo_tail + 1) % total_frames;
    fifo_count++;
    
    if (!config.unsafe) pthread_mutex_unlock(&allocator_mutex);
    
    return target_frame;
}