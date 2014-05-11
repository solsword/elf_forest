#ifndef PLANTS_H
#define PLANTS_H

// plants.h
// Plant generation.

#include "world/blocks.h"
#include "world/world.h"
#include "noise/noise.h"

/*************
 * Constants *
 *************/

extern int const SMALL_LEAF_MAX_HEIGHT;
extern int const SMALL_LEAF_MAX_WIDTH;
extern size_t const SMALL_LEAF_DEFAULT_BREADTH;

extern float const MAX_BULB_SPREAD;

/*********
 * Enums *
 *********/

// Leaf types:
typedef enum {
  LT_SIMPLE,
  LT_TRIPARTITE,
  LT_NEEDLES
} leaf_type;

typedef enum {
  LS_SMALL,
  LS_LARGE
} leaf_size;

/******************************
 * Filter Argument Structures *
 ******************************/

struct branch_filter_args_s;
typedef struct branch_filter_args_s branch_filter_args;

struct leaf_filter_args_s;
typedef struct leaf_filter_args_s leaf_filter_args;

struct leaves_filter_args_s;
typedef struct leaves_filter_args_s leaves_filter_args;

struct bulb_leaves_filter_args_s;
typedef struct bulb_leaves_filter_args_s bulb_leaves_filter_args;

/*************************
 * Structure Definitions *
 *************************/

struct branch_filter_args_s {
  size_t seed; // integer seed
  int rough; // whether to generate rough- or smooth-type branches (0 or 1)
  float scale; // scale of the worley noise (~0.125)
  float width; // branch width [0.5, 1.5]
  float dscale; // scale of distortion noise (~0.125)
  float distortion; // amount of distortion ([0, 3] pixels)
  float squash;
  // how much to squash the result in x or y (>1 means squash x, <1 means
  // squash y; ~[0.6, 1.7])
  pixel center_color, mid_color, outer_color; // Colors for the center, middle,
  // and outer parts of the branches.
};

struct leaf_filter_args_s {
  size_t seed; // integer seed
  leaf_type type; // which algorithm to use
  leaf_size size; // LS_SMALL -> 8x8 leaf; LS_LARGE -> 16x16 leaf
  pixel main_color, vein_color, dark_color; // Colors for the main leaf
  // surface, the leaf veins (if any) and the shadow details.
};

struct leaves_filter_args_s {
  size_t seed; // integer seed
  size_t x_spacing, y_spacing; // leaf spacing
  leaf_filter_args leaf_args; // arguments for the individual leaves
};

struct bulb_leaves_filter_args_s {
  size_t seed;
  size_t count; // approximate stalk count
  float spread; // [0, 1] how spread out are the bases of the stalks?
  float angle; // maximum base angle of the leaves
  float bend; // how far do the stalks bend (upper/lower limit in radians)?
  float shape; // how far along the stalk does it bend [0, 1]
  float length; // how long the stalks are (in pixels)
  float width; // how wide the stalks should be (at their bases)
  pixel main_color, vein_color, dark_color; // Colors for the main leaf
  // surface, the leaf veins, and the leaf shadows.
};

/*************
 * Functions *
 *************/

// Function for the width of a bulb leaf as a function of t
float bulb_width_func(float t, void *arg);

// Function for drawing a bulb leaf:
void draw_bulb_leaf(
  texture *tx,
  size_t base_width,
  pixel main_color,
  pixel vein_color,
  pixel shade_color,
  float frx, float fry,
  float twx, float twy,
  float gtx, float gty,
  float tox, float toy
);

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
void fltr_bulb_leaves(texture *tx, void const * const fargs);

#endif // ifndef PLANTS_H
