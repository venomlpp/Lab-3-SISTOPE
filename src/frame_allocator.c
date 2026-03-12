#include <stdlib.h>
#include <pthread.h>
#include "frame_allocator.h"
#include "simulator.h"

static bool *frames_bitmap;
static int total_frames;
static int free_frames;
static pthread_mutex_t allocator_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_frame_allocator() {
    total_frames = config.frames;
    free_frames = total_frames;
    // calloc inicializa todo en 0 (false), indicando que están libres
    frames_bitmap = calloc(total_frames, sizeof(bool));
}

void free_frame_allocator() {
    if (frames_bitmap) free(frames_bitmap);
}

int allocate_frame() {
    int allocated_frame = -1;
    
    // Proteger la memoria compartida si no estamos en modo unsafe
    if (!config.unsafe) pthread_mutex_lock(&allocator_mutex);
    
    if (free_frames > 0) {
        // Búsqueda lineal del primer frame libre
        for (int i = 0; i < total_frames; i++) {
            if (!frames_bitmap[i]) {
                frames_bitmap[i] = true; // Marcar como ocupado
                free_frames--;
                allocated_frame = i;
                break;
            }
        }
    }
    
    if (!config.unsafe) pthread_mutex_unlock(&allocator_mutex);
    
    return allocated_frame;
}