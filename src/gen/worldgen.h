#ifndef WORLDGEN_H
#define WORLDGEN_H

// worldgen.h
// World map generation.

#include <stdint.h>

#include "noise/noise.h"
#include "world/blocks.h"

#include "geology.h"

/************************
 * Types and Structures *
 ************************/

typedef size_t wm_pos_t;

// Position within a world map.
struct world_map_pos_s;
typedef struct world_map_pos_s world_map_pos;

// Geological information for a world region.
struct strata_info_s;
typedef struct strata_info_s strata_info;

// Climate information for a world region.
struct climate_info_s;
typedef struct climate_info_s climate_info;

// Ecological information for a world region.
struct biome_info_s;
typedef struct biome_info_s biome_info;

// Civilization information for a world region.
struct civ_info_s;
typedef struct civ_info_s civ_info;

// A biome is a mix of species.
struct biome_s;
typedef struct biome_s biome;

// A region of the world map holds information about geology, climate, ecology,
// and anthropology.
struct world_region_s;
typedef struct world_region_s world_region;

// A world map is composed of many small rectangular regions.
struct world_map_s;
typedef struct world_map_s world_map;

/*************
 * Constants *
 *************/

// Bits per world region (6 -> 64x64 blocks).
#define WORLD_REGION_BITS 6

// Controls number of strata to generate as a multiple of MAX_STRATA_LAYERS,
// and indirectly controls strata size.
#define STRATA_COMPLEXITY 3

// Maximum number of biomes that can overlap in the same world region
#define MAX_BIOME_OVERLAP 4

// Number of seasons in the year
#define N_SEASONS 5

// Biome plant variant caps
#define MAX_BIOME_MUSHROOMS 16
#define MAX_BIOME_MOSSES 16
#define MAX_BIOME_GRASSES 32
#define MAX_BIOME_VINES 16
#define MAX_BIOME_HERBS 128
#define MAX_BIOME_BUSHES 32
#define MAX_BIOME_SHRUBS 32
#define MAX_BIOME_TREES 64

/***********
 * Globals *
 ***********/

extern world_map* THE_WORLD;

extern ptrdiff_t const WORLD_SEED;

extern wm_pos_t const WORLD_WIDTH;
extern wm_pos_t const WORLD_HEIGHT;

/*************************
 * Structure Definitions *
 *************************/

struct world_map_pos_s {
  wm_pos_t x, y;
};

struct strata_info_s { // indexed starting from the bottom
  size_t stratum_count;
  stratum *strata[MAX_STRATA_LAYERS]; // the layers that intersect this region
};

struct climate_info_s {
  float rainfall[N_SEASONS]; // rainfall per season
  float temp_low[N_SEASONS]; // temperature low, mean and high throughout the
  float temp_mean[N_SEASONS]; // day, in each season
  float temp_high[N_SEASONS];
  size_t water_table; // how high the water table is, in blocks
  uint8_t fresh_water; // whether groundwater here is fresh or salty
};

struct biome_info_s {
  biome* biomes[MAX_BIOME_OVERLAP];
};

struct civ_info_s {
  // TODO: HERE
  size_t population;
};

struct biome_s {
  // species IDs and frequencies for plant types:
  // mushrooms, mosses, grasses, vines, herbs, bushes, shrubs, and trees
  // (aquatic grasses, plants and corals are stored as grasses, vines, and
  // bushes respectively)
  species  mushroom_species[MAX_BIOME_MUSHROOMS];
  float    mushroom_frequencies[MAX_BIOME_MUSHROOMS];
  species  moss_species[MAX_BIOME_MOSSES];
  float    moss_frequencies[MAX_BIOME_MOSSES];
  species  grass_species[MAX_BIOME_GRASSES];
  float    grass_frequencies[MAX_BIOME_GRASSES];
  species  vine_species[MAX_BIOME_VINES];
  float    vine_frequencies[MAX_BIOME_VINES];
  species  herb_species[MAX_BIOME_HERBS];
  float    herb_frequencies[MAX_BIOME_HERBS];
  species  bush_species[MAX_BIOME_BUSHES];
  float    bush_frequencies[MAX_BIOME_BUSHES];
  species  shrub_species[MAX_BIOME_SHRUBS];
  float    shrub_frequencies[MAX_BIOME_SHRUBS];
  species  tree_species[MAX_BIOME_TREES];
  float    tree_frequencies[MAX_BIOME_TREES];
};

// Each world region stores info on geology, climate, ecology, and
// anthropoloogy.
struct world_region_s {
  strata_info geology;
  climate_info climate;
  biome_info ecology;
  civ_info anthropology;
};

// The world map is a grid of regions, with lists of all strata, biomes, and
// civilizations.
struct world_map_s {
  ptrdiff_t seed;
  wm_pos_t width, height;
  world_region *regions;
  list *all_strata;
  list *all_biomes;
  list *all_civs;
};

/********************
 * Inline Functions *
 ********************/

static inline void wmpos__rpos(
  world_map_pos const * const wmpos,
  region_pos *rpos
) {
  rpos->x = ((r_pos_t) wmpos->x) << WORLD_REGION_BITS;
  rpos->y = ((r_pos_t) wmpos->y) << WORLD_REGION_BITS;
  rpos->z = 0;
}

static inline void rpos__wmpos(
  region_pos const * const rpos,
  world_map_pos *wmpos
) {
  wmpos->x = ((wm_pos_t) rpos->x) >> WORLD_REGION_BITS;
  wmpos->y = ((wm_pos_t) rpos->y) >> WORLD_REGION_BITS;
}

static inline world_region* get_world_region(
    world_map *wm,
    world_map_pos *wmpos
) {
  return &(wm->regions[wmpos->x+wmpos->y*wm->width]);
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new world map of the given size.
world_map *create_world_map(ptrdiff_t seed, wm_pos_t width, wm_pos_t height);

// Cleans up the given world map and frees the associated memory.
void cleanup_world_map(world_map *wm);

/*************
 * Functions *
 *************/

// Sets up the world map system, including generating the main world map.
void setup_world_map();

// Computes the cell contents at the given position.
void world_cell(region_pos *pos, cell *result);

// Generates geology for the given world.
void generate_geology(world_map *wm);

// Computes the cell contents at the given position based on strata.
void strata_cell(
  world_region *wr,
  region_pos *rpos,
  cell *result
);

#endif // ifndef WORLDGEN_H
