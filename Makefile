CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude

SRC_DIR = src
OBJ_DIR = obj
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

REPL_SRC = $(SRC_DIR)/repl.c
REPL_OBJ = $(OBJ_DIR)/repl.o

TARGET = tesser
REPL_TARGET = tesser-repl 

all: $(TARGET)
$(TARGET): $(filter-out $(REPL_OBJ), $(OBJS))
	$(CC) $(CFLAGS) -o $@ $^

$(REPL_TARGET): $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) $(REPL_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

repl: $(REPL_TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(REPL_TARGET)

run: $(TARGET)
	./tesser sample_file.tesseract

run-repl: $(TARGET)
	./tesser

.PHONY: all clean run run-repl repl