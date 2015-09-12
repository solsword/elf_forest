#ifndef ELEMENT_SPECIES_H
#define ELEMENT_SPECIES_H
// Element species types.

#include "world/measures.h"

/*********
 * Enums *
 *********/

// TODO: Any of these?

/**************
 * Structures *
 **************/

// Primary species structures:
struct element_species_s;
typedef struct element_species_s element_species;

/*************************
 * Structure Definitions *
 *************************/

struct element_species_s {
  pH pH; // acidic/basic tendency; neutral = 7.0

  // physical properties of the pure element:
  density density;
  hardness hardness;
  plasticity plasticity;

  // Tendencies when forming stone:
  // TODO: HERE
};

#endif // #ifndef ELEMENT_SPECIES_H
