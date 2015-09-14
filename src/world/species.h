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

  SPT_ELEMENT = 1,

  SPT_GAS = 2,

  SPT_DIRT = 3,
  SPT_CLAY = 4,
  SPT_STONE = 5,
  SPT_METAL = 7,
  
  SPT_FUNGUS = 7,
  SPT_MOSS = 8,
  SPT_GRASS = 9,
  SPT_VINE = 10,
  SPT_HERB = 11,
  SPT_BUSH = 12,
  SPT_SHRUB = 13,
  SPT_TREE = 14,
  SPT_AQUATIC_GRASS = 15,
  SPT_AQUATIC_PLANT = 16,
  SPT_CORAL = 17,
  
  SPT_ANIMAL = 18,
  SPT_MYTHICAL = 19,
  SPT_SENTIENT = 20,
  
  SPT_FIBER = 21,
  SPT_PIGMENT = 22
};
typedef enum species_type_e species_type;

/************************
 * Secondary Structures *
 ************************/


/**********************
 * Primary Structures *
 **********************/

// Elements:
#include "world/species/element.h"

// Inorganic:
#include "world/species/substance.h"

// Minerals:
#include "world/species/mineral.h"

// Plants:
#include "world/species/vegetable.h"

// Animals:
#include "world/species/animal.h"

// Items:
#include "world/species/material.h"

// Any species type:
// The any_species structure just holds a species id along with a species_type
// identifier. Together this information can be used to look up a specific
// species structure.
struct any_species_s;
typedef struct any_species_s any_species;

/***********
 * Globals *
 ***********/

// Elements:
extern map *ELEMENT_SPECIES;

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

SPECIES_ACCESS_FUNCTIONS_DECL(element);

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
