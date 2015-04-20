#ifndef WORLD_MAP_H
#define WORLD_MAP_H

// world_map.h
// World map structure definition.

#include <stdint.h>

#include "noise/noise.h"
#include "world/blocks.h"
#include "world/species.h"
#include "datatypes/list.h"
#include "datatypes/vector.h"
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
  HYDRO_RIVER = 3,
};
typedef enum hydro_state_e hydro_state;

enum salinity_e {
  SALINITY_FRESH = 0,
  SALINITY_BRACKISH = 1,
  SALINITY_SALINE = 2,
  SALINITY_BRINY = 3,
};
typedef enum salinity_e salinity;

// Search/fill iteration algorithm steps:
enum search_step_e {
  SSTEP_INIT = 0,
  SSTEP_PROCESS = 1,
  SSTEP_CLEANUP = 2,
  SSTEP_FINISH = 3,
};
typedef enum search_step_e search_step;

// Search/fill iteration algorithm results:
enum step_result_e {
  SRESULT_CONTINUE = 0,
  SRESULT_FINISHED = 1,
  SRESULT_ABORT = 2,
  SRESULT_IGNORE = 3,
};
typedef enum step_result_e step_result;

/************************
 * Types and Structures *
 ************************/

// General
// -------

typedef ptrdiff_t wm_pos_t;

// Position within a world map using floats on [0, 1] packed into a pointer
// using fxy__ptr from utils.h. It can't represent locations outside the world
// map.
typedef void* geopt;

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

// An underlying model of tectonics represented by a sheet of interconnected
// points.
struct tectonic_sheet_s;
typedef struct tectonic_sheet_s tectonic_sheet;

// A layer of material that encompasses some region of the world.
struct stratum_s;
typedef struct stratum_s stratum;

// Climate & Hydrology
// -------------------

struct body_of_water_s;
typedef struct body_of_water_s body_of_water;

struct river_s;
typedef struct river_s river;

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
// 128*108 = 13824 regions / world
// 13824 * 256*256*(?=512) = 463856467968 chunks / world
// 463856467968 chunks * 256 KB/chunk = 108 petabytes / world
// 96*256*32 = 786432 blocks ~= 524300 meters ~= 525 km
// 108*256*32 = 884736 blocks ~= 589800 meters ~= 590 km <-
// 320*256*32 = 2621440 blocks ~= 1747500 meters ~= 1750 km
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
#define WM_REGION_CONTENTION_NOISE_STRENGTH 0.5
#define WM_REGION_CONTENTION_NOISE_SCALE (1.0 / 50.0)
#define WM_REGION_CONTENTION_POLAR_STRENGTH 0.7
#define WM_REGION_CONTENTION_POLAR_SCALE 1.6

// How far away from region boundaries "inner" points should be in fractions of
// a world region.
#define INNER_GEOPT_MARGIN 0.05

// Geology
// -------

// Maximum number of stone layers per world region
#define WM_MAX_STRATA_LAYERS 256

// Maximum number of material types present in other layers as veins
#define WM_N_VEIN_TYPES 2

// Maximum number of material types included in other layers
#define WM_N_INCLUSION_TYPES 8


// Climate & Hydrology
// -------------------

// Number of seasons in the year
#define WM_N_SEASONS 4

// Maximum alternate dirt/sand types:
#define WM_MAX_ALT_DIRTS 5
#define WM_MAX_ALT_SANDS 3

// Maximum rivers in a single region:
#define WM_MAX_RIVERS 4

// Biology
// -------

// Maximum number of biomes that can overlap in the same world region
#define WM_MAX_BIOME_OVERLAP 4

// Biome plant variant caps
#define WM_MAX_BIOME_MUSHROOMS 16
#define WM_MAX_BIOME_MOSSES 16
#define WM_MAX_BIOME_GRASSES 32
#define WM_MAX_BIOME_VINES 16
#define WM_MAX_BIOME_HERBS 128
#define WM_MAX_BIOME_BUSHES 32
#define WM_MAX_BIOME_SHRUBS 32
#define WM_MAX_BIOME_TREES 64

/***********
 * Globals *
 ***********/

// The globally-accessible world:
extern world_map* THE_WORLD;

/*************************
 * Structure Definitions *
 *************************/

// Pre
// ---

struct world_map_pos_s {
  wm_pos_t x, y;
};

// Biology
// -------

struct biome_s {
  // species IDs and frequencies for plant types:
  // mushrooms, mosses, grasses, vines, herbs, bushes, shrubs, and trees
  // (aquatic grasses, plants and corals are stored as grasses, vines, and
  // bushes respectively)
  species  mushroom_species[WM_MAX_BIOME_MUSHROOMS];
  float    mushroom_frequencies[WM_MAX_BIOME_MUSHROOMS];
  species  moss_species[WM_MAX_BIOME_MOSSES];
  float    moss_frequencies[WM_MAX_BIOME_MOSSES];
  species  grass_species[WM_MAX_BIOME_GRASSES];
  float    grass_frequencies[WM_MAX_BIOME_GRASSES];
  species  vine_species[WM_MAX_BIOME_VINES];
  float    vine_frequencies[WM_MAX_BIOME_VINES];
  species  herb_species[WM_MAX_BIOME_HERBS];
  float    herb_frequencies[WM_MAX_BIOME_HERBS];
  species  bush_species[WM_MAX_BIOME_BUSHES];
  float    bush_frequencies[WM_MAX_BIOME_BUSHES];
  species  shrub_species[WM_MAX_BIOME_SHRUBS];
  float    shrub_frequencies[WM_MAX_BIOME_SHRUBS];
  species  tree_species[WM_MAX_BIOME_TREES];
  float    tree_frequencies[WM_MAX_BIOME_TREES];
};

// Climate & Hydrology
// -------------------

struct body_of_water_s {
  world_map_pos origin;
  world_map_pos shorigin; // an arbitrary point on the shore
  float level;
  salinity salt;
  size_t area;
  size_t shore_area;
  list *rivers;
};

struct river_s {
  list *path;
  list *control_points;
  list *widths;
  float branch_prob;
};

struct hydrology_s {
  hydro_state state; // what kind of region this is
  body_of_water *body; // what body of water this region belongs to
  river *rivers[WM_MAX_RIVERS]; // Which rivers flow through this region
  gl_pos_t water_table; // how high the water table is, in blocks
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
  float rainfall[WM_N_SEASONS]; // rainfall per season in mm/year
  float temp_low[WM_N_SEASONS]; // temperature low, mean and high throughout the
  float temp_mean[WM_N_SEASONS]; // day, in each season
  float temp_high[WM_N_SEASONS];
};

struct soil_composition_s {
  species base_dirt; // dirt (/sand/mud) species for normal soil
  block alt_dirt_blocks[WM_MAX_ALT_DIRTS];
  species alt_dirt_species[WM_MAX_ALT_DIRTS];
  float alt_dirt_strengths[WM_MAX_ALT_DIRTS];
  float alt_dirt_hdeps[WM_MAX_ALT_DIRTS]; // height-dependence
  species base_sand; // sand species (for beaches, rivers, & oceans)
  block alt_sand_blocks[WM_MAX_ALT_SANDS];
  species alt_sand_species[WM_MAX_ALT_SANDS];
  float alt_sand_strengths[WM_MAX_ALT_SANDS];
  float alt_sand_hdeps[WM_MAX_ALT_SANDS]; // height-dependence
};

// Geology
// -------

struct tectonic_sheet_s {
  ptrdiff_t seed; // seed for various noise purposes

  size_t width, height; // the size of the sheet
  vector *points; // the points in the sheet
  vector *forces; // the forces on each point in the sheet
  float *avgcounts; // counts for averaging at each point
}

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
  float vein_scale[WM_N_VEIN_TYPES]; // scale of different veins (in blocks)
  float vein_strength[WM_N_VEIN_TYPES]; // thickness/frequency of veins (0-1)
  float inclusion_frequency[WM_N_INCLUSION_TYPES]; // inclusion frequency (0-1)

 // Derived species information:
 // ----------------------------------
  species base_species; // exact material type for main mass
  species vein_species[WM_N_VEIN_TYPES]; // types for veins
  species inclusion_species[WM_N_INCLUSION_TYPES]; // types for inclusions
};

// Info
// ----

struct strata_info_s { // indexed starting from the bottom
  size_t stratum_count;
  stratum* strata[WM_MAX_STRATA_LAYERS]; // the layers that touch this region
  float total_height; // cumulative height in blocks (as float for division)
  float bottoms[WM_MAX_STRATA_LAYERS]; // the bottom of each layer on [0, 1]
};

struct climate_info_s {
  hydrology water;
  weather atmosphere;
  soil_composition soil;
};

struct biome_info_s {
  biome* biomes[WM_MAX_BIOME_OVERLAP];
};

struct civ_info_s {
  // TODO: this
  size_t population;
};

// General
// -------

// Each world region stores info on geology, climate, ecology, and
// anthropoloogy, as well as an anchor position that's randomly placed
// somewhere within the region and an estimate of local terrain height.
struct world_region_s {
  ptrdiff_t seed;
  world_map *world;
  world_map_pos pos;
  global_pos anchor;
  // topology info:
  float terrain_height;
  float geologic_height;
  world_region *downhill;
  // various info modules:
  strata_info geology;
  climate_info climate;
  biome_info ecology;
  civ_info anthropology;
};

// The world map is a grid of regions, with lists of all strata, water, rivers,
// biomes, and civilizations.
struct world_map_s {
  ptrdiff_t seed;
  wm_pos_t width, height;
  world_region *regions;
  tectonic_sheet *tectonics;
  list *all_strata;
  list *all_water;
  list *all_rivers;
  list *all_biomes;
  list *all_civs;
};

/********************
 * Inline Functions *
 ********************/

static inline void wmpos__glpos(
  world_map_pos const * const wmpos,
  global_pos *glpos
) {
  glpos->x = ((gl_pos_t) wmpos->x) << (WORLD_REGION_BITS + CHUNK_BITS);
  glpos->y = ((gl_pos_t) wmpos->y) << (WORLD_REGION_BITS + CHUNK_BITS);
  glpos->z = 0;
}

static inline void glpos__wmpos(
  global_pos const * const glpos,
  world_map_pos *wmpos
) {
  wmpos->x = (wm_pos_t) (glpos->x >> (WORLD_REGION_BITS + CHUNK_BITS));
  wmpos->y = (wm_pos_t) (glpos->y >> (WORLD_REGION_BITS + CHUNK_BITS));
}

static inline void wmpos__glcpos(
  world_map_pos const * const wmpos,
  global_chunk_pos *glcpos
) {
  glcpos->x = ((gl_cpos_t) wmpos->x) << WORLD_REGION_BITS;
  glcpos->y = ((gl_cpos_t) wmpos->y) << WORLD_REGION_BITS;
  glcpos->z = 0;
}

static inline void glcpos__wmpos(
  global_chunk_pos const * const glcpos,
  world_map_pos *wmpos
) {
  wmpos->x = (wm_pos_t) (glcpos->x >> WORLD_REGION_BITS);
  wmpos->y = (wm_pos_t) (glcpos->y >> WORLD_REGION_BITS);
}

static inline void copy_wmpos(
  world_map_pos const * const from,
  world_map_pos *to
) {
  to->x = from->x;
  to->y = from->y;
}

static inline void wmpos__geopt(
  world_map *wm,
  world_map_pos const * const wmpos,
  geopt *gpt
) {
  *gpt = fxy__ptr(
    (wmpos->x) / ((float) wm->width),
    (wmpos->y) / ((float) wm->height)
  );
}

static inline void geopt__wmpos(
  world_map *wm,
  geopt const * const gpt,
  world_map_pos *wmpos
) {
  wmpos->x = fastfloor(ptr__fx(*gpt) * wm->width);
  wmpos->y = fastfloor(ptr__fy(*gpt) * wm->height);
}

// Ignores the z value.
static inline void glpos__geopt(
  world_map *wm,
  global_pos const * const glpos,
  geopt *gpt
) {
  *gpt = fxy__ptr(
    (glpos->x) / ((float) (wm->width * WORLD_REGION_BLOCKS)),
    (glpos->y) / ((float) (wm->height * WORLD_REGION_BLOCKS))
  );
}

// Sets the z value to 0.
static inline void geopt__glpos(
  world_map *wm,
  geopt const * const gpt,
  global_pos *glpos
) {
  glpos->x = fastfloor(ptr__fx(*gpt) * wm->width * WORLD_REGION_BLOCKS);
  glpos->y = fastfloor(ptr__fy(*gpt) * wm->height * WORLD_REGION_BLOCKS);
  glpos->z = 0;
}

// Takes a world region and a seed and returns a random geopt within the
// region, excluding some positions right at the edges. In the process it
// scrambles the given seed.
static inline geopt inner_pt(world_region *wr, ptrdiff_t *seed) {
  float lat, lon;
  float xmin, xmax, ymin, ymax;

  xmin = wr->pos.x / ((float) (wr->world->width));
  xmax = (wr->pos.x + 1) / ((float) (wr->world->width));
  ymin = wr->pos.y / ((float) (wr->world->height));
  ymax = (wr->pos.y + 1) / ((float) (wr->world->height));

  lon = (
    xmin + (xmax - xmin) * INNER_GEOPT_MARGIN
  +
    (xmax - xmin) * (1.0 - 2.0 * INNER_GEOPT_MARGIN) * ptrf(*seed)
  );
  *seed = prng(*seed);
  lat = (
    ymin + (ymax - ymin) * INNER_GEOPT_MARGIN
  +
    (ymax - ymin) * (1.0 - 2.0 * INNER_GEOPT_MARGIN) * ptrf(*seed)
  );
  *seed = prng(*seed);

  return fxy__ptr(lon, lat);
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

// Computes the region anchor for the given world map position.
static inline void compute_region_anchor(
  world_map *wm,
  world_map_pos const * const wmpos,
  global_pos *anchor
) {
  ptrdiff_t hash = hash_3d(wmpos->x, wmpos->y, wm->seed + 71);
  wmpos__glpos(wmpos, anchor);
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
  global_pos *glpos,
  world_region **best, world_region **secondbest,
  float *strbest, float *strsecond
);

// Takes a world map position and uses downhill information to find a terrain
// local minimum. The input position is edited directly.
void find_valley(world_map *wm, world_map_pos *pos);

int breadth_first_iter(
  world_map *wm,
  world_map_pos *origin,
  int min_size,
  int max_size,
  void *arg,
  step_result (*process)(search_step, world_region*, void*)
);

#endif // ifndef WORLD_MAP_H
