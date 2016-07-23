#ifndef TXG_MINERALS_H
#define TXG_MINERALS_H

// txg_minerals.h
// Mineral and soil texture generation.

#include "noise/noise.h"

#include "tex/tex.h"

/**************
 * Structures *
 **************/

struct mineral_filter_args_s;
typedef struct mineral_filter_args_s mineral_filter_args;

/*************************
 * Structure Definitions *
 *************************/

struct mineral_filter_args_s {
  size_t seed; // integer seed
  float scale; // base scale for noise (~0.125)

  float gritty; // per-pixel noise [0, 1]
  float contoured; // perlin noise [0, 1]
  float porous; // rounded veins [0, 1]
  float bumpy; // sharp veins/shadows [0, 1]
  float layered; // horizontal stripes [0, 1]
  float layerscale; // scale of stripes in fractions of a block
  float layerwaves; // waviness of stripes [0, 1]
  float wavescale; // scale of stripe waves waviness in fractions of a block

  float inclusions; // threshold for alternate-color material

  float dscale; // scale of distortion noise (~0.125)
  float distortion; // amount of distortion ([0, 3] pixels)
  float squash;
  // how much to squash the result in x or y (>1 means squash x, <1 means
  // squash y; ~[0.6, 1.7])

  pixel base_color, alt_color; // Base and alternate colors
  float sat_noise; // post-hoc saturation noise for the base color
  float desaturate; // post-hoc saturation attenuation [0, 1]
  float brightness; // how much to brighten/darken the color [-1, 1]
  // (practically, this should be within about [-0.2, 0.5])
};

/********************
 * Filter Functions *
 ********************/

// Generates mineral textures.
void fltr_mineral(texture *tx, void const * const fargs);

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocate and return new mineral_filter args.
mineral_filter_args *create_mineral_filter_args(void);

// Copy mineral filter args, returning a newly-allocated set of args.
mineral_filter_args *copy_mineral_filter_args(
  mineral_filter_args const * const src
);
void *copy_v_mineral_filter_args(void *v_src);

// Frees the memory associated with mineral filter args.
CLEANUP_DECL(mineral_filter_args);

// Copies values from one mineral filter args to another.
void set_mineral_filter_args(
  mineral_filter_args *dest,
  mineral_filter_args const * const src
);

/*************
 * Functions *
 *************/

// Functions for generating per-species textures:
texture* gen_dirt_texture(species s);
texture* gen_mud_texture(species s);
texture* gen_sand_texture(species s);
texture* gen_clay_texture(species s);
texture* gen_stone_texture(species s);

void mutate_mineral_appearance(
  mineral_filter_args const * const src,
  mineral_filter_args *dst,
  ptrdiff_t seed
);

#endif // ifndef TXG_MINERALS_H
