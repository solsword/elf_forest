#ifndef WORLD_MAP_H
#define WORLD_MAP_H

// world_map.h
// World map structure definition.

#include <stdint.h>

#include "noise/noise.h"
#include "world/blocks.h"
#include "world/species.h"
#include "datatypes/list.h"
#include "math/manifold.h"
#include "math/functions.h"

/*********
 * Enums *
 *********/

// Geology
// -------

// Geologic sources influence how material types and other stratum parameters
// are chosen.
enum geologic_source_e {
  GEO_IGNEOUS,
  GEO_METAMORPHIC,
  GEO_SEDIMENTAY
};
typedef enum geologic_source_e geologic_source;

// Climate & Hydrology
// -------------------

enum hydro_state_e {
  HYDRO_LAND = 0,
  HYDRO_WATER = 1,
  HYDRO_SHORE = 2,
};
typedef enum hydro_state_e hydro_state;

enum salinity_e {
  SALINITY_FRESH = 0,
  SALINITY_BRACKISH = 1,
  SALINITY_SALINE = 2,
  SALINITY_BRINY = 3,
};
typedef enum salinity_e salinity;

/************************
 * Types and Structures *
 ************************/

// General
// -------

typedef ptrdiff_t wm_pos_t;

// Position within a world map.
struct world_map_pos_s;
typedef struct world_map_pos_s world_map_pos;

// A region of the world map holds information about geology, climate, ecology,
// and anthropology.
struct world_region_s;
typedef struct world_region_s world_region;

// A world map is composed of many small rectangular regions.
struct world_map_s;
typedef struct world_map_s world_map;

// Info
// ----

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

// Geology
// -------

// A layer of material that encompasses some region of the world.
struct stratum_s;
typedef struct stratum_s stratum;

// Climate & Hydrology
// -------------------

struct body_of_water_s;
typedef struct body_of_water_s body_of_water;

struct hydrology_s;
typedef struct hydrology_s hydrology;

struct weather_s;
typedef struct weather_s weather;

struct soil_composition_s;
typedef struct soil_composition_s soil_composition;

// Biology
// -------

// A biome is a mix of species.
struct biome_s;
typedef struct biome_s biome;

/*************
 * Constants *
 *************/

// General
// -------

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

// World region anchors will be between 20% and 80% of this height. This should
// be related to the various TR_HEIGHT constants in gen/terrain.h
#define WORLD_REGION_ANCHOR_HEIGHT 16000

// Maximum distance between two world region anchors:
#define MAX_REGION_ANCHOR_DISTANCE sqrtf( \
  WORLD_REGION_BLOCKS * WORLD_REGION_BLOCKS + \
  WORLD_REGION_BLOCKS * WORLD_REGION_BLOCKS + \
  WORLD_REGION_ANCHOR_HEIGHT * 0.6 * WORLD_REGION_ANCHOR_HEIGHT * 0.6 \
)

// The strength and base scale of the noise that affects region contenders:
#define REGION_CONTENTION_NOISE_STRENGTH 0.5
#define REGION_CONTENTION_NOISE_SCALE (1.0 / 50.0)
#define REGION_CONTENTION_POLAR_STRENGTH 0.7
#define REGION_CONTENTION_POLAR_SCALE 1.6


// Geology
// -------

// Maximum number of stone layers per world region
#define MAX_STRATA_LAYERS 256

// Maximum number of material types present in other layers as veins
#define N_VEIN_TYPES 2

// Maximum number of material types included in other layers
#define N_INCLUSION_TYPES 8


// Climate & Hydrology
// -------------------

// Number of seasons in the year
#define N_SEASONS 4

// Maximum alternate dirt/sand types:
#define MAX_ALT_DIRTS 5
#define MAX_ALT_SANDS 3


// Biology
// -------

// Maximum number of biomes that can overlap in the same world region
#define MAX_BIOME_OVERLAP 4

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

// The globally-accessible world:
extern world_map* THE_WORLD;

/*************************
 * Structure Definitions *
 *************************/

// Biology
// -------

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

// Climate & Hydrology
// -------------------

struct body_of_water_s {
  float level;
  salinity salt;
};

struct hydrology_s {
  hydro_state state; // what kind of region this is
  body_of_water *body; // what body of water this region belongs to
  r_pos_t water_table; // how high the water table is, in blocks
  salinity salt; // the salinity of local groundwater
};

struct weather_s {
  // TODO: wind chaos?
  float wind_strength, wind_direction; // wind strength & direction
  float mean_temp; // overall average temperature
  float cloud_potential; // average annual precipitation potential in mm/year
  float next_cloud_potential; // next iteration cloud potential
  float precipitation_quotient; // How much of local clouds rains here
  float total_precipitation; // summed precipitation during simulation
  float next_total_precipitation; // next iteration precipitation
  float rainfall[N_SEASONS]; // rainfall per season in mm/year
  float temp_low[N_SEASONS]; // temperature low, mean and high throughout the
  float temp_mean[N_SEASONS]; // day, in each season
  float temp_high[N_SEASONS];
};

struct soil_composition_s {
  species base_dirt; // dirt (/sand/mud) species for normal soil
  block alt_dirt_blocks[MAX_ALT_DIRTS];
  species alt_dirt_species[MAX_ALT_DIRTS];
  float alt_dirt_strengths[MAX_ALT_DIRTS];
  float alt_dirt_hdeps[MAX_ALT_DIRTS]; // height-dependence
  species base_sand; // sand species (for beaches, rivers, & oceans)
  block alt_sand_blocks[MAX_ALT_SANDS];
  species alt_sand_species[MAX_ALT_SANDS];
  float alt_sand_strengths[MAX_ALT_SANDS];
  float alt_sand_hdeps[MAX_ALT_SANDS]; // height-dependence
};

// Geology
// -------

struct stratum_s {
  ptrdiff_t seed; // seed for various noise sources

 // Base parameters:
 // ----------------
  float cx, cy; // center x/y
  float size; // base radius
  float thickness; // base thickness
  map_function profile; // base profile shape
  geologic_source source; // where the material for this layer comes from

 // Derived noise parameters:
 // -------------------------
  float persistence; // how much this layer extends at the expense of others
   // Note that larger values are stronger; [0.5, 2] is reasonable.
  float scale_bias; // biases the noise scales

  // radial variance:
  float radial_frequency;
  float radial_variance;

  // distortion:
  float gross_distortion; // large-scale distortion
  float fine_distortion; // small-scale distortion

  // core variation (expressed in max blocks)
  float large_var; // large-scale variation
  float med_var; // medium-scale variation
  float small_var; // small-scale variation
  float tiny_var; // tiny-scale variation

  // positive detail (expressed in max blocks)
  float detail_var; // detail-scale variation
  float ridges; // amplitude of ridges

  // negative detail:
  float smoothing; // amount of smooth weathering (fraction of detail removed)

 // Derived vein and inclusion information:
 // ---------------------------------------
  float vein_scale[N_VEIN_TYPES]; // scale of different veins (in blocks)
  float vein_strength[N_VEIN_TYPES]; // thickness and frequency of veins (0-1)
  float inclusion_frequency[N_INCLUSION_TYPES]; // frequency of inclusions (0-1)

 // Derived species information:
 // ----------------------------------
  species base_species; // exact material type for main mass
  species vein_species[N_VEIN_TYPES]; // types for veins
  species inclusion_species[N_INCLUSION_TYPES]; // types for inclusions
};

// Info
// ----

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

// General
// -------

struct world_map_pos_s {
  wm_pos_t x, y;
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
  anchor->z = WORLD_REGION_ANCHOR_HEIGHT * ( 0.2 + 0.6 * float_hash_1d(hash));
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new world map of the given size. More work is needed
// to fill in proper values: see init_world_map in gen/worldgen.c.
world_map *create_world_map(ptrdiff_t seed, wm_pos_t width, wm_pos_t height);

// Cleans up the given world map and frees the associated memory.
void cleanup_world_map(world_map *wm);

/*************
 * Functions *
 *************/

// Computes the two closest world region anchors to the given point, along with
// the strengths of each.
void compute_region_contenders(
  world_map *wm,
  world_region* neighborhood[],
  region_pos *rpos,
  world_region **best, world_region **secondbest,
  float *strbest, float *strsecond
);

// Takes a world map position and uses downhill information to find a terrain
// local minimum. The input position is edited directly.
void find_valley(world_map *wm, world_map_pos *pos);

#endif // ifndef WORLD_MAP_H
