#ifndef TERRAIN_H
#define TERRAIN_H

// terrain.h
// Terrain generation.

#include <math.h>

#include "blocks.h"
#include "noise.h"
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

/***********
 * Globals *
 ***********/

extern float TR_NOISE_OFFSET;

/********************
 * Inline Functions *
 ********************/

static inline void get_noise(
  int x, int y,
  float *lowest, float *low, float *medium, float *high, float *highest
) {
  x += TR_NOISE_OFFSET;
  y += TR_NOISE_OFFSET;
  *lowest  = sxnoise_2d( x * TR_FREQUENCY_LOWEST ,   y * TR_FREQUENCY_LOWEST  );
  *low     = sxnoise_2d( x * TR_FREQUENCY_LOW    ,   y * TR_FREQUENCY_LOW     );
  *medium  = sxnoise_2d( x * TR_FREQUENCY_MID    ,   y * TR_FREQUENCY_MID     );
  *high    = sxnoise_2d( x * TR_FREQUENCY_HIGH   ,   y * TR_FREQUENCY_HIGH    );
  *highest = sxnoise_2d( x * TR_FREQUENCY_HIGHEST,   y * TR_FREQUENCY_HIGHEST );
}

static inline void compute_geoforms(
  float index,
  float *depths, float *oceans, float *plains, float *hills, float *mountains
) {
  if (index < TR_GEOMAP_DEPTHS) {
    *depths = 1.0;
  } else if (index < TR_GEOMAP_OCEAN) {
    *depths = fmax(
      0.0,
      (TR_GEOMAP_OCEAN - index) / (TR_GEOMAP_OCEAN - TR_GEOMAP_DEPTHS)
    );
    *oceans = 1.0 - *depths;
  } else if (index < TR_GEOMAP_PLAINS) {
    *oceans = fmax(
      0.0,
        (TR_GEOMAP_PLAINS - index) / (TR_GEOMAP_PLAINS - TR_GEOMAP_OCEAN)
    );
    *plains = 1.0 - *oceans;
  } else if (index < TR_GEOMAP_HILLS) {
    *plains = fmax(
      0.0,
        (TR_GEOMAP_HILLS - index) / (TR_GEOMAP_HILLS - TR_GEOMAP_PLAINS)
    );
    *hills = 1.0 - *plains;
  } else if (index < TR_GEOMAP_MOUNTAINS) {
    *hills = fmax(
      0.0,
        (TR_GEOMAP_MOUNTAINS - index) / (TR_GEOMAP_MOUNTAINS - TR_GEOMAP_HILLS)
    );
    *mountains = 1.0 - *hills;
  } else {
    *mountains = 1.0;
  }
}

static inline float oabs(float noise) {
  return (0.999 - fabs(noise));
}

static inline float oabssq(float noise) {
  noise = oabs(noise);
  return noise * noise;
}

static inline float oabscb(float noise) {
  noise = oabs(noise);
  return noise * noise * noise;
}

static inline int get_terrain_height(
  float nlst, float nlow, float nmid, float nhig, float nhst,
  float depths, float oceans, float plains, float hills, float mountains
) {
  int result;
  float roughness;
  result = (
    (
      oabssq(nlow)
    *
      (
        depths * TR_DEPTHS_VAR +
        oceans * TR_OCEANS_VAR +
        plains * TR_PLAINS_VAR +
        hills * TR_HILLS_VAR +
        mountains * TR_MOUNTAINS_VAR 
      )
    )
  +
    (
      depths * TR_DEPTHS_HEIGHT +
      oceans * TR_OCEANS_HEIGHT +
      plains * TR_PLAINS_HEIGHT +
      hills * TR_HILLS_HEIGHT +
      mountains * TR_MOUNTAINS_HEIGHT 
    )
  );
  roughness = 0.3 * oabssq(nmid) + 0.7 * (
    depths * TR_DEPTHS_ROUGHNESS +
    oceans * TR_OCEANS_ROUGHNESS +
    plains * TR_PLAINS_ROUGHNESS +
    hills * TR_HILLS_ROUGHNESS +
    mountains * TR_MOUNTAINS_ROUGHNESS 
  );
  result += TR_DETAIL_LOW * oabs(nmid);
  result += (0.5 + 0.5 * roughness) * TR_DETAIL_MID * oabs(nhig);
  result += roughness * TR_DETAIL_HIGH * nhst;
  return result;
}

static inline void get_cave_layers(
  float nlst, float nlow, float nmid, float nhig, float nhst,
  float depths, float oceans, float plains, float hills, float mountains,
  int *cave_layer_1_b, int *cave_layer_1_t,
  int *cave_layer_2_b, int *cave_layer_2_t,
  int *cave_layer_3_b, int *cave_layer_3_t
) {
}

static inline int get_tunnel(
  region_pos *pos,
  float depths, float oceans, float plains, float hills, float mountains
) {
  float xz, yz, xyz;
  xz = sxnoise_2d(
    pos->x * TR_FREQUENCY_TUNNEL_REGIONS,
    pos->z * TR_FREQUENCY_TUNNEL_REGIONS
  );
  xz = (1 + xz) / 2.0;
  xz += sxnoise_2d(
    pos->x * TR_FREQUENCY_TUNNELS,
    pos->z * TR_FREQUENCY_TUNNELS
  );
  xz = (1 + xz) / 3.0;

  yz = sxnoise_2d(
    pos->y * TR_FREQUENCY_TUNNEL_REGIONS + TR_TUNNEL_YZ_OFFSET,
    pos->z * TR_FREQUENCY_TUNNEL_REGIONS
  );
  yz = (1 + yz) / 2.0;
  yz += sxnoise_2d(
    pos->y * TR_FREQUENCY_TUNNELS + TR_TUNNEL_YZ_OFFSET,
    pos->z * TR_FREQUENCY_TUNNELS
  );
  yz = (1 + yz) / 3.0;
  xyz = sxnoise_3d(
    pos->x * TR_FREQUENCY_TUNNEL_DETAILS,
    pos->y * TR_FREQUENCY_TUNNEL_DETAILS,
    pos->z * TR_FREQUENCY_TUNNEL_DETAILS
  );
  xyz = (1 + xyz) / 2.0;
  return (0.6 * (xz * yz) + 0.4 * xyz) > TR_TUNNEL_THRESHOLD;
}

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
