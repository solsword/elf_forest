#ifndef SOIL_H
#define SOIL_H

// soil.h
// Soil generation.

#include "world/world_map.h"
#include "world/species.h"

/*********
 * Enums *
 *********/

enum soil_altitude_category_e {
  SL_AC_OCEAN_DEPTHS,
  SL_AC_CONTINENTAL_SHELF,
  SL_AC_COASTAL_PLAINS,
  SL_AC_INLAND_HILLS,
  SL_AC_HIGHLANDS,
  SL_AC_MOUNTAIN_SLOPES,
  SL_AC_MOUNTAIN_PEAKS
};
typedef enum soil_altitude_category_e soil_altitude_category;

enum soil_precipitation_category_e {
  SL_PC_DESERT,
  SL_PC_ARID,
  SL_PC_DRY,
  SL_PC_NORMAL,
  SL_PC_SEASONAL,
  SL_PC_WET,
  SL_PC_SOAKING,
  SL_PC_FLOODED
};
typedef enum soil_precipitation_category_e soil_precipitation_category;

enum soil_temperature_regime_e {
  SL_TR_ARCTIC,
  SL_TR_TUNDRA,
  SL_TR_COLD_FROST,
  SL_TR_COLD_RARE_FROST,
  SL_TR_MILD_FROST,
  SL_TR_MILD_RARE_FROST,
  SL_TR_WARM_FROST,
  SL_TR_WARM_NO_FROST,
  SL_TR_HOT,
  SL_TR_BAKING
};
typedef enum soil_temperature_regime_e soil_temperature_regime;

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
  soil_altitude_category altitude;
  soil_precipitation_category precipitation;
  soil_temperature_regime temperature;
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
  soil_type *soil,
  soil_factors *factors
);

// Creates a local silt to fill in the given soil_type.
void create_local_silt(
  world_region *wr,
  soil_type *soil,
  soil_factors *factors
);

// Creates a local mud to fill in the given soil_type.
void create_local_mud(
  world_region *wr,
  soil_type *soil,
  soil_factors *factors
);

// Creates a local sand to fill in the given soil_type.
void create_local_sand(
  world_region *wr,
  soil_type *soil,
  soil_factors *factors
);

// Takes an original soil_type and fills in the result soil_type with a variant
// of that soil suitable for topsoil.
void create_topsoil_variant(
  world_region *wr,
  soil_type *original,
  soil_type *result,
  soil_factors *factors
);

// Picks which alternate dirt table to use based on local conditions:
rngtable* pick_alt_dirt_table(soil_factors *factors, ptrdiff_t seed);

// As above but for sand:
rngtable* pick_alt_sand_table(soil_factors *factors, ptrdiff_t seed);

// Skims information from the given world region into the given struct.
void glean_soil_factors(world_region *wr, soil_factors *result);

// Discretizes the given altitude (given in blocks).
soil_altitude_category classify_altitude(float altitude);

// Discretizes precipitation (takes a per-season array).
soil_precipitation_category classify_precipitation(float *precipitation);

// Discretizes temperature (takes two per-season arrays).
soil_temperature_regime classify_temperature(float *lows, float *means);

// Fills in details of a single dirt species.
void fill_dirt_species(
  dirt_species *dsp,
  soil_factors *factors,
  ptrdiff_t seed
);

// Fills in a dirt species as a variant of the given model species.
void fill_dirt_variant(
  dirt_species *model,
  dirt_species *dsp,
  soil_factors *factors
);

// As above but for clay.
void fill_clay_species(
  clay_species *dsp,
  soil_factors *factors,
  ptrdiff_t seed
);
void fill_clay_variant(
  clay_species *model,
  clay_species *dsp,
  soil_factors *factors
);

void determine_new_dirt_appearance(dirt_species *species, ptrdiff_t seed);

void determine_new_dirt_material(dirt_species *species, ptrdiff_t seed);

void compute_combined_dirt_color(
  dirt_species *species,
  precise_color *color,
  ptrdiff_t seed
);

#endif // ifndef SOIL_H
