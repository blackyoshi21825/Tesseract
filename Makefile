# Detect the best compiler available
ifneq ($(shell which clang 2>/dev/null),)
    CC = clang
else
    CC = gcc
endif

# Detect number of CPU cores for optimal parallel compilation
NUM_CORES := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Add optimization flags and enable faster math operations
ifeq ($(CC),clang)
    CFLAGS = -Wall -Wextra -std=c99 -Iinclude -O3 -ffast-math -flto `curl-config --cflags`
else
    CFLAGS = -Wall -Wextra -std=c99 -Iinclude -O3 -ffast-math -flto -march=native `curl-config --cflags`
endif
LDFLAGS = `curl-config --libs` -flto


# Add debug flags only when needed
DEBUGFLAGS = -g

SRC_DIR = src
OBJ_DIR = obj
DEP_DIR = $(OBJ_DIR)/.deps
PKG_DIR = packages
STDLIB_DIR = $(PKG_DIR)/stdlib
STDLIB_OBJ_DIR = $(STDLIB_DIR)/obj
SRCS = $(wildcard $(SRC_DIR)/*.c)
STDLIB_SRCS = $(wildcard $(STDLIB_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
STDLIB_OBJS = $(patsubst $(STDLIB_DIR)/%.c,$(STDLIB_OBJ_DIR)/%.o,$(STDLIB_SRCS))
DEPS = $(patsubst $(SRC_DIR)/%.c,$(DEP_DIR)/%.d,$(SRCS))

REPL_SRC = $(SRC_DIR)/repl.c
REPL_OBJ = $(OBJ_DIR)/repl.o

# Precompiled header settings
PCH = include/tesseract_pch.h
PCH_GCH = $(OBJ_DIR)/tesseract_pch.gch


TARGET = tesser
REPL_TARGET = tesser-repl
TPM_TARGET = tpm

# Enable parallel compilation with detected number of cores
MAKEFLAGS += -j$(NUM_CORES)

.PHONY: all clean run run-repl debug release pch tpm

all: release

pch: $(PCH_GCH)

release: pch $(TARGET)

debug: CFLAGS += $(DEBUGFLAGS)
debug: pch
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -o $(TARGET) $(SRC_DIR)/*.c packages/core/package_loader.c packages/stdlib/*.c -Iinclude -lm -lcurl

$(TARGET): $(filter-out $(REPL_OBJ), $(OBJS)) packages/package_loader.o $(STDLIB_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm $(LDFLAGS)

packages/package_loader.o: packages/core/package_loader.c
	$(CC) $(CFLAGS) -c $< -o $@

$(STDLIB_OBJ_DIR)/%.o: $(STDLIB_DIR)/%.c | $(STDLIB_OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(REPL_TARGET): $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) $(REPL_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Create directories if they don't exist
$(OBJ_DIR) $(DEP_DIR) $(STDLIB_OBJ_DIR):
	@mkdir -p $@

# Precompiled header rule
$(PCH_GCH): $(PCH)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile source files with dependency tracking and using precompiled header
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(PCH_GCH) | $(OBJ_DIR) $(DEP_DIR)
	$(CC) $(CFLAGS) -include $(PCH) -MMD -MP -MF $(DEP_DIR)/$*.d -c $< -o $@

# Include dependency files
-include $(DEPS)

repl: $(REPL_TARGET)

clean:
	rm -rf $(OBJ_DIR) $(STDLIB_OBJ_DIR) $(TARGET) $(REPL_TARGET) $(TPM_TARGET) $(PCH_GCH) packages/package_loader.o

run: $(TARGET)
	./tesser test.tesseract

clear: $(TARGET)
	./tesser test.tesseract

run-repl: $(TARGET)
	./tesser

tpm: $(TPM_TARGET)

$(TPM_TARGET): packages/core/tpm.c packages/core/package_manager.c
	$(CC) $(CFLAGS) -o $@ $^ -lm