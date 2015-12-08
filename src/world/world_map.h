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

#include "util.h"

/*********
 * Enums *
 *********/

// Search Management
// -----------------

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


// Climate & Hydrology
// -------------------

enum hydro_state_e {
  WM_HS_LAND = 0x01,
  WM_HS_OCEAN = 0x02,
  WM_HS_LAKE = 0x04,
  WM_HS_OCEAN_SHORE = 0x08,
  WM_HS_LAKE_SHORE = 0x10,
  WM_HS_RIVER = 0x20,
};
typedef enum hydro_state_e hydro_state;

enum salinity_e {
  WM_SL_FRESH = 0x01,
  WM_SL_BRACKISH = 0x02,
  WM_SL_SALINE = 0x04,
  WM_SL_BRINY = 0x08,
};
typedef enum salinity_e salinity;


// Summarization Categories
// ------------------------

// Discretization of altitudes:
enum altitude_category_e {
  WM_AC_OCEAN_DEPTHS = 0x01,
  WM_AC_CONTINENTAL_SHELF = 0x02,
  WM_AC_COASTAL_PLAINS = 0x04,
  WM_AC_INLAND_HILLS = 0x08,
  WM_AC_HIGHLANDS = 0x10,
  WM_AC_MOUNTAIN_SLOPES = 0x20,
  WM_AC_MOUNTAIN_PEAKS = 0x40
};
typedef enum altitude_category_e altitude_category;

// Discretization of precipitation values:
enum precipitation_category_e {
  WM_PC_DESERT = 0x01,
  WM_PC_ARID = 0x02,
  WM_PC_DRY = 0x04,
  WM_PC_NORMAL = 0x08,
  WM_PC_SEASONAL = 0x10,
  WM_PC_WET = 0x20,
  WM_PC_SOAKING = 0x40,
  WM_PC_FLOODED = 0x80
};
typedef enum precipitation_category_e precipitation_category;

// Discretization of temperature information:
enum temperature_category_e {
  WM_TC_ARCTIC = 0x001,
  WM_TC_TUNDRA = 0x002,
  WM_TC_COLD_FROST = 0x004,
  WM_TC_COLD_RARE_FROST = 0x008,
  WM_TC_MILD_FROST = 0x00f,
  WM_TC_MILD_RARE_FROST = 0x010,
  WM_TC_WARM_FROST = 0x020,
  WM_TC_WARM_NO_FROST = 0x040,
  WM_TC_HOT = 0x080,
  WM_TC_TROPICAL = 0x100
};
typedef enum temperature_category_e temperature_category;


// Biomes
// ------

enum biome_category_e {
  BIOME_CAT_UNKNOWN = 0,

  // Deep ocean biomes:
  // These biomes describe life in the deep ocean, below the thermocline in the
  // aphotic zone.
  BIOME_CAT_DEEP_AQUATIC = 1,
  BIOME_CAT_OCEAN_VENTS = 2,

  // Pelagic ocean biomes:
  // These biomes describe species that live above the thermocline in the
  // photic zone of the open ocean, including some non-swimming animals like
  // migratory birds and ice-dwelling mammals.
  BIOME_CAT_SEA_ICE = 3,
  BIOME_CAT_TEMPERATE_PELAGIC = 4,
  BIOME_CAT_TROPICAL_PELAGIC = 5,

  // Offshore ocean biomes:
  // These describe species that live above the continental shelf, from
  // bottom-dwelling invertebrates to seabirds.
  BIOME_CAT_TEMPERATE_OFFSHORE = 6,
  BIOME_CAT_TROPICAL_OFFSHORE = 7,
  BIOME_CAT_TEMPERATE_AQUATIC_GRASSLAND = 8,
  BIOME_CAT_TROPICAL_AQUATIC_GRASSLAND = 9,
  BIOME_CAT_TEMPERATE_AQUATIC_FOREST = 10,
  BIOME_CAT_TROPICAL_AQUATIC_FOREST = 11,
  BIOME_CAT_COLD_REEF = 12,
  BIOME_CAT_WARM_REEF = 13,

  // Beach biomes:
  // These biomes are found at the edges of oceans (lake beaches don't get
  // their own biomes). They describe species endemic to these zones, and
  // usually overlap with at least one ocean biome and at least one land biome.
  BIOME_CAT_FROZEN_BEACH = 14,
  BIOME_CAT_COLD_BEACH = 15,
  BIOME_CAT_WARM_BEACH = 16,
  BIOME_CAT_TROPICAL_BEACH = 17,

  // Lake biomes:
  // These biomes describe the animals endemic to and dependent upon lakes.
  // Terrestrial animals may be included when depend on the lake.
  BIOME_CAT_FROZEN_LAKE = 18,
  BIOME_CAT_COLD_LAKE = 19,
  BIOME_CAT_TEMPERATE_LAKE = 20,
  BIOME_CAT_WARM_LAKE = 21,
  BIOME_CAT_TROPICAL_LAKE = 22,
  BIOME_CAT_SALT_LAKE = 23,

  // River biomes:
  // These biomes describe animals that live in or depend on rivers. Many
  // species endemic to riparian zones are included.
  BIOME_CAT_FROZEN_RIVER = 24,
  BIOME_CAT_COLD_RIVER = 25,
  BIOME_CAT_TEMPERATE_RIVER = 26,
  BIOME_CAT_WARM_RIVER = 27,
  BIOME_CAT_TROPICAL_RIVER = 28,

  // Alpine biomes:
  // These biomes are prevalent in areas of high elevation (above the
  // treeline). They are characterized by the absence of trees, as well as a
  // plethora of adaptations for high-altitude living. Their species are
  // usually present below the treeline as well, but another biome will be used
  // to describe most of a mountain's flora and fauna.
  BIOME_CAT_FROZEN_ALPINE = 29,
  BIOME_CAT_COLD_ALPINE = 30,
  BIOME_CAT_TEMPERATE_WET_ALPINE = 31,
  BIOME_CAT_TEMPERATE_DRY_ALPINE = 32,
  BIOME_CAT_WARM_WET_ALPINE = 33,
  BIOME_CAT_WARM_DRY_ALPINE = 34,
  BIOME_CAT_TROPICAL_WET_ALPINE = 35,
  BIOME_CAT_TROPICAL_DRY_ALPINE = 36,

  // Desert biomes:
  // These are characterized by extreme dryness, although they will include
  // some oasis species.
  BIOME_CAT_FROZEN_DESERT = 37,
  BIOME_CAT_COLD_DESERT = 38,
  BIOME_CAT_TEMPERATE_DESERT = 39,
  BIOME_CAT_WARM_DESERT = 40,
  BIOME_CAT_HOT_DESERT = 41,

  // Grassland biomes:
  // Biomes dominated by grassy herbs, usually due to some combination of poor
  // soil fertility, regular disruptions (grazing, fire, etc.) and/or low
  // annual rainfall.
  BIOME_CAT_COLD_GRASSLAND = 42,
  BIOME_CAT_TEMPERATE_GRASSLAND = 43,
  BIOME_CAT_WARM_GRASSLAND = 44,
  BIOME_CAT_TROPICAL_GRASSLAND = 45,

  // Shrubland biomes:
  // Biomes where shrubs, bushes, and herbs are common, with few trees.
  BIOME_CAT_COLD_SHRUBLAND = 46,
  BIOME_CAT_TEMPERATE_SHRUBLAND = 47,
  BIOME_CAT_WARM_SHRUBLAND = 48,
  BIOME_CAT_TROPICAL_SHRUBLAND = 49,

  // Savanna biomes:
  // Biomes where trees may be common, but do not form a canopy, allowing
  // grasses and shrubs to grow beneath and between them.
  BIOME_CAT_TEMPERATE_SAVANNA = 50,
  BIOME_CAT_WARM_SAVANNA = 51,
  BIOME_CAT_TROPICAL_SAVANNA = 52,

  // Coniferous forest biomes:
  // Biomes dominated by coniferous trees, often extremely homogeneous.
  // Broadleaf trees may also be present, but are distinctly outnumbered.
  BIOME_CAT_COLD_CONIFEROUS_FOREST = 53,
  BIOME_CAT_TEMPERATE_CONIFEROUS_FOREST = 54,
  BIOME_CAT_WARM_CONIFEROUS_FOREST = 55,
  BIOME_CAT_TROPICAL_CONIFEROUS_FOREST = 56,

  // Broadleaf forest biomes:
  // Biomes dominated by broadleaf trees which are usually quite diverse. Some
  // conifers may also be present, but they are usually rare.
  BIOME_CAT_TEMPERATE_BROADLEAF_FOREST = 57,
  BIOME_CAT_WARM_WET_BROADLEAF_FOREST = 58,
  BIOME_CAT_WARM_DRY_BROADLEAF_FOREST = 59,
  BIOME_CAT_TROPICAL_WET_BROADLEAF_FOREST = 60,
  BIOME_CAT_TROPICAL_DRY_BROADLEAF_FOREST = 61,

  // Wetland biomes:
  // Biomes with seasonal or sustained flooding, usually found near lakes,
  // rivers, or oceans. Wetlands adjacent to the ocean are brackish.
  BIOME_CAT_TUNDRA = 62,
  BIOME_CAT_COLD_FRESHWATER_WETLAND = 63,
  BIOME_CAT_COLD_SALTWATER_WETLAND = 64,
  BIOME_CAT_TEMPERATE_FRESHWATER_WETLAND = 65,
  BIOME_CAT_TEMPERATE_SALTWATER_WETLAND = 66,
  BIOME_CAT_WARM_FRESHWATER_WETLAND = 67,
  BIOME_CAT_WARM_FRESHWATER_FORESTED_WETLAND = 68,
  BIOME_CAT_WARM_SALTWATER_WETLAND = 69,
  BIOME_CAT_TROPICAL_FRESHWATER_WETLAND = 70,
  BIOME_CAT_TROPICAL_FRESHWATER_FORESTED_WETLAND = 71,
  BIOME_CAT_TROPICAL_SALTWATER_WETLAND = 72,
  BIOME_CAT_TROPICAL_SALTWATER_FORESTED_WETLAND = 73
};
typedef enum biome_category_e biome_category;

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

// Topographical information for a world region.
struct topography_info_s;
typedef struct topography_info_s topography_info;

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

struct soil_type_s;
typedef struct soil_type_s soil_type;

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

// World width and height in regions (used in worldgen.c):
// 768*512 = 393216 regions
//#define WORLD_WIDTH 768
//#define WORLD_HEIGHT 512
// 400*320 = 144000 regions
//#define WORLD_WIDTH 400
//#define WORLD_HEIGHT 360
// 240*200 = 48000 regions
//#define WORLD_WIDTH 240
//#define WORLD_HEIGHT 200
// 128*108 = 13824 regions
//#define WORLD_WIDTH 128
//#define WORLD_HEIGHT 108
// 96*96 = 9216 regions
//#define WORLD_WIDTH 96
//#define WORLD_HEIGHT 96
// 32*32 = 1024 regions
#define WORLD_WIDTH 32
#define WORLD_HEIGHT 32

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

// Maximum range at which a world region exerts influence:
#define MAX_REGION_INFULENCE_DISTANCE (1.05 * sqrtf( \
  WORLD_REGION_BLOCKS * WORLD_REGION_BLOCKS + \
  WORLD_REGION_BLOCKS * WORLD_REGION_BLOCKS \
))

// The sigmoid inflection point for the world region influence distribution:
#define WORLD_REGION_INFLUENCE_SHAPE 0.3

// Maximum 3D distance between two world region anchors:
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

// Approximate number of world regions per tectonic sheet triangle
#define TECTONIC_SHEET_SCALE 3.2

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

// Maximum alternate dirt/mud/clay/sand/gravel/scree/stone types:
#define WM_MAX_SOIL_ALTS 5

// Maximum rivers in a single region:
#define WM_MAX_RIVERS 4

// Biology
// -------

// Maximum number of biomes that can overlap in the same world region
#define WM_MAX_BIOME_OVERLAP 6

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

// A type that combines a 32-bit float with an any_species specifying both an
// exact species and its relative frequency at once.
typedef uint64_t frequent_species;

#define      FR_SP_FREQ_MASK umaxof(uint32_t)
#define     FR_SP_FREQ_SHIFT 0
#define   FR_SP_SPECIES_MASK umaxof(any_species)
#define  FR_SP_SPECIES_SHIFT 32

struct biome_s {
  biome_category category;
  // species types, IDs, and frequencies for flora (each list entry is a
  // frequent_species)
  list *hanging_terrestrial_flora;
  list *ephemeral_terrestrial_flora;
  list *ubiquitous_terrestrial_flora;
  list *close_spaced_terrestrial_flora;
  list *medium_spaced_terrestrial_flora;
  list *wide_spaced_terrestrial_flora;

  list *hanging_subterranean_flora;
  list *ephemeral_subterranean_flora;
  list *ubiquitous_subterranean_flora;
  list *close_spaced_subterranean_flora;
  list *medium_spaced_subterranean_flora;
  list *wide_spaced_subterranean_flora;

  list *ephemeral_aquatic_flora;
  list *ubiquitous_aquatic_flora;
  list *close_spaced_aquatic_flora;
  list *medium_spaced_aquatic_flora;
  list *wide_spaced_aquatic_flora;

  // fauna
  // TODO: HERE
};

// Climate & Hydrology
// -------------------

struct body_of_water_s {
  world_map_pos origin;
  world_map_pos shorigin; // an arbitrary point on the shore
  hydro_state type;
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
  list *depths;
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
  float temp_low[WM_N_SEASONS]; // temperature daily low, mean and high in each
  float temp_mean[WM_N_SEASONS]; // season
  float temp_high[WM_N_SEASONS];
};

struct soil_type_s {
  block main_block_type; // should be one of B_DIRT, B_MUD, B_SAND, or B_CLAY
  species main_species; // the species specifier

  block alt_block_types[WM_MAX_SOIL_ALTS]; // other soil types
  species alt_species[WM_MAX_SOIL_ALTS];
  float alt_strengths[WM_MAX_SOIL_ALTS];
  float alt_hdeps[WM_MAX_SOIL_ALTS]; // height-dependence (should be in [-1, 1])
    // if positive, heigh-within-soil will be multiplied with strength as:
    //   str *= 1.0 + 0.4 * (h*hdep)^(1 - hdep/3)
    // if negative, heigh-within-soil will be multiplied with strength as:
    //   str *= 1.0 + 0.4 * ((1-h)*-hdep)^(1 + hdep/3)
};

struct soil_composition_s {
  soil_type base_soil; // normal soil
  soil_type top_soil;
  soil_type river_banks;
  soil_type river_bank_topsoil;
  soil_type river_bottoms;
  soil_type river_bottom_topsoil;
  soil_type lake_shores;
  soil_type lake_shore_topsoil;
  soil_type lake_bottoms;
  soil_type lake_bottom_topsoil;
  soil_type beaches;
  soil_type beach_topsoil;
  soil_type ocean_floor;
  soil_type ocean_floor_topsoil;
};

// Geology
// -------

struct tectonic_sheet_s {
  ptrdiff_t seed; // seed for various noise purposes

  size_t width, height; // the size of the sheet
  vector *points; // the points in the sheet
  vector *forces; // the forces on each point in the sheet
  uint8_t *avgcounts; // counts for averaging at each point
};

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
  // TODO: Clay deposits?
  // TODO: Native metal veins / inclusions?
  species base_species; // exact stone species for main mass
  species vein_species[WM_N_VEIN_TYPES]; // types for veins
  species inclusion_species[WM_N_INCLUSION_TYPES]; // types for inclusions
};

// Info
// ----

struct topography_info_s {
  manifold_point terrain_height; // height of this region
  float geologic_height; // "geologic height" is >= terrain height and allows
                         // for stratified mountains/canyons/etc.
  float flow_potential; // the "flow potential" of a region is used for erosion
  world_region *downhill; // the region downhill from here
  world_region *uphill; // the region uphill from here
};

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
  size_t biome_count;
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

  // summary information:
  altitude_category s_altitude;
  precipitation_category s_precipitation;
  temperature_category s_temperature;

  // various info modules:
  topography_info topography;
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

  list *air_elements;
  list *water_elements;
  list *life_elements;
  list *stone_elements;
  list *metal_elements;
  list *rare_elements;

  list *all_elements;
  list *all_nutrients;
  list *all_strata;
  list *all_water;
  list *all_rivers;
  list *all_biomes;
  list *all_civs;
};

/********************
 * Inline Functions *
 ********************/

static inline any_species frequent_species_any_species(frequent_species fqsp) {
  return (any_species) ((fqsp >> FR_SP_SPECIES_SHIFT) & FR_SP_SPECIES_MASK);
}

static inline species_type frequent_species_species_type(frequent_species fqsp){
  return any_species_type(frequent_species_any_species(fqsp));
}

static inline species frequent_species_species(frequent_species fqsp) {
  return any_species_species(frequent_species_any_species(fqsp));
}

static inline float frequent_species_frequency(frequent_species fqsp) {
#ifdef DEBUG
  assert(sizeof(float) == sizeof(uint32_t));
#endif
  uint32_t int_freq = (uint32_t) (fqsp >> FR_SP_FREQ_SHIFT) & FR_SP_FREQ_MASK;
  return *((float*) &int_freq); // reinterpret as a float
}

static inline void frequent_species_set_any_species(
  frequent_species *fqsp,
  any_species asp
) {
  *fqsp &= ~(FR_SP_SPECIES_MASK << FR_SP_SPECIES_SHIFT);
  *fqsp |= (asp & FR_SP_SPECIES_MASK) << FR_SP_SPECIES_SHIFT;
}

static inline void frequent_species_set_species_type(
  frequent_species *fqsp,
  species_type t
) {
  any_species asp = frequent_species_any_species(*fqsp);
  any_species_set_species_type(&asp, t);
  frequent_species_set_any_species(fqsp, asp);
}

static inline void frequent_species_set_species(
  frequent_species *fqsp,
  species sp
) {
  any_species asp = frequent_species_any_species(*fqsp);
  any_species_set_species(&asp, sp);
  frequent_species_set_any_species(fqsp, asp);
}

static inline frequent_species_set_frequency(frequent_species *fqsp, float f) {
#ifdef DEBUG
  assert(sizeof(float) == sizeof(uint32_t));
#endif
  *fqsp &= ~(FR_SP_FREQ_MASK << FR_SP_FREQ_SHIFT);
  *fqsp |= ((*((uint32_t*) &f)) & FR_SP_FREQ_MASK) << FR_SP_FREQ_SHIFT;
}

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
  world_map const * const wm,
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

// Computes the (2D) distance in blocks between two geopoints on the given map:
static inline float geodist(world_map *wm, geopt from, geopt to) {
  global_pos gl_from, gl_to;
  geopt__glpos(wm, &from, &gl_from);
  geopt__glpos(wm, &to, &gl_to);
  return sqrtf(
    (gl_from.x - gl_to.x) * (gl_from.x - gl_to.x)
  + (gl_from.y - gl_to.y) * (gl_from.y - gl_to.y)
  );
}

// Returns a geopt that's halfway between the two given geopts:
static inline geopt geomid(geopt from, geopt to) {
  return fxy__ptr(
    (ptr__fx(from) + ptr__fx(to))/2.0,
    (ptr__fy(from) + ptr__fy(to))/2.0
  );
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

// Zeroes out soil data.
static inline void erase_soil_type(soil_type *target) {
  size_t i;
  target->main_block_type = B_VOID;
  target->main_species = 0;
  for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
    target->alt_block_types[i] = B_VOID;
    target->alt_species[i] = 0;
    target->alt_strengths[i] = 0.0;
    target->alt_hdeps[i] = 0.0;
  }
}

// Copies the data for a soil type.
static inline void copy_soil_type(soil_type *from, soil_type *to) {
  size_t i;
  to->main_block_type = from->main_block_type;
  to->main_species = from->main_species;

  for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
    to->alt_block_types[i] = from->alt_block_types[i];
    to->alt_species[i] = from->alt_species[i];
    to->alt_strengths[i] = from->alt_strengths[i];
    to->alt_hdeps[i] = from->alt_hdeps[i];
  }
}

static inline void erase_soil(soil_composition *comp) {
  erase_soil_type(&(comp->base_soil));
  erase_soil_type(&(comp->top_soil));
  erase_soil_type(&(comp->river_banks));
  erase_soil_type(&(comp->river_bank_topsoil));
  erase_soil_type(&(comp->river_bottoms));
  erase_soil_type(&(comp->river_bottom_topsoil));
  erase_soil_type(&(comp->lake_shores));
  erase_soil_type(&(comp->lake_shore_topsoil));
  erase_soil_type(&(comp->lake_bottoms));
  erase_soil_type(&(comp->lake_bottom_topsoil));
  erase_soil_type(&(comp->beaches));
  erase_soil_type(&(comp->beach_topsoil));
  erase_soil_type(&(comp->ocean_floor));
  erase_soil_type(&(comp->ocean_floor_topsoil));
}

// Copies all soil data from one region to another.
static inline void copy_soil(world_region *from, world_region *to) {
  soil_composition *from_s, *to_s;
  from_s = &(from->climate.soil);
  to_s = &(to->climate.soil);

  copy_soil_type(&(from_s->base_soil), &(to_s->base_soil));
  copy_soil_type(&(from_s->top_soil), &(to_s->top_soil));
  copy_soil_type(&(from_s->river_banks), &(to_s->river_banks));
  copy_soil_type(&(from_s->river_bank_topsoil), &(to_s->river_bank_topsoil));
  copy_soil_type(&(from_s->river_bottoms), &(to_s->river_bottoms));
  copy_soil_type(&(from_s->river_bottom_topsoil),&(to_s->river_bottom_topsoil));
  copy_soil_type(&(from_s->lake_shores), &(to_s->lake_shores));
  copy_soil_type(&(from_s->lake_shore_topsoil), &(to_s->lake_shore_topsoil));
  copy_soil_type(&(from_s->lake_bottoms), &(to_s->lake_bottoms));
  copy_soil_type(&(from_s->lake_bottom_topsoil), &(to_s->lake_bottom_topsoil));
  copy_soil_type(&(from_s->beaches), &(to_s->beaches));
  copy_soil_type(&(from_s->beach_topsoil), &(to_s->beach_topsoil));
  copy_soil_type(&(from_s->ocean_floor), &(to_s->ocean_floor));
  copy_soil_type(&(from_s->ocean_floor_topsoil), &(to_s->ocean_floor_topsoil));
}

// Convenience function for returning the uppermost rock species in a region:
static inline species get_bedrock(world_region *wr) {
  return wr->geology.strata[wr->geology.stratum_count-1].base_species;
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new world map of the given size. More work is needed
// to fill in proper values: see init_world_map in gen/worldgen.c.
world_map* create_world_map(ptrdiff_t seed, wm_pos_t width, wm_pos_t height);

// Cleans up the given world map and frees the associated memory.
void cleanup_world_map(world_map *wm);

// Allocates a new blank biome with the given category.
biome* create_biome(biome_category category);

// Allocates a new biome and merges information from all of the biomes in the
// given two world regions into it.
biome* create_merged_biome(
  world_region *wr1,
  world_region *wr2,
  float str1,
  float str2
);

// Frees the given biome.
void cleanup_biome(biome* b);

/*************
 * Functions *
 *************/

// Classification functions for discretizing altitude, precipitation, and
// temperature:
altitude_category classify_altitude(float altitude);

precipitation_category classify_precipitation(float *precipitation);

temperature_category classify_temperature(float *lows, float *means);

// Fills out discretized information for the given world region. Altitude,
// precipitation, and temperature information should already be present.
void summarize_region(world_region *wr);

// Just applies summarize_region to each world map region.
void summarize_all_regions(world_map *wm);

// Stores the 9 (_small) or 25 world region pointers surrounding the given
// world map position into the given neighborhood array. Some or all of the
// neighbors may be NULL if positions run off the edge of the map. If the
// neighborhood arrays aren't big enough, memory corruption will result.
void get_world_neighborhood_small(
  world_map *wm,
  world_map_pos *wmpos,
  world_region* neighborhood[]
);

void get_world_neighborhood(
  world_map *wm,
  world_map_pos *wmpos,
  world_region* neighborhood[]
);

// Computes interpolation values for the given large world neighborhood at the
// given global position, just using x/y values. The result array must have
// room for 25 entries (which must be the size of the neighborhood array).
// Interpolation can be performed by taking a weighted average of values from
// each region using the results as weights. Unlike compute_region_contenders,
// this does not introduce any extra noise, so interpolation should be smooth.
void compute_region_interpolation_values(
  world_map *wm,
  world_region* neighborhood[],
  global_pos *glpos,
  manifold_point result[]
);

// Computes the two closest world region anchors to the given point out of the
// 9 anchors in the given small neighborhood, along with the strengths of each.
// Simplex noise is used to mix up strengths giving the effect of a bubbly
// mixture where regions have inclusions of their neighbors. Different seeds
// result in different bubbly mixtures.
void compute_region_contenders(
  world_map *wm,
  world_region* neighborhood[],
  global_pos *glpos,
  ptrdiff_t seed,
  world_region **r_best, world_region **r_secondbest,
  float *r_strbest, float *r_strsecond
);

// A match function for iterating over geopoints and stopping when one is
// inside the given world region. Use with e.g., l_scan_elements.
int find_geopt_in_wr(void *v_gpt, void *v_wr_ptr);

// Takes a world map position and uses downhill information to find a terrain
// local minimum. The input position is edited directly.
void find_valley(world_map *wm, world_map_pos *pos);

// Arbitrary breadth-first iteration within a world map. Returns 1 on success
// and 0 on failure (as determined by either an SRESULT_ABORT returned by the
// step function or failing to meet the min/max size criteria). The given
// function is  called with SSTEP_INIT once, and then repeatedly with
// SSTEP_PROCESS until there are no more spaces to expand, the max_size is
// exceeded, or it returns SRESULT_ABORT or SRESULT_FINISHED. If either the min
// or max size limits aren't met or SRESULT_ABORT was returned, the iteration
// is successful and 1 is returned after calling the process function with
// SSTEP_FINISH and then SSTEP_CLEANUP. Otherwise, SSTEP_FINISH is not called
// (just SSTEP_CLEANUP) and 0 is returned. The given arg is passed to the
// process function as the third argument. SSTEP_VALIDATE, SSTEP_GET_MIN_SIZE,
// and SSTEP_GET_MAX_SIZE aren't used by this function.
int breadth_first_iter(
  world_map *wm,
  world_map_pos *origin,
  int min_size,
  int max_size,
  void *arg,
  step_result (*process)(search_step, world_region*, void*)
);

// Fills valid regions of the map with multiple blob-shaped regions by first
// running the validate function to figure out which regions count as valid
// (1 -> valid, 0-> invalid), and then calling the fill function on a random
// valid region. This entire process is repeated until there are no valid
// regions left to fill or until the fill function returns 0 (so whatever the
// fill function does should ensure that subsequent calls to the validate
// function return 0 for filled regions), or this function won't terminate. The
// seed is used for the random region selection step, and a fixed derivative
// seed is passed into each call to the validate and fill functions.
int fill_with_regions(
  world_map *wm,
  void *arg,
  int (*validate)(world_region*, void*, ptrdiff_t seed),
  int (*fill)(world_region*, void*, ptrdiff_t seed),
  ptrdiff_t seed
);

// Adds a biome to the given world region, or does nothing if that region
// already has the maximum number of biomes allowed.
void add_biome(world_region *wr, biome_category category);

// Takes a world region and merges information from every biome in that region
// into the target biome, scaling species frequencies by the given strength.
void merge_all_biomes_into(world_region *wr, float strength, biome *target);

// Merges a list of frequent_species into another such list, scaling each
// frequency as it goes.
void merge_and_scale_frequent_species(
  list const * const from,
  float str,
  list *to
);

// Picks an element present in the given world using a uniform distribution
// over all elements subject to the given constraints: a category constraint,
// and a list of elements to ignore (which may be given as NULL if there aren't
// any elements to ignore). If it's impossible to pick such an element, it will
// return 0 (the generic/invalid species).
element_species* pick_element(
  world_map *wm,
  element_categorization constraints,
  list *exclude,
  ptrdiff_t seed
);

#endif // ifndef WORLD_MAP_H
