DOXYFILE := docs/Doxyfile
CC ?= clang
CFLAGS := -g -I./include -I./src $(shell pkg-config --cflags portmidi portaudio-2.0) 
LDFLAGS := $(shell pkg-config --libs portmidi portaudio-2.0)
DBGFLAGS := -g
COBJFLAGS := $(CFLAGS) -c

# path macros
INC_PATH := include
SRC_PATH := src
OBJ_PATH := obj
BIN_PATH := bin
EX_PATH := examples

# compile macros
TARGET_LIB = $(BIN_PATH)/workbench.a

SOURCES = $(wildcard $(SRC_PATH)/*.c)
OBJECTS = $(SOURCES:$(SRC_PATH)/%.c=$(OBJ_PATH)/%.o)

EXAMPLES = $(wildcard $(EX_PATH)/*.c)
EXE_FILES = $(EXAMPLES:$(EX_PATH)/%.c=$(BIN_PATH)/%)
EXE_OBJECTS = $(EXAMPLES:$(EX_PATH)/%.c=$(OBJ_PATH)/%.o)

CLEAN_LIST := $(EXE_OBJECTS) \
			  $(OBJECTS) \
			  $(BIN_PATH)/* \
			  $(OBJ_PATH)/*

# default rule
default: makedir all

# non-phony targets
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c*
	$(CC) $(COBJFLAGS) $(LDFLAGS) -c $< -o $@

$(OBJ_PATH)/%.o: $(EX_PATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_LIB): $(OBJECTS)
	ar rcs $@ $^

$(BIN_PATH)/%: $(EX_PATH)/%.c $(TARGET_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@ $(TARGET_LIB)

# phony rules
.PHONY: all makedir docs clean

makedir:
	@mkdir -p $(BIN_PATH) $(OBJ_PATH)

all: $(TARGET_LIB) $(EXE_FILES)

clean:
	@echo CLEAN $(CLEAN_LIST)
	@rm -rf $(OBJ_PATH)/*
	@rm -rf $(BIN_PATH)/*
#   @rm -rf $(OBJ_PATH)
#   @rm -rf $(BIN_PATH)

docs:
	@doxygen $(DOXYFILE)
