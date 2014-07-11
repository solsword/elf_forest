#ifndef TXG_MINERALS_H
#define TXG_MINERALS_H

// txg_minerals.h
// Mineral and soil texture generation.

#include "noise/noise.h"

/*************
 * Constants *
 *************/

// TODO: Any of these?
//extern float const MAX_BULB_SPREAD;

/*********
 * Enums *
 *********/

// TODO: These?
// Stone types:
/*
enum stone_type_e {
  ST_SIMPLE,
  ST_TRIPARTITE,
  ST_NEEDLES
};
typedef enum stone_type_e stone_type;
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
  float bumpy; // how bumpy [0, 1]
  float noisy; // how noisy [0, 1]
  float veins; // vein strength [0, 1]
  float dscale; // scale of distortion noise (~0.125)
  float distortion; // amount of distortion ([0, 3] pixels)
  float squash;
  // how much to squash the result in x or y (>1 means squash x, <1 means
  // squash y; ~[0.6, 1.7])
  pixel base_color, alt_color; // Base and alternate colors
};

/*************
 * Functions *
 *************/

// TODO: Any of these?

/********************
 * Filter Functions *
 ********************/

// Generates stone textures.
void fltr_stone(texture *tx, void const * const fargs);

#endif // ifndef TXG_MINERALS_H
