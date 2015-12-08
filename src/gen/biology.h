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

// Cell grammars for sprouting plants:
extern cell_grammar *BIO_CG_SPROUT_IN_SOIL;
extern cell_grammar *BIO_CG_SPROUT_ABOVE_SOIL;
extern cell_grammar *BIO_CG_SPROUT_IN_SOIL_UNDERWATER;
extern cell_grammar *BIO_CG_SPROUT_ABOVE_SOIL_UNDERWATER;

// Spacing for plant distributions: a single plant of the appropriate type is
// generally found within every NxN bin:
#define BIO_DSTR_CLOSE_SPACING 4
#define BIO_DSTR_MEDIUM_SPACING 7
#define BIO_DSTR_WIDE_SPACING 18

// Probabilities for placing two plants instead of one in a distribution bin:
#define BIO_DSTR_CLOSE_DOUBLE_FREQ 0.02
#define BIO_DSTR_MEDIUM_DOUBLE_FREQ 0.05
#define BIO_DSTR_WIDE_DOUBLE_FREQ 0.08

// When a large number of species are possible, low-frequency species are
// especially likely to just never get picked. These parameters define a bit of
// probability smoothing that makes very-rare species a bit more common by
// smoothing probabilities.
#define BIO_RARE_SPECIES_SMOOTHING_EXP 5.0
#define BIO_RARE_SPECIES_SMOOTHING_ADJUST 1.0

/********************
 * Inline Functions *
 ********************/

static inline int any_spacing_hit(
  global_pos glpos,
  ptrdiff_t seed,
  ptrdiff_t spacing,
  float double_freq
) {
  ptrdiff_t cell_x = glpos.x / spacing;
  ptrdiff_t cell_y = glpos.y / spacing;
  ptrdiff_t cell_z = glpos.z / spacing;
  ptrdiff_t row = posmod(prng(cell_x + cell_z + prng(cell_y + 7182)), spacing);
  ptrdiff_t col = posmod(prng(cell_x + row), spacing);
  ptrdiff_t row2 = posmod(prng(cell_y + col), spacing);
  ptrdiff_t col2 = posmod(prng(cell_z + row2), spacing);
  return (
     posmod(glpos.x, spacing) == col
  && posmod(glpos.y, spacing) == row
  ) || (
    ptrf(prng(cell_x + cell_y + cell_z + seed + 12771312)) < double_freq
  && posmod(glpos.x, spacing) == col2
  && posmod(glpos.y, spacing) == row2
  );
}

static inline int wide_spacing_hit(global_pos glpos, ptrdiff_t seed) {
  return any_spacing_hit(
    glpos,
    seed,
    BIO_DSTR_WIDE_SPACING,
    BIO_DSTR_WIDE_DOUBLE_FREQ
  );
}

static inline int medium_spacing_hit(global_pos glpos, ptrdiff_t seed) {
  return any_spacing_hit(
    glpos,
    seed,
    BIO_DSTR_MEDIUM_SPACING,
    BIO_DSTR_MEDIUM_DOUBLE_FREQ
  );
}

static inline int close_spacing_hit(global_pos glpos, ptrdiff_t seed) {
  return any_spacing_hit(
    glpos,
    seed,
    BIO_DSTR_CLOSE_SPACING,
    BIO_DSTR_CLOSE_DOUBLE_FREQ
  );
}

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

// Picks a frequent_species from a list using the frequency of each species as
// well as its compatibility with the given substrate block.
frequent_species pick_appropriate_frequent_species(
  list *sp_list,
  block substrate,
  ptrdiff_t seed
);

// Computes the compatibility between the given species and the given substrate
// block. Values returned are between 0.0 and 1.0.
float species_compatability(frequent_species fqsp, block substrate);

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
