CC = gcc
CFLAGS = -std=c11 -pthread -Wall -Wextra -Iinclude
SRC_DIR = src
OBJ_DIR = obj
OUT_DIR = out

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET = simulator

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(OUT_DIR)

run: all
	./$(TARGET) --mode seg --stats

reproduce: all
	@echo "Ejecutando Experimento 1..."
	./$(TARGET) --mode seg --threads 1 --workload uniform --ops-per-thread 10000 --segments 4 --seg-limits 1024,2048,4096,8192 --seed 100 --stats
	@echo "\nEjecutando Experimento 2..."
	./$(TARGET) --mode page --threads 1 --workload 80-20 --ops-per-thread 50000 --pages 128 --frames 64 --page-size 4096 --tlb-size 32 --tlb-policy fifo --seed 200 --stats
	@echo "\nEjecutando Experimento 3..."
	./$(TARGET) --mode page --threads 8 --workload uniform --ops-per-thread 10000 --pages 64 --frames 8 --page-size 4096 --tlb-size 16 --seed 300 --stats

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(OUT_DIR)/*.json

.PHONY: all run reproduce clean