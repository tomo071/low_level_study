CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -std=c11
LDFLAGS :=

SRC_DIR := src
BUILD_DIR := build
TARGET := $(BUILD_DIR)/main

# pointer_demo.c は main を持つ別プログラムのため通常ビルドから除外
SRCS := $(filter-out $(SRC_DIR)/pointer_demo.c, $(wildcard $(SRC_DIR)/*.c))
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

POINTER_TARGET := $(BUILD_DIR)/pointer_demo

.PHONY: all clean run debug pointer pointer-run help docker-build docker-up docker-shell docker-down

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

pointer: $(POINTER_TARGET)

$(POINTER_TARGET): $(SRC_DIR)/pointer_demo.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

pointer-run: pointer
	./$(POINTER_TARGET)

debug: CFLAGS += -g -O0
debug: clean all

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Usage:"
	@echo "  make          - build build/main"
	@echo "  make run          - build and run hello (build/main)"
	@echo "  make pointer      - build pointer demo"
	@echo "  make pointer-run  - build and run pointer demo"
	@echo "  make debug        - build with -g -O0"
	@echo "  make clean        - remove build artifacts"
	@echo ""
	@echo "Docker (手順1: up -d + exec で同一コンテナに2シェル):"
	@echo "  make docker-build  - build image"
	@echo "  make docker-up     - start container in background"
	@echo "  make docker-shell  - exec into running container"
	@echo "  make docker-down   - stop container"

docker-build:
	docker compose build

docker-up:
	docker compose up -d

docker-shell:
	docker compose exec dev bash

docker-down:
	docker compose down
