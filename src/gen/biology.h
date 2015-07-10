#ifndef BIOLOGY_H
#define BIOLOGY_H

// biology.h
// General biology generation.

#include "world/world_map.h"
#include "world/species.h"

/*************
 * Constants *
 *************/

extern cell_grammar BIO_CG_SPROUT_ABOVE_SOIL;
extern cell_grammar BIO_CG_SPROUT_IN_SOIL;

/*************
 * Functions *
 *************/

// Adds biology to the given chunk as part of chunk initialization. Should be
// called after the chunk's base cell contents (rocks, soil, air, water) have
// been added (see generate_chunk in worldgen). If data for the chunk's
// neighbors isn't available in detail, or if the given chunk's CF_HAS_BIOLOGY
// flag is already set, it will fail and return immediately. If it succeeds, it
// will set the chunk's CF_HAS_BIOLOGY flag.
void add_biology(chunk *c);

// Species generation functions:

species create_new_fungus_species(ptrdiff_t seed);
species create_new_moss_species(ptrdiff_t seed);
species create_new_grass_species(ptrdiff_t seed);
species create_new_vine_species(ptrdiff_t seed);
species create_new_herb_species(ptrdiff_t seed);
species create_new_bush_species(ptrdiff_t seed);
species create_new_shrub_species(ptrdiff_t seed);
species create_new_tree_species(ptrdiff_t seed);
species create_new_aquatic_grass_species(ptrdiff_t seed);
species create_new_aquatic_plant_species(ptrdiff_t seed);
species create_new_coral_species(ptrdiff_t seed);

// Individual attribute generation functions:

void determine_new_plant_materials(plant_materials *target, ptrdiff_t seed);
void determine_new_herb_appearance(herb_appearance *target, seed);
void determine_new_herb_core_growth(core_growth *target, seed);

#endif // ifndef BIOLOGY_H
