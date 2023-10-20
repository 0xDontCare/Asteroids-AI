CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude -pthread -D_DEFAULT_SOURCE
LDFLAGS = -Llib -lraylib -lm -lpthread -lrt -lX11 -lGL -lm -lpthread -ldl

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
BIN = $(BIN_DIR)/game

.PHONY: build clean

build: $(BIN)

$(BIN): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	$(RM) -r $(OBJ_DIR) $(BIN_DIR)