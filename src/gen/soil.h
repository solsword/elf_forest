#ifndef SOIL_H
#define SOIL_H

// soil.h
// Soil generation.

#include "world/world_map.h"
#include "world/species.h"

/**************
 * Structures *
 **************/

struct soil_factors_s;
typedef struct soil_factors_s soil_factors;

/*************
 * Constants *
 *************/

// Layout of block_data for soils:
// All soils:
#define              SL_BS_HYDRATION 0

// Fertile soils:
#define               SL_BS_PH_SHIFT 2
#define              SL_BS_FERTILITY 4
#define               SL_BS_POISONED 6

// TODO: Any way to buy more space for this stuff?

// Post-shift masks:
#define              SL_BM_HYDRATION 0x3
#define               SL_BM_PH_SHIFT 0x3
#define              SL_BM_FERTILITY 0x3
#define               SL_BM_POISONED 0x3

// Values:
#define         SL_HYDRATION_DROUGHT 0
#define             SL_HYDRATION_DRY 1
#define             SL_HYDRATION_WET 2
#define         SL_HYDRATION_FLOODED 3

#define      SL_PH_SHIFT_VERY_ACIDIC 0
#define           SL_PH_SHIFT_ACIDIC 1
#define          SL_PH_SHIFT_NEUTRAL 2
#define            SL_PH_SHIFT_BASIC 3

#define        SL_FERTILITY_DEPLETED 0
#define          SL_FERTILITY_NORMAL 1
#define         SL_FERTILITY_FERTILE 2
#define      SL_FERTILITY_OVERLOADED 3

#define            SL_POISONED_CLEAN 0
#define     SL_POISONED_CONTAMINATED 1
#define            SL_POISONED_TOXIC 2
#define           SL_POISONED_DEADLY 3


// Soil generation parameters:
#define SL_REGION_MAX_SIZE 320

// "Change Points" define difference thresholds above which soil types are
// forced to change:
#define SL_CP_ALTITUDE 450 // blocks
#define SL_CP_MEAN_TEMP 8 // degrees Celsius
#define SL_CP_PRECIPITATION 250 // mm/year

#define SL_COMP_MAX_SAND 0.75
#define SL_COMP_MAX_CLAY 0.5

#define SL_RICH_ORGANICS 25.0
#define SL_POOR_ORGANICS 2.3

#define SL_COLOR_REDDISH_BROWN 0.433
#define SL_COLOR_DIRT_BROWN 0.483
#define SL_COLOR_GREENISH_BROWN 0.511

/*************************
 * Structure Definitions *
 *************************/

struct soil_factors_s {
  world_region *region;
  altitude_category altitude;
  precipitation_category precipitation;
  temperature_category temperature;
  salinity salt;
  stone_species *bedrock;
};

/********************
 * Inline Functions *
 ********************/

static inline block_data b_sl_hydration(block b) {
  return (block_data) ((b_data(b) >> SL_BS_HYDRATION) & SL_BM_HYDRATION);
}

static inline block_data b_sl_ph_shift(block b) {
  return (block_data) ((b_data(b) >> SL_BS_PH_SHIFT) & SL_BM_PH_SHIFT);
}

static inline block_data b_sl_fertility(block b) {
  return (block_data) ((b_data(b) >> SL_BS_FERTILITY) & SL_BM_FERTILITY);
}

static inline block_data b_sl_poisoned(block b) {
  return (block_data) ((b_data(b) >> SL_BS_POISONED) & SL_BM_POISONED);
}

static inline void b_sl_set_hydration(block *b, block_data hydration) {
  block_data d = b_data(*b);
  d &= ~(SL_BM_HYDRATION << SL_BS_HYDRATION);
  d |= hydration << SL_BS_HYDRATION;
  b_set_data(b, d);
}

static inline void b_sl_set_ph_shift(block *b, block_data ph_shift) {
  block_data d = b_data(*b);
  d &= ~(SL_BM_PH_SHIFT << SL_BS_PH_SHIFT);
  d |= ph_shift << SL_BS_PH_SHIFT;
  b_set_data(b, d);
}

static inline void b_sl_set_fertility(block *b, block_data fertility) {
  block_data d = b_data(*b);
  d &= ~(SL_BM_FERTILITY << SL_BS_FERTILITY);
  d |= fertility << SL_BS_FERTILITY;
  b_set_data(b, d);
}

static inline void b_sl_set_poisoned(block *b, block_data poisoned) {
  block_data d = b_data(*b);
  d &= ~(SL_BM_POISONED << SL_BS_POISONED);
  d |= poisoned << SL_BS_POISONED;
  b_set_data(b, d);
}

/*************
 * Functions *
 *************/

// Generates soil information for the given world map.
void generate_soil(world_map *wm);

// Creates new soil species that are appropriate for the given world region,
// filling in the region's soil_composition.
void create_appropriate_soil(world_region *wr);

// Creates a local dirt to fill in the given soil_type.
void create_local_dirt(
  world_region *wr,
  soil_type *soil
);

// Creates a local silt to fill in the given soil_type.
void create_local_silt(
  world_region *wr,
  soil_type *soil
);

// Creates a local mud to fill in the given soil_type.
void create_local_mud(
  world_region *wr,
  soil_type *soil
);

// Creates a local sand to fill in the given soil_type.
void create_local_sand(
  world_region *wr,
  soil_type *soil
);

// Takes an original soil_type and fills in the result soil_type with a variant
// of that soil suitable for topsoil.
void create_topsoil_variant(
  world_region *wr,
  soil_type *original,
  soil_type *result
);

// Picks which alternate dirt table to use based on local conditions:
rngtable* pick_alt_dirt_table(world_region *wr, ptrdiff_t seed);

// As above but for sand:
rngtable* pick_alt_sand_table(world_region *wr, ptrdiff_t seed);

// Fills in details of a single dirt species.
void fill_dirt_species(
  dirt_species *dsp,
  world_region *wr,
  ptrdiff_t seed
);

// Fills in a dirt species as a variant of the given model species.
void fill_dirt_variant(
  dirt_species *model,
  dirt_species *dsp,
  world_region *wr
);

// As above but for clay.
void fill_clay_species(
  clay_species *dsp,
  world_region *wr,
  ptrdiff_t seed
);
void fill_clay_variant(
  clay_species *model,
  clay_species *dsp,
  world_region *wr
);

void determine_new_dirt_appearance(dirt_species *species, ptrdiff_t seed);

void determine_new_dirt_material(dirt_species *species, ptrdiff_t seed);

void compute_combined_dirt_color(
  dirt_species *species,
  precise_color *color,
  ptrdiff_t seed
);

#endif // ifndef SOIL_H
