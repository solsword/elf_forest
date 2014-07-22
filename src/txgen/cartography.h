#ifndef CARTOGRAPHY_H
#define CARTOGRAPHY_H

// cartography.h
// Map drawing functions.

#include "tex/tex.h"
#include "gen/worldgen.h"

/*************
 * Constants *
 *************/

// The number of sea elevation colors
#define EC_SEA_COLORS 12

// The number of land elevation colors
#define EC_LAND_COLORS 23

// Elevation colors for coloring maps:
extern pixel const SEA_COLORS[EC_SEA_COLORS];
extern pixel const LAND_COLORS[EC_LAND_COLORS];

/********************
 * Inline Functions *
 ********************/

// TODO: Any of these?

/*************
 * Functions *
 *************/

// Returns the terrain height of the given world at the given x/y coordinates
// with [0, 1] mapped to [0, wm->width/height].
r_pos_t world_map_height(world_map *wm, float x, float y);

// Renders a map of the given world into the given texture.
void render_map(world_map *wm, texture *tx);

#endif // ifndef CARTOGRAPHY_H
