#ifndef CARTOGRAPHY_H
#define CARTOGRAPHY_H

// cartography.h
// Map drawing functions.

#include "tex/tex.h"
#include "txgen/txgen.h"
#include "gen/worldgen.h"

/*************
 * Constants *
 *************/

#define CART_VECTOR_SPACING 8
#define CART_MAX_VECTOR_LENGTH 6.0

// Gradients for coloring maps:
extern gradient const BW_GRADIENT;
extern gradient const SEA_GRADIENT;
extern gradient const LAND_GRADIENT;
extern gradient const RAIN_GRADIENT;
extern gradient const CLOUDS_GRADIENT;
extern gradient const TEMPERATURE_GRADIENT;

extern gradient_map const GEOREGIONS_GRADIENT;

/********************
 * Inline Functions *
 ********************/

// TODO: Any of these?

/*************
 * Functions *
 *************/

// Renders a map of the given layer of the given world into the given texture.
void render_map_layer(
  world_map *wm,
  texture *tx,
  pixel (*layer_fn)(world_region*)
);

// Like render_map_layer but renders spaced-out vectors instead of a color for
// every pixel.
void render_map_vectors(
  world_map *wm,
  texture *tx,
  pixel start, pixel end,
  void (*vector_layer_fn)(world_region*, float*, float*)
);

/*******************
 * Layer Functions *
 *******************/

// Layer functions return a color for each world region. render_map_layer can
// be used to draw an entire world map using colors from a particular layer.
pixel ly_terrain_height(world_region *wr);
pixel ly_georegions(world_region *wr);
pixel ly_temperature(world_region *wr);
pixel ly_evaporation(world_region *wr);
pixel ly_cloud_cover(world_region *wr);
pixel ly_precipitation_quotient(world_region *wr);
pixel ly_precipitation(world_region *wr);
pixel ly_land_precipitation(world_region *wr);

// Draws wind vectors
void vly_wind_vectors(world_region *wr, float *r, float *theta);

#endif // ifndef CARTOGRAPHY_H
