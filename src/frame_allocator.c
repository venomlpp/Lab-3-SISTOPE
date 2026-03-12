#include <stdlib.h>
#include <pthread.h>
#include "frame_allocator.h"
#include "simulator.h"

typedef struct {
    int owner_thread_id;
    uint64_t vpn;
    bool is_free;
} physical_frame_t;

static physical_frame_t *frames;
static int total_frames;
static pthread_mutex_t allocator_mutex = PTHREAD_MUTEX_INITIALIZER;

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

/**
 * Intenta asignar un frame libre en memoria física. Si no existe disponibilidad,
 * utiliza la política FIFO de expulsión (Eviction) para seleccionar un objetivo.
 */
int allocate_frame(int thread_id, uint64_t vpn, int *evicted_thread, uint64_t *evicted_vpn) {
    int target_frame = -1;
    *evicted_thread = -1; 
    
    if (!config.unsafe) pthread_mutex_lock(&allocator_mutex);
    
    if (fifo_count < total_frames) {
        for (int i = 0; i < total_frames; i++) {
            if (frames[i].is_free) {
                target_frame = i;
                frames[i].is_free = false;
                break;
            }
        }
    } 
    else {
        target_frame = fifo_queue[fifo_head]; 
        
        *evicted_thread = frames[target_frame].owner_thread_id;
        *evicted_vpn = frames[target_frame].vpn;
        
        fifo_head = (fifo_head + 1) % total_frames;
        fifo_count--;
    }

    frames[target_frame].owner_thread_id = thread_id;
    frames[target_frame].vpn = vpn;
    
    fifo_queue[fifo_tail] = target_frame;
    fifo_tail = (fifo_tail + 1) % total_frames;
    fifo_count++;
    
    if (!config.unsafe) pthread_mutex_unlock(&allocator_mutex);
    
    return target_frame;
}