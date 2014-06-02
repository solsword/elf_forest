#ifndef MATERIALS_H
#define MATERIALS_H

// materials.h
// Material management and properties.

#include <stdint.h>

#include "blocks.h"

/************************
 * Types and Structures *
 ************************/

// TODO: Fix this!!!
typedef block material;

// A material carries with it the basic properties of origin and form as well
// as a species ID. It also has general physical properties, some of which are
// only applicable to certain material types.

/*
struct material {
  // species ID:
  species sp;
  // 4 bits (higher) of origin and 4 bits (lower) of form:
  uint8_t base_info;
  // density of the material, its packing within a block:
  uint8_t density;
  uint8_t packing;
  // temperature info:
  uint8_t specific_heat;
  uint8_t cold_damage_temp;
  uint8_t flashpoint;
  uint8_t freezing_point;
  uint8_t boiling_point;
  // fine structure info:
  uint8_t malleability; // viscosity for liquids
  uint8_t hardness;
  uint8_t brittleness;
  // 4 bits each of impact, compressive, tensile, and shear strength:
  uint16_t strength;
};
*/

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

// Block-based material information:
extern material const MAT_INFO[TOTAL_BLOCK_TYPES];

static material const          MO_UNKNOWN = 0x0; // ???
static material const       MO_ATMOSPHERE = 0x1; // air
static material const            MO_WATER = 0x2; // water; clouds
static material const          MO_MINERAL = 0x3; // stone
static material const         MO_METALLIC = 0x4; // iron
static material const           MO_ERODED = 0x5; // clay; sand; gravel
static material const       MO_DECOMPOSED = 0x6; // dirt; coal
static material const          MO_ORGANIC = 0x7; // hay; flesh; wood
static material const         MO_REACTION = 0x8; // acid
static material const       MO_COMBUSTION = 0x9; // charcoal; smoke
static material const            MO_MAGIC = 0xa; // ectoplasm; ether
static material const            MO_MIXED = 0xb; // plaster; cement
static material const       MO_SUBTRACTED = 0xc; // engraving

static material const           MF_FORMLESS = 0x0; // ???
static material const            MF_GASEOUS = 0x1; // air
static material const             MF_LIQUID = 0x2; // water
static material const            MF_COATING = 0x3; // plaster
static material const        MF_FINE_GRAINS = 0x4; // sand
static material const       MF_ROUGH_GRAINS = 0x5; // gravel
static material const       MF_LARGE_CHUNKS = 0x6; // scree; dry-stone wall
static material const        MF_LOOSE_SOLID = 0x7; // dirt
static material const   MF_COMPRESSED_SOLID = 0x8; // clay; rammed earth
static material const        MF_BOUND_SOLID = 0x9; // rock
static material const      MF_UNIFORM_SOLID = 0xa; // crystal; metal
static material const        MF_SOFT_TISSUE = 0xb; // flesh; leaves
static material const        MF_HARD_TISSUE = 0xc; // wood; shell; bone
static material const      MF_PACKED_FIBERS = 0xd; // wool; paper
static material const        MF_SPUN_FIBERS = 0xe; // yarn; cloth
static material const        MF_CONSTRUCTED = 0xf; // a chair

/***********************
 * Getters and Setters *
 ***********************/

static inline material mt_origin(material m) {
  return (m >> MS_ORIGIN) & MAT_ORI_BITS;
}

static inline material mt_form(material m) {
  return (m >> MS_FORM) & MAT_FORM_BITS;
}

static inline block mt_species(material m) {
  return (m >> MS_SPECIES) & BLOCK_SPC_BITS;
}

/********************
 * Inline Functions *
 ********************/

// Calculates the material associated with the given block:
static inline material mt_from_block(block b) {
  material result = (((material) b_species(b)) << MS_SPECIES);
  result &= ((material) MAT_INFO[b_id(b)]) << MS_FORM;
  return result;
}

// Creates a block of the given block type (as just an ID) with species data
// from the given material:
static inline block mt_make_block(block id, material m) {
  return (id << BS_ID) + (((block) mt_species(m)) << BS_SPC);
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
