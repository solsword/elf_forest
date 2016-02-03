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
