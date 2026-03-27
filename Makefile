# output paths
BUILD = build
BIN = bin

# input paths
INC = include
SRC = src
TEST = test
DEPS = glad stb
WRM = wrm

# module names
WRMLIB = libwrmutils
WRMMOD = common gui input linmath log memory graphics

# paths
MODPATH = $(SRC)/$(WRM)
HPATH = $(INC)/$(WRM)

# compiler variables
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
IFLAGS = -I$(INC) -I/usr/local/include/freetype2 -I/usr/include/freetype2 -I/usr/include/libpng16 -I/usr/include/harfbuzz -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include
LFLAGS = -lm -lSDL2 -lGL -lfreetype -lconfig

# linker variables
AR = ar 
AFLAGS = rcs

#--- RULES --------------------------------------------------------------------

.PHONY: all clean tests fresh

all: deps modules library tests

clean:
	rm -rf build/*
	rm -rf bin/*

fresh: clean all

tests:


# create directories to mirror src/ in build/
$(BUILD)/%: 
	@echo create $@
	@mkdir $@

# compile src/a/b.c to build/a/b.o
$(BUILD)/%.o: $(SRC)/%.c
	$(CC) -c $(CFLAGS) $(IFLAGS) $< -o $@

# compile test/a.c to test/bin/a (executable)

#--- BUILD DEPENDENCIES -------------------------------------------------------

deps: $(DEPS)

.PHONY:
$(DEPS): $(BUILD)/$@ $(shell find $(SRC)/$@/*.c)
	@echo Building dependency $@

#--- BUILD WRM MODULES --------------------------------------------------------

modules: $(WRMMOD)

.PHONY:
$(WRMMOD): $(BUILD)/$(WRM)/$@ $(shell find $(MODPATH)/$@/*.c)
	@echo Building module $@

#--- BUILD WRM LIBRARY --------------------------------------------------------

library: deps modules $(BIN)/$(WRMLIB)

$(BIN)/$(WRMLIB): 

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

#--- TO TEST VARIABLES
.PHONY:
vars:
	@echo BUILD_DIRS: 
	@echo $(BUILD_DIRS)
	@echo MODULES: 
	@echo $(MODULES)
	@echo OBJS: 
	@echo $(OBJS)
	@echo TESTS: 
	@echo $(TESTS)
	@echo WRM: 
	@echo $(WRM_LIB)
