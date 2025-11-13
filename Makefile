# important directories and names
OBJ_DIR = build
INC_DIR = include
SRC_DIR = src
BIN_DIR = bin

WRM_SUBDIRS = render menu memory
WRM_ALLDIRS = wrm $(patsubst %,wrm/%,$(WRM_SUBDIRS))
OBJ_SUBDIRS = test glad stb $(WRM_ALLDIRS)
OBJ_ALLDIRS = $(OBJ_DIR) $(patsubst %,$(OBJ_DIR)/%,$(OBJ_SUBDIRS))

BUILD_DIRS = $(BIN_DIR) $(OBJ_ALLDIRS)

# all source files
SRCS = $(shell find $(SRC_DIR) -name '*.c')
# all objects
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# the target executable
EXE_NAMES = test-render
EXES = $(BIN_DIR)/$(EXE_NAMES)

# compiler variables
CC = gcc
CFLAGS = -std=c99 -Wall -g -I$(INC_DIR) -I/usr/local/include/freetype2 -I/usr/include/libpng16 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LDFLAGS = -lSDL2 -lGL -lm -lfreetype -lconfig

.PHONY:
all: $(BUILD_DIRS) $(EXES)

# setup build directory structure
$(BUILD_DIRS):
	@echo create $@
	@mkdir $@

# compile all
# for some reason this doesn't work if I don't explicitly type the patterns
build/%.o: src/%.c
	@echo $@:
	@$(CC) -c $(CFLAGS) $^ -o $@

$(EXES): $(OBJS)
	@echo $@:
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

.PHONY:
test:
	@echo BUILD_DIRS:
	@echo $(BUILD_DIRS)
	@echo 
	@echo OBJS:
	@echo $(OBJS)

.PHONY:
clean:
	-rm -rf $(OBJ_DIR)
	-rm -rf $(BIN_DIR)

.PHONY:
fresh: clean all