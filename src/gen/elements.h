#ifndef ELEMENTS_H
#define ELEMENTS_H

// elements.h
// Element generation.

#include "datatypes/rngtable.h"

#include "world/species.h"
#include "world/world_map.h"

/*********
 * Enums *
 *********/

enum pH_category_e {
  PHC_NEUTRAL,
  PHC_ACIDIC,
  PHC_BASIC
};
typedef enum pH_category_e pH_category;

/*************
 * Constants *
 *************/

// Note: many generation parameters are stored as static variables in the .c
// file (rngtables specifically).

// Corrosion resistance numbers:
#define MIN_SOLUBLE_CORROSION_RESISTANCE 0
#define MAX_SOLUBLE_CORROSION_RESISTANCE 32
#define MIN_SLIGHTLY_CORROSION_RESISTANCE 0
#define MAX_SLIGHTLY_CORROSION_RESISTANCE 128
#define MIN_INSOLUBLE_CORROSION_RESISTANCE 64
#define MAX_INSOLUBLE_CORROSION_RESISTANCE 255

// Extra critical nutrients beyond the life nutrients:
#define MIN_EXTRA_PLANT_CRITICAL_NUTRIENTS 1
#define MAX_EXTRA_PLANT_CRITICAL_NUTRIENTS 2
#define MIN_EXTRA_ANIMAL_CRITICAL_NUTRIENTS 1
#define MAX_EXTRA_ANIMAL_CRITICAL_NUTRIENTS 2

// Beneficial, harmful, and poison nutrients:
#define MIN_PLANT_BENEFICIAL_NUTRIENTS 2
#define MAX_PLANT_BENEFICIAL_NUTRIENTS 6
#define MIN_PLANT_DETRIMENTAL_NUTRIENTS 2
#define MAX_PLANT_DETRIMENTAL_NUTRIENTS 7
#define MIN_PLANT_POISONS 2
#define MAX_PLANT_POISONS 5

#define MIN_ANIMAL_BENEFICIAL_NUTRIENTS 2
#define MAX_ANIMAL_BENEFICIAL_NUTRIENTS 6
#define MIN_ANIMAL_DETRIMENTAL_NUTRIENTS 2
#define MAX_ANIMAL_DETRIMENTAL_NUTRIENTS 7
#define MIN_ANIMAL_POISONS 2
#define MAX_ANIMAL_POISONS 5

/*************
 * Functions *
 *************/

// Generates elements for the world.
void generate_elements(world_map *wm);

// Takes a categorized element and fills out all of its properties.
void fill_out_element(element_species *esp, ptrdiff_t seed);

#endif // ifndef ELEMENTS_H
