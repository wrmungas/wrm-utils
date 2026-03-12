# output paths
BUILD = build
BIN = bin

# input paths
INC = include
SRC = src
TEST = test
DEPS = glad stb
WRM = wrm
MODULES = common gui input linmath log memory graphics
MOD_PATH = $(SRC)/$(WRM)
H_PATH = $(INC)/$(WRM)

# compiler variables
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
IFLAGS = -I$(INC) -I/usr/local/include/freetype2 -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LFLAGS = -lm -lSDL2 -lGL -lfreetype -lconfig

# linker variables
AR = ar 
AFLAGS = rcs

#--- setup build directories --------------------------------------------------

# list of all directories needed for building
BUILD_DIRS = $(BIN) $(BIN)/$(TEST) $(BUILD) $(patsubst %,$(BUILD)/%,$(DEPS)) $(BUILD)/$(WRM) $(patsubst %,$(BUILD)/$(WRM)/%,$(MODULES))

$(BUILD_DIRS):
	@echo create $@
	@mkdir $@

#--- build objects (Gemini) ---------------------------------------------------

# 1. Improved Object Generation Logic
SRCS = $(shell find $(SRC) -name '*.c')
OBJS = $(patsubst $(SRC)/%.c,$(BUILD)/%.o,$(SRCS))

# 2. Module Mapping Function
# This selects objects belonging to src/wrm/<module_name>
define get_mod_objs
$(filter $(BUILD)/$(WRM)/$(1)/%,$(OBJS))
endef

# 3. Module Targets
.PHONY: $(MODULES)
$(MODULES): %: | $(BUILD_DIRS)
	@echo "Building module [$@] and its dependencies..."
	@# This recursively calls make for the specific object files in this module
	$(MAKE) $(call get_mod_objs,$@)

# 4. Specific module-to-module dependencies
# If you run 'make gui', it will recursively check 'graphics' -> 'log' -> etc.
memory: common
input: common
log: common memory
graphics: log
gui: graphics

# Add those dependencies to the module targets
$(foreach mod,$(MODULES),$(eval $(mod): $($(mod))))

# 5. Standard Object Rule
$(BUILD)/%.o: $(SRC)/%.c | $(BUILD_DIRS)
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $(IFLAGS) $< -o $@


#--- tests --------------------------------------------------------------------

TEST_SRCS = $(wildcard $(TEST)/*.c)
TESTS = $(patsubst $(TEST)/%.c,$(BIN)/$(TEST)/%,$(TEST_SRCS)) # only here to verify my patsubst logic

.PHONY:
.SECONDEXPANSION:
$(TESTS): $$(patsubst $$(BIN)/$$(TEST)/%,$$(TEST)/%.c,$$@)
	@$(CC) $(CFLAGS) $(IFLAGS) $^ -o $@ $(LFLAGS)

#--- libwrm -------------------------------------------------------------------

WRM_LIB = $(BIN)/libwrm.a
$(WRM_LIB): $(OBJS) | $(BIN)
	@echo $@:
	@$(AR) $(AFLAGS) $@ $(OBJS)

#--- Major build commands -----------------------------------------------------

.PHONY:
dirs: $(BUILD_DIRS)

.PHONY:
tests: $(TESTS)

.PHONY:
all: $(BUILD_DIRS) $(MODULES) $(TESTS) $(WRM_LIB)

.PHONY:
default: all

.PHONY:
vars:
	@echo BUILD_DIRS: $(BUILD_DIRS)
	@echo MODULES: $(MODULES)
	@echo OBJS: $(OBJS)
	@echo TESTS: $(TESTS)
	@echo WRM: $(WRM_LIB)


.PHONY:
clean:
	-rm -rf $(BUILD_DIRS)

.PHONY:
fresh: clean all