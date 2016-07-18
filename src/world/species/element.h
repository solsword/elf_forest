#ifndef ELEMENT_SPECIES_H
#define ELEMENT_SPECIES_H
// Element species types.

#include "world/measures.h"
#include "world/materials.h"
#include "tex/tex.h"
#include "efd/efd_gl.h"

/*********
 * Enums *
 *********/

enum element_property_e {
  EFD_GL(i, EL_PRP_I_CATEGORIES = 0),
  EFD_GL(i, EL_PRP_I_FREQUENCY = 1),
  EFD_GL(i, EL_PRP_F_PH_TENDENCY = 2),
  EFD_GL(i, EL_PRP_I_SOLUBILITY = 3),
  EFD_GL(i, EL_PRP_F_CORROSION_RESISTANCE = 4),

  EFD_GL(i, EL_PRP_F_STONE_TND_DENSITY = 5),
  EFD_GL(i, EL_PRP_F_STONE_TND_SP_HEAT = 6),
  EFD_GL(i, EL_PRP_F_STONE_TND_TR_TEMP = 7),
  EFD_GL(i, EL_PRP_F_STONE_TND_PLASTICITY = 8),
  EFD_GL(i, EL_PRP_F_STONE_TND_HARDNESS = 9),
  EFD_GL(i, EL_PRP_F_STONE_TND_BRIGHTNESS = 10),
  EFD_GL(i, EL_PRP_F_STONE_TND_CHROMA = 11),
  EFD_GL(i, EL_PRP_F_STONE_TND_OX_CHROMA = 12),
  EFD_GL(i, EL_PRP_F_STONE_TND_TN_CHROMA = 13),

  EFD_GL(i, EL_PRP_F_METAL_TND_LUSTER = 14),
  EFD_GL(i, EL_PRP_F_METAL_TND_HARDNESS = 15),
  EFD_GL(i, EL_PRP_F_METAL_TND_PLASTICITY = 16),
  EFD_GL(i, EL_PRP_F_METAL_TND_BRIGHTNESS = 17),
  EFD_GL(i, EL_PRP_F_METAL_TND_CHROMA = 18),
  EFD_GL(i, EL_PRP_F_METAL_TND_OX_CHROMA = 19),
  EFD_GL(i, EL_PRP_F_METAL_TND_TN_CHROMA = 20),

  EFD_GL(i, EL_PRP_F_ALLOY_PERFORMANCE = 21),

  EFD_GL(i, EL_PRP_I_PlANT_NUTRITION = 22),
  EFD_GL(i, EL_PRP_I_ANIMAL_NUTRITION = 23),
};
typedef enum element_property_e element_property;

enum element_frequency_e {
  EFD_GL(i, EL_FREQ_UBIQUITOUS = 0),
  EFD_GL(i, EL_FREQ_COMMON = 1),
  EFD_GL(i, EL_FREQ_UNCOMMON = 2),
  EFD_GL(i, EL_FREQ_RARE = 3)
};
typedef enum element_frequency_e element_frequency;

enum solubility_e {
  EFD_GL(i, SOLUBILITY_INSOLUBLE = 0),
  EFD_GL(i, SOLUBILITY_SLIGHTLY_SOLUBLE = 1),
  EFD_GL(i, SOLUBILITY_SOLUBLE = 2)
};
typedef enum solubility_e solubility;

enum nutrient_category_e {
  EFD_GL(i, NT_CAT_NONE = 0),
  EFD_GL(i, NT_CAT_CRITICAL = 1),
  EFD_GL(i, NT_CAT_CRITICAL_CAN_OVERDOSE = 2),
  EFD_GL(i, NT_CAT_BENEFICIAL = 3),
  EFD_GL(i, NT_CAT_BENEFICIAL_CAN_OVERDOSE = 4),
  EFD_GL(i, NT_CAT_DETRIMENTAL = 5),
  EFD_GL(i, NT_CAT_POISONOUS = 6)
};
typedef enum nutrient_category_e nutrient_category;

/*********
 * Types *
 *********/

// Each element is a member of one or more categories:
typedef int element_categorization;

static element_categorization const EFD_GL(i, EL_CATEGORY_NONE  = 0x00);
static element_categorization const EFD_GL(i, EL_CATEGORY_AIR   = 0x01);
static element_categorization const EFD_GL(i, EL_CATEGORY_WATER = 0x02);
static element_categorization const EFD_GL(i, EL_CATEGORY_LIFE  = 0x04);
static element_categorization const EFD_GL(i, EL_CATEGORY_STONE = 0x08);
static element_categorization const EFD_GL(i, EL_CATEGORY_METAL = 0x10);
static element_categorization const EFD_GL(i, EL_CATEGORY_RARE  = 0x20);
static element_categorization const EFD_GL(i, EL_CATEGORY_ANY   = 0x3f);

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

#endif // #ifndef ELEMENT_SPECIES_H
