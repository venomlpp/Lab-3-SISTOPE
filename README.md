# Lab-3-SISTOPE
Desarrollo del tercer laboratorio del ramo SISTOPE año 2025 segundo semestre, simulador de memoria virtual.

# Simulador Concurrente de Memoria Virtual

## Descripción del Proyecto
Este proyecto es un simulador concurrente desarrollado en C11 que modela la gestión de memoria virtual de un sistema operativo moderno. [cite_start]Permite explorar dos esquemas de administración de memoria mutuamente excluyentes: **Segmentación** y **Paginación**[cite: 29]. 

[cite_start]El simulador utiliza hilos POSIX (`pthreads`) para representar múltiples procesos concurrentes que compiten por una memoria física global compartida[cite: 31, 35]. Además, implementa características avanzadas como:
* [cite_start]Traducción de direcciones virtuales a físicas[cite: 22].
* [cite_start]Caché TLB (Translation Lookaside Buffer) por hilo con política de reemplazo FIFO[cite: 25].
* [cite_start]Simulación de latencia de disco ante Page Faults[cite: 185].
* [cite_start]Asignador global de marcos (Frame Allocator) con política de expulsión (Eviction) FIFO [cite: 186-188].
* [cite_start]Protección de memoria para prevenir condiciones de carrera (Modo SAFE) y una demostración controlada de problemas de concurrencia (Modo UNSAFE) [cite: 45-48].

## Requisitos
Para compilar y ejecutar este proyecto, necesitas:
* [cite_start]Un compilador de C compatible con el estándar **C11** (ej. GCC)[cite: 415, 428].
* [cite_start]Soporte para la librería **pthreads** (nativo en entornos Linux/POSIX)[cite: 415, 428].
* [cite_start]Herramienta **Make** instalada en el sistema[cite: 416].

## Instrucciones de Compilación
[cite_start]Abre una terminal en el directorio raíz del proyecto y ejecuta[cite: 416]:
```bash
make
```
[cite_start]*(También puedes usar `make all`).* Esto compilará el ejecutable principal llamado `simulator`[cite: 424].

[cite_start]Para limpiar el entorno de trabajo (borrar el ejecutable, los objetos y los reportes `.json`), ejecuta[cite: 427]:
```bash
make clean
```

## Instrucciones de Ejecución
[cite_start]Para ejecutar el simulador con un ejemplo base por defecto (Modo Segmentación, 1 hilo, mostrando métricas), utiliza el siguiente comando[cite: 417, 425]:
```bash
make run
```

## Instrucciones de Reproducción (Experimentos)
[cite_start]El proyecto incluye un comando automatizado para ejecutar los 3 experimentos obligatorios del laboratorio (Segfaults controlados, Impacto del TLB y Thrashing con Múltiples Threads)[cite: 419, 426]. 

[cite_start]Para reproducirlos todos secuencialmente y generar sus respectivos archivos de salida en la carpeta `out/summary.json`, ejecuta[cite: 419, 426]:
```bash
make reproduce
```

## Ejemplos de Comandos (CLI)
[cite_start]El simulador soporta una amplia variedad de banderas para personalizar la ejecución[cite: 420]. Aquí tienes algunos ejemplos:

**1. Segmentación Básica:**
[cite_start]Simula 4 procesos con un patrón de acceso aleatorio (uniforme) y muestra las estadísticas finales [cite: 221-222]:
```bash
./simulator --mode seg --threads 4 --workload uniform --ops-per-thread 5000 --stats
```

**2. Paginación Avanzada (Condiciones de Carrera):**
[cite_start]Simula 8 procesos peleando por solo 16 marcos físicos, forzando la pérdida de sincronización (sin mutexes) usando la bandera `--unsafe` [cite: 223-225]:
```bash
./simulator --mode page --threads 8 --frames 16 --tlb-size 32 --tlb-policy fifo --evict-policy fifo --unsafe --stats
```

**3. Paginación (Thrashing):**
[cite_start]Simula el cuello de botella cuando 8 procesos compiten por solo 4 marcos físicos [cite: 226-228]:
```bash
./simulator --mode page --threads 8 --frames 4 --workload uniform --ops-per-thread 10000 --seed 123 --stats
```