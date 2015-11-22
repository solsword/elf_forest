#ifndef TXG_PLANTS_H
#define TXG_PLANTS_H

// txg_plants.h
// Plant texture generation.

#include "world/blocks.h"
#include "world/world.h"
#include "math/curve.h"
#include "noise/noise.h"

#include "txg_minerals.h"

/*********
 * Types *
 *********/

typedef float (*leaf_width_func)(float, void*);

/*************
 * Constants *
 *************/

extern size_t const LEAF_TEXTURE_SIZE;

extern float const STEM_WIDTH_SHARPNESS;

extern float const MAX_BULB_SPREAD;

/*********
 * Enums *
 *********/

// Leaf types:
enum leaf_type_e {
  LT_SIMPLE,
  LT_TRIPARTITE,
  LT_NEEDLES
};
typedef enum leaf_type_e leaf_type;

// Leaf types:
enum leaf_shape_e {
  LS_NEEDLE, // needle-shaped; just a line
  LS_OVAL, // oval
  LS_RHOMBOID, // diamond-shaped
  LS_ACUMINATE, // rounded at the base but tapers to a point
  LS_FLABELLATE, // fan-shaped
  LS_OVATE, // egg-shaped w/ wide base
  LS_OBOVATE, // egg-shaped w/ narrow base
  LS_HASTATE, // triangular with lobes at the base
  LS_SPATULATE, // spoon-shaped
  LS_LANCEOLATE, // pointed at both ends
  LS_LINEAR, // parallel margins (useful for stems)
  LS_DELTOID, // triangular
  LS_CUNEATE, // fish-shaped w/ stem as tail
  N_LEAF_SHAPES,
};
typedef enum leaf_shape_e leaf_shape;

/**************
 * Structures *
 **************/

struct seeds_filter_args_s;
typedef struct seeds_filter_args_s seed_filter_args;

struct branch_filter_args_s;
typedef struct branch_filter_args_s branch_filter_args;

struct leaf_filter_args_s;
typedef struct leaf_filter_args_s leaf_filter_args;

struct leaves_filter_args_s;
typedef struct leaves_filter_args_s leaves_filter_args;

struct herb_leaves_filter_args_s;
typedef struct herb_leaves_filter_args_s herb_leaves_filter_args;

struct width_func_args_s;
typedef struct width_func_args_s width_func_args;

// These appearance structures bundle several filter structures that define
// different parts and/or growth stages of an organism:
struct herbaceous_appearance_s;
typedef struct herbaceous_appearance_s herbaceous_appearance;

struct bush_appearance_s;
typedef struct bush_appearance_s bush_appearance;

struct tree_appearance_s;
typedef struct tree_appearance_s tree_appearance;

struct coral_appearance_s;
typedef struct coral_appearance_s coral_appearance;

/*************************
 * Structure Definitions *
 *************************/

struct branch_filter_args_s {
  size_t seed; // random seed
  int gnarled; // whether to generate gnarled- or normal-type branches (0 or 1)
  int direction;
    // directionality of branch pattern:
    //   0 - up, 1 - down, 2 - outwards, 3 - inwards, 4 - random
    // has no effect on gnarled-type branches
  float scale; // scale of the noise (~0.125)
  float width;
    // relative branch width:
    //   0 is default width; higher -> thicker; lower -> thinner
    // [-1, 1] is a reasonable range, but it also depends on the scale
  float dscale; // scale of distortion noise (~0.125)
  float distortion; // amount of distortion ([0, 3] pixels)
  float squash;
    // how much to squash the result in x or y (>1 means squash x, <1 means
    // squash y; ~[0.6, 1.7])
  pixel center_color, mid_color, outer_color; // Colors for the center, middle,
  // and outer parts of the branches.
};

struct leaf_filter_args_s {
  size_t seed; // random seed
  leaf_type type; // which algorithm to use
  leaf_shape shape; // which width function to use
  float angle; // what angle to pose at
  float angle_var; // variance of the angle
  float bend; // how much to bend (approximate in radians)
  float width; // leaf width
  float length; // total length in pixels
  float stem_length; // stem length as a fraction of total length
  pixel main_color, vein_color, dark_color; // Colors for the main leaf
  // surface, the leaf veins (if any) and the shadow details.
};

struct leaves_filter_args_s {
  size_t seed; // random seed
  size_t x_spacing, y_spacing; // leaf spacing
  leaf_filter_args leaf_args; // arguments for the individual leaves
};

struct herb_leaves_filter_args_s {
  size_t seed;
  size_t count; // approximate stalk count
  float spread;
    // [0, 1] how spread out are the bases of the stalks?
    // 0 -> same place; 1.0 -> random over texture width
  float angle; // maximum base angle of the leaves
  float bend; // how far do the stalks bend (upper/lower limit in radians)?
  float shape; // how far along the stalk does it bend [0, 1]
  float length; // how long the stalks are (in pixels)
  float width; // how wide the stalks should be (at their bases)
  pixel main_color, vein_color, dark_color;
    // Colors for the main leaf surface, the leaf veins, and the leaf shadows.
};

struct width_func_args_s {
  float stem_length; // how long the stem should be
  float base_width; // width of the widest point
};

struct herbaceous_appearance_s {
  // TODO: More appearance diversity?
  leaves_filter_args seeds;
  branch_filter_args roots;
  herb_leaves_filter_args shoots;
  herb_leaves_filter_args stems;
  herb_leaves_filter_args leaves;
  leaves_filter_args buds;
  leaves_filter_args flowers;
  leaves_filter_args fruit;
};

struct bush_appearance_s {
  leaves_filter_args seeds;
  branch_filter_args roots;
  branch_filter_args thick_roots;
  herb_leaves_filter_args shoots;
  leaves_filter_args sprouting_leaves;
  leaves_filter_args leaves;
  leaves_filter_args shedding_leaves;
  branch_filter_args thin_branches;
  branch_filter_args thick_branches;
  leaves_filter_args buds;
  leaves_filter_args flowers;
  leaves_filter_args fruit;
};

struct tree_appearance_s {
  leaves_filter_args seeds;
  branch_filter_args roots;
  branch_filter_args thick_roots;
  herb_leaves_filter_args shoots;
  leaves_filter_args leaves;
  leaves_filter_args shedding_leaves;
  branch_filter_args trunk;
  branch_filter_args thin_branches;
  branch_filter_args thick_branches;
  leaves_filter_args buds;
  leaves_filter_args flowers;
  leaves_filter_args fruit;
};

struct coral_appearance_s {
  mineral_filter_args skin;
  leaves_filter_args decorations;
  branch_filter_args fronds;
};

/*************
 * Functions *
 *************/

// Function for drawing a leaf along the given curve using the given leaf
// filter arguments:
void draw_leaf(texture *tx, curve *c, leaf_filter_args *lfargs);

// High-level functions that switch on block types:
texture *gen_mushroom_texture(block b);
texture *gen_giant_mushroom_texture(block b);
texture *gen_moss_texture(block b);
texture *gen_grass_texture(block b);
texture *gen_vine_texture(block b);
texture *gen_herb_texture(block b);
texture *gen_bush_texture(block b);
texture *gen_shrub_texture(block b);
texture *gen_tree_texture(block b);
texture *gen_aquatic_grass_texture(block b);
texture *gen_aquatic_plant_texture(block b);
texture *gen_coral_texture(block b);

// Low-level functions that don't care about block types:
texture* gen_herbaceous_seeds_texture(herbaceous_appearance *appearance);
texture* gen_herbaceous_roots_texture(herbaceous_appearance *appearance);
texture* gen_herbaceous_shoots_texture(herbaceous_appearance *appearance);
texture* gen_herbaceous_stems_texture(herbaceous_appearance *appearance);
texture* gen_herbaceous_leaves_texture(herbaceous_appearance *appearance);
texture* gen_herbaceous_budding_texture(herbaceous_appearance *appearance);
texture* gen_herbaceous_flowering_texture(herbaceous_appearance *appearance);
texture* gen_herbaceous_fruiting_texture(herbaceous_appearance *appearance);

texture* gen_bush_seeds_texture(bush_appearance *appearance);
texture* gen_bush_roots_texture(bush_appearance *appearance);
texture* gen_bush_thick_roots_texture(bush_appearance *appearance);
texture* gen_bush_shoots_texture(bush_appearance *appearance);
texture* gen_bush_sprouting_branches_texture(bush_appearance *appearance);
texture* gen_bush_branches_texture(bush_appearance *appearance);
texture* gen_bush_budding_branches_texture(bush_appearance *appearance);
texture* gen_bush_flowering_branches_texture(bush_appearance *appearance);
texture* gen_bush_fruiting_branches_texture(bush_appearance *appearance);
texture* gen_bush_shedding_branches_texture(bush_appearance *appearance);
texture* gen_bush_dormant_branches_texture(bush_appearance *appearance);
texture* gen_bush_sprouting_leaves_texture(bush_appearance *appearance);
texture* gen_bush_leaves_texture(bush_appearance *appearance);
texture* gen_bush_budding_leaves_texture(bush_appearance *appearance);
texture* gen_bush_flowering_leaves_texture(bush_appearance *appearance);
texture* gen_bush_fruiting_leaves_texture(bush_appearance *appearance);
texture* gen_bush_shedding_leaves_texture(bush_appearance *appearance);
texture* gen_bush_dormant_leaves_texture(bush_appearance *appearance);

texture* gen_tree_seeds_texture(tree_appearance *appearance);
texture* gen_tree_roots_texture(tree_appearance *appearance);
texture* gen_tree_thick_roots_texture(tree_appearance *appearance);
texture* gen_tree_heart_roots_texture(tree_appearance *appearance);
texture* gen_tree_shoots_texture(tree_appearance *appearance);
texture* gen_tree_trunk_texture(tree_appearance *appearance);
texture* gen_tree_sprouting_branches_texture(tree_appearance *appearance);
texture* gen_tree_branches_texture(tree_appearance *appearance);
texture* gen_tree_budding_branches_texture(tree_appearance *appearance);
texture* gen_tree_flowering_branches_texture(tree_appearance *appearance);
texture* gen_tree_fruiting_branches_texture(tree_appearance *appearance);
texture* gen_tree_shedding_branches_texture(tree_appearance *appearance);
texture* gen_tree_dormant_branches_texture(tree_appearance *appearance);
texture* gen_tree_sprouting_leaves_texture(tree_appearance *appearance);
texture* gen_tree_leaves_texture(tree_appearance *appearance);
texture* gen_tree_budding_leaves_texture(tree_appearance *appearance);
texture* gen_tree_flowering_leaves_texture(tree_appearance *appearance);
texture* gen_tree_fruiting_leaves_texture(tree_appearance *appearance);
texture* gen_tree_shedding_leaves_texture(tree_appearance *appearance);
texture* gen_tree_dormant_leaves_texture(tree_appearance *appearance);

texture* gen_young_coral_texture(coral_appearance *appearance);
texture* gen_coral_body_texture(coral_appearance *appearance);
texture* gen_coral_frond_texture(coral_appearance *appearance);

/*******************
 * Width Functions *
 *******************/

// Function that just takes stem width into account.
float stem_width_func(float t, void *args);

// Just draws a line:
float needle_width_func(float t, void *arg);

// An oval leaf after the stem:
float oval_width_func(float t, void *args);

// Includes a stem, sides run parallel (useful for stems):
float linear_width_func(float t, void *arg);

// Includes a stem, but linear after that; widest at the base.
float deltoid_width_func(float t, void *arg);

// The width function table allows looking up a width function from a leaf
extern leaf_width_func leaf_width_functions_table[N_LEAF_SHAPES];

/********************
 * Filter Functions *
 ********************/

// Generates branch-like textures.
void fltr_branches(texture *tx, void const * const fargs);

// Generates a leaf texture (either 8x8 or 16x16 depending on the args).
void fltr_leaf(texture *tx, void const * const fargs);

// Generates a block leaves texture by scattering several leaves over the area.
void fltr_leaves(texture *tx, void const * const fargs);

// Generates a block leaves texture by drawing leaves sprouting from the bottom.
void fltr_herb_leaves(texture *tx, void const * const fargs);

// Generates a block leaves texture by drawing leaves sprouting from the bottom.
void fltr_herb_stems(texture *tx, void const * const fargs);

#endif // ifndef TXG_PLANTS_H
