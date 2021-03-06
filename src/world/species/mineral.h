#ifndef MINERAL_SPECIES_H
#define MINERAL_SPECIES_H
// Mineral-based species types.

#include "snek/feed.h"

/*********
 * Enums *
 *********/

// Geologic sources influence how stone species are generated.
enum geologic_source_e {
  GEO_IGNEOUS,
  GEO_METAMORPHIC,
  GEO_SEDIMENTARY
};
typedef enum geologic_source_e geologic_source;

FEED_SNEK(i, GEO_IGNEOUS)
FEED_SNEK(i, GEO_METAMORPHIC)
FEED_SNEK(i, GEO_SEDIMENTARY)

// Categories for the primary composition of minerals:
enum mineral_composition_e {
  MNRL_COMP_STONE,
  MNRL_COMP_LIFE,
  MNRL_COMP_STONE_AIR,
  MNRL_COMP_STONE_WATER,
  MNRL_COMP_STONE_LIFE,
  MNRL_COMP_STONE_STONE,
  MNRL_COMP_STONE_STONE_LIFE,
  MNRL_COMP_STONE_STONE_STONE,
  MNRL_COMP_STONE_METAL,
  MNRL_COMP_STONE_STONE_METAL,
  MNRL_COMP_STONE_METAL_METAL,
  MNRL_COMP_STONE_METAL_RARE,
  MNRL_COMP_STONE_RARE,
  MNRL_COMP_STONE_STONE_RARE,
  MNRL_COMP_STONE_RARE_RARE,
  MNRL_COMP_RARE_RARE
};
typedef enum mineral_composition_e mineral_composition;

FEED_SNEK(i, MNRL_COMP_STONE)
FEED_SNEK(i, MNRL_COMP_LIFE)
FEED_SNEK(i, MNRL_COMP_STONE_AIR)
FEED_SNEK(i, MNRL_COMP_STONE_WATER)
FEED_SNEK(i, MNRL_COMP_STONE_LIFE)
FEED_SNEK(i, MNRL_COMP_STONE_STONE)
FEED_SNEK(i, MNRL_COMP_STONE_STONE_LIFE)
FEED_SNEK(i, MNRL_COMP_STONE_STONE_STONE)
FEED_SNEK(i, MNRL_COMP_STONE_METAL)
FEED_SNEK(i, MNRL_COMP_STONE_STONE_METAL)
FEED_SNEK(i, MNRL_COMP_STONE_METAL_METAL)
FEED_SNEK(i, MNRL_COMP_STONE_METAL_RARE)
FEED_SNEK(i, MNRL_COMP_STONE_RARE)
FEED_SNEK(i, MNRL_COMP_STONE_STONE_RARE)
FEED_SNEK(i, MNRL_COMP_STONE_RARE_RARE)
FEED_SNEK(i, MNRL_COMP_RARE_RARE)

// Categories for mineral trace content:
enum mineral_trace_composition_e {
  MNRL_TRACE_NONE,
  MNRL_TRACE_AIR,
  MNRL_TRACE_WATER,
  MNRL_TRACE_LIFE,
  MNRL_TRACE_STONE,
  MNRL_TRACE_STONE_METAL,
  MNRL_TRACE_METAL,
  MNRL_TRACE_METAL_METAL,
  MNRL_TRACE_METAL_RARE,
  MNRL_TRACE_RARE,
  MNRL_TRACE_RARE_RARE
};
typedef enum mineral_trace_composition_e mineral_trace_composition;

FEED_SNEK(i, MNRL_TRACE_NONE)
FEED_SNEK(i, MNRL_TRACE_AIR)
FEED_SNEK(i, MNRL_TRACE_WATER)
FEED_SNEK(i, MNRL_TRACE_LIFE)
FEED_SNEK(i, MNRL_TRACE_STONE)
FEED_SNEK(i, MNRL_TRACE_STONE_METAL)
FEED_SNEK(i, MNRL_TRACE_METAL)
FEED_SNEK(i, MNRL_TRACE_METAL_METAL)
FEED_SNEK(i, MNRL_TRACE_METAL_RARE)
FEED_SNEK(i, MNRL_TRACE_RARE)
FEED_SNEK(i, MNRL_TRACE_RARE_RARE)

/**************
 * Structures *
 **************/

// Primary species structures:
struct dirt_species_s;
typedef struct dirt_species_s dirt_species;
struct clay_species_s;
typedef struct clay_species_s clay_species;
struct stone_species_s;
typedef struct stone_species_s stone_species;
struct metal_species_s;
typedef struct metal_species_s metal_species;

/*************
 * Constants *
 *************/

// Maximum number of primary constituent elements of stone etc.
#define MN_MAX_PRIMARY_CONSTITUENTS 3
// Maximum trace elements
#define MN_MAX_TRACE_CONSTITUENTS 2

// Maximum number of elements present in an alloy
#define MN_MAX_ALLOY_CONSTITUENTS 8

// Nutrient levels:
#define       MN_NT_CRIT_LEVEL_STARVED    5
#define      MN_NT_CRIT_LEVEL_DEPRIVED    9
#define       MN_NT_CRIT_LEVEL_LACKING   16
#define       MN_NT_CRIT_LEVEL_REDUCED   22
#define    MN_NT_CRIT_LEVEL_SUFFICIENT   45
#define     MN_NT_CRIT_LEVEL_INCREASED   96
#define      MN_NT_CRIT_LEVEL_ABUNDANT  160
#define  MN_NT_CRIT_LEVEL_OVERABUNDANT  192
#define       MN_NT_CRIT_LEVEL_CHOKING  220

#define       MN_NT_BFCL_LEVEL_LACKING   12
#define       MN_NT_BFCL_LEVEL_MINIMAL   24
#define       MN_NT_BFCL_LEVEL_PRESENT   60
#define      MN_NT_BFCL_LEVEL_ABUNDANT  128
#define  MN_NT_BFCL_LEVEL_OVERABUNDANT  180
#define       MN_NT_BFCL_LEVEL_CHOKING  220

#define        MN_NT_DTML_LEVEL_ABSENT   12
#define       MN_NT_DTML_LEVEL_PRESENT   32
#define      MN_NT_DTML_LEVEL_ABUNDANT   96
#define        MN_NT_DTML_LEVEL_DEADLY  192

#define          MN_NT_PSNS_LEVEL_SAFE    9
#define       MN_NT_PSNS_LEVEL_HARMFUL   16
#define     MN_NT_PSNS_LEVEL_DANGEROUS   32
#define        MN_NT_PSNS_LEVEL_DEADLY   96

#define MN_NT_TINY_ADJUST 3
#define MN_NT_SMALL_ADJUST 8
#define MN_NT_LARGE_ADJUST 16

/*************************
 * Structure Definitions *
 *************************/

struct dirt_species_s {
  species id;

  material material;
  mineral_filter_args appearance;

  // These three primary properties should add up to 1.0. Along with
  // organic_content and acidity, they determine most of the other properties
  // of the soil. Note that the sand_percent should not exceed 0.75 and the
  // clay_percent should not exceed 0.5. Under these conditions, the "dirt"
  // would be better classified as either sand or clay, and the use of B_SAND
  // or B_CLAY along with a stone_species or clay_species is more appropriate.
  // B_DIRT and dirt_species thus represent both pure silts and all manner of
  // loams that fill the rest of a sand/clay/silt triangle composition graph.
  float sand_percent;
  float clay_percent;
  float silt_percent;

  // Organic content ranges from 0 to 100, indicating the percentage of
  // non-mineral soil content. The rest of the soil is composed of sand, clay,
  // and silt as indicated by their respective percentage parameters.
  //
  // Data for Europe:
  //
  // http://eusoils.jrc.ec.europa.eu/projects/Soil_Atlas/HTmL/images/
  //   80-128%2022_img_1.jpg
  //
  // A rough guide is as follows:
  //
  //    0 - 1      inorganic (found in deserts)
  //    1 - 2      low (typically less-fertile)
  //    3 - 6      medium-low (includes prime agricultural land)
  //    7 - 12     medium-high (includes prime agricultural land)
  //   13 - 20     high (conditions must be favorable for deposition)
  //   21 - 40     very high (decomposition rates are probably low)
  //     41+       extreme (high deposition and/or low decomposition)
  uint8_t organic_content;

  // pH ranges from 0 to 14, but for most soils falls within 4.6 to 8.3 (the
  // range in which most plants can grow). Acidic soils are often the product
  // of ecological activity or simply accumulated organic matter, whereas basic
  // soils are usually found in regions were clay is present. Note that the
  // base_ph value represents a soil species' intrinsic pH value, but this can
  // be adjusted according to the block data information (see gen/soil.h).
  // Some ranges:
  //
  //     <4.6       extremely acid (most plants cannot grow)
  //   4.6 - 5.5    highly acidic (only specially-adapted or tolerant plants)
  //   5.5 - 6.0    acidic (favors acid-loving plants)
  //   6.0 - 6.8    slightly acidic (most plants can tolerate this range)
  //   6.8 - 7.2    "near-neutral;" few specific effects
  //   7.2 - 7.5    slightly basic (most plants can tolerate this range)
  //   7.5 - 8.3    basic (favors basic-loving plants)
  //     >8.3       extremely basic; most plants cannot grow
  //
  //  In general, most plants do well in the 6-7 range, while some acid-loving
  //  plants like blueberries, azaleas, and conifers tolerate and even thrive
  //  at pH 5-6. Some plants directly affect soil acidity (TODO: model this?).
  //  Above 7.5 pH, only a few plants can thrive.
  float base_ph;

  // These are the primary nutrient values for the soil. Levels are expressed
  // on a scale of 1-255, where 32 is a "standard level." Effects are roughly
  // as follows:
  //   Category:    Concentration:  Effect:
  //     critical        0-5          fast starvation
  //     critical        6-9          slow starvation
  //     critical       10-16         sickness
  //     critical       17-22         slowed growth
  //     critical       23-45         normal growth
  //     critical       46-96         accelerated growth factor
  //     critical       97-160        strong accelerated growth factor
  //     critical      161-192        *slowed growth
  //     critical      193-220        *sickness
  //     critical        221+         *slow death
  //
  //     * same as previous positive category if nutrient is non-overdosing
  //
  //     helpful         0-12         sickness
  //     helpful        13-24         no effect
  //     helpful        25-60         accelerated growth factor
  //     helpful        61-128        strong accelerated growth factor
  //     helpful       129-180        *slowed growth
  //     helpful       181-220        *sickness
  //     helpful         221+         *slow death
  //
  //     * same as previous positive category if nutrient is non-overdosing
  //
  //     detrimental     0-12         no effect
  //     detrimental    13-32         slowed growth
  //     detrimental    33-96         sickness
  //     detrimental    97-192        slow death
  //     detrimental     193+         fast death
  //
  //     poisonous       0-9          no effect
  //     poisonous      10-16         slowed growth
  //     poisonous      17-32         sickness
  //     poisonous      33-96         slow death
  //     poisonous       97+          fast death
  uint8_t nutrients[MAX_TOTAL_NUTRIENTS];

  // Trace minerals help determine things like soil color
  species trace_minerals[MN_MAX_TRACE_CONSTITUENTS];
};

struct clay_species_s {
  species id;

  material material;
  mineral_filter_args appearance;
};

struct stone_species_s {
  species id;

  // Reference: http://www.physicalgeography.net/fundamentals/10d.html
  geologic_source source;
  material material;
  mineral_filter_args appearance;

  // Fundamental composition:
  // Note: pure metals use B_NATIVE_METAL and metal_species
  mineral_composition composition;
  mineral_trace_composition trace_composition;
  species constituents[MN_MAX_PRIMARY_CONSTITUENTS];
  species traces[MN_MAX_TRACE_CONSTITUENTS];
};

struct metal_species_s {
  species id;

  material material;
  // metal_texture_params appearance;

  // The elements that make up the alloy:
  species constituents[MN_MAX_ALLOY_CONSTITUENTS];
  // The relative abundance of the constituents:
  uint8_t composition[MN_MAX_ALLOY_CONSTITUENTS];
};

/********************
 * Inline Functions *
 ********************/

static inline int dirt_contains_trace_element(
  dirt_species *sp,
  species el
) {
  size_t i;
  for (i = 0; i < MN_MAX_TRACE_CONSTITUENTS; ++i) {
    if (sp->trace_minerals[i] == el) {
      return 1;
    }
  }
  return 0;
}

static inline int stone_contains_element(
  stone_species *sp,
  species el
) {
  size_t i;
  for (i = 0; i < MN_MAX_PRIMARY_CONSTITUENTS; ++i) {
    if (sp->constituents[i] == el) {
      return 1;
    }
  }
  return 0;
}

static inline int stone_contains_trace_element(
  stone_species *sp,
  species el
) {
  size_t i;
  for (i = 0; i < MN_MAX_TRACE_CONSTITUENTS; ++i) {
    if (sp->traces[i] == el) {
      return 1;
    }
  }
  return 0;
}

#endif // #ifndef MINERAL_SPECIES_H
