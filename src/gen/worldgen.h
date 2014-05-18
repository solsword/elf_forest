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

// A world map is composed of many small rectangular regions.
struct world_map_s;
typedef struct world_map_s world_map;

// A region of the world map holds information about geology, climate, ecology,
// and anthropology.
struct world_region_s;
typedef struct world_region_s world_region;

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

/*************
 * Constants *
 *************/

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

/*************************
 * Structure Definitions *
 *************************/

struct world_map_s {
  size_t width, height;
  world_region *regions;
};

struct world_region_s {
  strata_info geology;
  climate_info climate;
  biome_info ecology;
  civ_info anthropology;
};

struct strata_info_s { // indexed starting from the bottom
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
  block_variant   mushroom_species[MAX_BIOME_MUSHROOMS];
  float           mushroom_frequencies[MAX_BIOME_MUSHROOMS];
  block_variant   moss_species[MAX_BIOME_MOSSES];
  float           moss_frequencies[MAX_BIOME_MOSSES];
  block_variant   grass_species[MAX_BIOME_GRASSES];
  float           grass_frequencies[MAX_BIOME_GRASSES];
  block_variant   vine_species[MAX_BIOME_VINES];
  float           vine_frequencies[MAX_BIOME_VINES];
  block_variant   herb_species[MAX_BIOME_HERBS];
  float           herb_frequencies[MAX_BIOME_HERBS];
  block_variant   bush_species[MAX_BIOME_BUSHES];
  float           bush_frequencies[MAX_BIOME_BUSHES];
  block_variant   shrub_species[MAX_BIOME_SHRUBS];
  float           shrub_frequencies[MAX_BIOME_SHRUBS];
  block_variant   tree_species[MAX_BIOME_TREES];
  float           tree_frequencies[MAX_BIOME_TREES];
};

/********************
 * Inline Functions *
 ********************/


/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new world map of the given size.
world_map *create_world_map(size_t width, size_t height);

/*************
 * Functions *
 *************/

#endif // ifndef WORLDGEN_H
