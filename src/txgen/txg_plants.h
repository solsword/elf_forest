#ifndef TXG_PLANTS_H
#define TXG_PLANTS_H

// txg_plants.h
// Plant texture generation.

#include "world/blocks.h"
#include "world/world.h"
#include "math/curve.h"
#include "noise/noise.h"

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
  LS_LINEAR, // parallel margins for much of length
  LS_DELTOID, // triangular
  LS_CUNEATE, // fish-shaped w/ stem as tail
  N_LEAF_SHAPES,
};
typedef enum leaf_shape_e leaf_shape;

/**************
 * Structures *
 **************/

struct branch_filter_args_s;
typedef struct branch_filter_args_s branch_filter_args;

struct leaf_filter_args_s;
typedef struct leaf_filter_args_s leaf_filter_args;

struct leaves_filter_args_s;
typedef struct leaves_filter_args_s leaves_filter_args;

struct bulb_leaves_filter_args_s;
typedef struct bulb_leaves_filter_args_s bulb_leaves_filter_args;

struct width_func_args_s;
typedef struct width_func_args_s width_func_args;

/*************************
 * Structure Definitions *
 *************************/

struct branch_filter_args_s {
  size_t seed; // integer seed
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
  size_t seed; // integer seed
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

struct width_func_args_s {
  float stem_length; // how long the stem should be
  float base_width; // width of the widest point
};

/*************
 * Functions *
 *************/

// Function for drawing a leaf along the given curve using the given leaf
// filter arguments:
void draw_leaf(texture *tx, curve *c, leaf_filter_args *lfargs);

/*******************
 * Width Functions *
 *******************/

// Function that just takes stem width into account.
float stem_width_func(float t, void *args);

// Just draws a line:
float needle_width_func(float t, void *arg);

// An oval leaf after the stem:
float oval_width_func(float t, void *args);

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
void fltr_bulb_leaves(texture *tx, void const * const fargs);

#endif // ifndef TXG_PLANTS_H
