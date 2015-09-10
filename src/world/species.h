#ifndef SPECIES_H
#define SPECIES_H

#include "datatypes/map.h"

#include "txgen/txg_minerals.h"
#include "txgen/txg_plants.h"

#include "grammar.h"
#include "materials.h"

// species.h
// Manages species info for different block/item/entity types.

/**********
 * Macros *
 **********/

#define SPECIES_ACCESS_FUNCTIONS_DECL(SP_LOWER) \
  void add_ ## SP_LOWER ## _species(species s, SP_LOWER ## _species* sp); \
  SP_LOWER ## _species* get_ ## SP_LOWER ## _species(species s); \
  species create_ ## SP_LOWER ## _species(void);

// TODO: More graceful failure for both adding and getting.
#define SPECIES_ACCESS_FUNCTIONS(SP_LOWER, SP_CAPS) \
  void add_ ## SP_LOWER ## _species(species s, SP_LOWER ## _species* sp) { \
_Pragma("GCC diagnostic ignored \"-Wint-to-pointer-cast\"") \
    SP_LOWER ## _species* old = (SP_LOWER ## _species*) m1_put_value( \
      SP_CAPS ## _SPECIES, \
      sp, \
      (map_key_t) s \
    ); \
_Pragma("GCC diagnostic warning \"-Wint-to-pointer-cast\"") \
    if (old != NULL) { \
      fprintf( \
        stderr, \
        "Error: Attempt to add new " #SP_LOWER " species %d which already " \
        "exists.", \
        s \
      ); \
      exit(-1); \
    } \
  } \
  \
  SP_LOWER ## _species* get_ ## SP_LOWER ## _species(species s) { \
_Pragma("GCC diagnostic ignored \"-Wint-to-pointer-cast\"") \
    SP_LOWER ## _species* result = (SP_LOWER ## _species*) m1_get_value( \
      SP_CAPS ## _SPECIES, \
      (map_key_t) s \
    ); \
_Pragma("GCC diagnostic warning \"-Wint-to-pointer-cast\"") \
    if (result == NULL) { \
      fprintf( \
        stderr, \
        "Error: tried to lookup unknown " #SP_LOWER " species %d.\n" \
        "Species count is: %zu\n", \
        s, \
        m_get_count(SP_CAPS ## _SPECIES) \
      ); \
      exit(-1); \
    } \
    return result; \
  } \
  \
  species create_ ## SP_LOWER ## _species(void) { \
    species result = m_get_count(SP_CAPS ## _SPECIES); \
    SP_LOWER ## _species* new_species = (SP_LOWER ## _species*) calloc( \
      1, \
      sizeof(SP_LOWER ## _species) \
    ); \
    add_ ## SP_LOWER ## _species(result, new_species); \
    return result; \
  }

// TODO: thread safety here!

/*********
 * Enums *
 *********/

enum species_type_e {
  SPT_NO_SPECIES = 0,

  SPT_GAS = 1,

  SPT_DIRT = 2,
  SPT_CLAY = 3,
  SPT_STONE = 4,
  SPT_METAL = 5,
  
  SPT_FUNGUS = 6,
  SPT_MOSS = 7,
  SPT_GRASS = 8,
  SPT_VINE = 9,
  SPT_HERB = 10,
  SPT_BUSH = 11,
  SPT_SHRUB = 12,
  SPT_TREE = 13,
  SPT_AQUATIC_GRASS = 14,
  SPT_AQUATIC_PLANT = 15,
  SPT_CORAL = 16,
  
  SPT_ANIMAL = 17,
  SPT_MYTHICAL = 18,
  SPT_SENTIENT = 19,
  
  SPT_FIBER = 20,
  SPT_PIGMENT = 21
};
typedef enum species_type_e species_type;

/************************
 * Secondary Structures *
 ************************/

// Growth properties are common across all growing things and contain
// information like growth priority and patterns.
struct growth_properties_s;
typedef struct growth_properties_s growth_properties;

// Growth patterns determine how growing things grow.
// Seed growth patterns are used for seed-type blocks.
struct seed_growth_pattern_s;
typedef struct seed_growth_pattern_s seed_growth_pattern;

// Core growth patterns are used for growth-core-type blocks.
struct core_growth_pattern_s;
typedef struct core_growth_pattern_s core_growth_pattern;


// These materials structures each bundle together a few materials that define
// different parts of an organism:

struct fungus_materials_s;
typedef struct fungus_materials_s fungus_materials;

struct plant_materials_s;
typedef struct plant_materials_s plant_materials;

struct coral_materials_s;
typedef struct coral_materials_s coral_materials;


// Various parameters that determine the appearance of an herb.
struct herb_appearance_s;
typedef struct herb_appearance_s herb_appearance;

/**********************
 * Primary Structures *
 **********************/

// Inorganics:
struct gas_species_s;
typedef struct gas_species_s gas_species;

// Minerals:
struct dirt_species_s;
typedef struct dirt_species_s dirt_species;
struct clay_species_s;
typedef struct clay_species_s clay_species;
struct stone_species_s;
typedef struct stone_species_s stone_species;
struct metal_species_s;
typedef struct metal_species_s metal_species;

// Plants:
struct fungus_species_s;
typedef struct fungus_species_s fungus_species;
struct moss_species_s;
typedef struct moss_species_s moss_species;
struct grass_species_s;
typedef struct grass_species_s grass_species;
struct vine_species_s;
typedef struct vine_species_s vine_species;
struct herb_species_s;
typedef struct herb_species_s herb_species;
struct bush_species_s;
typedef struct bush_species_s bush_species;
struct shrub_species_s;
typedef struct shrub_species_s shrub_species;
struct tree_species_s;
typedef struct tree_species_s tree_species;
struct aquatic_grass_species_s;
typedef struct aquatic_grass_species_s aquatic_grass_species;
struct aquatic_plant_species_s;
typedef struct aquatic_plant_species_s aquatic_plant_species;
struct coral_species_s;
typedef struct coral_species_s coral_species;

// Animals:
struct animal_species_s; // non-sentient animals
typedef struct animal_species_s animal_species;
struct mythical_species_s; // mythical creatures
typedef struct mythical_species_s mythical_species;
struct sentient_species_s; // sentient species
typedef struct sentient_species_s sentient_species;

// Items:
struct fiber_species_s;
typedef struct fiber_species_s fiber_species;

struct pigment_species_s;
typedef struct pigment_species_s pigment_species;

// Any species type:
// The any_species structure just holds a species id along with a species_type
// identifier. Together this information can be used to look up a specific
// species structure.
struct any_species_s;
typedef struct any_species_s any_species;

/***********
 * Globals *
 ***********/

// Inorganics:
extern map *GAS_SPECIES;

// Minerals:
extern map *DIRT_SPECIES;
extern map *CLAY_SPECIES;
extern map *STONE_SPECIES; // also used for sand, gravel etc.
extern map *METAL_SPECIES;

// Plants:
extern map *FUNGUS_SPECIES;
extern map *MOSS_SPECIES;
extern map *GRASS_SPECIES;
extern map *VINE_SPECIES;
extern map *HERB_SPECIES; // also used for bales, thatch, straw mats, etc.
extern map *BUSH_SPECIES;
extern map *SHRUB_SPECIES;
extern map *TREE_SPECIES; // also used for wood
extern map *AQUATIC_GRASS_SPECIES;
extern map *AQUATIC_PLANT_SPECIES;
extern map *CORAL_SPECIES;

// Animals:
extern map *ANIMAL_SPECIES;
extern map *MYTHICAL_SPECIES;
extern map *SENTIENT_SPECIES;

// Items:
extern map *FIBER_SPECIES; // various sources; uniform use
extern map *PIGMENT_SPECIES; // various sources; uniform use

/*************************
 * Structure Definitions *
 *************************/

// Secondary structures:

struct seed_growth_pattern_s {
  cell_grammar *grammar;
  // TODO: More complex growth speeds (by block type; age; for different growth
  // events; etc.)
  int sprout_time;
  // TODO: More here?
};

struct core_growth_pattern_s {
  // TODO: HERE
};

struct growth_properties_s {
  int seed_growth_resist;
  int growth_resist;
  int growth_strength;
  seed_growth_pattern seed_growth;
  core_growth_pattern core_growth;
};

struct fungus_materials_s {
  material spores;
  material mycelium;
  material stalk;
  material cap;
};

struct plant_materials_s {
  material seed;
  material root;
  material wood;
  material dry_wood;
  material stem;
  material leaf;
  material fruit;
};

struct coral_materials_s {
  material anchor;
  material frond;
};

struct herb_appearance_s {
  // TODO: More appearance diversity?
  leaves_filter_args seeds;
  branch_filter_args roots;
  herb_leaves_filter_args shoots;
  herb_leaves_filter_args leaves;
  leaves_filter_args buds;
  leaves_filter_args flowers;
  leaves_filter_args fruit;
};


// Primary structures:
// TODO: Fill in all structures here.

struct gas_species_s {
  material material;
  // gas_texture_params appearance;
};


struct dirt_species_s {
  // These three primary properties should add up to 100. Along with
  // organic_content and acidity, they determine most of the other properties
  // of the soil. Note that the sand_percent should not exceed 75 and the
  // clay_percent should not exceed 50. Under these conditions, the "dirt"
  // would be better classified as either sand or clay, and the use of B_SAND
  // or B_CLAY along with a stone_species or clay_species is more appropriate.
  // B_DIRT and dirt_species thus represent both pure silts and all manner of
  // loams that fill the rest of a sand/clay/silt triangle composition graph.
  uint8_t sand_percent;
  uint8_t clay_percent;
  uint8_t silt_percent;

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
  // on a scale of 1-255, where 32 is a "standard level" which neither enhances
  // nor impedes growth.
  // TODO: nitrogen fixation?
  uint8_t base_phosphorus;
  uint8_t base_nitrogen;
  uint8_t base_potassium;

  // These are secondary nutrient levels, which again use a scale of 0-255 with
  // 32 as the "standard level."
  uint8_t base_sulfur;
  uint8_t base_magnesium;
  uint8_t base_calcium;

  material material;
  // dirt_texture_params appearance;
};

struct clay_species_s {
  material material;
  // clay_texture_params appearance;
};

struct stone_species_s {
  material material;
  stone_filter_args appearance;
};

struct metal_species_s {
  material material;
  // metal_texture_params appearance;
};

struct fungus_species_s {
  fungus_materials materials;
  // fungus_texture_params appearance;
  growth_properties growth;
};

struct moss_species_s {
  plant_materials materials;
  // moss_texture_params appearance;
  growth_properties growth;
};

struct grass_species_s {
  plant_materials materials;
  // grass_texture_params appearance;
  growth_properties growth;
};

struct vine_species_s {
  plant_materials materials;
  // vine_texture_params appearance;
  growth_properties growth;
};

struct herb_species_s {
  plant_materials materials;
  herb_appearance appearance;
  growth_properties growth;
};

struct bush_species_s {
  plant_materials materials;
  // bush_texture_params appearance;
  growth_properties growth;
};

struct shrub_species_s {
  plant_materials materials;
  // shrub_texture_params appearance;
  growth_properties growth;
};

struct tree_species_s {
  plant_materials materials;
  // tree_texture_params appearance;
  growth_properties growth;
};

struct aquatic_grass_species_s {
  plant_materials materials;
  // aquatic_grass_texture_params appearance;
  growth_properties growth;
};

struct aquatic_plant_species_s {
  plant_materials materials;
  // aquatic_plant_texture_params appearance;
  growth_properties growth;
};

struct coral_species_s {
  coral_materials materials;
  // coral_texture_params appearance;
  growth_properties growth;
};

struct animal_species_s {
  material material;
  // entity_texture_params appearance;
};

struct mythical_species_s {
  material material;
  // entity_texture_params appearance;
};

struct sentient_species_s {
  material material;
  // entity_texture_params appearance;
};

struct fiber_species_s {
  material material;
  // fiber_texture_params appearance;
};

struct pigment_species_s {
  material material;
  // pigment_texture_params appearance;
};
/*
*/

// The any_species structure

struct any_species_s {
  species_type type;
  species id;
};

/********************
 * Inline Functions *
 ********************/

// Fills in the given any_species struct with information from the given block.
static inline void block__any_species(block b, any_species *sp) {
  sp->type = bi_species_type(b);
  sp->id = b_species(b);
}

/*************
 * Functions *
 *************/

// Required setup for the species tables.
void setup_species(void);

// Returns a pointer to a species info struct for the given species. Which
// species struct type to cast the pointer to can be determined by the type
// parameter of the any_species argument.
void* get_species_data(any_species sp);

SPECIES_ACCESS_FUNCTIONS_DECL(gas);

SPECIES_ACCESS_FUNCTIONS_DECL(dirt);
SPECIES_ACCESS_FUNCTIONS_DECL(clay);
SPECIES_ACCESS_FUNCTIONS_DECL(stone);
SPECIES_ACCESS_FUNCTIONS_DECL(metal);

SPECIES_ACCESS_FUNCTIONS_DECL(fungus);
SPECIES_ACCESS_FUNCTIONS_DECL(moss);
SPECIES_ACCESS_FUNCTIONS_DECL(grass);
SPECIES_ACCESS_FUNCTIONS_DECL(vine);
SPECIES_ACCESS_FUNCTIONS_DECL(herb);
SPECIES_ACCESS_FUNCTIONS_DECL(bush);
SPECIES_ACCESS_FUNCTIONS_DECL(shrub);
SPECIES_ACCESS_FUNCTIONS_DECL(tree);
SPECIES_ACCESS_FUNCTIONS_DECL(aquatic_grass);
SPECIES_ACCESS_FUNCTIONS_DECL(aquatic_plant);
SPECIES_ACCESS_FUNCTIONS_DECL(coral);

SPECIES_ACCESS_FUNCTIONS_DECL(animal);
SPECIES_ACCESS_FUNCTIONS_DECL(mythical);
SPECIES_ACCESS_FUNCTIONS_DECL(sentient);

SPECIES_ACCESS_FUNCTIONS_DECL(fiber);
SPECIES_ACCESS_FUNCTIONS_DECL(pigment);

#endif // ifndef SPECIES_H
