// species.c
// Manages species info for different block/item/entity types.

#include "datatypes/map.h"

#include "materials.h"

#include "species.h"

/***********
 * Globals *
 ***********/

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

/*************
 * Functions *
 *************/

void setup_species(void) {
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
}

stone_species* get_stone_species(species s) {
  stone_species* result = (stone_species*) m_get_value(STONE_SPECIES, s);
  if (result == NULL) {
    // TODO: Something more graceful here?
    fprintf(stderr, "Error: tried to lookup unknown stone species.\n");
    exit(-1);
  }
  return result;
}
