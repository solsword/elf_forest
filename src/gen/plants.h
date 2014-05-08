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

extern float const CURVE_DRAWING_RESOLUTION;

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

// Arguments to the branch texture filter:
// roughness flag, scale, width, distortion scale, distortion, squash, and
// center, mid, and outer colors.
struct branch_filter_args_s;
typedef struct branch_filter_args_s branch_filter_args;

// Arguments to the leaf texture filters:
// type, 
struct leaf_filter_args_s;
typedef struct leaf_filter_args_s leaf_filter_args;

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
  leaf_filter_args leaf_args;
  int spacing; // leaf spacing
};

/****************************
 * Example filter arguments *
 ****************************/

extern branch_filter_args const example_branch_args;

extern leaf_filter_args const example_leaf_args;

extern leaves_filter_args const example_leaves_args;

/********************
 * Inline Functions *
 ********************/

/*************
 * Functions *
 *************/

void draw_bulb_leaf(
  texture *tx,
  float frx, float fry,
  float twx, float twy,
  float gtx, float gty,
  float tox, float toy
);

/********************
 * Filter Functions *
 ********************/

// Generates branch-like textures.
void fltr_branches(texture *tx, void *fargs);

// Generates a leaf texture (either 8x8 or 16x16 depending on the args).
void fltr_leaf(texture *tx, void *fargs);

// Generates a 32x32 leaves texture by scattering several leaves over the area.
void fltr_leaves(texture *tx, void *fargs);

#endif // ifndef PLANTS_H
