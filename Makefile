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

# Precompiled header settings
PCH = include/tesseract_pch.h
PCH_GCH = $(PCH).gch
# Add debug flags only when needed
DEBUGFLAGS = -g

SRC_DIR = src
OBJ_DIR = obj
DEP_DIR = $(OBJ_DIR)/.deps
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(patsubst $(SRC_DIR)/%.c,$(DEP_DIR)/%.d,$(SRCS))

REPL_SRC = $(SRC_DIR)/repl.c
REPL_OBJ = $(OBJ_DIR)/repl.o

TARGET = tesser
REPL_TARGET = tesser-repl

# Enable parallel compilation with detected number of cores
MAKEFLAGS += -j$(NUM_CORES)

.PHONY: all clean run run-repl debug release pch

all: release

pch: $(PCH_GCH)

release: pch $(TARGET)

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)
	$(CC) $(DEBUGFLAGS) -o $(TARGET) $(SRC_DIR)/*.c -Iinclude -lm -lcurl

$(TARGET): $(filter-out $(REPL_OBJ), $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^ -lm $(LDFLAGS)

$(REPL_TARGET): $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) $(REPL_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Create directories if they don't exist
$(OBJ_DIR) $(DEP_DIR):
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
	rm -rf $(OBJ_DIR) $(TARGET) $(REPL_TARGET) $(PCH_GCH)

run: $(TARGET)
	./tesser test.tesseract

clear: $(TARGET)
	./tesser test.tesseract

run-repl: $(TARGET)
	./tesser