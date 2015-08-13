#ifndef SPECIES_H
#define SPECIES_H

#include "datatypes/map.h"

#include "txgen/txg_minerals.h"
#include "txgen/txg_plants.h"

#include "world/grammar.h"

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
struct animal_species_s; // non-sentient animals & mythical beasts
typedef struct animal_species_s animal_species;
struct sentient_species_s; // sentient species
typedef struct sentient_species_s sentient_species;

// Items:
struct fiber_species_s;
typedef struct fiber_species_s fiber_species;

/***********
 * Globals *
 ***********/

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
extern map *HERB_SPECIES;
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

/*************************
 * Structure Definitions *
 *************************/

// Secondary structures:

struct growth_properties_s {
  ptrdiff_t seed_growth_resist;
  ptrdiff_t growth_resist;
  ptrdiff_t growth_strength;
  seed_growth_pattern seed_growth;
  core_growth_pattern core_growth;
}

struct seed_growth_pattern_s {
  cell_grammar *grammar;
  // TODO: More complex growth speeds (by block type; age; for different growth
  // events; etc.)
  ptrdiff_t sprout_time;
  // TODO: More here?
};

struct core_growth_pattern_s {
  // TODO: HERE
};

struct fungus_materials_s {
  material spores;
  material mycelium;
  material stalk;
  material cap;
};

struct plant_materials_s {
  material seeds;
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

/*
struct dirt_species_s {
  material material;
  dirt_texture_params appearance;
};

struct clay_species_s {
  material material;
  clay_texture_params appearance;
};
*/

struct stone_species_s {
  material material;
  stone_filter_args appearance;
};

/*
struct metal_species_s {
  material material;
  metal_texture_params appearance;
};
*/

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

/*
struct animal_species_s {
  material material;
  entity_texture_params appearance;
};

struct sentient_species_s {
  material material;
  entity_texture_params appearance;
};

struct fiber_species_s {
  material material;
  fiber_texture_params appearance;
};
*/


/********************
 * Inline Functions *
 ********************/


/*************
 * Functions *
 *************/

void setup_species(void);

SPECIES_ACCESS_FUNCTIONS_DECL(stone);

#endif // ifndef SPECIES_H
