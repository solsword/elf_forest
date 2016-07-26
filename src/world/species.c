// species.c
// Manages species info for different block/item/entity types.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include "datatypes/map.h"

#include "materials.h"

#include "species.h"

/***********
 * Globals *
 ***********/

map *ELEMENT_SPECIES = NULL;

map *GAS_SPECIES = NULL;

map *DIRT_SPECIES = NULL;
map *CLAY_SPECIES = NULL;
map *STONE_SPECIES = NULL; // also used for sand, gravel etc.
map *METAL_SPECIES = NULL;

map *FUNGUS_SPECIES = NULL;
map *MOSS_SPECIES = NULL;
map *GRASS_SPECIES = NULL;
map *VINE_SPECIES = NULL;
map *HERB_SPECIES = NULL;
map *BUSH_SPECIES = NULL;
map *SHRUB_SPECIES = NULL;
map *TREE_SPECIES = NULL; // also used for wood
map *AQUATIC_GRASS_SPECIES = NULL;
map *AQUATIC_PLANT_SPECIES = NULL;
map *CORAL_SPECIES = NULL;

map *ANIMAL_SPECIES = NULL;
map *MYTHICAL_SPECIES = NULL;
map *SENTIENT_SPECIES = NULL;

map *FIBER_SPECIES = NULL; // various sources; uniform use
map *PIGMENT_SPECIES = NULL; // various sources; uniform use

/*************
 * Functions *
 *************/

// create/get functions for various species types:
SPECIES_ACCESS_FUNCTIONS(element, ELEMENT)

SPECIES_ACCESS_FUNCTIONS(gas, GAS)

SPECIES_ACCESS_FUNCTIONS(dirt, DIRT)
SPECIES_ACCESS_FUNCTIONS(clay, CLAY)
SPECIES_ACCESS_FUNCTIONS(stone, STONE)
SPECIES_ACCESS_FUNCTIONS(metal, METAL)

SPECIES_ACCESS_FUNCTIONS(fungus, FUNGUS)
SPECIES_ACCESS_FUNCTIONS(moss, MOSS)
SPECIES_ACCESS_FUNCTIONS(grass, GRASS)
SPECIES_ACCESS_FUNCTIONS(vine, VINE)
SPECIES_ACCESS_FUNCTIONS(herb, HERB)
SPECIES_ACCESS_FUNCTIONS(bush, BUSH)
SPECIES_ACCESS_FUNCTIONS(shrub, SHRUB)
SPECIES_ACCESS_FUNCTIONS(tree, TREE)
SPECIES_ACCESS_FUNCTIONS(aquatic_grass, AQUATIC_GRASS)
SPECIES_ACCESS_FUNCTIONS(aquatic_plant, AQUATIC_PLANT)
SPECIES_ACCESS_FUNCTIONS(coral, CORAL)

SPECIES_ACCESS_FUNCTIONS(animal, ANIMAL)
SPECIES_ACCESS_FUNCTIONS(mythical, MYTHICAL)
SPECIES_ACCESS_FUNCTIONS(sentient, SENTIENT)

SPECIES_ACCESS_FUNCTIONS(fiber, FIBER)
SPECIES_ACCESS_FUNCTIONS(pigment, PIGMENT)


void setup_species(void) {
  ELEMENT_SPECIES = create_map(1, 1024);

  GAS_SPECIES = create_map(1, 1024);

  DIRT_SPECIES = create_map(1, 1024);
  CLAY_SPECIES = create_map(1, 1024);
  STONE_SPECIES = create_map(1, 1024); // also used for sand, gravel etc.
  METAL_SPECIES = create_map(1, 1024);

  FUNGUS_SPECIES = create_map(1, 1024);
  MOSS_SPECIES = create_map(1, 1024);
  GRASS_SPECIES = create_map(1, 1024);
  VINE_SPECIES = create_map(1, 1024);
  HERB_SPECIES = create_map(1, 1024);
  BUSH_SPECIES = create_map(1, 1024);
  SHRUB_SPECIES = create_map(1, 1024);
  TREE_SPECIES = create_map(1, 1024); // also used for wood
  AQUATIC_GRASS_SPECIES = create_map(1, 1024);
  AQUATIC_PLANT_SPECIES = create_map(1, 1024);
  CORAL_SPECIES = create_map(1, 1024);

  ANIMAL_SPECIES = create_map(1, 1024);
  MYTHICAL_SPECIES = create_map(1, 1024);
  SENTIENT_SPECIES = create_map(1, 1024);

  FIBER_SPECIES = create_map(1, 1024); // various sources; uniform use
  PIGMENT_SPECIES = create_map(1, 1024); // various sources; uniform use
}

void* get_species_data(any_species sp) {
  species id = any_species_species(sp);
  switch(any_species_type(sp)) {
    case SPT_ELEMENT:
      return (void*) get_element_species(id);
    case SPT_GAS:
      return (void*) get_gas_species(id);
    case SPT_DIRT:
      return (void*) get_dirt_species(id);
    case SPT_CLAY:
      return (void*) get_clay_species(id);
    case SPT_STONE:
      return (void*) get_stone_species(id);
    case SPT_METAL:
      return (void*) get_metal_species(id);

    case SPT_FUNGUS:
      return (void*) get_fungus_species(id);
    case SPT_MOSS:
      return (void*) get_moss_species(id);
    case SPT_GRASS:
      return (void*) get_grass_species(id);
    case SPT_VINE:
      return (void*) get_vine_species(id);
    case SPT_HERB:
      return (void*) get_herb_species(id);
    case SPT_BUSH:
      return (void*) get_bush_species(id);
    case SPT_SHRUB:
      return (void*) get_shrub_species(id);
    case SPT_TREE:
      return (void*) get_tree_species(id);
    case SPT_AQUATIC_GRASS:
      return (void*) get_aquatic_grass_species(id);
    case SPT_AQUATIC_PLANT:
      return (void*) get_aquatic_plant_species(id);
    case SPT_CORAL:
      return (void*) get_coral_species(id);

    case SPT_ANIMAL:
      return (void*) get_animal_species(id);
    case SPT_MYTHICAL:
      return (void*) get_mythical_species(id);
    case SPT_SENTIENT:
      return (void*) get_sentient_species(id);

    case SPT_FIBER:
      return (void*) get_fiber_species(id);
    case SPT_PIGMENT:
      return (void*) get_pigment_species(id);

    default:
#ifdef DEBUG
      fprintf(
        stderr,
        "ERROR: Requested species data for unknown species type %d.\n",
        any_species_type(sp)
      );
#endif
      return NULL;
  }
}

/*********************
 * Element Functions *
 *********************/
// see element.h

int el_int_property(
  element_species *sp,
  element_property property
) {
  switch (property) {
    case EL_PRP_I_CATEGORIES:
      return sp->categories;
    case EL_PRP_I_FREQUENCY:
      return sp->frequency;
    case EL_PRP_I_SOLUBILITY:
      return sp->solubility;
    case EL_PRP_I_PlANT_NUTRITION:
      return sp->plant_nutrition;
    case EL_PRP_I_ANIMAL_NUTRITION:
      return sp->animal_nutrition;

    default:
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
      fprintf(
        stderr,
        "ERROR: Attempt to access float property %d via el_int_property.\n",
        property
      );
      exit(EXIT_FAILURE);
  }
}

float el_float_property(
  element_species *sp,
  element_property property
) {
  switch (property) {
    default:
    case EL_PRP_I_CATEGORIES:
    case EL_PRP_I_FREQUENCY:
    case EL_PRP_I_SOLUBILITY:
    case EL_PRP_I_PlANT_NUTRITION:
    case EL_PRP_I_ANIMAL_NUTRITION:
      fprintf(
        stderr,
        "ERROR: Attempt to access integer property %d via el_float_property.\n",
        property
      );
      exit(EXIT_FAILURE);

    case EL_PRP_F_PH_TENDENCY:
      return sp->pH_tendency;
    case EL_PRP_F_CORROSION_RESISTANCE:
      return sp->corrosion_resistance;
    case EL_PRP_F_STONE_TND_DENSITY:
      return sp->stone_density_tendency;
    case EL_PRP_F_STONE_TND_SP_HEAT:
      return sp->stone_specific_heat_tendency;
    case EL_PRP_F_STONE_TND_TR_TEMP:
      return sp->stone_transition_temp_tendency;
    case EL_PRP_F_STONE_TND_PLASTICITY:
      return sp->stone_plasticity_tendency;
    case EL_PRP_F_STONE_TND_HARDNESS:
      return sp->stone_hardness_tendency;
    case EL_PRP_F_STONE_TND_BRIGHTNESS:
      return sp->stone_brightness_tendency;
    case EL_PRP_F_STONE_TND_CHROMA:
      return sp->stone_chroma;
    case EL_PRP_F_STONE_TND_OX_CHROMA:
      return sp->stone_oxide_chroma;
    case EL_PRP_F_STONE_TND_TN_CHROMA:
      return sp->stone_tint_chroma;

    case EL_PRP_F_METAL_TND_LUSTER:
      return sp->metal_luster_tendency;
    case EL_PRP_F_METAL_TND_HARDNESS:
      return sp->metal_hardness_tendency;
    case EL_PRP_F_METAL_TND_PLASTICITY:
      return sp->metal_plasticity_tendency;
    case EL_PRP_F_METAL_TND_BRIGHTNESS:
      return sp->metal_brightness_tendency;
    case EL_PRP_F_METAL_TND_CHROMA:
      return sp->metal_chroma;
    case EL_PRP_F_METAL_TND_OX_CHROMA:
      return sp->metal_oxide_chroma;
    case EL_PRP_F_METAL_TND_TN_CHROMA:
      return sp->metal_tint_chroma;

    case EL_PRP_F_ALLOY_PERFORMANCE:
      return sp->alloy_performance;
  }
}

float el_avg_property(
  list const * const species,
  element_property property
) {
  size_t i;
  element_species *sp;
  float denom = 0;
  float result = 0;

  for (i = 0; i < l_get_length(species); ++i) {
    sp = l_get_item(species, i);
    result += el_float_property(sp, property);
    denom += 1;
  }
  if (denom > 0) {
    result /= denom;
  }
  return result;
}

float el_weighted_property(
  list const * const species,
  list const * const weights,
  element_property property
) {
  size_t i;
  element_species *sp;
  void * v_weight;
  float weight;
  float denom = 0;
  float result = 0;

#ifdef DEBUG
  if (l_get_length(species) != l_get_length(weights)) {
    fprintf(
      stderr,
      "ERROR: species/weight list lengths don't match "
      "in el_weighted_property [%d]!\n",
      property
    );
    fprintf(
      stderr,
      "(species -> %zu elements; weights -> %zu elements)\n",
      l_get_length(species),
      l_get_length(weights)
    );
  }
#endif

  for (i = 0; i < l_get_length(species); ++i) {
    sp = l_get_item(species, i);
    v_weight = l_get_item(weights, i);
    weight = p_as_f(v_weight);
    result += weight * el_float_property(sp, property);
    denom += weight;
  }
  if (denom > 0) {
    result /= denom;
  }
  return result;
}
