# important directories
BUILD_DIR = build
INC_DIR = include
SRC_DIR = src
BIN_DIR = bin
TEST_DIR = test
DEP_DIRS = glad stb
WRM_DIR = wrm
WRM_SUBDIRS = common gui input linmath memory render


# compiler variables
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
IFLAGS = -I$(INC_DIR) -I/usr/local/include/freetype2 -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LFLAGS = -lm -lSDL2 -lGL -lfreetype -lconfig
AR = ar 
AFLAGS = rcs

# target 1: build directories
BUILD_DIRS = $(BIN_DIR) $(TEST_DIR)/$(BIN_DIR) $(BUILD_DIR) $(patsubst %,$(BUILD_DIR)/%,$(DEP_DIRS)) $(BUILD_DIR)/$(WRM_DIR) $(patsubst %,$(BUILD_DIR)/$(WRM_DIR)/%,$(WRM_SUBDIRS))

$(BUILD_DIRS):
	@echo create $@
	@mkdir $@


# target 2: all objects
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

.SECONDEXPANSION:
$(OBJS): $$(patsubst $$(BUILD_DIR)/%.o,$$(SRC_DIR)/%.c,$$@)
	@echo $@:
	@cd $(CC) -c $(CFLAGS) $(IFLAGS) $^ -o $@ 


# target 3: libwrm
WRM = $(BIN_DIR)/libwrm.a

$(WRM): $(OBJS)
	@echo $@:
	@$(AR) $(AFLAGS) $@ $(OBJS)


# target 4: tests
TEST_SRCS = $(wildcard $(TEST_DIR)/$(SRC_DIR)/*.c)
TESTS = $(patsubst $(TEST_DIR)/$(SRC_DIR)/%.c,$(TEST_DIR)/$(BIN_DIR)/%,$(TEST_SRCS)) # only here to verify my patsubst logic

.PHONY:
.SECONDEXPANSION:
$(TESTS): $$(patsubst $$(TEST_DIR)/$$(BIN_DIR)/%,$$(TEST_DIR)/$$(SRC_DIR)/%.c,$$@)
	@$(CC) $(CFLAGS) $(IFLAGS) $^ $(WRM) -o $@ $(LFLAGS)


.PHONY:
dirs: $(BUILD_DIRS)

.PHONY:
all: $(BUILD_DIRS) $(WRM) $(TESTS)

.PHONY:
default: all

.PHONY:
vars:
	@echo BUILD_DIRS: $(BUILD_DIRS)
	@echo OBJS: $(OBJS)
	@echo TESTS: $(TESTS)


.PHONY:
clean:
	-rm -rf $(BUILD_DIRS)

.PHONY:
fresh: clean all