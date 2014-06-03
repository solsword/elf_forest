#ifndef TERRAIN_H
#define TERRAIN_H

// terrain.h
// Legacy terrain generation (see worldgen.h/c).

#include <math.h>

#include "noise/noise.h"
#include "world/blocks.h"
#include "world/world.h"

/**************
 * Parameters *
 **************/

// Top-level terrain parameters:
#define TR_SEA_LEVEL 0
#define TR_BEACH_THRESHOLD 0.15
#define TR_DIRT_MID 6
#define TR_DIRT_VAR 5

// Basic frequencies:
#define TR_FREQUENCY_LOWEST 0.00001 // 6250-chunk features
#define TR_FREQUENCY_LOWER 0.0005 // 125-chunk features
#define TR_FREQUENCY_LOW 0.003 // ~20-chunk features
#define TR_FREQUENCY_MID 0.01 // ~6-chunk features
#define TR_FREQUENCY_HIGH 0.05 // ~20-cell features
#define TR_FREQUENCY_HIGHEST 0.25 // ~4-cell variance

// Geoform parameters:

// Noise->geoform mapping (see compute_geoforms):
#define TR_GEOMAP_DEPTHS (-0.5)
#define TR_GEOMAP_OCEAN 0.0
#define TR_GEOMAP_PLAINS 0.3
#define TR_GEOMAP_HILLS 0.7
#define TR_GEOMAP_MOUNTAINS 0.95

// Heights (lowest):
#define TR_DEPTHS_HEIGHT (-600)
#define TR_OCEANS_HEIGHT (-180)
#define TR_PLAINS_HEIGHT 3
#define TR_HILLS_HEIGHT 150
#define TR_MOUNTAINS_HEIGHT 380

// Landforms (lower):
#define TR_DEPTHS_LANDFORMS 80
#define TR_OCEANS_LANDFORMS 60
#define TR_PLAINS_LANDFORMS 10
#define TR_HILLS_LANDFORMS 140
#define TR_MOUNTAINS_LANDFORMS 250

// Hills (low):
#define TR_DEPTHS_HILLS 30
#define TR_OCEANS_HILLS 40
#define TR_PLAINS_HILLS 6
#define TR_HILLS_HILLS 50
#define TR_MOUNTAINS_HILLS 70

// Ridges (mid):
#define TR_DEPTHS_RIDGES 16
#define TR_OCEANS_RIDGES 12
#define TR_PLAINS_RIDGES 4
#define TR_HILLS_RIDGES 20
#define TR_MOUNTAINS_RIDGES 22

// Roughnesses (low-mid):
#define TR_DEPTHS_ROUGHNESS 0.3
#define TR_OCEANS_ROUGHNESS 0.2
#define TR_PLAINS_ROUGHNESS 0.1
#define TR_HILLS_ROUGHNESS 0.6
#define TR_MOUNTAINS_ROUGHNESS 0.9

// Details:
#define TR_DETAIL_MID 25
#define TR_DETAIL_HIGH 5
#define TR_DETAIL_HIGHEST 3
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

// A salt that gets added to noise input coordinates:
extern float TR_NOISE_OFFSET;

// Amplification factor for terrain height:
extern float TR_TERRAIN_HEIGHT_AMP;

/********************
 * Inline Functions *
 ********************/

static inline void get_noise(
  int x, int y,
  float *lowest, float *lower, float *low,
  float *medium, float *high, float *highest
) {
  x += TR_NOISE_OFFSET;
  y += TR_NOISE_OFFSET;
  *lowest  = sxnoise_2d( x * TR_FREQUENCY_LOWEST ,   y * TR_FREQUENCY_LOWEST  );
  *lower   = sxnoise_2d( x * TR_FREQUENCY_LOWER  ,   y * TR_FREQUENCY_LOWER   );
  *low     = sxnoise_2d( x * TR_FREQUENCY_LOW    ,   y * TR_FREQUENCY_LOW     );
  *medium  = sxnoise_2d( x * TR_FREQUENCY_MID    ,   y * TR_FREQUENCY_MID     );
  *high    = sxnoise_2d( x * TR_FREQUENCY_HIGH   ,   y * TR_FREQUENCY_HIGH    );
  *highest = sxnoise_2d( x * TR_FREQUENCY_HIGHEST,   y * TR_FREQUENCY_HIGHEST );
}

// A cheap (hopefully) sigmoid-like function on [0, 1]:
static inline void stretch(float *value) {
  if (*value > 0.5) {
    *value = 1 - (2 *(1 - *value)*(1 - *value));
  } else {
    *value = (2 *(*value)*(*value));
  }
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
    stretch(depths);
    *oceans = 1.0 - *depths;
  } else if (index < TR_GEOMAP_PLAINS) {
    *oceans = fmax(
      0.0,
        (TR_GEOMAP_PLAINS - index) / (TR_GEOMAP_PLAINS - TR_GEOMAP_OCEAN)
    );
    stretch(oceans);
    *plains = 1.0 - *oceans;
  } else if (index < TR_GEOMAP_HILLS) {
    *plains = fmax(
      0.0,
        (TR_GEOMAP_HILLS - index) / (TR_GEOMAP_HILLS - TR_GEOMAP_PLAINS)
    );
    stretch(plains);
    *hills = 1.0 - *plains;
  } else if (index < TR_GEOMAP_MOUNTAINS) {
    *hills = fmax(
      0.0,
        (TR_GEOMAP_MOUNTAINS - index) / (TR_GEOMAP_MOUNTAINS - TR_GEOMAP_HILLS)
    );
    stretch(mountains);
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
  float nlst, float nlwr, float nlow, float nmid, float nhig, float nhst,
  float depths, float oceans, float plains, float hills, float mountains
) {
  float roughness;
  // Compute roughness:
  roughness = (
    depths * TR_DEPTHS_ROUGHNESS +
    oceans * TR_OCEANS_ROUGHNESS +
    plains * TR_PLAINS_ROUGHNESS +
    hills * TR_HILLS_ROUGHNESS +
    mountains * TR_MOUNTAINS_ROUGHNESS 
  );
  // Vary the roughness a bit:
  roughness = (
    0.3 * oabssq(fmin(1.0, nmid + nlow)) +
    0.2 * oabssq(nlow) * roughness +
    0.5 * roughness
  );
  // Exaggerate the roughness:
  roughness = fmin(1.0, 0.2 + 1.2*roughness);

  // Compute the result:
  return (
    ( // base height - geoform variation
      depths * TR_DEPTHS_HEIGHT +
      oceans * TR_OCEANS_HEIGHT +
      plains * TR_PLAINS_HEIGHT +
      hills * TR_HILLS_HEIGHT +
      mountains * TR_MOUNTAINS_HEIGHT 
    )
  //**/ ); /* DEBUG: enable this comment to view pure base heights.
  +
    ( // lower-frequency variation - landforms
      nlwr *
      (
        depths * TR_DEPTHS_LANDFORMS +
        oceans * TR_OCEANS_LANDFORMS +
        plains * TR_PLAINS_LANDFORMS +
        hills * TR_HILLS_LANDFORMS +
        mountains * TR_MOUNTAINS_LANDFORMS 
      )
    )
  +
    ( // low-frequency variation - hills
      oabssq(nlow) *
      (
        depths * TR_DEPTHS_HILLS +
        oceans * TR_OCEANS_HILLS +
        plains * TR_PLAINS_HILLS +
        hills * TR_HILLS_HILLS +
        mountains * TR_MOUNTAINS_HILLS 
      )
    )
  +
    ( // mid-frequency variation - ridges
      oabs(nmid) * (
        0.5 * oabs(nlow) * TR_DETAIL_MID +
        0.5 * (
          depths * TR_DEPTHS_RIDGES +
          oceans * TR_OCEANS_RIDGES +
          plains * TR_PLAINS_RIDGES +
          hills * TR_HILLS_RIDGES +
          mountains * TR_MOUNTAINS_RIDGES 
        )
      )
    )
  +
    ( // high-frequency variation - chunk-to-chunk variance
      (0.5 * oabs(nhig) + 0.5 * roughness) * TR_DETAIL_HIGH
    )
  +
    ( // highest-frequency variation - bumps
      roughness * oabs(nhst) * TR_DETAIL_HIGHEST
    )
  );
  // */
}

static inline void get_cave_layers(
  float nlst, float nlwr, float nlow, float nmid, float nhig, float nhst,
  float depths, float oceans, float plains, float hills, float mountains,
  int *cave_layer_1_b, int *cave_layer_1_t,
  int *cave_layer_2_b, int *cave_layer_2_t,
  int *cave_layer_3_b, int *cave_layer_3_t
) {
  // TODO: this function!
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

// Creates a cell for the given position:
void terrain_cell(region_pos *pos, cell *result);

// Computes geoform weights at the given location. Note that this isn't used
// internally because the noise values that it generates are reused elsewhere
// in terrain_cell, but it should produce exactly the same geoform values as
// terrain_cell uses.
void get_geoforms(
  int x, int y,
  float *depths, float *oceans, float *plains, float *hills, float *mountains
);

#endif // ifndef TERRAIN_H
