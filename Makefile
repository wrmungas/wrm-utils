# important directories and names
OBJ_DIR = build
INC_DIR = include
SRC_DIR = src
BIN_DIR = bin
TEST_DIR = test

WRM_SUBDIRS = render menu memory gui
WRM_ALLDIRS = wrm $(patsubst %,wrm/%,$(WRM_SUBDIRS))
SRC_SUBDIRS = glad stb $(WRM_ALLDIRS)
SRC_ALLDIRS = $(SRC_DIR) $(patsubst %,$(SRC_DIR)/%,$(SRC_SUBDIRS))
OBJ_SUBDIRS = test $(SRC_ALLDIRS)
OBJ_ALLDIRS = $(OBJ_DIR) $(patsubst %,$(OBJ_DIR)/%,$(OBJ_SUBDIRS))

BUILD_DIRS = $(BIN_DIR) $(OBJ_ALLDIRS)

# all source files
SRCS = $(shell find $(SRC_DIR) -name '*.c')
# all objects
.PRECIOUS:
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))

TEST_SRCS = $(shell find $(TEST_DIR) -name '*.c')
TEST_OBJS = $(patsubst %,$(OBJ_DIR)/%,$(TEST_SRCS))

TEST_NAMES = test-render test-menu
TESTS = $(patsubst %,$(BIN_DIR)/%,$(TEST_NAMES))

# compiler variables
CC = gcc
CFLAGS = -std=c99 -Wall -g -I$(INC_DIR) -I/usr/local/include/freetype2 -I/usr/include/libpng16 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LDFLAGS = -lSDL2 -lGL -lm -lfreetype -lconfig

.PHONY:
all: $(BUILD_DIRS) $(TESTS)

# setup build directory structure
$(BUILD_DIRS):
	@echo create $@
	@mkdir $@

# compile all
# for some reason this doesn't work if I don't explicitly type the patterns
build/src/%.o: src/%.c
	@echo $@:
	@$(CC) -c $(CFLAGS) $^ -o $@

build/test/%.o: test/%.c
	@echo $@:
	@$(CC) -c $(CFLAGS) $^ -o $@

bin/%: build/test/%.o $(OBJS)
	@echo $@:
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# empty rules so make doesn't fucking delete every built file
$(OBJS):

$(TEST_OBJS):


.PHONY:
vars:
	@echo BUILD_DIRS:
	@echo $(BUILD_DIRS)
	@echo 
	@echo OBJS:
	@echo $(OBJS)
	@echo
	@echo TESTS:
	@echo $(TESTS)

.PHONY:
clean:
	-rm -rf $(OBJ_DIR)
	-rm -rf $(BIN_DIR)

.PHONY:
fresh: clean all