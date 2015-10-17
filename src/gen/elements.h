#ifndef ELEMENTS_H
#define ELEMENTS_H

// elements.h
// Element generation.

#include "datatypes/rngtable.h"

#include "world/species.h"

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

// Element generation parameters:

// TODO: Any of these here?

/*************
 * Functions *
 *************/

// Generates elements for the world.
void generate_elements(world_map *wm);

// Takes a categorized element and fills out all of its properties.
void fill_out_element(element_species *esp);

#endif // ifndef ELEMENTS_H
