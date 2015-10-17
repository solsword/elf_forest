#ifndef MATERIALS_H
#define MATERIALS_H

// materials.h
// Material management and properties.

#include <stdint.h>
#include <stdio.h>

#include "blocks.h"
#include "measures.h"

/************************
 * Types and Structures *
 ************************/

enum material_origin_e {
  MO_UNKNOWN, // ???
  MO_PURE_ELEMENT, // a pure element
  MO_ATMOSPHERE, // air
  MO_WATER, // water; clouds
  MO_IGNEOUS_MINERAL, // stone
  MO_SEDIMENTARY_MINERAL, // stone
  MO_METAMORPHIC_MINERAL, // stone
  MO_METALLIC, // iron
  MO_ERODED, // clay
  MO_DECOMPOSED, // dirt; coal
  MO_ORGANIC, // hay; flesh; wood
  MO_REACTION, // acid
  MO_COMBUSTION, // charcoal; smoke
  MO_MIXTURE, // plaster; cement
  MO_MAGIC, // ectoplasm; ether
};
typedef enum material_origin_e material_origin;

// A material carries with it the basic properties of origin and form as well
// as general physical properties, some of which are only applicable to certain
// material types.

struct material_s;
typedef struct material_s material;


/*************************
 * Structure Definitions *
 *************************/

struct material_s {
  // origin:
  material_origin origin;
  // density of the material:
  density solid_density;
  density liquid_density;
  density gas_density;
  // specific heat in various forms:
  specific_heat solid_specific_heat;
  specific_heat liquid_specific_heat;
  specific_heat gas_specific_heat;
  // temperature info:
  temperature cold_damage_temp;
  temperature solidus;
  temperature liquidus;
  temperature boiling_point;
  temperature ignition_point;
  temperature flash_point;
  // fine structure info:
  temperature cold_plastic_temp; // temperature of minimum plasticity
  temperature warm_plastic_temp; // temperature of maximum plasticity
  plasticity cold_plasticity; // plasticity at the cold plastic point
  plasticity warm_plasticity; // plasticity at the warm plastic point
  hardness hardness; // in solid phase
  viscosity viscosity; // in liquid phase, measured relative to water at 1.0
  pH pH; // in liquid phase
};

/********************
 * Inline Functions *
 ********************/

// Calculates the material associated with the given block:
/* TODO: Do we need this? (If so it needs to do a species lookup.)
static inline material mt_from_block(block b) {
  material result = (((material) b_species(b)) << MS_SPECIES);
  result &= ((material) MAT_INFO[b_id(b)]) << MS_FORM;
  return result;
}
*/

// Takes a specific weight (floating point density in terms of a base density)
// and returns an absolute material density. Prints a warning if the specific
// weight given can't be accurately represented in absolute terms.
static inline density mat_density(float specific_weight) {
  ptrdiff_t i = fastfloor(((float) BASE_DENSITY) * specific_weight);
#ifdef DEBUG
  if (specific_weight < (1.0/(1.5*((float) BASE_DENSITY)))) {
    fprintf(
      stderr,
      "Warning: specific weight %.4f is too small to be accurately "
      "represented in terms of absolute density.\n",
      specific_weight
    );
  } else if (i > umaxof(density) + 4) {
    fprintf(
      stderr,
      "Warning: specific weight %.2f is too large to be accurately "
      "represented in terms of absolute density.\n",
      specific_weight
    );
  }
#endif
  if (i > umaxof(density)) {
    return umaxof(density);
  } else if (i < 1) {
    return 1;
  } else {
    return (density) i;
  }
}

// Works like mat_density, but for specific heat instead of density.
static inline specific_heat mat_specific_heat(float relative) {
  ptrdiff_t i = fastfloor(((float) BASE_SPECIFIC_HEAT) * relative);
#ifdef DEBUG
  if (relative < (1.0/(1.5*((float) BASE_DENSITY)))) {
    fprintf(
      stderr,
      "Warning: relative specific heat %.4f is too small to be accurately "
      "represented in terms of absolute specific heat.\n",
      relative
    );
  } else if (i > umaxof(specific_heat) + 4) {
    fprintf(
      stderr,
      "Warning: relative specific heat %.2f is too large to be accurately "
      "represented in terms of absolute specific heat.\n",
      relative
    );
  }
#endif
  if (i > umaxof(specific_heat)) {
    return umaxof(specific_heat);
  } else if (i < 1) {
    return 1;
  } else {
    return (specific_heat) i;
  }
}

/******************************
 * Constructors & Destructors *
 ******************************/

/*************
 * Functions *
 *************/

#endif // ifndef MATERIALS_H
