#ifndef SPECIES_H
#define SPECIES_H

#include "datatypes/map.h"
#include "datatypes/list.h"

#include "ecology/adaptations.h"
#include "txgen/txg_minerals.h"
#include "txgen/txg_plants.h"

#include "grammar.h"
#include "materials.h"
#include "blocks.h"

// species.h
// Manages species info for different block/item/entity types.

/**********
 * Macros *
 **********/

#define SPECIES_ACCESS_FUNCTIONS_DECL(SP_LOWER) \
  species add_ ## SP_LOWER ## _species(SP_LOWER ## _species* sp); \
  SP_LOWER ## _species* get_ ## SP_LOWER ## _species(species s); \
  species create_ ## SP_LOWER ## _species(void);

// TODO: More graceful failure for both adding and getting.
#ifdef DEBUG
#define SPECIES_ACCESS_FUNCTIONS(SP_LOWER, SP_CAPS) \
  species add_ ## SP_LOWER ## _species(SP_LOWER ## _species* sp) { \
    species result = 1 + m_get_count(SP_CAPS ## _SPECIES); \
    sp->id = result; \
_Pragma("GCC diagnostic ignored \"-Wint-to-pointer-cast\"") \
    SP_LOWER ## _species* old = (SP_LOWER ## _species*) m1_put_value( \
      SP_CAPS ## _SPECIES, \
      sp, \
      (map_key_t) result \
    ); \
_Pragma("GCC diagnostic warning \"-Wint-to-pointer-cast\"") \
    if (old != NULL) { \
      fprintf( \
        stderr, \
        "Error: Attempt to add new " #SP_LOWER " species %d which already " \
        "exists.", \
        result \
      ); \
      exit(EXIT_FAILURE); \
    } \
    return result; \
  } \
  \
  SP_LOWER ## _species* get_ ## SP_LOWER ## _species(species s) { \
_Pragma("GCC diagnostic ignored \"-Wint-to-pointer-cast\"") \
    SP_LOWER ## _species* result = (SP_LOWER ## _species*) m1_get_value( \
      SP_CAPS ## _SPECIES, \
      (map_key_t) s \
    ); \
_Pragma("GCC diagnostic warning \"-Wint-to-pointer-cast\"") \
    if (s != 0 && result == NULL) { \
      fprintf( \
        stderr, \
        "Error: tried to lookup unknown " #SP_LOWER " species %d.\n" \
        "Species count is: %zu\n", \
        s, \
        m_get_count(SP_CAPS ## _SPECIES) \
      ); \
      exit(EXIT_FAILURE); \
    } \
    return result; \
  } \
  \
  species create_ ## SP_LOWER ## _species(void) { \
    species result; \
    SP_LOWER ## _species* new_species = (SP_LOWER ## _species*) calloc( \
      1, \
      sizeof(SP_LOWER ## _species) \
    ); \
    result = add_ ## SP_LOWER ## _species(new_species); \
    return result; \
  }
#else // ifdef DEBUG
#define SPECIES_ACCESS_FUNCTIONS(SP_LOWER, SP_CAPS) \
  species add_ ## SP_LOWER ## _species(SP_LOWER ## _species* sp) { \
    species result = 1 + m_get_count(SP_CAPS ## _SPECIES); \
    sp->id = result; \
_Pragma("GCC diagnostic ignored \"-Wint-to-pointer-cast\"") \
    SP_LOWER ## _species* old = (SP_LOWER ## _species*) m1_put_value( \
      SP_CAPS ## _SPECIES, \
      sp, \
      (map_key_t) result \
    ); \
_Pragma("GCC diagnostic warning \"-Wint-to-pointer-cast\"") \
    return result; \
  } \
  \
  SP_LOWER ## _species* get_ ## SP_LOWER ## _species(species s) { \
_Pragma("GCC diagnostic ignored \"-Wint-to-pointer-cast\"") \
    SP_LOWER ## _species* result = (SP_LOWER ## _species*) m1_get_value( \
      SP_CAPS ## _SPECIES, \
      (map_key_t) s \
    ); \
_Pragma("GCC diagnostic warning \"-Wint-to-pointer-cast\"") \
    return result; \
  } \
  \
  species create_ ## SP_LOWER ## _species(void) { \
    species result; \
    SP_LOWER ## _species* new_species = (SP_LOWER ## _species*) calloc( \
      1, \
      sizeof(SP_LOWER ## _species) \
    ); \
    result = add_ ## SP_LOWER ## _species(new_species); \
    return result; \
  }
#endif // ifdef DEBUG

// TODO: thread safety here!

/*********
 * Types *
 *********/

// Would be an enum but we want a strict size cap here (see any_species below).
typedef uint8_t species_type;

#define SPT_NO_SPECIES 0

#define SPT_ELEMENT 1

#define SPT_GAS 2

#define SPT_DIRT 3
#define SPT_CLAY 4
#define SPT_STONE 5
#define SPT_METAL 6
  
#define SPT_FUNGUS 7
#define SPT_MOSS 8
#define SPT_GRASS 9
#define SPT_VINE 10
#define SPT_HERB 11
#define SPT_BUSH 12
#define SPT_SHRUB 13
#define SPT_TREE 14
#define SPT_AQUATIC_GRASS 15
#define SPT_AQUATIC_PLANT 16
#define SPT_CORAL 17
  
#define SPT_ANIMAL 18
#define SPT_MYTHICAL 19
#define SPT_SENTIENT 20
  
#define SPT_FIBER 21
#define SPT_PIGMENT 22

// The any_species structure just holds a species id along with a species_type
// identifier. Together this information can be used to look up a specific
// species structure.
typedef uint32_t any_species;

#define     ANY_SP_TYPE_SHIFT sizeof(species)
#define      ANY_SP_TYPE_MASK ((any_species) umaxof(species_type))
#define  ANY_SP_SPECIES_SHIFT 0
#define   ANY_SP_SPECIES_MASK ((any_species) umaxof(species))

/***************************
 * Cross-Species Constants *
 ***************************/

// Species '0' is reserved as an 'invalid species' indicator.
#define SP_INVALID 0

ELFSCRIPT_GL(i, SP_INVALID)

// This number needs to be at least as large as the maximum number of life
// elements (each of which is a nutrient) plus the sum of the maximums of each
// nutrient category (for animals and plants). That number is currently:
//   life     plants            animals
//    4 + (2 + 6 + 7 + 5) + (2 + 6 + 7 + 5)
//  = 44
// This number is used for sizing arrays intended to represent comprehensive
// nutrient compositions.
#define MAX_TOTAL_NUTRIENTS 50

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

/********************
 * Inline Functions *
 ********************/

static inline void * species__v(species s) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  return (void*) s;
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
}

static inline species v__species(void *v_species) {
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
  return (species) v_species;
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
}

// Fills in the given any_species struct with information from the given block.
static inline void block__any_species(block b, any_species *sp) {
  *sp = 0;
  *sp |= (
    ((any_species) bi_species_type(b)) & ANY_SP_TYPE_MASK
  ) << ANY_SP_TYPE_SHIFT;
  *sp |= (
    ((any_species) b_species(b)) & ANY_SP_SPECIES_MASK
  ) << ANY_SP_SPECIES_SHIFT;
}

static inline species_type any_species_type(any_species asp) {
  return (species_type) ((asp >> ANY_SP_TYPE_SHIFT) & ANY_SP_TYPE_MASK);
}

static inline species any_species_species(any_species asp) {
  return (species) ((asp >> ANY_SP_SPECIES_SHIFT) & ANY_SP_SPECIES_MASK);
}

static inline void any_species_set_type(
  any_species *asp,
  species_type t
) {
  *asp &= ~(ANY_SP_TYPE_MASK << ANY_SP_TYPE_SHIFT);
  *asp |= (t & ANY_SP_TYPE_MASK) << ANY_SP_TYPE_SHIFT;
}

static inline void any_species_set_species(any_species *asp, species sp) {
  *asp &= ~(ANY_SP_SPECIES_MASK << ANY_SP_SPECIES_SHIFT);
  *asp |= (sp & ANY_SP_SPECIES_MASK) << ANY_SP_SPECIES_SHIFT;
}

/*********************
 * General Functions *
 *********************/

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

/*********************
 * Element Functions *
 *********************/
// see element.h

// Returns one of the integer properties of an element looked up via an
// element_property constant. If given a float property, it prints an error
// message and exits the program.
int el_int_property(
  element_species *sp,
  element_property property
);

// As above for a floating point property. Note that the constant names include
// _I_ or _F_ specifying whether a property is an integer or float.
float el_float_property(
  element_species *sp,
  element_property property
);

// Returns the average of a float property across several element species
// (given as a list of pointers to element_species structs). The given list is
// not modified and the caller retains responsibility for it.
float el_avg_property(
  list const * const species,
  element_property property
);

// As above, but also accepts a list of weights (use f_as_p to get floating
// point values into a list). The list of weights must be the same length as
// the list of species.
float el_weighted_property(
  list const * const species,
  list const * const weights,
  element_property property
);

#endif // ifndef SPECIES_H
