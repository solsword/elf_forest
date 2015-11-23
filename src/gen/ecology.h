#ifndef ECOLOGY_H
#define ECOLOGY_H

// ecology.h
// Generation of ecology.

#include "world/world_map.h"
#include "world/species.h"

/**************
 * Structures *
 **************/

struct biome_factors_s;
typedef struct biome_factors_s biome_factors;

/*************************
 * Structure Definitions *
 *************************/

struct biome_factors_s {
  // TODO: HERE
};

/*************
 * Functions *
 *************/

// Initializes the given biome.
void setup_biome(biome *b);

// Cleans up the given biome's allocated memory (but doesn't free the biome).
void cleanup_biome(biome *b);

// Generates ecological information for the given world map.
void generate_ecology(world_map *wm);

// Functions to initialize single biomes of various types appropriate to a
// given world region.
void init_ocean_biome(world_region *wr, biome *b, ptrdiff_t seed);
void init_beach_biome(world_region *wr, biome *b, ptrdiff_t seed);
void init_lake_biome(world_region *wr, biome *b, ptrdiff_t seed);
void init_river_biome(world_region *wr, biome *b, ptrdiff_t seed);
void init_alpine_biome(world_region *wr, biome *b, ptrdiff_t seed);
void init_terrestrial_biome(world_region *wr, biome *b, ptrdiff_t seed);

// Fills out any kind of biome (just delegates to one of functions below).
void fill_any_biome(biome *b, biome_factors *factors);

void fill_sea_ice_biome(biome *b, biome_factors *factors)
  // TODO: etc.

#endif // ifndef ECOLOGY_H
