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
#define TR_SEA_LEVEL 0
#define TR_BEACH_THRESHOLD 0.25
#define TR_DIRT_MID 6
#define TR_DIRT_VAR 5

// Basic frequencies:
//#define TR_FREQUENCY_LOWEST 0.0001
// DEBUG:
#define TR_FREQUENCY_LOWEST 0.0005
#define TR_FREQUENCY_LOW 0.003
#define TR_FREQUENCY_MID 0.01
#define TR_FREQUENCY_HIGH 0.05
#define TR_FREQUENCY_HIGHEST 0.25

// Geoform parameters:

// Noise->geoform mapping:
#define TR_GEOMAP_DEPTHS (-0.1)
#define TR_GEOMAP_OCEAN 0.0
#define TR_GEOMAP_PLAINS 0.3
#define TR_GEOMAP_HILLS 0.8
#define TR_GEOMAP_MOUNTAINS 0.95

// Heights:
#define TR_DEPTHS_HEIGHT (-180)
#define TR_OCEANS_HEIGHT (-60)
#define TR_PLAINS_HEIGHT 3
#define TR_HILLS_HEIGHT 40
#define TR_MOUNTAINS_HEIGHT 170

// Variances:
#define TR_DEPTHS_VAR 90
#define TR_OCEANS_VAR 50
#define TR_PLAINS_VAR 7
#define TR_HILLS_VAR 40
#define TR_MOUNTAINS_VAR 120

// Roughnesses:
#define TR_DEPTHS_ROUGHNESS 0.4
#define TR_OCEANS_ROUGHNESS 0.2
#define TR_PLAINS_ROUGHNESS 0.1
#define TR_HILLS_ROUGHNESS 0.5
#define TR_MOUNTAINS_ROUGHNESS 0.7

// Details:
#define TR_DETAIL_LOW 7
#define TR_DETAIL_MID 4
#define TR_DETAIL_HIGH 3
// TODO: trenches/canyons etc?

// Tunnels:
#define TR_FREQUENCY_TUNNEL_REGIONS 0.015
#define TR_FREQUENCY_TUNNELS 0.031
#define TR_FREQUENCY_TUNNEL_DETAILS 0.11
#define TR_TUNNEL_YZ_OFFSET 73
#define TR_TUNNEL_THRESHOLD 0.45
#define TR_TUNNEL_UNDERSEA_OFFSET 8 // TODO: handle undersea tunnels elsehow?

/*************
 * Functions *
 *************/

// Returns the block ID for the block at the given position:
block terrain_block(region_pos pos);

// Computes geoform weights at the given location. Note that this isn't used
// internally because the noise values that it generates are reused elsewhere
// in terrain_block, but it should produce exactly the same geoform values as
// terrain_block uses.
void get_geoforms(
  int x, int y,
  float *depths, float *oceans, float *plains, float *hills, float *mountains
);

#endif // ifndef TERRAIN_H
