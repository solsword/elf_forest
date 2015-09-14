#ifndef ELEMENTS_H
#define ELEMENTS_H

// elements.h
// Element generation.

#include "datatypes/rngtable.h"

#include "world/species.h"

/*************
 * Constants *
 *************/

// Element generation parameters:
#define MIN_MAIN_ELEMENTS 3
#define MAX_MAIN_ELEMENTS 5
#define MIN_COMMON_ELEMENTS 7
#define MAX_COMMON_ELEMENTS 12
#define MIN_UNCOMMON_ELEMENTS 6
#define MAX_UNCOMMON_ELEMENTS 10

#define MIN_LIQUID_ELEMENTS 2.0
#define MIN_GASEOUS_ELEMENTS 3.0

#define MIN_MAIN_METALLIC_ELEMENTS 0
#define MAX_MAIN_METALLIC_ELEMENTS 2

#define MIN_COMMON_METALLIC_ELEMENTS 2
#define MAX_COMMON_METALLIC_ELEMENTS 4

#define MIN_UNCOMMON_METALLIC_ELEMENTS 4
#define MAX_UNCOMMON_METALLIC_ELEMENTS 7

/*************
 * Functions *
 *************/

// Generates elements for the world.
void generate_elements(world_map *wm);


// Creates a new element.
void create_new_element(
  element_frequency frequency,
  phase phase,
  size_t is_metallic,
);

#endif // ifndef ELEMENTS_H
