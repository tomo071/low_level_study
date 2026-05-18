CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -std=c11
LDFLAGS :=

SRC_DIR := src
BUILD_DIR := build
TARGET := $(BUILD_DIR)/main

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.PHONY: all clean run debug help docker-build docker-up docker-shell docker-down

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

run: $(TARGET)
	./$(TARGET)

debug: CFLAGS += -g -O0
debug: clean all

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Usage:"
	@echo "  make          - build build/main"
	@echo "  make run      - build and run"
	@echo "  make debug    - build with -g -O0"
	@echo "  make clean    - remove build artifacts"
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
