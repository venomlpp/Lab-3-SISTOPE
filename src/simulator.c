#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "simulator.h"
#include <pthread.h>

// Inicializar con valores por defecto
sim_config_t config = {
    .mode = MODE_NONE,
    .threads = 1,
    .ops_per_thread = 1000,
    .workload = WORKLOAD_UNIFORM,
    .seed = 42,
    .unsafe = false,
    .stats = false,
    .segments = 4,
    .seg_limits = NULL,
    .pages = 64,
    .frames = 32,
    .page_size = 4096,
    .tlb_size = 16
};

void print_usage() {
    printf("Uso: ./simulator --mode {seg|page} [opciones...]\n");
    exit(EXIT_FAILURE);
}

void parse_arguments(int argc, char *argv[]) {
    int c;
    strcpy(config.tlb_policy, "fifo");
    strcpy(config.evict_policy, "fifo");

    struct option long_options[] = {
        {"mode", required_argument, 0, 'm'},
        {"threads", required_argument, 0, 't'},
        {"ops-per-thread", required_argument, 0, 'o'},
        {"workload", required_argument, 0, 'w'},
        {"seed", required_argument, 0, 's'},
        {"unsafe", no_argument, 0, 'u'},
        {"stats", no_argument, 0, 'S'},
        {"segments", required_argument, 0, 'g'},
        {"seg-limits", required_argument, 0, 'l'},
        {"pages", required_argument, 0, 'p'},
        {"frames", required_argument, 0, 'f'},
        {"page-size", required_argument, 0, 'z'},
        {"tlb-size", required_argument, 0, 'b'},
        {"tlb-policy", required_argument, 0, 'P'},
        {"evict-policy", required_argument, 0, 'E'},
        {0, 0, 0, 0}
    };

    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case 'm':
                if (strcmp(optarg, "seg") == 0) config.mode = MODE_SEG;
                else if (strcmp(optarg, "page") == 0) config.mode = MODE_PAGE;
                else print_usage();
                break;
            case 't': config.threads = atoi(optarg); break;
            case 'o': config.ops_per_thread = atoi(optarg); break;
            case 'w':
                if (strcmp(optarg, "80-20") == 0) config.workload = WORKLOAD_80_20;
                break;
            case 's': config.seed = atoi(optarg); break;
            case 'u': config.unsafe = true; break;
            case 'S': config.stats = true; break;
            case 'g': config.segments = atoi(optarg); break;
            case 'l':
                // TODO: Parsear CSV de límites (lo haremos en un momento)
                break;
            case 'p': config.pages = atoi(optarg); break;
            case 'f': config.frames = atoi(optarg); break;
            case 'z': config.page_size = atoi(optarg); break;
            case 'b': config.tlb_size = atoi(optarg); break;
            case 'P': strncpy(config.tlb_policy, optarg, 15); break;
            case 'E': strncpy(config.evict_policy, optarg, 15); break;
            default: print_usage();
        }
    }

    if (config.mode == MODE_NONE) {
        fprintf(stderr, "Error: --mode es obligatorio.\n");
        print_usage();
    }
}

// Rutina que ejecutará cada hilo
void *thread_routine(void *arg) {
    int thread_id = *(int *)arg;
    free(arg); 
    
    // Semilla única por thread para reproducibilidad
    unsigned int thread_seed = config.seed + thread_id; 

    for (int i = 0; i < config.ops_per_thread; i++) {
        virtual_addr_t va = generate_address(&thread_seed);
        
        // Debug temporal para verificar que generan direcciones
        // if (i == 0) printf("[Thread %d] Primera VA -> ID: %lu, Offset: %lu\n", thread_id, va.id, va.offset);
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    parse_arguments(argc, argv);

    printf("Iniciando simulador (%s) con %d hilo(s)...\n", 
           config.mode == MODE_SEG ? "Segmentacion" : "Paginacion", config.threads);

    pthread_t *threads = malloc(sizeof(pthread_t) * config.threads);

    for (int i = 0; i < config.threads; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        if (pthread_create(&threads[i], NULL, thread_routine, id) != 0) {
            perror("Error creando hilo");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < config.threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    printf("Simulacion finalizada.\n");

    return 0;
}