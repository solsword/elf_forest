#ifndef WORLDGEN_H
#define WORLDGEN_H

// worldgen.h
// World map generation.

#include <stdint.h>

#include "noise/noise.h"
#include "world/blocks.h"
#include "datatypes/list.h"

#include "terrain.h"
#include "geology.h"
#include "climate.h"

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

// Bits per world region (8 -> 256x256 chunks).
// 128*96 = 12288 regions
// 12288 * 256*256*(?=512) = 412316860416 chunks
// 412316860416 chunks * 384 KB/chunk = 144 petabytes
// 96*256*32 = 786432 blocks ~= 524300 meters ~= 525 km
// 320*256*32 = 2621440 blocks ~= 1747500 meters ~= 1750 km <-
// 512*256*32 = 4194304 blocks ~= 2796000 meters ~= 2800 km
#define WORLD_REGION_BITS 8
#define WORLD_REGION_SIZE (1 << WORLD_REGION_BITS)
#define WORLD_REGION_BLOCKS (WORLD_REGION_SIZE * CHUNK_SIZE)

// How often to sample world regions (in chunks) to determine approximate
// min/max/mean height data. This should be a power of 2, and of course should
// be less than WORLD_REGION_SIZE.
//#define REGION_HEIGHT_SAMPLE_FREQUENCY 16
#define REGION_HEIGHT_SAMPLE_FREQUENCY 32

// Controls the size of strata relative to the world map size.
#define STRATA_AVG_SIZE 0.25

// Controls how many strata to generate (a multiple of MAX_STRATA_LAYERS).
//#define STRATA_COMPLEXITY 3.0
#define STRATA_COMPLEXITY (1/32.0)

// The base stratum thickness (before an exponential distribution).
#define BASE_STRATUM_THICKNESS 10.0

// Maximum number of biomes that can overlap in the same world region
#define MAX_BIOME_OVERLAP 4

// Beach height above sea level:
#define BEACH_BASE_HEIGHT 7
#define BEACH_HEIGHT_VAR 6
#define BEACH_HEIGHT_NOISE_SCALE (1.0 / 70.0)

// Soil alt base scale
#define SOIL_ALT_NOISE_SCALE (1.0 / 120.0)

// Soil alternate threshold:
#define SOIL_ALT_THRESHOLD 0.5

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
extern char const * const WORLD_MAP_FILE_BASE;
extern char const * const WORLD_MAP_FILE_WIND;
extern char const * const WORLD_MAP_FILE_RAIN;

// Maximum distance between two world region anchors:
#define MAX_REGION_ANCHOR_DISTANCE sqrtf( \
  WORLD_REGION_BLOCKS * WORLD_REGION_BLOCKS + \
  WORLD_REGION_BLOCKS * WORLD_REGION_BLOCKS + \
  TR_MAX_HEIGHT * 0.6 * TR_MAX_HEIGHT * 0.6 \
)

// The strength and base scale of the noise that affects region contenders:
#define REGION_CONTENTION_NOISE_STRENGTH 0.4
#define REGION_CONTENTION_NOISE_SCALE (1.0 / 50.0)

// The variance and frequency of noise used to determine which region's stratum
// information is used when generating terrain.
#define REGION_GEO_STRENGTH_VARIANCE 0.5
#define REGION_GEO_STRENGTH_FREQUENCY 1.6

// The strength and base scale of the noise that distorts strata boundaries:
#define STRATA_FRACTION_NOISE_STRENGTH 16
#define STRATA_FRACTION_NOISE_SCALE (1.0 / 40.0)

// How long the water cycle should be simulated:
#define WATER_CYCLE_SIM_STEPS 64
#define WATER_CYCLE_FINISH_STEPS 1

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

struct climate_info_s {
  hydrology water;
  weather atmosphere;
  soil_composition soil;
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
  world_map *world;
  world_map_pos pos;
  region_pos anchor;
  // topology info:
  float min_height;
  float mean_height;
  float max_height;
  manifold_point gross_height; // an averaged local manifold
  world_region *downhill;
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
  list *all_water;
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
  anchor->x += float_hash_1d(hash) * (WORLD_REGION_BLOCKS - 1);
  hash += 1;
  anchor->y += float_hash_1d(hash) * (WORLD_REGION_BLOCKS - 1);
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

// Temperature influence on evaporation.
static inline float temp_evap_influence(float temp) {
  temp *= EVAPORATION_TEMP_SCALING;
  temp = 0.5 + 0.6 * temp;
  if (temp < 0) { temp = 0; }
  return temp;
}

// Computes base evaporation for the given world region.
static inline float evaporation(world_region *wr) {
  float temp, elev, slope;
  temp = temp_evap_influence(wr->climate.atmosphere.mean_temp);
  if (wr->climate.water.body != NULL) {
    return BASE_WATER_CLOUD_POTENTIAL * temp;
    // TODO: depth/size-based differentials?
  } else {
    elev = (
      (wr->mean_height - TR_HEIGHT_SEA_LEVEL)
    /
      (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL)
    );
    if (elev < 0) { elev = 0; }
    elev *= elev;
    slope = mani_slope(&(wr->gross_height));
    if (slope > 1.5) { slope = 1.5; }
    slope /= 1.5;
    slope = pow(slope, 0.4);
    return BASE_LAND_CLOUD_POTENTIAL * temp * (1 - elev) * (1 - slope);
  }
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
void setup_worldgen(ptrdiff_t seed);

// Cleans up the world map system.
void cleanup_worldgen();

// Computes the cell contents at the given position.
void world_cell(world_map *wm, region_pos *pos, cell *result);

// Generates geology for the given world.
void generate_geology(world_map *wm);

// Generates hydrology (rivers, lakes, and the oceans) for the given world.
void generate_hydrology(world_map *wm);

// Generates climate for the given world.
void generate_climate(world_map *wm);

// Once base climate generation is complete, this will (crudely) simulate the
// water cycle, populating precipitation information.
void simulate_water_cycle(world_map *wm);

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
  float h, float elev,
  world_region *wr,
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

// Takes an origin point and a water body and fills ares of the given world map
// as part of that water body between the given size limits. The size limits
// will be ignored if they are negative. If the body of water turns out to be
// too small or too large, the entire operation is cancelled, and the return
// value will be 0. Otherwise, the return value is 1 and the regions filled
// will have their hydrology info set to point to the given body of water.
int fill_water(
  world_map *wm,
  body_of_water *body,
  world_map_pos *origin,
  int min_size,
  int max_size
);

// Takes a world map position and uses downhill information to find a terrain
// local minimum. The input position is edited directly.
void find_valley(world_map *wm, world_map_pos *pos);

#endif // ifndef WORLDGEN_H
