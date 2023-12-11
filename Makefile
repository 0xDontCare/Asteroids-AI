# compiler and linker flags
CC = gcc
LDFLAGS = -Llib -lraylib -lm -lpthread -lrt -lX11 -lGL -lm -lpthread -ldl

# program directories
COMMON_DIR = common
GAME_DIR = game
MANAGER_DIR = manager
NEURONS_DIR = neurons

# program source files
COMMON_SRC = $(wildcard $(COMMON_DIR)/src/*.c)
GAME_SRC = $(wildcard $(GAME_DIR)/src/*.c)
MANAGER_SRC = $(wildcard $(MANAGER_DIR)/src/*.c)
NEURONS_SRC = $(wildcard $(NEURONS_DIR)/src/*.c)

# program object files (derived from source files)
COMMON_OBJS = $(patsubst $(COMMON_DIR)/src/%.c,$(COMMON_DIR)/obj/%.o,$(COMMON_SRC))
GAME_OBJS = $(patsubst $(GAME_DIR)/src/%.c,$(GAME_DIR)/obj/%.o,$(GAME_SRC))
MANAGER_OBJS = $(patsubst $(MANAGER_DIR)/src/%.c,$(MANAGER_DIR)/obj/%.o,$(MANAGER_SRC))
NEURONS_OBJS = $(patsubst $(NEURONS_DIR)/src/%.c,$(NEURONS_DIR)/obj/%.o,$(NEURONS_SRC))

# output executable directory
BIN_DIR = bin

.PHONY: all common game manager neurons clean

all: game manager neurons

# test target which prints out all above defined variables
test_vars: | $(BIN_DIR)
	@echo COMMON_DIR: $(COMMON_DIR)
	@echo COMMON_SRC: $(COMMON_SRC)
	@echo COMMON_OBJS: $(COMMON_OBJS)
	@echo GAME_DIR: $(GAME_DIR)
	@echo GAME_SRC: $(GAME_SRC)
	@echo GAME_OBJS: $(GAME_OBJS)
	@echo MANAGER_DIR: $(MANAGER_DIR)
	@echo MANAGER_SRC: $(MANAGER_SRC)
	@echo MANAGER_OBJS: $(MANAGER_OBJS)
	@echo NEURONS_DIR: $(NEURONS_DIR)
	@echo NEURONS_SRC: $(NEURONS_SRC)
	@echo NEURONS_OBJS: $(NEURONS_OBJS)
	@echo BIN_DIR: $(BIN_DIR)

common: $(COMMON_OBJS)

game: $(COMMON_OBJS) $(GAME_OBJS) | $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/game $(COMMON_OBJS) $(GAME_OBJS) $(LDFLAGS)

manager: $(COMMON_OBJS) $(MANAGER_OBJS) | $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/manager $(COMMON_OBJS) $(MANAGER_OBJS) $(LDFLAGS)

neurons: $(COMMON_OBJS) $(NEURONS_OBJS) | $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/neurons $(COMMON_OBJS) $(NEURONS_OBJS) $(LDFLAGS)

#$(GAME_OBJS) $(MANAGER_OBJS) $(NEURONS_OBJS): | $(COMMON_OBJS)

$(COMMON_DIR)/obj/%.o:
	$(MAKE) -C common build

$(GAME_DIR)/obj/%.o:
	$(MAKE) -C game build

$(MANAGER_DIR)/obj/%.o:
	$(MAKE) -C manager build

$(NEURONS_DIR)/obj/%.o:
	$(MAKE) -C neurons build

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	$(MAKE) -C common clean
	$(MAKE) -C game clean
	$(MAKE) -C manager clean
	$(MAKE) -C neurons clean
	$(RM) -r bin

help:
	@echo "Usage:"
	@echo "  make [target]"
	@echo ""
	@echo "Available targets:"
	@echo "  all      Build all targets (default)"
	@echo "  common   Build common library (produces no executable)"
	@echo "  game     Build game"
	@echo "  manager  Build manager"
	@echo "  neurons  Build neural network program"
	@echo "  clean    Remove all generated files"
	@echo "  help     Show this help message"

#vpath %.o $(COMMON_DIR) $(GAME_DIR) $(MANAGER_DIR) $(NEURONS_DIR)
