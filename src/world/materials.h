#ifndef MATERIALS_H
#define MATERIALS_H

// materials.h
// Material management and properties.

#include <stdint.h>

#include "blocks.h"

/************************
 * Types and Structures *
 ************************/

typedef uint8_t material_origin;

// Various material properties:
typedef uint8_t density; // see BASE_DENSITY
typedef uint8_t specific_heat; // see BASE_SPECIFIC_HEAT
typedef int16_t temperature; // in degrees Celsius

// A material carries with it the basic properties of origin and form as well
// as general physical properties, some of which are only applicable to certain
// material types.

struct material_s;
typedef struct material_s material;

/*************
 * Constants *
 *************/

static material_origin const              MO_UNKNOWN = 0x0; // ???
static material_origin const           MO_ATMOSPHERE = 0x1; // air
static material_origin const                MO_WATER = 0x2; // water; clouds
static material_origin const      MO_IGNEOUS_MINERAL = 0x3; // stone
static material_origin const  MO_SEDIMENTARY_MINERAL = 0x4; // stone
static material_origin const  MO_METAMORPHIC_MINERAL = 0x5; // stone
static material_origin const             MO_METALLIC = 0x6; // iron
static material_origin const               MO_ERODED = 0x7; // clay
static material_origin const           MO_DECOMPOSED = 0x8; // dirt; coal
static material_origin const              MO_ORGANIC = 0x9; // hay; flesh; wood
static material_origin const             MO_REACTION = 0xa; // acid
static material_origin const           MO_COMBUSTION = 0xb; // charcoal; smoke
static material_origin const              MO_MIXTURE = 0xc; // plaster; cement
static material_origin const                MO_MAGIC = 0xd; // ectoplasm; ether

// Density of water (~1000 kg/m^3) on the 0-255 scale used for solid and liquid
// densities and of air (~1.2 kg/m^3) on the 0-255 scale used for gas
// densities.
static density const BASE_DENSITY = 12;

// Specific heat of air (~1 J/gK) on the 0-255 scale used for specific heat.
static specific_heat const BASE_SPECIFIC_HEAT = 16;


/*************************
 * Structure Definitions *
 *************************/

struct material_s {
  // origin:
  material_origin origin;
  // density of the material:
  // For solid and liquid density, water (density ~1000 kg/m^3) is set at a
  // value of 12, so the lightest representable material (value 1) has a
  // density of ~83 kg/m^3 while the heaviest (value 255) weighs 21250 kg/m^3.
  // For gas density, air (~1.2 kg/m^3) is set at a value of 12, so the
  // lightest representable gas has a density of ~0.1 kg/m^3 while the heaviest
  // has a density of ~25.5 kg/m^3. See http://en.wikipedia.org/wiki/Density
  // for a table of densities for various substances.
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
  uint8_t malleability; // in solid phase
  float viscosity; // in liquid phase, measured relative to water at 1.0
  uint8_t hardness;
  uint8_t brittleness;
  // 4 bits each of impact, compressive, tensile, and shear strength:
  uint16_t strength;
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
    fprtinf(
      stderr,
      "Warning: specific weight %.4f is too small to be accurately " +
      "represented in terms of absolute density.\n",
      specific_weight
    );
  } else if (i > umaxof(density) + 4) {
    fprtinf(
      stderr,
      "Warning: specific weight %.2f is too large to be accurately " +
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
    fprtinf(
      stderr,
      "Warning: relative specific heat %.4f is too small to be accurately " +
      "represented in terms of absolute specific heat.\n",
      relative
    );
  } else if (i > umaxof(specific_heat) + 4) {
    fprtinf(
      stderr,
      "Warning: relative specific heat %.2f is too large to be accurately " +
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
