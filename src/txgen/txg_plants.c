// txg_plants.c
// Plant texture generation.

#include "world/blocks.h"
#include "world/world.h"
#include "world/species.h"
#include "noise/noise.h"
#include "tex/tex.h"
#include "tex/draw.h"
#include "math/functions.h"

#include "util.h"

#include "txgen.h"

#include "txg_plants.h"

/*************
 * Constants *
 *************/

size_t const LEAF_TEXTURE_SIZE = 16;

float const STEM_WIDTH_SHARPNESS = 5.0;

float const MAX_BULB_SPREAD = 0.9;

leaf_width_func leaf_width_functions_table[N_LEAF_SHAPES] = {
  &needle_width_func, // needle-shaped; just a line
  &oval_width_func, // circular
  NULL, // diamond-shaped
  NULL, // rounded at the base but tapers to a point
  NULL, // fan-shaped
  NULL, // egg-shaped w/ wide base
  NULL, // egg-shaped w/ narrow base
  NULL, // triangular with lobes at the base
  NULL, // spoon-shaped
  NULL, // pointed at both ends
  &linear_width_func, // parallel margins (useful for stems)
  &deltoid_width_func, // simple triangular
  NULL, // fish-shaped w/ stem as tail
};

/*************
 * Functions *
 *************/

void draw_leaf(texture *tx, curve *c, leaf_filter_args *lfargs) {
  width_func_args wfargs;
  wfargs.stem_length = lfargs->stem_length;
  wfargs.base_width = lfargs->width;
  draw_thick_curve(
    tx,
    c,
    lfargs->main_color,
    lfargs->dark_color,
    (void *) &wfargs,
    leaf_width_functions_table[lfargs->shape]
  );
  draw_curve(tx, c, lfargs->vein_color);
  // TODO: secondary veins
}

texture *gen_mushroom_texture(block b) {
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad mushroom block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_MUSHROOM_SPORES:
      return NULL;
    case B_MUSHROOM_SHOOTS:
      return NULL;
    case B_MUSHROOM_GROWN:
      return NULL;
  }
}

texture *gen_giant_mushroom_texture(block b) {
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad giant mushroom block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_GIANT_MUSHROOM_SPORES:
      return NULL;
    case B_GIANT_MUSHROOM_CORE:
      return NULL;
    case B_GIANT_MUSHROOM_MYCELIUM:
      return NULL;
    case B_GIANT_MUSHROOM_SPROUT:
      return NULL;
    case B_GIANT_MUSHROOM_STALK:
      return NULL;
    case B_GIANT_MUSHROOM_CAP:
      return NULL;
  }
}

texture *gen_moss_texture(block b) {
  moss_species *msp = get_moss_species(b_species(b));
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad moss block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_MOSS_SPORES:
      return gen_herbaceous_seeds_texture(&(msp->appearance));
    case B_MOSS_SHOOTS:
      return gen_herbaceous_shoots_texture(&(msp->appearance));
    case B_MOSS_GROWN:
      return gen_herbaceous_leaves_texture(&(msp->appearance));
    case B_MOSS_FLOWERING:
      return gen_herbaceous_flowering_texture(&(msp->appearance));
    case B_MOSS_FRUITING:
      return gen_herbaceous_fruiting_texture(&(msp->appearance));
  }
}

texture *gen_grass_texture(block b) {
  grass_species *gsp = get_grass_species(b_species(b));
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad grass block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_GRASS_SEEDS:
      return gen_herbaceous_seeds_texture(&(gsp->appearance));
    case B_GRASS_ROOTS:
      return gen_herbaceous_roots_texture(&(gsp->appearance));
    case B_GRASS_SHOOTS:
      return gen_herbaceous_shoots_texture(&(gsp->appearance));
    case B_GRASS_GROWN:
      return gen_herbaceous_leaves_texture(&(gsp->appearance));
    case B_GRASS_BUDDING:
      return gen_herbaceous_budding_texture(&(gsp->appearance));
    case B_GRASS_FLOWERING:
      return gen_herbaceous_flowering_texture(&(gsp->appearance));
    case B_GRASS_FRUITING:
      return gen_herbaceous_fruiting_texture(&(gsp->appearance));
  }
}

texture *gen_vine_texture(block b) {
  vine_species *vsp = get_vine_species(b_species(b));
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad vine block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_VINE_SEEDS:
      return gen_bush_seeds_texture(&(vsp->appearance));
    case B_VINE_CORE:
    case B_VINE_ROOTS:
      return gen_bush_roots_texture(&(vsp->appearance));
    case B_VINE_SHOOTS:
      return gen_bush_shoots_texture(&(vsp->appearance));
    case B_VINE_SPROUTING:
      return gen_bush_sprouting_branches_texture(&(vsp->appearance));
    case B_VINE_GROWN:
      return gen_bush_branches_texture(&(vsp->appearance));
    case B_VINE_BUDDING:
      return gen_bush_budding_branches_texture(&(vsp->appearance));
    case B_VINE_FLOWERING:
      return gen_bush_flowering_branches_texture(&(vsp->appearance));
    case B_VINE_FRUITING:
      return gen_bush_fruiting_branches_texture(&(vsp->appearance));
    case B_VINE_SHEDDING:
      return gen_bush_shedding_branches_texture(&(vsp->appearance));
    case B_VINE_DORMANT:
      return gen_bush_dormant_branches_texture(&(vsp->appearance));
  }
}

texture *gen_herb_texture(block b) {
  herb_species *hsp = get_herb_species(b_species(b));
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad herb block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_HERB_SEEDS:
      return gen_herbaceous_seeds_texture(&(hsp->appearance));
    case B_HERB_CORE:
    case B_HERB_ROOTS:
      return gen_herbaceous_roots_texture(&(hsp->appearance));
    case B_HERB_SHOOTS:
      return gen_herbaceous_shoots_texture(&(hsp->appearance));
    case B_HERB_STEMS:
      return gen_herbaceous_stems_texture(&(hsp->appearance));
    case B_HERB_LEAVES:
      return gen_herbaceous_leaves_texture(&(hsp->appearance));
    case B_HERB_BUDDING:
      return gen_herbaceous_budding_texture(&(hsp->appearance));
    case B_HERB_FLOWERING:
      return gen_herbaceous_flowering_texture(&(hsp->appearance));
    case B_HERB_FRUITING:
      return gen_herbaceous_fruiting_texture(&(hsp->appearance));
  }
}

texture *gen_bush_texture(block b) {
  bush_species *bsp = get_bush_species(b_species(b));
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad bush block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_BUSH_SEEDS:
      return gen_bush_seeds_texture(&(bsp->appearance));
    case B_BUSH_CORE:
    case B_BUSH_ROOTS:
      return gen_bush_roots_texture(&(bsp->appearance));
    case B_BUSH_SHOOTS:
      return gen_bush_shoots_texture(&(bsp->appearance));
    case B_BUSH_BRANCHES_SPROUTING:
      return gen_bush_sprouting_branches_texture(&(bsp->appearance));
    case B_BUSH_BRANCHES_GROWN:
      return gen_bush_branches_texture(&(bsp->appearance));
    case B_BUSH_BRANCHES_BUDDING:
      return gen_bush_budding_branches_texture(&(bsp->appearance));
    case B_BUSH_BRANCHES_FLOWERING:
      return gen_bush_flowering_branches_texture(&(bsp->appearance));
    case B_BUSH_BRANCHES_FRUITING:
      return gen_bush_fruiting_branches_texture(&(bsp->appearance));
    case B_BUSH_BRANCHES_SHEDDING:
      return gen_bush_shedding_branches_texture(&(bsp->appearance));
    case B_BUSH_BRANCHES_DORMANT:
      return gen_bush_dormant_branches_texture(&(bsp->appearance));
    case B_BUSH_LEAVES_SPROUTING:
      return gen_bush_sprouting_leaves_texture(&(bsp->appearance));
    case B_BUSH_LEAVES_GROWN:
      return gen_bush_leaves_texture(&(bsp->appearance));
    case B_BUSH_LEAVES_BUDDING:
      return gen_bush_budding_leaves_texture(&(bsp->appearance));
    case B_BUSH_LEAVES_FLOWERING:
      return gen_bush_flowering_leaves_texture(&(bsp->appearance));
    case B_BUSH_LEAVES_FRUITING:
      return gen_bush_fruiting_leaves_texture(&(bsp->appearance));
    case B_BUSH_LEAVES_SHEDDING:
      return gen_bush_shedding_leaves_texture(&(bsp->appearance));
    case B_BUSH_LEAVES_DORMANT:
      return gen_bush_dormant_leaves_texture(&(bsp->appearance));
  }
}

texture *gen_shrub_texture(block b) {
  shrub_species *ssp = get_shrub_species(b_species(b));
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad shrub block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_SHRUB_SEEDS:
      return gen_bush_seeds_texture(&(ssp->appearance));
    case B_SHRUB_ROOTS:
      return gen_bush_roots_texture(&(ssp->appearance));
    case B_SHRUB_CORE:
    case B_SHRUB_THICK_ROOTS:
      return gen_bush_thick_roots_texture(&(ssp->appearance));
    case B_SHRUB_SHOOTS:
      return gen_bush_shoots_texture(&(ssp->appearance));
    case B_SHRUB_BRANCHES_SPROUTING:
      return gen_bush_sprouting_branches_texture(&(ssp->appearance));
    case B_SHRUB_BRANCHES_GROWN:
      return gen_bush_branches_texture(&(ssp->appearance));
    case B_SHRUB_BRANCHES_BUDDING:
      return gen_bush_budding_branches_texture(&(ssp->appearance));
    case B_SHRUB_BRANCHES_FLOWERING:
      return gen_bush_flowering_branches_texture(&(ssp->appearance));
    case B_SHRUB_BRANCHES_FRUITING:
      return gen_bush_fruiting_branches_texture(&(ssp->appearance));
    case B_SHRUB_BRANCHES_SHEDDING:
      return gen_bush_shedding_branches_texture(&(ssp->appearance));
    case B_SHRUB_BRANCHES_DORMANT:
      return gen_bush_dormant_branches_texture(&(ssp->appearance));
    case B_SHRUB_LEAVES_SPROUTING:
      return gen_bush_sprouting_leaves_texture(&(ssp->appearance));
    case B_SHRUB_LEAVES_GROWN:
      return gen_bush_leaves_texture(&(ssp->appearance));
    case B_SHRUB_LEAVES_BUDDING:
      return gen_bush_budding_leaves_texture(&(ssp->appearance));
    case B_SHRUB_LEAVES_FLOWERING:
      return gen_bush_flowering_leaves_texture(&(ssp->appearance));
    case B_SHRUB_LEAVES_FRUITING:
      return gen_bush_fruiting_leaves_texture(&(ssp->appearance));
    case B_SHRUB_LEAVES_SHEDDING:
      return gen_bush_shedding_leaves_texture(&(ssp->appearance));
    case B_SHRUB_LEAVES_DORMANT:
      return gen_bush_dormant_leaves_texture(&(ssp->appearance));
  }
}

texture *gen_tree_texture(block b) {
  tree_species *tsp = get_tree_species(b_species(b));
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad tree block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_TREE_SEEDS:
      return gen_tree_seeds_texture(&(tsp->appearance));
    case B_TREE_ROOTS:
      return gen_tree_roots_texture(&(tsp->appearance));
    case B_TREE_CORE:
    case B_TREE_THICK_ROOTS:
      return gen_tree_thick_roots_texture(&(tsp->appearance));
    case B_TREE_HEART_CORE:
    case B_TREE_HEART_ROOTS:
      return gen_tree_heart_roots_texture(&(tsp->appearance));
    case B_TREE_SHOOTS:
      return gen_tree_shoots_texture(&(tsp->appearance));
    case B_TREE_TRUNK:
      return gen_tree_trunk_texture(&(tsp->appearance));
    case B_TREE_BRANCHES_SPROUTING:
      return gen_tree_sprouting_branches_texture(&(tsp->appearance));
    case B_TREE_BRANCHES_GROWN:
      return gen_tree_branches_texture(&(tsp->appearance));
    case B_TREE_BRANCHES_BUDDING:
      return gen_tree_budding_branches_texture(&(tsp->appearance));
    case B_TREE_BRANCHES_FLOWERING:
      return gen_tree_flowering_branches_texture(&(tsp->appearance));
    case B_TREE_BRANCHES_FRUITING:
      return gen_tree_fruiting_branches_texture(&(tsp->appearance));
    case B_TREE_BRANCHES_SHEDDING:
      return gen_tree_shedding_branches_texture(&(tsp->appearance));
    case B_TREE_BARE_BRANCHES:
    case B_TREE_BRANCHES_DORMANT:
      return gen_tree_dormant_branches_texture(&(tsp->appearance));
    case B_TREE_LEAVES_SPROUTING:
      return gen_tree_sprouting_leaves_texture(&(tsp->appearance));
    case B_TREE_LEAVES_GROWN:
      return gen_tree_leaves_texture(&(tsp->appearance));
    case B_TREE_LEAVES_BUDDING:
      return gen_tree_budding_leaves_texture(&(tsp->appearance));
    case B_TREE_LEAVES_FLOWERING:
      return gen_tree_flowering_leaves_texture(&(tsp->appearance));
    case B_TREE_LEAVES_FRUITING:
      return gen_tree_fruiting_leaves_texture(&(tsp->appearance));
    case B_TREE_LEAVES_SHEDDING:
      return gen_tree_shedding_leaves_texture(&(tsp->appearance));
    case B_TREE_LEAVES_DORMANT:
      return gen_tree_dormant_leaves_texture(&(tsp->appearance));
  }
}

texture *gen_aquatic_grass_texture(block b) {
  aquatic_grass_species *asp = get_aquatic_grass_species(b_species(b));
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad aquatic grass block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_AQUATIC_GRASS_SEEDS:
      return gen_herbaceous_seeds_texture(&(asp->appearance));
    case B_AQUATIC_GRASS_ROOTS:
      return gen_herbaceous_roots_texture(&(asp->appearance));
    case B_AQUATIC_GRASS_SHOOTS:
      return gen_herbaceous_shoots_texture(&(asp->appearance));
    case B_AQUATIC_GRASS_GROWN:
      return gen_herbaceous_leaves_texture(&(asp->appearance));
    case B_AQUATIC_GRASS_BUDDING:
      return gen_herbaceous_budding_texture(&(asp->appearance));
    case B_AQUATIC_GRASS_FLOWERING:
      return gen_herbaceous_flowering_texture(&(asp->appearance));
    case B_AQUATIC_GRASS_FRUITING:
      return gen_herbaceous_fruiting_texture(&(asp->appearance));
  }
}

texture *gen_aquatic_plant_texture(block b) {
  aquatic_plant_species *asp = get_aquatic_plant_species(b_species(b));
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad aquatic plant block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_AQUATIC_PLANT_SEEDS:
      return gen_herbaceous_seeds_texture(&(asp->appearance));
    case B_AQUATIC_PLANT_CORE:
    case B_AQUATIC_PLANT_ANCHORS:
      return gen_herbaceous_roots_texture(&(asp->appearance));
    case B_AQUATIC_PLANT_SHOOTS:
      return gen_herbaceous_shoots_texture(&(asp->appearance));
    case B_AQUATIC_PLANT_STEMS:
      return gen_herbaceous_stems_texture(&(asp->appearance));
    case B_AQUATIC_PLANT_LEAVES_GROWN:
      return gen_herbaceous_leaves_texture(&(asp->appearance));
    case B_AQUATIC_PLANT_LEAVES_BUDDING:
      return gen_herbaceous_budding_texture(&(asp->appearance));
    case B_AQUATIC_PLANT_LEAVES_FLOWERING:
      return gen_herbaceous_flowering_texture(&(asp->appearance));
    case B_AQUATIC_PLANT_LEAVES_FRUITING:
      return gen_herbaceous_fruiting_texture(&(asp->appearance));
  }
}

texture *gen_coral_texture(block b) {
  coral_species *csp = get_coral_species(b_species(b));
  switch (b_id(b)) {
    default:
#ifdef DEBUG
      printf("Bad aquatic plant block type during txgen: %d\n", b_id(b));
#endif
      return NULL;
    case B_YOUNG_CORAL:
      return gen_young_coral_texture(&(csp->appearance));
    case B_CORAL_CORE:
    case B_CORAL_BODY:
      return gen_coral_body_texture(&(csp->appearance));
    case B_CORAL_FROND:
      return gen_coral_frond_texture(&(csp->appearance));
  }
}

texture* gen_herbaceous_seeds_texture(herbaceous_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_leaves(result, &(appearance->seeds));
  return result;
}

texture* gen_herbaceous_roots_texture(herbaceous_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_branches(result, &(appearance->roots));
  return result;
}

texture* gen_herbaceous_shoots_texture(herbaceous_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_herb_leaves(result, &(appearance->shoots));
  return result;
}

texture* gen_herbaceous_stems_texture(herbaceous_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_herb_stems(result, &(appearance->stems));
  return result;
}

texture* gen_herbaceous_leaves_texture(herbaceous_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_herb_leaves(result, &(appearance->leaves));
  return result;
}

texture* gen_herbaceous_budding_texture(herbaceous_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_herb_leaves(result, &(appearance->leaves));
  fltr_leaves(result, &(appearance->buds));
  return result;
}

texture* gen_herbaceous_flowering_texture(herbaceous_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_herb_leaves(result, &(appearance->leaves));
  fltr_leaves(result, &(appearance->flowers));
  return result;
}

texture* gen_herbaceous_fruiting_texture(herbaceous_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_herb_leaves(result, &(appearance->leaves));
  fltr_leaves(result, &(appearance->fruit));
  return result;
}


texture* gen_bush_seeds_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_leaves(result, &(appearance->seeds));
  return result;
}

texture* gen_bush_roots_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_branches(result, &(appearance->roots));
  return result;
}

texture* gen_bush_thick_roots_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_branches(result, &(appearance->thick_roots));
  return result;
}

texture* gen_bush_shoots_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_herb_leaves(result, &(appearance->shoots));
  return result;
}

texture* gen_bush_sprouting_branches_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_branches(result, &(appearance->thin_branches));
  fltr_leaves(result, &(appearance->sprouting_leaves));
  return result;
}

texture* gen_bush_branches_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_branches(result, &(appearance->thick_branches));
  fltr_leaves(result, &(appearance->leaves));
  return result;
}

texture* gen_bush_budding_branches_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_branches(result, &(appearance->thick_branches));
  fltr_leaves(result, &(appearance->leaves));
  fltr_leaves(result, &(appearance->buds));
  return result;
}

texture* gen_bush_flowering_branches_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_branches(result, &(appearance->thick_branches));
  fltr_leaves(result, &(appearance->leaves));
  fltr_leaves(result, &(appearance->flowers));
  return result;
}

texture* gen_bush_fruiting_branches_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_branches(result, &(appearance->thick_branches));
  fltr_leaves(result, &(appearance->leaves));
  fltr_leaves(result, &(appearance->fruit));
  return result;
}

texture* gen_bush_shedding_branches_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_branches(result, &(appearance->thick_branches));
  fltr_leaves(result, &(appearance->shedding_leaves));
  return result;
}

texture* gen_bush_dormant_branches_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_branches(result, &(appearance->thick_branches));
  return result;
}

texture* gen_bush_sprouting_leaves_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_leaves(result, &(appearance->sprouting_leaves));
  return result;
}

texture* gen_bush_leaves_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_leaves(result, &(appearance->leaves));
  return result;
}

texture* gen_bush_budding_leaves_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_leaves(result, &(appearance->leaves));
  fltr_leaves(result, &(appearance->buds));
  return result;
}

texture* gen_bush_flowering_leaves_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_leaves(result, &(appearance->leaves));
  fltr_leaves(result, &(appearance->flowers));
  return result;
}

texture* gen_bush_fruiting_leaves_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_leaves(result, &(appearance->leaves));
  fltr_leaves(result, &(appearance->fruit));
  return result;
}

texture* gen_bush_shedding_leaves_texture(bush_appearance *appearance) {
  // TODO: Fix compiler segfault on this function when optimizing!!
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_leaves(result, &(appearance->shedding_leaves));
  return result;
}

texture* gen_bush_dormant_leaves_texture(bush_appearance *appearance) {
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_leaves(result, &(appearance->shedding_leaves));
  // TODO: something else here?
  return result;
}


texture* gen_tree_seeds_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_roots_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_thick_roots_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_heart_roots_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_shoots_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_trunk_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_sprouting_branches_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_branches_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_budding_branches_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_flowering_branches_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_fruiting_branches_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_shedding_branches_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_dormant_branches_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_sprouting_leaves_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_leaves_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_budding_leaves_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_flowering_leaves_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_fruiting_leaves_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_shedding_leaves_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_tree_dormant_leaves_texture(tree_appearance *appearance) {
  // TODO: HERE
  return NULL;
}


texture* gen_young_coral_texture(coral_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_coral_body_texture(coral_appearance *appearance) {
  // TODO: HERE
  return NULL;
}

texture* gen_coral_frond_texture(coral_appearance *appearance) {
  // TODO: HERE
  return NULL;
}



/*******************
 * Width Functions *
 *******************/

float stem_width_func(float t, void *args) {
  width_func_args *wfargs = (width_func_args *) args;
  float start = wfargs->stem_length;
  float end = wfargs->stem_length;
  start -= wfargs->stem_length / STEM_WIDTH_SHARPNESS;
  end += wfargs->stem_length / STEM_WIDTH_SHARPNESS;
  return sigmoid(t, start, end);
}

float needle_width_func(float t, void *args) {
  return 0;
}

float oval_width_func(float t, void *args) {
  width_func_args *wfargs = (width_func_args *) args;
  float v = 0;
  float tprime = (t - wfargs->stem_length) / (1 - wfargs->stem_length);
  if (0 < tprime && tprime <= 0.5) {
    v = sqrtf(0.25 - (0.5 - tprime) * (0.5 - tprime));
  } else if (tprime < 1) {
    v = sqrtf(0.25 - (tprime - 0.5) * (tprime - 0.5));
  }
  return stem_width_func(t, args) * wfargs->base_width * v;
}

float linear_width_func(float t, void *args) {
  width_func_args *wfargs = (width_func_args *) args;
  return wfargs->base_width * stem_width_func(t, args);
}

float deltoid_width_func(float t, void *args) {
  width_func_args *wfargs = (width_func_args *) args;
  return (1 - t) * wfargs->base_width * stem_width_func(t, args);
}

/******************
 * Filter Helpers *
 ******************/

// Private helper function for fltr_leaves w/ its own argument structure.
struct leaves_helper_args_s {
  texture *tx;
  texture *leaf;
  leaf_filter_args *lfargs;
};
void fltr_leaves_helper(int x, int y, void * arg) {
  struct leaves_helper_args_s *lhargs = (struct leaves_helper_args_s *) arg;
  // Scramble the leaf filter seed:
  lhargs->lfargs->seed = prng(prng(lhargs->lfargs->seed + x) * y);
  // Render a single randomized leaf into the leaf texture:
  fltr_leaf(lhargs->leaf, lhargs->lfargs);
  // Draw the fresh leaf onto the main texture at the given position:
  tx_draw_wrapped(lhargs->tx, lhargs->leaf, x, y);
}

// Private helper functions for fltr_branches: underlying manifold functions
// for different branch directions.
float branches_direction_up(float x, float y, ptrdiff_t seed) { return -y; }
float branches_direction_down(float x, float y, ptrdiff_t seed) { return y; }
float branches_direction_outwards(float x, float y, ptrdiff_t seed) {
  int half = BLOCK_TEXTURE_SIZE/2;
  float d = (x - half)*(x - half) + (y - half * y - half);
  return -d;
}
float branches_direction_inwards(float x, float y, ptrdiff_t seed) {
  int half = BLOCK_TEXTURE_SIZE/2;
  float d = (x - half)*(x - half) + (y - half * y - half);
  return d;
}
float branches_direction_random(float x, float y, ptrdiff_t seed) {
  float half = ((float) BLOCK_TEXTURE_SIZE)/2.0;
  return sxnoise_2d(x*1/half, y*1/half, seed);
}

// Common random decisions and setup for herb leaves/stems: hash setup,
// nstalks, and spread.
static inline void herb_setup(
  herb_leaves_filter_args *hlfargs,
  ptrdiff_t *hash,
  float *noise,
  int *nstalks,
  float *spread,
  leaf_filter_args *lfargs
) {
  *hash = prng(hlfargs->seed);
  *noise = 0.7 + 0.6 * ptrf(*hash); // [0.7, 1.3]
  *hash = prng(*hash);
  *nstalks = (int) (hlfargs->count * (*noise) + 0.5);
  if (*nstalks < 1) {
    *nstalks = 1;
  }
  *noise = 0.75 + 0.5 * ptrf(*hash); // [0.75, 1.25]
  *hash = prng(*hash);
  *spread = hlfargs->spread * (*noise);
  if (*spread > MAX_BULB_SPREAD) {
    *spread = MAX_BULB_SPREAD;
  }
  *spread *= BLOCK_TEXTURE_SIZE / 2.0;

  lfargs->main_color = hlfargs->main_color;
  lfargs->vein_color = hlfargs->vein_color;
  lfargs->dark_color = hlfargs->dark_color;
  lfargs->bend = hlfargs->bend;
  lfargs->length = hlfargs->length;
  lfargs->width = hlfargs->width;
  lfargs->stem_length = 0;
  // shape is set differently for leaves/stems
}


/********************
 * Filter Functions *
 ********************/

void fltr_branches(texture *tx, void const * const fargs) {
  int row, col;
  float dx, dy, x, y;
  float noise;
  float dontcare;
  ptrdiff_t salt1, salt2, salt3;
  float (*dirfunc)(float, float, ptrdiff_t);
  gradient_map grmap;
  branch_filter_args *bfargs = (branch_filter_args *) fargs;
  salt1 = prng(bfargs->seed+73);
  salt2 = prng(salt1);
  salt3 = prng(salt2);
  if (bfargs->gnarled) {
    grmap.colors[0] = PX_EMPTY;
    grmap.colors[1] = bfargs->outer_color;
    grmap.colors[2] = bfargs->mid_color;
    grmap.colors[3] = bfargs->center_color;
    grmap.colors[4] = bfargs->mid_color;
    grmap.colors[5] = bfargs->outer_color;
    grmap.colors[6] = PX_EMPTY;
#ifdef DEBUG
    // Use orange for out-of-range noise results.
    grmap.colors[7] = 0xff0088ff;
#else
    // Ignore out-of-range results (make them transparent).
    grmap.colors[7] = 0x00000000;
#endif
    grmap.thresholds[0] = 0.08;
    grmap.thresholds[1] = 0.1;
    grmap.thresholds[2] = 0.15;
    grmap.thresholds[3] = 0.22;
    grmap.thresholds[4] = 0.27;
    grmap.thresholds[5] = 0.31;
    grmap.thresholds[6] = 1.0;
    grmap.thresholds[7] = 1000.0;
  } else {
    grmap.colors[0] = bfargs->center_color;
    grmap.colors[1] = bfargs->mid_color;
    grmap.colors[2] = bfargs->outer_color;
    grmap.colors[3] = PX_EMPTY;
#ifdef DEBUG
    // Use orange for out-of-range noise results.
    grmap.colors[4] = 0xff0088ff;
#else
    // Ignore out-of-range results (make them transparent).
    grmap.colors[4] = 0x00000000;
#endif
    grmap.thresholds[0] = 0.07;
    grmap.thresholds[1] = 0.15;
    grmap.thresholds[2] = 0.22;
    grmap.thresholds[3] = 1.0;
    grmap.thresholds[4] = 1000.0;
  }
  switch (bfargs->direction) {
    case 0:
      dirfunc = &branches_direction_up;
      break;
    case 1:
      dirfunc = &branches_direction_down;
      break;
    case 2:
      dirfunc = &branches_direction_outwards;
      break;
    case 3:
      dirfunc = &branches_direction_inwards;
      break;
    default:
    case 4:
      dirfunc = &branches_direction_random;
      break;
  }
  for (col = 0; col < tx->width; col += 1) {
    for (row = 0; row < tx->height; row += 1) {
      // TODO: consider wrapping?
      // TODO: wrap w/ squash taken into account
      dx = tiled_func(
        &sxnoise_2d,
        col * bfargs->dscale,
        row * bfargs->dscale,
        tx->width * bfargs->dscale,
        tx->height * bfargs->dscale,
        salt1
      );
      dy = tiled_func(
        &sxnoise_2d,
        col * bfargs->dscale,
        row * bfargs->dscale,
        tx->width * bfargs->dscale,
        tx->height * bfargs->dscale,
        salt2
      );
      x = (col * bfargs->squash + dx * bfargs->distortion) * bfargs->scale;
      y = (row / bfargs->squash + dy * bfargs->distortion) * bfargs->scale;
      if (bfargs->gnarled) {
        noise = wrnoise_2d_fancy(
          x, y, salt3,
          tx->width * bfargs->scale, tx->height * bfargs->scale,
          &dontcare, &dontcare,
          0
        );
      } else {
        noise = dnnoise_2d(
          x, y,
          salt3,
          dirfunc,
          salt3
        );
      }
      if (noise > 1) { noise = 1; }
      if (bfargs->gnarled) {
        noise = smooth(noise, -bfargs->width, 0.185);
      } else {
        noise = smooth(noise, -bfargs->width, 0.05);
      }
      tx_set_px(
        tx,
        gradient_map_result_sharp(&grmap, noise),
        col,
        row
      );
    }
  }
}

void fltr_leaf(texture *tx, void const * const fargs) {
  leaf_filter_args *lfargs = (leaf_filter_args *) fargs;
  ptrdiff_t hash = prng(lfargs->seed);
  float noise = 0;
  float angle = 0, bend = 0;
  float mid = LEAF_TEXTURE_SIZE / 2.0;
  curve c;
  c.from.x = 0; c.from.y = 0; c.from.z = 0;
  c.go_towards.x = 0; c.go_towards.y = 0; c.go_towards.z = 0;
  c.come_from.x = 0; c.come_from.y = 0; c.come_from.z = 0;
  c.to.x = 0; c.to.y = 0; c.to.z = 0;
  tx_clear(tx);
  if (lfargs->type == LT_SIMPLE) {
    // pick an angle in [-1, 1] * lfargs->angle_var (0 will be straight down)
    noise = -1 + 2.0 * ptrf(prng(hash + lfargs->seed));
    angle = lfargs->angle + lfargs->angle_var * noise;

    // the leaf starts opposite that angle:
    c.from.x = mid + (mid - 1) * sinf(angle + M_PI);
    c.from.y = mid + (mid - 1) * cosf(angle + M_PI);

    // its midpoint is in the direction of the given angle:
    c.go_towards.x = c.from.x + (lfargs->length / 2.0) * sinf(angle);
    c.go_towards.y = c.from.y + (lfargs->length / 2.0) * cosf(angle);
    // make sure we're still inside the texture:
    if (c.go_towards.x < 1) {
      c.go_towards.x = 1;
    } else if (c.go_towards.x > LEAF_TEXTURE_SIZE - 2) {
      c.go_towards.x = LEAF_TEXTURE_SIZE - 2;
    }

    // its endpoint bends:
    noise = 0.7 + 0.6 * ptrf((hash + 2)*noise); // [0.7, 1.3]
    bend = lfargs->bend * noise;
    if ((angle > 0 && angle < M_PI) || (angle < -M_PI) ) {
      bend = angle - bend;
    } else {
      bend = angle + bend;
    }
    c.to.x = c.go_towards.x + (lfargs->length / 2.0) * sinf(bend);
    c.to.y = c.go_towards.y + (lfargs->length / 2.0) * cosf(bend);
    // check bounds again:
    if (c.to.x < 1) {
      c.go_towards.x += (1 - c.to.x) / 2.0;
      c.to.x = 1;
    } else if (c.to.x > LEAF_TEXTURE_SIZE - 2) {
      c.go_towards.x -= (c.to.x - (LEAF_TEXTURE_SIZE - 2)) / 2.0;
      c.to.x = LEAF_TEXTURE_SIZE - 2;
    }

    // copy the go_towards point as the come_from point:
    vcopy_as(&c.come_from, &c.go_towards);

    // draw the leaf:
    draw_leaf(tx, &c, lfargs);
  } else if (lfargs->type == LT_TRIPARTITE) {
    // TODO: Tripartite leaves!
  } else if (lfargs->type == LT_NEEDLES) {
    // TODO: Needles!
  }
#ifdef DEBUG
  else {
    fprintf(stderr, "Bad leaf filter type %d!\n", lfargs->type);
  }
#endif
}

void fltr_leaves(texture *tx, void const * const fargs) {
  leaves_filter_args *lfargs = (leaves_filter_args *) fargs;
  // Manually set up a texture on the stack to generate leaves into:
  pixel pixels[LEAF_TEXTURE_SIZE*LEAF_TEXTURE_SIZE];
  texture one_leaf;
  one_leaf.width = LEAF_TEXTURE_SIZE;
  one_leaf.height = LEAF_TEXTURE_SIZE;
  one_leaf.pixels = pixels;
  tx_clear(&one_leaf);

  // Set up arguments for our helper function:
  struct leaves_helper_args_s lhargs;
  lhargs.tx = tx;
  lhargs.leaf = &one_leaf;
  lhargs.lfargs = &(lfargs->leaf_args);

  ateach_scattered(
    lfargs->seed,
    lfargs->x_spacing, lfargs->y_spacing,
    lfargs->x_spacing/3, lfargs->y_spacing/3,
    0, BLOCK_TEXTURE_SIZE,
    0, BLOCK_TEXTURE_SIZE,
    &lhargs,
    &fltr_leaves_helper
  );
}

void fltr_herb_leaves(texture *tx, void const * const fargs) {
  herb_leaves_filter_args *hlfargs = (herb_leaves_filter_args *) fargs;
  ptrdiff_t hash;
  int i = 0;
  float noise;
  int nstalks;
  float spread;
  leaf_filter_args lfargs; // stores individual leaf parameters
  curve c; // stores individual leaf shapes
  c.from.y = BLOCK_TEXTURE_SIZE;
  c.from.z = 0;
  c.come_from.z = 0;
  c.go_towards.z = 0;
  c.to.z = 0;

  herb_setup(hlfargs, &hash, &noise, &nstalks, &spread, &lfargs);
  lfargs.shape = LS_DELTOID;

  float th_base = 0, th_mid = 0;
  float mid = (BLOCK_TEXTURE_SIZE / 2);

  for (i = 0; i < nstalks; ++i) {
    // generate x in [mid - spread, mid + spread]
    c.from.x = mid - spread;
    c.from.x += 2 * spread * ptrf(hash + i);
    hash = prng(hash + i + c.from.x);
    // pick a starting angle in [-1, 1]
    noise = 1.0 - 2 * ptrf(hash);
    hash = prng(hash + i);
    th_base = hlfargs->angle * noise;
    // pick a bend angle in [0.6, 1.4] of the specified angle
    noise = 0.6 + 0.8 * ptrf(hash);
    hash = prng(hash + i);
    th_mid = hlfargs->bend * noise * th_base / hlfargs->angle;
    th_mid += th_base;
    // compute midpiont and endpoint
    c.go_towards.x = c.from.x + sinf(th_base) * hlfargs->length*hlfargs->shape;
    c.go_towards.y = c.from.y - cosf(th_base) * hlfargs->length*hlfargs->shape;
    c.to.x = c.go_towards.x + sinf(th_mid) * hlfargs->length*(1-hlfargs->shape);
    c.to.y = c.go_towards.y - cosf(th_mid) * hlfargs->length*(1-hlfargs->shape);
    // keep things in-bounds
    /*
    if (c.to.x < 1) {
      c.go_towards.x += (1 - c.to.x) / 2.0;
      c.to.x = 1;
    } else if (c.to.x > BLOCK_TEXTURE_SIZE - 2) {
      c.go_towards.x -= (c.to.x - (BLOCK_TEXTURE_SIZE - 2)) / 2.0;
      c.to.x = BLOCK_TEXTURE_SIZE - 2;
    }
    if (c.to.y < 1) {
      c.go_towards.y += (1 - c.to.y) / 2.0;
      c.to.y = 1;
    }
    // */
    // Copy the come_from vector from the go_towards vector:
    vcopy_as(&c.come_from, &c.go_towards);
    /* DEBUG:
    printf("th_base: %.3f\n", th_base);
    printf("th_mid: %.3f\n", th_mid);
    printf("th_mid_atten: %.3f\n", th_base / hlfargs->angle);
    printf("th_mid_roll: %.3f\n", noise);
    printf("xy_mid: %.2f, %.2f\n", c.go_towards.x, c.go_towards.y);
    printf("xy_end: %.2f, %.2f\n", c.to.x, c.to.y);
    // */
    // pick a base width in [0.75, 1.25]
    noise = 0.75 + 0.5 * ptrf(hash);
    hash = prng(hash + i);
    lfargs.width = hlfargs->width * noise;

    // draw the leaf:
    draw_leaf(tx, &c, &lfargs);
  }
}

void fltr_herb_stems(texture *tx, void const * const fargs) {
  herb_leaves_filter_args *hlfargs = (herb_leaves_filter_args *) fargs;
  ptrdiff_t hash;
  int i = 0;
  float noise;
  int nstalks;
  float spread;
  leaf_filter_args lfargs; // stores individual leaf parameters
  curve c; // stores individual leaf shapes
  c.from.y = BLOCK_TEXTURE_SIZE;
  c.to.y = 0;
  c.from.z = 0;
  c.come_from.z = 0;
  c.go_towards.z = 0;
  c.to.z = 0;

  herb_setup(hlfargs, &hash, &noise, &nstalks, &spread, &lfargs);
  lfargs.shape = LS_LINEAR;

  float th_base = 0;
  float mid = (BLOCK_TEXTURE_SIZE / 2);

  for (i = 0; i < nstalks; ++i) {
    // generate x in [mid - spread, mid + spread]
    // for stems, we use the same top/bottom values so that they'll fit
    // together vertically.
    c.from.x = mid - spread;
    c.from.x += 2 * spread * ptrf(hash + i);
    c.to.x = c.from.x;
    hash = prng(hash + i + c.from.x);
    // pick the same starting angle that leaves use [-1, 1]
    noise = 1.0 - 2 * ptrf(hash);
    hash = prng(hash + i);
    th_base = hlfargs->angle * noise;
    // An extra hash to maintain stride with fltr_herb_leaves:
    hash = prng(hash + i);
    // stems use the same angle at the top and bottom so that they can stack.
    c.come_from.x = c.to.x - sinf(th_base) * hlfargs->length*hlfargs->shape;
    c.come_from.y = c.to.y + cosf(th_base) * hlfargs->length*hlfargs->shape;
    c.go_towards.x = c.from.x + sinf(th_base) * hlfargs->length*hlfargs->shape;
    c.go_towards.y = c.from.y - cosf(th_base) * hlfargs->length*hlfargs->shape;
    // pick a base width in [0.75, 1.25]
    noise = 0.75 + 0.5 * ptrf(hash);
    hash = prng(hash + i);
    lfargs.width = hlfargs->width * noise;

    // draw the leaf:
    draw_leaf(tx, &c, &lfargs);
  }
}
