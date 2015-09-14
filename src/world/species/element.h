#ifndef ELEMENT_SPECIES_H
#define ELEMENT_SPECIES_H
// Element species types.

#include "world/measures.h"
#include "tex/tex.h"

/*********
 * Enums *
 *********/

enum element_frequency_e {
  EL_FREQ_MAIN,
  EL_FREQ_COMMON,
  EL_FREQ_UNCOMMON
};
typedef enum element_frequency_e element_frequency;

enum phase_e {
  PHASE_SOLID,
  PHASE_LIQUID,
  PHASE_GAS
};
typedef enum phase_e phase;

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

/*************************
 * Structure Definitions *
 *************************/

struct element_species_s {
  element_frequency frequency; // main, common, or uncommon
  phase natural_phase; // phase of the pure element at room temperature
  pH pH; // acidic/basic tendency; neutral = 7.0

  // physical properties of the pure element:
  density density;
  hardness hardness;
  plasticity plasticity;
  uint8_t luster;
  uint8_t corrosion_resistance;

  // Tendencies when forming stone:
  float stone_density_tendency;
  float stone_hardness_tendency;
  float stone_cohesion_tendency;
  solubility stone_solubility;
  float stone_light_dark_tendency;
  pixel stone_chroma;
  pixel stone_oxide_chroma;
  pixel stone_tint_chroma;

  // Biological properties:
  size_t plant_essential_nutrient;
  size_t plant_micronutrient;
  size_t plant_poison;
  size_t animal_poison;
};

#endif // #ifndef ELEMENT_SPECIES_H
