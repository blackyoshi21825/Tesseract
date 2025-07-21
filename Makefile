CC = gcc
# Add optimization flags and enable faster math operations
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -O2 -ffast-math
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

# Enable parallel compilation
MAKEFLAGS += -j8

.PHONY: all clean run run-repl debug release

all: release

release: $(TARGET)

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(TARGET)

$(TARGET): $(filter-out $(REPL_OBJ), $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(REPL_TARGET): $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) $(REPL_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Create directories if they don't exist
$(OBJ_DIR) $(DEP_DIR):
	@mkdir -p $@

# Compile source files with dependency tracking
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) $(DEP_DIR)
	$(CC) $(CFLAGS) -MMD -MP -MF $(DEP_DIR)/$*.d -c $< -o $@

# Include dependency files
-include $(DEPS)

repl: $(REPL_TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(REPL_TARGET)

run: $(TARGET)
	./tesser test.tesseract

clear: $(TARGET)
	./tesser test.tesseract

run-repl: $(TARGET)
	./tesser