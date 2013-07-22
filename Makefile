# Directories:
OBJ_DIR=obj
BIN_DIR=bin
OUT_DIR=out
TEST_DIR=$(OUT_DIR)/test

# Don't worry about modification times on directories:
MAKEFLAGS+=--assume-old=$(OBJ_DIR)
MAKEFLAGS+=--assume-old=$(BIN_DIR)
MAKEFLAGS+=--assume-old=$(OUT_DIR)
MAKEFLAGS+=--assume-old=$(TEST_DIR)

# Compiler & Flags:
CC=gcc
CFLAGS=-g -O2 -c -Wall -ffast-math
LFLAGS=-lm -lGL -lGLU -lglut -lglee -lpng

# TODO: headers like conv.h?

# Objects:
OBJECTS=$(OBJ_DIR)/world.o \
				$(OBJ_DIR)/render.o \
				$(OBJ_DIR)/gfx.o \
				$(OBJ_DIR)/display.o \
				$(OBJ_DIR)/ctl.o \
				$(OBJ_DIR)/tex.o \
				$(OBJ_DIR)/noise.o \
				$(OBJ_DIR)/main.o

# Noise test objects:
NTOBJECTS=$(OBJ_DIR)/noise.o \
					$(OBJ_DIR)/test_noise.o

# The default goal:
.DEFAULT_GOAL := game

.PHONY: all clean game test_noise checkgl

all: game test_noise
clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)
	rm -rf $(OUT_DIR)
game: $(BIN_DIR)/elf_forest
test_noise: $(TEST_DIR)/noise_test_2D.ppm $(TEST_DIR)/noise_test_3D.ppm \
	          $(TEST_DIR)/noise_test_2D_F.ppm $(TEST_DIR)/noise_test_3D_F.ppm \
						$(TEST_DIR)/noise_test_ex.ppm
checkgl:
	$(CC) -E check_gl_version.h | tail -n 8

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

*.c: ;
*.h: ;

$(OBJ_DIR)/obj.d:: *.c *.h $(OBJ_DIR)
	gcc -MM *.c | sed "s/^\([^ ]\)/$(OBJ_DIR)\/\1/" > $(OBJ_DIR)/obj.d

Makefile: ;

# Make sure that we remake the dependency file before building:
#Makefile: $(OBJ_DIR)/obj.d

include $(OBJ_DIR)/obj.d

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/elf_forest: $(OBJECTS) $(BIN_DIR)
	$(CC) $(OBJECTS) $(LFLAGS) -o $(BIN_DIR)/elf_forest

$(BIN_DIR)/test_noise: $(NTOBJECTS) $(BIN_DIR)
	$(CC) $(NTOBJECTS) $(LFLAGS) -o $(BIN_DIR)/test_noise

$(TEST_DIR)/noise_test%2D.ppm $(TEST_DIR)/noise_test%3D.ppm \
  $(TEST_DIR)/noise_test%2D_F.ppm $(TEST_DIR)/noise_test%3D_F.ppm \
  $(TEST_DIR)/noise_test%ex.ppm: $(BIN_DIR)/test_noise $(TEST_DIR)
	cd $(TEST_DIR) && ../../$(BIN_DIR)/test_noise
