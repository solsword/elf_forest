#ifndef TXG_MINERALS_H
#define TXG_MINERALS_H

// txg_minerals.h
// Mineral and soil texture generation.

#include "noise/noise.h"

#include "tex/tex.h"

/*************
 * Constants *
 *************/

// TODO: Any of these?
//extern float const MAX_BULB_SPREAD;

/*********
 * Enums *
 *********/

// TODO: Any of these?
// Stone types:
/*
enum stone_color_e {
  SC_DARK,
  ST_NORMAL,
  ST_BRIGHT
};
typedef enum stone_color_e stone_color;
*/

/**************
 * Structures *
 **************/

struct stone_filter_args_s;
typedef struct stone_filter_args_s stone_filter_args;

/*************************
 * Structure Definitions *
 *************************/

struct stone_filter_args_s {
  size_t seed; // integer seed
  float scale; // base scale for noise (~0.125)

  float gritty; // per-pixel noise [0, 1]
  float contoured; // perlin noise [0, 1]
  float porous; // rounded veins [0, 1]
  float bumpy; // sharp veins/shadows [0, 1]

  float inclusions; // threshold for alternate-color material

  float dscale; // scale of distortion noise (~0.125)
  float distortion; // amount of distortion ([0, 3] pixels)
  float squash;
  // how much to squash the result in x or y (>1 means squash x, <1 means
  // squash y; ~[0.6, 1.7])

  pixel base_color, alt_color; // Base and alternate colors
  float brightness; // how much to brighten/darken the color [-1, 1]
  // (practically, this should be within about [-0.2, 0.5])
};

/********************
 * Filter Functions *
 ********************/

// Generates stone textures.
void fltr_stone(texture *tx, void const * const fargs);

/*************
 * Functions *
 *************/

texture* gen_stone_texture(species s);

#endif // ifndef TXG_MINERALS_H
