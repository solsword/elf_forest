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

/*************************
 * Structure Definitions *
 *************************/

struct material_s {
  // origin:
  material_origin origin;
  // density of the material:
  uint8_t solid_density;
  uint8_t liquid_density;
  uint8_t gas_density;
  // specific heat in various forms:
  uint8_t solid_specific_heat;
  uint8_t liquid_specific_heat;
  uint8_t gas_specific_heat;
  // temperature info:
  uint8_t cold_damage_temp;
  uint8_t freezing_point;
  uint8_t boiling_point;
  uint8_t flashpoint;
  // fine structure info:
  uint8_t malleability; // in solid phase
  uint8_t viscosity; // in liquid phase
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

/******************************
 * Constructors & Destructors *
 ******************************/

/*************
 * Functions *
 *************/

#endif // ifndef MATERIALS_H
