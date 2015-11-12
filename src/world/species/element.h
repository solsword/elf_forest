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

enum nutrient_category_e {
  NT_CAT_NONE = 0,
  NT_CAT_CRITICAL = 1,
  NT_CAT_CRITICAL_CAN_OVERDOSE = 2,
  NT_CAT_BENEFICIAL = 3,
  NT_CAT_BENEFICIAL_CAN_OVERDOSE = 4,
  NT_CAT_DETRIMENTAL = 5,
  NT_CAT_POISONOUS = 6
};
typedef enum nutrient_category_e nutrient_category;

/*********
 * Types *
 *********/

// Each element is a member of one or more categories:
typedef uint8_t element_categorization;

#define EL_CATEGORY_AIR    0x01
#define EL_CATEGORY_WATER  0x02
#define EL_CATEGORY_LIFE   0x04
#define EL_CATEGORY_STONE  0x08
#define EL_CATEGORY_METAL  0x10
#define EL_CATEGORY_RARE   0x20

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
  species id;

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
  float stone_chroma;
  float stone_oxide_chroma;
  float stone_tint_chroma;

  // Tendencies when forming metals and alloys:
  float metal_luster_tendency;
  float metal_hardness_tendency;
  float metal_plasticity_tendency;
  float metal_light_dark_tendency;
  float metal_chroma;
  float metal_tint_chroma;
  float alloy_performance; // -1 to 1: tendency to synergize in alloys

  // Biological properties:
  nutrient_category plant_nutrition;
  nutrient_category animal_nutrition;

  // TODO: Magical properties!
};

/**********
 * Macros *
 **********/

// Given an array of species values that are element species, the length of
// that array, an iteration variable (should be a size_t) I, an
// element_species* variable EL, a DENOM float, a property name (such as
// 'stone_density_tendency'), and a RESULT float, this generates code to look
// up and average the given property across all elements in the given species
// array. The variables I, EL, SUM, DENOM, and RESULT will be overwritten as
// part of this process, with the result ending up in RESULT. Only works for
// floating point properties. If there are no non-zero species in the array,
// both RESULT and DENOM will be 0.
#define AVERAGE_ELEMENT_PROPERTY(SPECIES, LEN, I, EL, DENOM, PROP, RESULT) \
  DENOM = 0; \
  RESULT = 0; \
  for (I = 0; I < LEN; ++I) { \
    EL = get_element_species(SPECIES[I]); \
    if (EL != NULL) { \
      RESULT += (EL)->PROP; \
      DENOM += 1; \
    } \
  } \
  if (DENOM > 0) { \
    RESULT /= DENOM; \
  }

// As above, but also takes a weights array (same length as the species array)
// of floats and averages according to those weights. None of the weights
// should be negative.
#define WEIGHTED_ELEMENT_PROPERTY(SPC, WGTS, LEN, I, EL, DENOM, PROP, RESULT) \
  DENOM = 0; \
  RESULT = 0; \
  for (I = 0; I < LEN; ++I) { \
    EL = get_element_species(SPC[I]); \
    if (EL != NULL) { \
      RESULT += (WGTS[I]) * ((EL)->PROP); \
      DENOM += WGTS[I]; \
    } \
  } \
  if (DENOM > 0) { \
    RESULT /= DENOM; \
  }

/********************
 * Inline Functions *
 ********************/

static inline size_t el_is_member(
  element_species *sp,
  element_categorization category
) {
  return (sp->categories & category) == category;
}

static inline size_t el_is_member_of_any(
  element_species *sp,
  element_categorization categories
) {
  return (sp->categories & categories) != 0;
}

#endif // #ifndef ELEMENT_SPECIES_H
