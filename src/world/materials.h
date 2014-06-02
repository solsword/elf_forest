#ifndef MATERIALS_H
#define MATERIALS_H

// materials.h
// Material management and properties.

#include <stdint.h>

#include "blocks.h"

/************************
 * Types and Structures *
 ************************/

// A material carries with it the abstract properties of origin and form as
// well as a species ID.
// 4 bits of material origin
// 4 bits of material form
// 14 bits of block_species info
// 11 extra bits ???
typedef uint32_t material;

// Material origin divides materials into different gross types like igneous
// rock or plant material.
typedef uint8_t material_origin;

// Material form describes the overall cohesion and organization of a material.
typedef uint8_t material_form;

// For storing form+origin in the same place:
typedef uint8_t material_info;

/*************
 * Constants *
 *************/

// Bit widths for origin and form information:
#define MAT_ORI_BITS 4
#define MAT_FORM_BITS 4

// Shifts extracting individual fields:
#define MS_ORIGIN (MAT_FORM_BITS + BLOCK_SPC_BITS)
#define MS_FORM BLOCK_SPC_BITS
#define MS_SPECIES 0

// Shifts for material_info:
#define MIS_ORIGIN MAT_FORM_BITS
#define MIS_FORM 0

extern material_info const MAT_INFO[TOTAL_BLOCK_TYPES];

static material_origin const          MO_UNKNOWN = 0x0; // ???
static material_origin const       MO_ATMOSPHERE = 0x1; // air
static material_origin const            MO_WATER = 0x2; // water; clouds
static material_origin const          MO_MINERAL = 0x3; // stone
static material_origin const         MO_METALLIC = 0x4; // iron
static material_origin const           MO_ERODED = 0x5; // clay; sand; gravel
static material_origin const       MO_DECOMPOSED = 0x6; // dirt; coal
static material_origin const          MO_ORGANIC = 0x7; // hay; flesh; wood
static material_origin const         MO_REACTION = 0x8; // acid
static material_origin const       MO_COMBUSTION = 0x9; // charcoal; smoke
static material_origin const            MO_MAGIC = 0xa; // ectoplasm; ether
static material_origin const            MO_MIXED = 0xb; // plaster; cement
static material_origin const       MO_SUBTRACTED = 0xc; // engraving

static material_form const           MF_FORMLESS = 0x0; // ???
static material_form const            MF_GASEOUS = 0x1; // air
static material_form const        MF_THIN_LIQUID = 0x2; // water
static material_form const       MF_THICK_LIQUID = 0x3; // lava
static material_form const        MF_FINE_GRAINS = 0x4; // sand
static material_form const       MF_ROUGH_GRAINS = 0x5; // gravel
static material_form const       MF_LARGE_CHUNKS = 0x6; // scree; dry-stone wall
static material_form const        MF_LOOSE_SOLID = 0x7; // dirt
static material_form const   MF_COMPRESSED_SOLID = 0x8; // clay; rammed earth
static material_form const        MF_BOUND_SOLID = 0x9; // rock
static material_form const      MF_UNIFORM_SOLID = 0xa; // crystal; metal
static material_form const        MF_SOFT_TISSUE = 0xb; // flesh; leaves
static material_form const        MF_HARD_TISSUE = 0xc; // wood; shell; bone
static material_form const      MF_PACKED_FIBERS = 0xd; // wool; paper
static material_form const        MF_SPUN_FIBERS = 0xe; // yarn; cloth
static material_form const        MF_CONSTRUCTED = 0xf; // a chair

/***********************
 * Getters and Setters *
 ***********************/

static inline material_origin mt_origin(material m) {
  return (m >> MS_ORIGIN) & MAT_ORI_BITS;
}

static inline material_form mt_form(material m) {
  return (m >> MS_FORM) & MAT_FORM_BITS;
}

static inline block_species mt_species(material m) {
  return (m >> MS_SPECIES) & BLOCK_SPC_BITS;
}

/********************
 * Inline Functions *
 ********************/

static inline material mt_from_block(block b) {
  material result = (b_species(b) << MS_SPECIES);
  result &= MAT_INFO[b_id(b)] << MS_FORM;
  return result;
}

static inline float mt_erosion_rate(material m) {
  return 1.0; // TODO: HERE!
};

static inline float mt_weight(material m) {
  return 1.0; // TODO: HERE!
};

static inline float mt_compression(material m, float pressure) {
  return 1.0; // TODO: HERE!
};

static inline float mt_metamorphosis_rate(
  material m,
  float pressure,
  float temperature
) {
  return 1.0; // TODO: HERE!
};

static inline float mt_metamorphic_product(
  material m,
  float pressure,
  float temperature
) {
  return 1.0; // TODO: HERE!
};

/******************************
 * Constructors & Destructors *
 ******************************/

/*************
 * Functions *
 *************/

#endif // ifndef MATERIALS_H
