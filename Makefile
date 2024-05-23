# compiler and linker flags
CC = clang
CFLAGS = -Wall -Wextra -Wpedantic -Werror -Wshadow -Wstrict-overflow -fno-strict-aliasing -std=gnu11 -pthread -D_DEFAULT_SOURCE
LDFLAGS = -lraylib -lm -lpthread -lrt -lX11 -lGL -lm -ldl

# determining which build to use (release, debug or sanitizer)
SANITIZER ?= 0
DEBUG ?= 0
ifeq ($(SANITIZER), 1)
	CFLAGS += -fno-sanitize-recover=all -fsanitize=address,leak,undefined
	LDFLAGS += -fsanitize=address,leak,undefined
	DEBUG = 1
endif
ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -O0 -g
else
	CFLAGS += -O3
endif

# export variables to for sub-makefiles
export CC CFLAGS

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

all: common game manager neurons

common: $(COMMON_OBJS)

game: $(GAME_OBJS) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/game $(COMMON_OBJS) $(GAME_OBJS) $(LDFLAGS)

manager: $(MANAGER_OBJS) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/manager $(COMMON_OBJS) $(MANAGER_OBJS) $(LDFLAGS)

neurons: $(NEURONS_OBJS) $(COMMON_OBJS) | $(BIN_DIR)
	$(CC) -o $(BIN_DIR)/neurons $(COMMON_OBJS) $(NEURONS_OBJS) $(LDFLAGS)

$(COMMON_DIR)/obj/%.o:
	$(MAKE) -C common $(patsubst $(COMMON_DIR)/obj/%.o,obj/%.o,$@)

$(GAME_DIR)/obj/%.o:
	$(MAKE) -C game $(patsubst $(GAME_DIR)/obj/%.o,obj/%.o,$@)

$(MANAGER_DIR)/obj/%.o:
	$(MAKE) -C manager $(patsubst $(MANAGER_DIR)/obj/%.o,obj/%.o,$@)

$(NEURONS_DIR)/obj/%.o:
	$(MAKE) -C neurons $(patsubst $(NEURONS_DIR)/obj/%.o,obj/%.o,$@)

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

