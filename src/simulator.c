#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>
#include "simulator.h"
#include "workloads.h"
#include "segmentacion.h"
#include "paginacion.h"
#include "frame_allocator.h"
#include "tlb.h"

sim_config_t config = {
    .mode = MODE_NONE, .threads = 1, .ops_per_thread = 1000,
    .workload = WORKLOAD_UNIFORM, .seed = 42, .unsafe = false, .stats = false, .use_dirty_pages = false,
    .segments = 4, .seg_limits = NULL, .pages = 64, .frames = 32,
    .page_size = 4096, .tlb_size = 16
};

thread_stats_t *t_stats = NULL;
uint64_t global_translations_ok = 0;
uint64_t global_segfaults = 0;
pthread_mutex_t global_stats_mutex = PTHREAD_MUTEX_INITIALIZER;

page_table_t **global_pts = NULL;
tlb_t **global_tlbs = NULL;

void print_usage() {
    printf("Uso: ./simulator --mode {seg|page} [opciones...]\n");
    exit(EXIT_FAILURE);
}

/**
 * Analiza y carga los argumentos ingresados por la interfaz de línea de comandos.
 */
void parse_arguments(int argc, char *argv[]) {
    int c;
    strcpy(config.tlb_policy, "fifo"); strcpy(config.evict_policy, "fifo");

    struct option long_options[] = {
        {"mode", required_argument, 0, 'm'}, {"threads", required_argument, 0, 't'},
        {"ops-per-thread", required_argument, 0, 'o'}, {"workload", required_argument, 0, 'w'},
        {"seed", required_argument, 0, 's'}, {"unsafe", no_argument, 0, 'u'},
        {"stats", no_argument, 0, 'S'}, {"segments", required_argument, 0, 'g'},
        {"seg-limits", required_argument, 0, 'l'}, {"pages", required_argument, 0, 'p'},
        {"frames", required_argument, 0, 'f'}, {"page-size", required_argument, 0, 'z'},
        {"tlb-size", required_argument, 0, 'b'}, {"tlb-policy", required_argument, 0, 'P'},
        {"evict-policy", required_argument, 0, 'E'}, {"dirty-pages", no_argument, 0, 'D'}, {0, 0, 0, 0}, 
    };

    while ((c = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
        switch (c) {
            case 'm':
                if (strcmp(optarg, "seg") == 0) config.mode = MODE_SEG;
                else if (strcmp(optarg, "page") == 0) config.mode = MODE_PAGE;
                else {
                    print_usage();
                }
                break;
            case 't': config.threads = atoi(optarg); break;
            case 'o': config.ops_per_thread = atoi(optarg); break;
            case 'w': if (strcmp(optarg, "80-20") == 0) config.workload = WORKLOAD_80_20; break;
            case 's': config.seed = atoi(optarg); break;
            case 'u': config.unsafe = true; break;
            case 'S': config.stats = true; break;
            case 'g': config.segments = atoi(optarg); break;
            case 'l':
                config.seg_limits = malloc(sizeof(int) * config.segments);
                char *token = strtok(optarg, ",");
                int idx = 0;
                while (token != NULL && idx < config.segments) {
                    config.seg_limits[idx++] = atoi(token);
                    token = strtok(NULL, ",");
                }
                while (idx < config.segments) config.seg_limits[idx++] = 4096;
                break;
            case 'p': config.pages = atoi(optarg); break;
            case 'f': config.frames = atoi(optarg); break;
            case 'z': config.page_size = atoi(optarg); break;
            case 'b': config.tlb_size = atoi(optarg); break;
            case 'P': strncpy(config.tlb_policy, optarg, 15); break;
            case 'E': strncpy(config.evict_policy, optarg, 15); break;
            case 'D': config.use_dirty_pages = true; break;
            default: print_usage();
        }
    }
    if (config.mode == MODE_NONE) { fprintf(stderr, "Error: --mode es obligatorio.\n"); print_usage(); }
}

/**
 * Lógica principal concurrente delegada a los hilos de ejecución.
 */
void *thread_routine(void *arg) {
    int thread_id = *(int *)arg;
    free(arg); 
    
    unsigned int thread_seed = config.seed + thread_id; 
    segment_table_t *seg_table = NULL;
    page_table_t *page_table = NULL;
    tlb_t *tlb = NULL;
    
    if (config.mode == MODE_SEG) {
        seg_table = init_segment_table();
    } else if (config.mode == MODE_PAGE) {
        page_table = global_pts[thread_id];
        tlb = global_tlbs[thread_id];
    }

    for (int i = 0; i < config.ops_per_thread; i++) {
        virtual_addr_t va = generate_address(&thread_seed);
        bool success = false;
        
        if (config.mode == MODE_SEG) {
            uint64_t pa;
            success = translate_segment(seg_table, va, &pa);
        } else if (config.mode == MODE_PAGE) {
            uint64_t pa;
            
            int frame = tlb_lookup(tlb, va.id, va.is_write);
            if (frame != -1) {
                pa = (frame * config.page_size) + va.offset;
                success = true;
            } else {
                success = translate_page(thread_id, page_table, va, &pa);
                if (success) {
                    tlb_insert(tlb, va.id, page_table->entries[va.id].frame_number, va.is_write);
                }
            }
        }
        
        if (success) t_stats[thread_id].translations_ok++;
        else t_stats[thread_id].segfaults++; 

        if (!config.unsafe) pthread_mutex_lock(&global_stats_mutex);
        if (success) global_translations_ok++;
        else global_segfaults++;
        if (!config.unsafe) pthread_mutex_unlock(&global_stats_mutex);
    }
    
    if (config.mode == MODE_SEG) free_segment_table(seg_table);
    return NULL;
}

int main(int argc, char *argv[]) {
    parse_arguments(argc, argv);
    t_stats = calloc(config.threads, sizeof(thread_stats_t));

    if (config.mode == MODE_PAGE) {
        init_frame_allocator();
        global_pts = malloc(sizeof(page_table_t*) * config.threads);
        global_tlbs = malloc(sizeof(tlb_t*) * config.threads);
        for(int i = 0; i < config.threads; i++) {
            global_pts[i] = init_page_table();
            global_tlbs[i] = init_tlb();
        }
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t *threads = malloc(sizeof(pthread_t) * config.threads);
    for (int i = 0; i < config.threads; i++) {
        int *id = malloc(sizeof(int)); *id = i;
        if (pthread_create(&threads[i], NULL, thread_routine, id) != 0) {
            perror("Error creando hilo"); exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < config.threads; i++) pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);
    
    double runtime_sec = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    uint64_t total_ops = config.threads * config.ops_per_thread;
    double throughput = total_ops / runtime_sec;
    double avg_translation_ns = (runtime_sec * 1e9) / total_ops;

    uint64_t total_tlb_hits = 0;
    uint64_t total_tlb_misses = 0;

    if (config.mode == MODE_PAGE) {
        for(int i = 0; i < config.threads; i++) {
            total_tlb_hits += global_tlbs[i]->hits;
            total_tlb_misses += global_tlbs[i]->misses;
        }
    }

    if (config.stats) {
        printf("\n==============================\n");
        printf("SIMULADOR DE MEMORIA VIRTUAL\n");
        printf("Modo: %s\n", config.mode == MODE_SEG ? "SEGMENTACION" : "PAGINACION");
        printf("==============================\n");
        printf("Configuracion:\n");
        printf("Threads: %d\n", config.threads);
        printf("Ops por thread: %d\n", config.ops_per_thread);
        printf("Workload: %s\n", config.workload == WORKLOAD_UNIFORM ? "uniform" : "80-20");
        printf("Seed: %d\n", config.seed);
        printf("Dirty Pages: %s\n", config.use_dirty_pages ? "activo" : "no activo");
        
        printf("\nMetricas Globales:\n");
        printf("Translations OK: %lu\n", global_translations_ok);
        if (config.mode == MODE_SEG) {
            printf("Segfaults: %lu\n", global_segfaults);
        } else if (config.mode == MODE_PAGE) {
            printf("TLB Hits: %lu\n", total_tlb_hits);
            printf("TLB Misses: %lu\n", total_tlb_misses);
            printf("Hard Page Faults (Out of Memory/Error): %lu\n", global_segfaults);
        }
        
        printf("\nMetricas por Thread:\n");
        for (int i = 0; i < config.threads; i++) {
            if (config.mode == MODE_SEG) {
                printf("  Thread %d: Translations OK: %d, Segfaults/Misses: %d\n", 
                       i, t_stats[i].translations_ok, t_stats[i].segfaults);
            } else if (config.mode == MODE_PAGE) {
                printf("  Thread %d: Translations OK: %d, TLB Hits: %d, TLB Misses: %d\n", 
                       i, t_stats[i].translations_ok, global_tlbs[i]->hits, global_tlbs[i]->misses);
            }
        }

        printf("\nTiempo total: %.4f segundos\n", runtime_sec);
        printf("Throughput: %.2f ops/seg\n", throughput);
        printf("Avg Translation Time: %.2f ns\n", avg_translation_ns);
        printf("==============================\n");
    }

    FILE *f = fopen("out/summary.json", "w");
    if (f) {
        fprintf(f, "{\n  \"mode\": \"%s\",\n", config.mode == MODE_SEG ? "seg" : "page");
        fprintf(f, "  \"config\": {\n");
        fprintf(f, "    \"threads\": %d,\n", config.threads);
        fprintf(f, "    \"ops_per_thread\": %d,\n", config.ops_per_thread);
        fprintf(f, "    \"workload\": \"%s\",\n", config.workload == WORKLOAD_UNIFORM ? "uniform" : "80-20");
        fprintf(f, "    \"seed\": %d,\n", config.seed);
        fprintf(f, "    \"unsafe\": %s,\n", config.unsafe ? "true" : "false");
        fprintf(f, "    \"use_dirty_pages\": %s\n", config.use_dirty_pages ? "true" : "false");
        fprintf(f, "  },\n");
        fprintf(f, "  \"metrics\": {\n");
        fprintf(f, "    \"translations_ok\": %lu,\n", global_translations_ok);
        if (config.mode == MODE_PAGE) {
            fprintf(f, "    \"tlb_hits\": %lu,\n", total_tlb_hits);
            fprintf(f, "    \"tlb_misses\": %lu,\n", total_tlb_misses);
        } else {
            fprintf(f, "    \"segfaults\": %lu,\n", global_segfaults);
        }
        fprintf(f, "    \"avg_translation_time_ns\": %.2f,\n", avg_translation_ns);
        fprintf(f, "    \"throughput_ops_sec\": %.2f\n", throughput);
        fprintf(f, "  },\n  \"runtime_sec\": %.4f\n}\n", runtime_sec);
        fclose(f);
    }

    if (config.mode == MODE_PAGE) {
        free_frame_allocator();
        for(int i = 0; i < config.threads; i++) {
            free_page_table(global_pts[i]);
            free_tlb(global_tlbs[i]);
        }
        free(global_pts);
        free(global_tlbs);
    }

    free(threads);
    free(t_stats);
    if (config.seg_limits) free(config.seg_limits);
    
    return 0;
}