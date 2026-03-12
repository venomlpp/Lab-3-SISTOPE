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

---

## Análisis de Resultados (Experimentos Obligatorios)

### Experimento 1: Segmentación con Segfaults Controlados
**Objetivo:** Demostrar que la segmentación detecta correctamente violaciones de límite.

| Métrica | Resultado |
| :--- | :--- |
| **Translations OK** | 4721 |
| **Segfaults** | 5279 |
| **Avg Translation Time** | 48.49 ns |
| **Throughput (ops/seg)** | 20,622,638.71 |

**Análisis:**
Los resultados reflejan exactamente el comportamiento esperado de protección de memoria por hardware en un esquema de segmentación. Al emplear un generador de direcciones uniforme (`--workload uniform`) y configurar 4 segmentos con límites muy variados (1024, 2048, 4096, 8192), una proporción significativa de los offsets aleatorios generados excedió el límite permitido para su segmento correspondiente. El simulador detectó correctamente estas violaciones, deteniendo el acceso y contabilizando los 5279 *Segfaults*, mientras que los 4721 accesos que sí respetaron los límites (`offset < limit`) se tradujeron con éxito.

### Experimento 2: Impacto del TLB (Paginación)
**Objetivo:** Comparar performance con y sin TLB usando el mismo workload (80-20).

| Configuración | TLB Hits | TLB Misses | Hit Rate | Tiempo Total (s) | Throughput (ops/s) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Sin TLB (Size 0)** | 0 | 50000 | 0.00% | 15.5609 | 3213.19 |
| **Con TLB (Size 32)** | 31799 | 18201 | 63.60% | 15.5783 | 3209.59 |

**Análisis:**
Al utilizar el workload "80-20", que simula el principio de localidad espacial y temporal, observamos la efectividad de la caché TLB. Con la TLB activada (tamaño 32), el sistema logró un Hit Rate del 63.60% (31,799 aciertos), evitando ir a la Tabla de Páginas para la mayoría de las traducciones.
*Observación de rendimiento:* Aunque la lógica dictaría que el *Throughput* debería aumentar significativamente con la TLB, los tiempos resultaron casi idénticos (~3213 vs ~3209 ops/sec). Esto se debe a un detalle específico de la simulación: el retardo (`nanosleep`) penaliza los accesos a "disco" (Page Faults), pero la simple lectura de la Tabla de Páginas en RAM ocurre a velocidad de CPU. Aún así, la alta tasa de aciertos cumple el objetivo de demostrar teóricamente el impacto positivo de la TLB.

### Experimento 3: Thrashing con Múltiples Threads
**Objetivo:** Observar thrashing cuando hay más páginas activas que frames disponibles.

| Hilos Activos | Total Ops | TLB Misses | Miss Rate | Avg Trans. Time | Throughput (ops/s) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **1 Thread** | 10000 | 8783 | 87.83% | 1,809,279.01 ns | 552.71 |
| **8 Threads**| 80000 | 78679 | 98.35% | 253,704.94 ns | 3941.59 |

**Análisis:**
En este escenario forzamos una escasez severa de memoria física (solo 8 marcos compartidos para 64 páginas virtuales por hilo). Al inyectar 8 hilos de forma concurrente con un acceso uniforme, el sistema entra en un estado de **Thrashing**. La tasa de *TLB Misses* escala a un crítico 98.35%, lo que evidencia que los hilos se roban mutuamente los marcos disponibles. El sistema pasa la gran mayoría de su tiempo ejecutando rutinas de expulsión (Eviction FIFO) y simulando la carga desde disco, en lugar de realizar trabajo útil.
*Nota de concurrencia:* Curiosamente, el Throughput general aumentó de ~552 a ~3941 ops/sec al usar 8 hilos. Esto es un artefacto de la simulación multiproceso de I/O: como los *Page Faults* se simulan bloqueando el hilo con `nanosleep`, el planificador del SO aprovecha este tiempo muerto para ejecutar otros hilos. Este solapamiento de esperas (I/O overlap) aumenta la cantidad de operaciones completadas por segundo a nivel global, ocultando el hecho de que individualmente los hilos sufren latencias masivas por la contención de memoria.