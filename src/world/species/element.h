#ifndef ELEMENT_SPECIES_H
#define ELEMENT_SPECIES_H
// Element species types.

#include "world/measures.h"
#include "world/materials.h"
#include "tex/tex.h"

/*********
 * Enums *
 *********/

enum element_frequency_e {
  EL_FREQ_UBIQUITOUS,
  EL_FREQ_COMMON,
  EL_FREQ_UNCOMMON,
  EL_FREQ_RARE
};
typedef enum element_frequency_e element_frequency;

enum solubility_e {
  SOLUBILITY_INSOLUBLE,
  SOLUBILITY_SLIGHTLY_SOLUBLE,
  SOLUBILITY_SOLUBLE
};
typedef enum solubility_e solubility;

/**************
 * Structures *
 **************/

// Primary species structures:
struct element_species_s;
typedef struct element_species_s element_species;

/*************
 * Constants *
 *************/

// Each element is a member of one or more categories:
#define EL_CATEGORY_AIR    0x01
#define EL_CATEGORY_WATER  0x02
#define EL_CATEGORY_LIFE   0x04
#define EL_CATEGORY_STONE  0x08
#define EL_CATEGORY_METAL  0x10
#define EL_CATEGORY_RARE   0x20

typedef uint8_t element_categorization;

/*************************
 * Structure Definitions *
 *************************/

struct element_species_s {
  element_categorization categories; // Bitmask for categories we're part of
  element_frequency frequency; // ubiquitous, common, uncommon, or rare

  // Note: elements don't have a pure physical form. When isolated (using magic
  // for example) they quickly condense into one form or another, as each
  // element has at least one metalic, lithic, liquid, or gaseous form. For
  // this reason elements themselves don't have material properties.

  pH pH_tendency; // acidic/basic tendency; neutral = 7.0
  solubility solubility; // solubility of derived substances in water
  uint8_t corrosion_resistance; // tendency for metals to resist corrosion

  // Tendencies when forming stone:
  float stone_density_tendency;
  float stone_specific_heat_tendency;
  float stone_transition_temp_tendency;
  float stone_plasticity_tendency;
  float stone_hardness_tendency;
  float stone_cohesion_tendency;
  float stone_light_dark_tendency;
  pixel stone_chroma;
  pixel stone_oxide_chroma;
  pixel stone_tint_chroma;

  // Tendencies when forming metals and alloys:
  float metal_luster_tendency;
  float metal_hardness_tendency;
  float metal_plasticity_tendency;
  float metal_light_dark_tendency;
  pixel metal_chroma;
  pixel metal_tint_chroma;
  float alloy_performance; // -1 to 1: tendency to synergize in alloys

  // Biological properties:
  size_t nutrient;
  size_t poison;
};


/********************
 * Inline Functions *
 ********************/

static inline size_t el_is_member(
  element_species *sp,
  element_categorization category
) {
  return (sp->categories & category) == category;
}

#endif // #ifndef ELEMENT_SPECIES_H
