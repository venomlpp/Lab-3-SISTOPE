# Lab-3-SISTOPE
Desarrollo del tercer laboratorio del ramo SISTOPE año 2025 segundo semestre, simulador de memoria virtual.

# Simulador Concurrente de Memoria Virtual

## Descripción del Proyecto
Este proyecto es un simulador concurrente desarrollado en C11 que modela la gestión de memoria virtual de un sistema operativo moderno. Permite explorar dos esquemas de administración de memoria mutuamente excluyentes: **Segmentación** y **Paginación**. 

El simulador utiliza hilos POSIX (`pthreads`) para representar múltiples procesos concurrentes que compiten por una memoria física global compartida. Además, implementa características avanzadas como:
* Traducción de direcciones virtuales a físicas.
* Caché TLB (Translation Lookaside Buffer) por hilo con política de reemplazo FIFO.
* Simulación de latencia de disco ante Page Faults.
* Asignador global de marcos (Frame Allocator) con política de expulsión (Eviction) FIFO.
* Protección de memoria para prevenir condiciones de carrera (Modo SAFE) y una demostración controlada de problemas de concurrencia (Modo UNSAFE).

## Requisitos
Para compilar y ejecutar este proyecto, necesitas:
* Un compilador de C compatible con el estándar **C11** (ej. GCC).
* Soporte para la librería **pthreads** (nativo en entornos Linux/POSIX).
* Herramienta **Make** instalada en el sistema.

## Instrucciones de Compilación
Abre una terminal en el directorio raíz del proyecto y ejecuta:
```bash
make
```
*(También puedes usar `make all`).* Esto compilará el ejecutable principal llamado `simulator`.

Para limpiar el entorno de trabajo (borrar el ejecutable, los objetos y los reportes `.json`), ejecuta:
```bash
make clean
```

## Instrucciones de Ejecución
Para ejecutar el simulador con un ejemplo base por defecto (Modo Segmentación, 1 hilo, mostrando métricas), utiliza el siguiente comando:
```bash
make run
```

## Instrucciones de Reproducción (Experimentos)
El proyecto incluye un comando automatizado para ejecutar los 3 experimentos obligatorios del laboratorio (Segfaults controlados, Impacto del TLB y Thrashing con Múltiples Threads). 

Para reproducirlos todos secuencialmente y generar sus respectivos archivos de salida en la carpeta `out/summary.json`, ejecuta:
```bash
make reproduce
```

## Ejemplos de Comandos (CLI)
El simulador soporta una amplia variedad de banderas para personalizar la ejecución. Aquí tienes algunos ejemplos:

**1. Segmentación Básica:**
Simula 4 procesos con un patrón de acceso aleatorio (uniforme) y muestra las estadísticas finales:
```bash
./simulator --mode seg --threads 4 --workload uniform --ops-per-thread 5000 --stats
```

**2. Paginación Avanzada (Condiciones de Carrera):**
Simula 8 procesos peleando por solo 16 marcos físicos, forzando la pérdida de sincronización (sin mutexes) usando la bandera `--unsafe`:
```bash
./simulator --mode page --threads 8 --frames 16 --tlb-size 32 --tlb-policy fifo --evict-policy fifo --unsafe --stats
```

**3. Paginación (Thrashing):**
Simula el cuello de botella cuando 8 procesos compiten por solo 4 marcos físicos:
```bash
./simulator --mode page --threads 8 --frames 4 --workload uniform --ops-per-thread 10000 --seed 123 --stats
```