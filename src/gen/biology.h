#ifndef BIOLOGY_H
#define BIOLOGY_H

// biology.h
// General biology generation.

#include "world/world_map.h"
#include "world/species.h"
#include "world/grammar.h"

/*************
 * Constants *
 *************/

extern cell_grammar *BIO_CG_SPROUT_IN_SOIL;
extern cell_grammar *BIO_CG_SPROUT_ABOVE_SOIL;
extern cell_grammar *BIO_CG_SPROUT_IN_SOIL_UNDERWATER;
extern cell_grammar *BIO_CG_SPROUT_ABOVE_SOIL_UNDERWATER;

/*************
 * Functions *
 *************/

// Adds an expansion to one of the four default sprouting grammars for
// sprouting the given seed into the given root and shoots within the given
// substrate (or above it), possibly underwater.
void add_sprout_grammar(
  block seed,
  block root,
  block shoots,
  block substrate,
  int above_soil,
  int underwater
);

// Setup for the biology generation module.
void setup_biology_gen(void);

// Adds biology to the given chunk as part of chunk initialization. Should be
// called after the chunk's base cell contents (rocks, soil, air, water) have
// been added (see generate_chunk in worldgen). If data for the chunk's
// neighbors isn't available in detail, or if the given chunk's CF_HAS_BIOLOGY
// flag is already set, it will fail and return immediately. If it succeeds, it
// will set the chunk's CF_HAS_BIOLOGY flag.
void add_biology(chunk *c);

// Generalized access functions:

// Returns a pointer to a growth_properties struct for the given block, or NULL
// if the given block is not a growing block.
growth_properties* get_growth_properties(block b);

ptrdiff_t get_species_growth_strength(block b, int resist);

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
void determine_new_herb_appearance(
  herbaceous_appearance *target,
  ptrdiff_t seed
);
void determine_new_herb_core_growth(core_growth_pattern *target,ptrdiff_t seed);

#endif // ifndef BIOLOGY_H
