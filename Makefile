# Directories:
OBJ_DIR=obj
BIN_DIR=bin
OUT_DIR=out
SRC_DIR=src
TEST_DIR=$(OUT_DIR)/test

# Don't worry about modification times on output directories:
MAKEFLAGS+=--assume-old=$(OBJ_DIR)
MAKEFLAGS+=--assume-old=$(BIN_DIR)
MAKEFLAGS+=--assume-old=$(OUT_DIR)
MAKEFLAGS+=--assume-old=$(TEST_DIR)

# Compiler & Flags:
CC=gcc
#DEBUG_FLAGS=-g -O1 -DDEBUG -DDEBUG_DETECT_JUMPS -DPROFILE_MEM -DPROFILE_TIME
DEBUG_FLAGS=-g -O1 -DDEBUG -DDEBUG_DETECT_JUMPS -DPROFILE_MEM -DPROFILE_TIME
PROFILE_FLAGS=
#PROFILE_FLAGS=-pg
#PROFILE_FLAGS=-pg -fprofile-arcs -ftest-coverage
OPT_FLAGS=-O3
INCLUDE_FLAGS=-fopenmp -I/usr/include/freetype2 -I$(SRC_DIR)
CFLAGS=-c -Wall -ffast-math $(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(PROFILE_FLAGS)
#CFLAGS=-c -Wall -ffast-math $(INCLUDE_FLAGS) $(OPT_FLAGS) $(PROFILE_FLAGS)

LIBS_OPENGL=-lGLEW -lGL -lGLU
LIBS_GLFW=-lglfw -lrt -lXrandr -lXi -lXxf86vm -lXrender -lXext -lX11 \
          -lpthread -lxcb -lXau -lXdmcp
LIBS_FTGL=-lftgl
LIBS_PNG=-lpng
LIBS_OPENMP=-fopenmp
LIBS_UNISTRING=-lunistring
LIBS=$(LIBS_OPENGL) $(LIBS_GLFW) $(LIBS_FTGL) $(LIBS_PNG) $(LIBS_OPENMP) $(LIBS_UNISTRING)

LFLAGS=-lm $(LIBS) $(PROFILE_FLAGS)

# Objects:
CORE_OBJECTS=$(OBJ_DIR)/world.o \
             $(OBJ_DIR)/blocks.o \
             $(OBJ_DIR)/world_map.o \
             $(OBJ_DIR)/materials.o \
             $(OBJ_DIR)/chunk_data.o \
             $(OBJ_DIR)/render.o \
             $(OBJ_DIR)/gfx.o \
             $(OBJ_DIR)/display.o \
             $(OBJ_DIR)/dta.o \
             $(OBJ_DIR)/curve.o \
             $(OBJ_DIR)/functions.o \
             $(OBJ_DIR)/draw.o \
             $(OBJ_DIR)/pipeline.o \
             $(OBJ_DIR)/ctl.o \
             $(OBJ_DIR)/interact.o \
             $(OBJ_DIR)/tex.o \
             $(OBJ_DIR)/noise.o \
             $(OBJ_DIR)/physics.o \
             $(OBJ_DIR)/entities.o \
             $(OBJ_DIR)/species.o \
             $(OBJ_DIR)/vbo.o \
             $(OBJ_DIR)/tick.o \
             $(OBJ_DIR)/vector.o \
             $(OBJ_DIR)/list.o \
             $(OBJ_DIR)/queue.o \
             $(OBJ_DIR)/map.o \
             $(OBJ_DIR)/string.o \
             $(OBJ_DIR)/bitmap.o \
             $(OBJ_DIR)/heightmap.o \
             $(OBJ_DIR)/octree.o \
             $(OBJ_DIR)/terrain.o \
             $(OBJ_DIR)/worldgen.o \
             $(OBJ_DIR)/data.o \
             $(OBJ_DIR)/persist.o \
             $(OBJ_DIR)/climate.o \
             $(OBJ_DIR)/geology.o \
             $(OBJ_DIR)/biology.o \
             $(OBJ_DIR)/txgen.o \
             $(OBJ_DIR)/txg_plants.o \
             $(OBJ_DIR)/txg_minerals.o \
             $(OBJ_DIR)/cartography.o \
             $(OBJ_DIR)/grow.o \
             $(OBJ_DIR)/ptime.o \
             $(OBJ_DIR)/pmem.o \
             $(OBJ_DIR)/jobs.o \
             $(OBJ_DIR)/ui.o

MAIN_OBJECTS=$(OBJ_DIR)/main.o

TEST_OBJECTS=$(OBJ_DIR)/test.o

VIEWER_OBJECTS=$(OBJ_DIR)/viewer.o

UNIT_TEST_OBJECTS=$(OBJ_DIR)/test_suite.o \
						      $(OBJ_DIR)/unit_tests.o

NOISE_TEST_OBJECTS=$(OBJ_DIR)/noise.o \
          $(OBJ_DIR)/test_noise.o

NOISE_PERF_OBJECTS=$(OBJ_DIR)/noise.o \
          $(OBJ_DIR)/ptime.o \
          $(OBJ_DIR)/test_noiseperf.o

CHECKGL_OBJECTS=$(OBJ_DIR)/check_gl_version.o

# The default goal:
.DEFAULT_GOAL := game

.PHONY: all clean game test viewer unit_tests test_noise checkgl

all: game test viewer unit_tests test_noise
clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)
	rm -rf $(OUT_DIR)
game: $(BIN_DIR)/elf_forest
test: $(BIN_DIR)/test
viewer: $(BIN_DIR)/viewer
unit_tests: $(BIN_DIR)/unit_tests
noise_perf: $(BIN_DIR)/noise_perf
test_noise: $(BIN_DIR)/test_noise $(TEST_DIR)
	cd $(TEST_DIR) && ../../$(BIN_DIR)/test_noise
checkgl: $(BIN_DIR)/checkgl
	./$(BIN_DIR)/checkgl

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

$(SRC_DIR)/*.c: ;
$(SRC_DIR)/*/*.c: ;
$(SRC_DIR)/*.h: ;
$(SRC_DIR)/*/*.h: ;

$(OBJ_DIR)/obj.d: \
$(SRC_DIR)/*.c \
$(SRC_DIR)/*/*.c \
$(SRC_DIR)/*.h \
$(SRC_DIR)/*/*.h  \
$(OBJ_DIR)
	$(CC) -MM $(INCLUDE_FLAGS) $(SRC_DIR)/*.c $(SRC_DIR)/*/*.c | sed "s/^\([^ ]\)/$(OBJ_DIR)\/\1/" >\
		$(OBJ_DIR)/obj.d

Makefile: ;

# Make sure that we remake the dependency file before building:
#Makefile: $(OBJ_DIR)/obj.d

include $(OBJ_DIR)/obj.d

# How to build things based on their dependencies as specified in obj.d:
$(OBJ_DIR)/%.o:
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/elf_forest: $(CORE_OBJECTS) $(MAIN_OBJECTS) $(BIN_DIR) $(OUT_DIR)
	$(CC) $(CORE_OBJECTS) $(MAIN_OBJECTS) $(LFLAGS) -o $(BIN_DIR)/elf_forest

$(BIN_DIR)/test: $(CORE_OBJECTS) $(TEST_OBJECTS) $(BIN_DIR) $(OUT_DIR)
	$(CC) $(CORE_OBJECTS) $(TEST_OBJECTS) $(LFLAGS) -o $(BIN_DIR)/test

$(BIN_DIR)/viewer: $(CORE_OBJECTS) $(VIEWER_OBJECTS) $(BIN_DIR) $(OUT_DIR)
	$(CC) $(CORE_OBJECTS) $(VIEWER_OBJECTS) $(LFLAGS) -o $(BIN_DIR)/viewer

$(BIN_DIR)/unit_tests: $(CORE_OBJECTS) $(UNIT_TEST_OBJECTS) $(BIN_DIR) \
$(TEST_DIR) $(OUT_DIR)
	$(CC) $(CORE_OBJECTS) $(UNIT_TEST_OBJECTS) $(LFLAGS) -o $(BIN_DIR)/unit_tests

$(BIN_DIR)/test_noise: $(NOISE_TEST_OBJECTS) $(BIN_DIR)
	$(CC) $(NOISE_TEST_OBJECTS) $(LFLAGS) -o $(BIN_DIR)/test_noise

$(BIN_DIR)/noise_perf: $(NOISE_PERF_OBJECTS) $(BIN_DIR)
	$(CC) $(NOISE_PERF_OBJECTS) $(LFLAGS) -o $(BIN_DIR)/noise_perf

$(BIN_DIR)/checkgl: $(CHECKGL_OBJECTS) $(BIN_DIR)
	$(CC) $(CHECKGL_OBJECTS) $(LFLAGS) -o $(BIN_DIR)/checkgl
