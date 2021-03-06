#ifndef ELEMENT_SPECIES_H
#define ELEMENT_SPECIES_H
// Element species types.

#include "world/measures.h"
#include "world/materials.h"
#include "tex/tex.h"
#include "snek/feed.h"

/*********
 * Enums *
 *********/

enum element_property_e {
  EL_PRP_I_CATEGORIES,
  EL_PRP_I_FREQUENCY,
  EL_PRP_F_PH_TENDENCY,
  EL_PRP_I_SOLUBILITY,
  EL_PRP_F_CORROSION_RESISTANCE,

  EL_PRP_F_STONE_TND_DENSITY,
  EL_PRP_F_STONE_TND_SP_HEAT,
  EL_PRP_F_STONE_TND_TR_TEMP,
  EL_PRP_F_STONE_TND_PLASTICITY,
  EL_PRP_F_STONE_TND_HARDNESS,
  EL_PRP_F_STONE_TND_BRIGHTNESS,
  EL_PRP_F_STONE_TND_CHROMA,
  EL_PRP_F_STONE_TND_OX_CHROMA,
  EL_PRP_F_STONE_TND_TN_CHROMA,

  EL_PRP_F_METAL_TND_LUSTER,
  EL_PRP_F_METAL_TND_HARDNESS,
  EL_PRP_F_METAL_TND_PLASTICITY,
  EL_PRP_F_METAL_TND_BRIGHTNESS,
  EL_PRP_F_METAL_TND_CHROMA,
  EL_PRP_F_METAL_TND_OX_CHROMA,
  EL_PRP_F_METAL_TND_TN_CHROMA,

  EL_PRP_F_ALLOY_PERFORMANCE,

  EL_PRP_I_PlANT_NUTRITION,
  EL_PRP_I_ANIMAL_NUTRITION,
};
typedef enum element_property_e element_property;

FEED_SNEK(i, EL_PRP_I_CATEGORIES)
FEED_SNEK(i, EL_PRP_I_FREQUENCY)
FEED_SNEK(i, EL_PRP_F_PH_TENDENCY)
FEED_SNEK(i, EL_PRP_I_SOLUBILITY)
FEED_SNEK(i, EL_PRP_F_CORROSION_RESISTANCE)

FEED_SNEK(i, EL_PRP_F_STONE_TND_DENSITY)
FEED_SNEK(i, EL_PRP_F_STONE_TND_SP_HEAT)
FEED_SNEK(i, EL_PRP_F_STONE_TND_TR_TEMP)
FEED_SNEK(i, EL_PRP_F_STONE_TND_PLASTICITY)
FEED_SNEK(i, EL_PRP_F_STONE_TND_HARDNESS)
FEED_SNEK(i, EL_PRP_F_STONE_TND_BRIGHTNESS)
FEED_SNEK(i, EL_PRP_F_STONE_TND_CHROMA)
FEED_SNEK(i, EL_PRP_F_STONE_TND_OX_CHROMA)
FEED_SNEK(i, EL_PRP_F_STONE_TND_TN_CHROMA)

FEED_SNEK(i, EL_PRP_F_METAL_TND_LUSTER)
FEED_SNEK(i, EL_PRP_F_METAL_TND_HARDNESS)
FEED_SNEK(i, EL_PRP_F_METAL_TND_PLASTICITY)
FEED_SNEK(i, EL_PRP_F_METAL_TND_BRIGHTNESS)
FEED_SNEK(i, EL_PRP_F_METAL_TND_CHROMA)
FEED_SNEK(i, EL_PRP_F_METAL_TND_OX_CHROMA)
FEED_SNEK(i, EL_PRP_F_METAL_TND_TN_CHROMA)

FEED_SNEK(i, EL_PRP_F_ALLOY_PERFORMANCE)

FEED_SNEK(i, EL_PRP_I_PlANT_NUTRITION)
FEED_SNEK(i, EL_PRP_I_ANIMAL_NUTRITION)

enum element_frequency_e {
  EL_FREQ_UBIQUITOUS,
  EL_FREQ_COMMON,
  EL_FREQ_UNCOMMON,
  EL_FREQ_RARE
};
typedef enum element_frequency_e element_frequency;

FEED_SNEK(i, EL_FREQ_UBIQUITOUS)
FEED_SNEK(i, EL_FREQ_COMMON)
FEED_SNEK(i, EL_FREQ_UNCOMMON)
FEED_SNEK(i, EL_FREQ_RARE)

enum solubility_e {
  SOLUBILITY_INSOLUBLE,
  SOLUBILITY_SLIGHTLY_SOLUBLE,
  SOLUBILITY_SOLUBLE
};
typedef enum solubility_e solubility;

FEED_SNEK(i, SOLUBILITY_INSOLUBLE)
FEED_SNEK(i, SOLUBILITY_SLIGHTLY_SOLUBLE)
FEED_SNEK(i, SOLUBILITY_SOLUBLE)

enum nutrient_category_e {
  NT_CAT_NONE,
  NT_CAT_CRITICAL,
  NT_CAT_CRITICAL_CAN_OVERDOSE,
  NT_CAT_BENEFICIAL,
  NT_CAT_BENEFICIAL_CAN_OVERDOSE,
  NT_CAT_DETRIMENTAL,
  NT_CAT_POISONOUS
};
typedef enum nutrient_category_e nutrient_category;

FEED_SNEK(i, NT_CAT_NONE)
FEED_SNEK(i, NT_CAT_CRITICAL)
FEED_SNEK(i, NT_CAT_CRITICAL_CAN_OVERDOSE)
FEED_SNEK(i, NT_CAT_BENEFICIAL)
FEED_SNEK(i, NT_CAT_BENEFICIAL_CAN_OVERDOSE)
FEED_SNEK(i, NT_CAT_DETRIMENTAL)
FEED_SNEK(i, NT_CAT_POISONOUS)

/*********
 * Types *
 *********/

// Each element is a member of one or more categories:
typedef int element_categorization;

static element_categorization const EL_CATEGORY_NONE  = 0x00;
static element_categorization const EL_CATEGORY_AIR   = 0x01;
static element_categorization const EL_CATEGORY_WATER = 0x02;
static element_categorization const EL_CATEGORY_LIFE  = 0x04;
static element_categorization const EL_CATEGORY_STONE = 0x08;
static element_categorization const EL_CATEGORY_METAL = 0x10;
static element_categorization const EL_CATEGORY_RARE  = 0x20;
static element_categorization const EL_CATEGORY_ANY   = 0x3f;

FEED_SNEK(i, EL_CATEGORY_NONE)
FEED_SNEK(i, EL_CATEGORY_AIR)
FEED_SNEK(i, EL_CATEGORY_WATER)
FEED_SNEK(i, EL_CATEGORY_LIFE)
FEED_SNEK(i, EL_CATEGORY_STONE)
FEED_SNEK(i, EL_CATEGORY_METAL)
FEED_SNEK(i, EL_CATEGORY_RARE)
FEED_SNEK(i, EL_CATEGORY_ANY)

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
  float corrosion_resistance; // tendency for metals to resist corrosion

  // Tendencies when forming stone:
  float stone_density_tendency;
  float stone_specific_heat_tendency;
  float stone_transition_temp_tendency;
  float stone_plasticity_tendency;
  float stone_hardness_tendency;
  float stone_brightness_tendency;
  float stone_chroma;
  float stone_oxide_chroma;
  float stone_tint_chroma;

  // Tendencies when forming metals and alloys:
  float metal_luster_tendency;
  float metal_hardness_tendency;
  float metal_plasticity_tendency;
  float metal_brightness_tendency;
  float metal_chroma;
  float metal_oxide_chroma;
  float metal_tint_chroma;
  float alloy_performance; // -1 to 1: tendency to synergize in alloys

  // Biological properties:
  nutrient_category plant_nutrition;
  nutrient_category animal_nutrition;

  // TODO: Magical properties!
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

static inline size_t el_is_member_of_any(
  element_species *sp,
  element_categorization categories
) {
  return (sp->categories & categories) != 0;
}

static inline int el_prp_is_integer(element_property prp) {
  switch (prp) {
    default: // TODO: Return something else here?
    case EL_PRP_I_CATEGORIES:
    case EL_PRP_I_FREQUENCY:
    case EL_PRP_I_SOLUBILITY:
    case EL_PRP_I_PlANT_NUTRITION:
    case EL_PRP_I_ANIMAL_NUTRITION:
      return 1;

    case EL_PRP_F_PH_TENDENCY:
    case EL_PRP_F_CORROSION_RESISTANCE:
    case EL_PRP_F_STONE_TND_DENSITY:
    case EL_PRP_F_STONE_TND_SP_HEAT:
    case EL_PRP_F_STONE_TND_TR_TEMP:
    case EL_PRP_F_STONE_TND_PLASTICITY:
    case EL_PRP_F_STONE_TND_HARDNESS:
    case EL_PRP_F_STONE_TND_BRIGHTNESS:
    case EL_PRP_F_STONE_TND_CHROMA:
    case EL_PRP_F_STONE_TND_OX_CHROMA:
    case EL_PRP_F_STONE_TND_TN_CHROMA:
    case EL_PRP_F_METAL_TND_LUSTER:
    case EL_PRP_F_METAL_TND_HARDNESS:
    case EL_PRP_F_METAL_TND_PLASTICITY:
    case EL_PRP_F_METAL_TND_BRIGHTNESS:
    case EL_PRP_F_METAL_TND_CHROMA:
    case EL_PRP_F_METAL_TND_OX_CHROMA:
    case EL_PRP_F_METAL_TND_TN_CHROMA:
    case EL_PRP_F_ALLOY_PERFORMANCE:
      return 0;
  }
}

#endif // #ifndef ELEMENT_SPECIES_H
