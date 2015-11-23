// ecology.c
// Generation of ecology.

#include "world/world_map.h"
#include "world/species.h"

#include "ecology.h"

/*************
 * Functions *
 *************/

void setup_biome(biome *b) {
  b->all_plants = create_list();
  b->mushrooms = create_list();
  b->giant_mushrooms = create_list();
  b->mosses = create_list();
  b->grasses = create_list();
  b->vines = create_list();
  b->herbs = create_list();
  b->bushes = create_list();
  b->shrubs = create_list();
  b->trees = create_list();
  b->aquatic_grasses = create_list();
  b->aquatic_plants = create_list();
  b->corals = create_list();
  // TODO: Animals as well!
}

void cleanup_biome(biome *b) {
  cleanup_list(b->all_plant_species); b->all_plant_species = NULL;
  cleanup_list(b->mushrooms); b->mushrooms = NULL;
  cleanup_list(b->giant_mushrooms); b->giant_mushrooms = NULL;
  cleanup_list(b->mosses); b->mosses = NULL;
  cleanup_list(b->grasses); b->grasses = NULL;
  cleanup_list(b->vines); b->vines = NULL;
  cleanup_list(b->herbs); b->herbs = NULL;
  cleanup_list(b->bushes); b->bushes = NULL;
  cleanup_list(b->shrubs); b->shrubs = NULL;
  cleanup_list(b->trees); b->trees = NULL;
  cleanup_list(b->aquatic_grasses); b->aquatic_grasses = NULL;
  cleanup_list(b->aquatic_plants); b->aquatic_plants = NULL;
  cleanup_list(b->corals); b->corals = NULL;
}

void generate_ecology(world_map *wm) {
  // TODO: Something here!
}

void init_ocean_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

void init_beach_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

void init_lake_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

void init_river_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

void init_alpine_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

void init_terrestrial_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

