#ifndef TERRAIN_H
#define TERRAIN_H

// terrain.h
// Terrain generation.

#include "blocks.h"
#include "world.h"

/**************
 * Parameters *
 **************/

// Top-level terrain parameters:
#define TR_SEA_LEVEL -5
#define TR_DIRT_MID 6
#define TR_DIRT_VAR 5

// Basic frequencies:
#define TR_FREQUENCY_LOWEST 0.0009
#define TR_FREQUENCY_LOW 0.003
#define TR_FREQUENCY_MID 0.01
#define TR_FREQUENCY_HIGH 0.05
#define TR_FREQUENCY_HIGHEST 0.25

// Geoform parameters:

// Noise->geoform mapping:
#define TR_GEOMAP_DEPTHS -0.1
#define TR_GEOMAP_OCEAN 0.0
#define TR_GEOMAP_PLAINS 0.3
#define TR_GEOMAP_HILLS 0.8
#define TR_GEOMAP_MOUNTAINS 0.95

// Heights:
#define TR_DEPTHS_HEIGHT -180
#define TR_OCEANS_HEIGHT -45
#define TR_PLAINS_HEIGHT 3
#define TR_HILLS_HEIGHT 15
#define TR_MOUNTAINS_HEIGHT 70

// Variances:
#define TR_DEPTHS_VAR 90
#define TR_OCEANS_VAR 50
#define TR_PLAINS_VAR 7
#define TR_HILLS_VAR 30
#define TR_MOUNTAINS_VAR 100

// Roughnesses:

#define TR_DETAIL_LOW 7
#define TR_DETAIL_MID 4
#define TR_DETAIL_HIGH 3
// TODO: trenches/canyons etc?

/*************
 * Functions *
 *************/

block terrain_block(region_pos pos);

#endif // ifndef TERRAIN_H
