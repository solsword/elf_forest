// species.c
// Manages species info for different block/item/entity types.

#include "datatypes/map.h"

#include "materials.h"

#include "species.h"

/***********
 * Globals *
 ***********/

map *DIRT_SPECIES;
map *CLAY_SPECIES;
map *STONE_SPECIES; // also used for sand, gravel etc.
map *METAL_SPECIES;

map *FUNGUS_SPECIES;
map *MOSS_SPECIES;
map *GRASS_SPECIES;
map *VINE_SPECIES;
map *HERB_SPECIES;
map *BUSH_SPECIES;
map *SHRUB_SPECIES;
map *TREE_SPECIES; // also used for wood
map *AQUATIC_GRASS_SPECIES;
map *AQUATIC_PLANT_SPECIES;
map *CORAL_SPECIES;

map *ANIMAL_SPECIES;
map *MYTHICAL_SPECIES;
map *SENTIENT_SPECIES;

map *FIBER_SPECIES; // various sources; uniform use
