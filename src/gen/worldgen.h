#ifndef WORLDGEN_H
#define WORLDGEN_H

// worldgen.h
// World map generation.

#include <stdint.h>

#include "noise/noise.h"
#include "world/blocks.h"
#include "world/world_map.h"

#include "terrain.h"
#include "geology.h"
#include "climate.h"

/*************
 * Constants *
 *************/

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

// Computes manifold information for the given world map.
void compute_manifold(world_map *wm);

// Computes the cell contents at the given position.
void world_cell(world_map *wm, global_pos *pos, cell *result);

// Generates the contents of the given chunk, which should be initialized with
// a position. The chunk's existing block data (if any) will be overwritten.
// Note that there are some further steps before a chunk is fully polished,
// like adding biology, but these extra steps require basic terrain data
// generated here from multiple chunks.
void generate_chunk(chunk *c);

#endif // ifndef WORLDGEN_H
