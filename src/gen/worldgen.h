#ifndef WORLDGEN_H
#define WORLDGEN_H

// worldgen.h
// World map generation.

#include <stdint.h>

#include "noise/noise.h"
#include "world/blocks.h"
#include "jobs/jobs.h"
#include "gen/terrain.h"

#include "geology.h"

/************************
 * Types and Structures *
 ************************/

typedef ptrdiff_t wm_pos_t;

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

// World width and height in regions:
// 768*512 = 393216 regions
//#define WORLD_WIDTH 768
//#define WORLD_HEIGHT 512
// 400*320 = 128000 regions
#define WORLD_WIDTH 400
#define WORLD_HEIGHT 320
// 128*96 = 12288 regions
//#define WORLD_WIDTH 128
//#define WORLD_HEIGHT 96

// Bits per world region (8 -> 256x256 chunks).
// 128*96 = 12288 regions
// 12288 * 256*256*(?=512) = 412316860416 chunks
// 412316860416 chunks * 384 KB/chunk = 144 petabytes
// 96*256*32 = 786432 blocks ~= 524300 meters ~= 525 km
// 320*256*32 = 2621440 blocks ~= 1747500 meters ~= 1750 km <-
// 512*256*32 = 4194304 blocks ~= 2796000 meters ~= 2800 km
#define WORLD_REGION_BITS 8
#define WORLD_REGION_SIZE (1 << WORLD_REGION_BITS)

// Controls the size of strata relative to the world map size.
#define STRATA_AVG_SIZE 0.25

// Controls how many strata to generate (a multiple of MAX_STRATA_LAYERS).
#define STRATA_COMPLEXITY 3.0
//#define STRATA_COMPLEXITY (1/32.0)

// The base stratum thickness (before an exponential distribution).
#define BASE_STRATUM_THICKNESS 10.0

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

// The name of the file to write a copy of the world map into:
extern char const * const WORLD_MAP_FILE;

// Maximum distance between two world region anchors:
#define MAX_REGION_ANCHOR_DISTANCE sqrtf( \
  (WORLD_REGION_SIZE * CHUNK_SIZE) * (WORLD_REGION_SIZE * CHUNK_SIZE) + \
  (WORLD_REGION_SIZE * CHUNK_SIZE) * (WORLD_REGION_SIZE * CHUNK_SIZE) + \
  TR_MAX_HEIGHT * 0.6 * TR_MAX_HEIGHT * 0.6 \
)

// The variance and frequency of noise used to determine which region's stratum
// information is used when generating terrain.
#define REGION_GEO_STRENGTH_VARIANCE 0.4
#define REGION_GEO_STRENGTH_FREQUENCY 2.0

// The strength and base scale of the noise that distorts strata boundaries:
#define STRATA_FRACTION_NOISE_STRENGTH 16
#define STRATA_FRACTION_NOISE_SCALE (1.0 / 40.0)

/***********
 * Globals *
 ***********/

// The globally-accessible world:
extern world_map* THE_WORLD;

/*************************
 * Structure Definitions *
 *************************/

struct world_map_pos_s {
  wm_pos_t x, y;
};

struct strata_info_s { // indexed starting from the bottom
  size_t stratum_count;
  stratum* strata[MAX_STRATA_LAYERS]; // the layers that intersect this region
  float total_height; // cumulative height in blocks (as float for division)
  float bottoms[MAX_STRATA_LAYERS]; // the bottom of each layer on [0, 1]
};

struct hydrology_info_s { // info on rivers, lakes, and the ocean
  size_t water_table; // how high the water table is, in blocks
  uint8_t water_salinity; // the salinity of local groundwater
}

struct climate_info_s {
  float rainfall[N_SEASONS]; // rainfall per season
  float temp_low[N_SEASONS]; // temperature low, mean and high throughout the
  float temp_mean[N_SEASONS]; // day, in each season
  float temp_high[N_SEASONS];
};

struct biome_info_s {
  biome* biomes[MAX_BIOME_OVERLAP];
};

struct civ_info_s {
  // TODO: this
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
// anthropoloogy, as well as an anchor position that's randomly placed
// somewhere within the region and an estimate of local terrain height.
struct world_region_s {
  ptrdiff_t seed;
  world_map_pos pos;
  region_pos anchor;
  // topology info:
  r_pos_t terrain_height;
  float downhill; // the local downhill direction in radians
  // various info modules:
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
  rpos->x = ((r_pos_t) wmpos->x) << (WORLD_REGION_BITS + CHUNK_BITS);
  rpos->y = ((r_pos_t) wmpos->y) << (WORLD_REGION_BITS + CHUNK_BITS);
  rpos->z = 0;
}

static inline void rpos__wmpos(
  region_pos const * const rpos,
  world_map_pos *wmpos
) {
  wmpos->x = (wm_pos_t) (rpos->x >> (WORLD_REGION_BITS + CHUNK_BITS));
  wmpos->y = (wm_pos_t) (rpos->y >> (WORLD_REGION_BITS + CHUNK_BITS));
}

static inline void wmpos__rcpos(
  world_map_pos const * const wmpos,
  region_chunk_pos *rcpos
) {
  rcpos->x = ((r_cpos_t) wmpos->x) << WORLD_REGION_BITS;
  rcpos->y = ((r_cpos_t) wmpos->y) << WORLD_REGION_BITS;
  rcpos->z = 0;
}

static inline void rcpos__wmpos(
  region_chunk_pos const * const rcpos,
  world_map_pos *wmpos
) {
  wmpos->x = (wm_pos_t) (rcpos->x >> WORLD_REGION_BITS);
  wmpos->y = (wm_pos_t) (rcpos->y >> WORLD_REGION_BITS);
}

static inline void copy_wmpos(
  world_map_pos const * const from,
  world_map_pos *to
) {
  to->x = from->x;
  to->y = from->y;
}


// Returns the region of the given world at the given position, or NULL if the
// given position is outside the given world.
static inline world_region* get_world_region(
    world_map *wm,
    world_map_pos *wmpos
) {
  if (
    (wmpos->x >= 0 && wmpos->x < wm->width)
  &&
    (wmpos->y >= 0 && wmpos->y < wm->height)
  ) {
    return &(wm->regions[wmpos->x+wmpos->y*wm->width]);
  } else {
    return NULL;
  }
}

static inline void compute_region_anchor(
  world_map *wm,
  world_map_pos const * const wmpos,
  region_pos *anchor
) {
  ptrdiff_t hash = hash_3d(wmpos->x, wmpos->y, wm->seed + 71);
  wmpos__rpos(wmpos, anchor);
  anchor->x += float_hash_1d(hash) * (WORLD_REGION_SIZE * CHUNK_SIZE - 1);
  hash += 1;
  anchor->y += float_hash_1d(hash) * (WORLD_REGION_SIZE * CHUNK_SIZE - 1);
  hash += 1;
  anchor->z = TR_MAX_HEIGHT * ( 0.2 + 0.6 * float_hash_1d(hash));
}

// Given a world region and a fractional height between 0 and 1, returns the
// stratum in that region at that height. If h is less than 0, it returns the
// bottom stratum in the given region, and likewise if h is greater than 1, it
// returns the top stratum.
static inline stratum* get_stratum(
  world_region* wr,
  float h
) {
  int i;
  stratum *result = wr->geology.strata[0];
  // Find out which layer we're in:
  for (i = 1; i < wr->geology.stratum_count; i += 1) {
    if (h < wr->geology.bottoms[i]) { // might not happen at all
      break;
    }
    result = wr->geology.strata[i];
  }
  return result;
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
void setup_worldgen();

// Cleans up the world map system.
void cleanup_worldgen();

// Computes the cell contents at the given position.
void world_cell(world_map *wm, region_pos *pos, cell *result);

// Generates geology for the given world.
void generate_geology(world_map *wm);

// Generates hydrology (rivers, lakes, and the ocean) for the given world.
void generate_hydrology(world_map *wm);

// Generates climate for the given world.
void generate_climate(world_map *wm);

// Computes the cell contents at the given position based on strata.
void strata_cell(
  world_map *wm,
  world_region* neighborhood[],
  region_pos* rpos,
  cell* result
);

// Computes a stone cell from within the base strata layers.
void stone_cell(
  world_map *wm, region_pos *rpos,
  float h, float ceiling,
  world_region *best, world_region *secondbest, float strbest, float strsecond,
  cell *result
);

// Computes a dirt cell from the dirt layer.
void dirt_cell(
  world_map *wm, region_pos *rpos,
  float ceiling,
  world_region *best, world_region *secondbest, float strbest, float strsecond,
  cell *result
);

// Computes the two closest world region anchors to the given point, along with
// the strengths of each.
void compute_region_contenders(
  world_map *wm,
  world_region* neighborhood[],
  region_pos *rpos,
  ptrdiff_t salt,
  world_region **best, world_region **secondbest,
  float *strbest, float *strsecond
);

/********
 * Jobs *
 ********/

// The 'gencolumn' job generates a single column of chunks, starting at z=0 and
// generating one chunk per step until enough chunks have been generated
// vertically that remaining chunks in that column are just air.
void launch_job_gencolumn(world_map *world, region_chunk_pos *target_chunk);
void (*job_gencolumn(void *jmem)) () ;
void (*job_gencolumn__init_column(void *jmem)) () ;
void (*job_gencolumn__fill_column(void *jmem)) () ;

#endif // ifndef WORLDGEN_H
