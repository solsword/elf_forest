#ifndef WORLDGEN_H
#define WORLDGEN_H

// worldgen.h
// World map generation.

#include <stdint.h>

#include "noise/noise.h"
#include "world/blocks.h"

#include "terrain.h"
#include "geology.h"
#include "climate.h"

/*************
 * Constants *
 *************/

// World width and height in regions:
// 768*512 = 393216 regions
//#define WORLD_WIDTH 768
//#define WORLD_HEIGHT 512
// 400*320 = 144000 regions
//#define WORLD_WIDTH 400
//#define WORLD_HEIGHT 360
// 128*108 = 13824 regions
#define WORLD_WIDTH 128
#define WORLD_HEIGHT 108
// 96*96 = 9216 regions
//#define WORLD_WIDTH 96
//#define WORLD_HEIGHT 96
// 32*32 = 1024 regions
//#define WORLD_WIDTH 32
//#define WORLD_HEIGHT 32

// The name of the file to write a copy of the world map into:
extern char const * const WORLD_MAP_FILE_BASE;
extern char const * const WORLD_MAP_FILE_REGIONS;
extern char const * const WORLD_MAP_FILE_TEMP;
extern char const * const WORLD_MAP_FILE_WIND;
extern char const * const WORLD_MAP_FILE_EVAP;
extern char const * const WORLD_MAP_FILE_CLOUDS;
extern char const * const WORLD_MAP_FILE_PQ;
extern char const * const WORLD_MAP_FILE_RAIN;
extern char const * const WORLD_MAP_FILE_LRAIN;

/*************
 * Functions *
 *************/

// Sets up the world map system, including generating the main world map.
void setup_worldgen(ptrdiff_t seed);

// Cleans up the world map system.
void cleanup_worldgen();

// Initializes the given world map.
void init_world_map(world_map *wm);

// Computes the cell contents at the given position.
void world_cell(world_map *wm, global_pos *pos, cell *result);

#endif // ifndef WORLDGEN_H
