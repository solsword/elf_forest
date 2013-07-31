// terrain.c
// Terrain generation.

#include <math.h>

#include "blocks.h"
#include "world.h"
#include "noise.h"
#include "terrain.h"

/********************
 * Inline Functions *
 ********************/

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

static inline float abssq(float noise) {
  noise = (0.999 - fabs(noise));
  return noise * noise;
}

static inline float abscb(float noise) {
  noise = (0.999 - fabs(noise));
  return noise * noise * noise;
}


/*************
 * Functions *
 *************/

block terrain_block(region_pos pos) {
  static int xcache = 3, ycache = 7;
  static int terrain = 0;
  static int dirt = 1;
  static int cave_layer_1_b = 0, cave_layer_1_t = 0;
  static int cave_layer_2_b = 0, cave_layer_2_t = 0;
  static int cave_layer_3_b = 0, cave_layer_3_t = 0;
  static float nlst = 0, nlow = 0, nmid = 0, nhig = 0, nhst = 0;
  float depths = 0, oceans = 0, plains = 0, hills = 0, mountains = 0;
  float roughness = 0.5;
  if (xcache != pos.x || ycache != pos.y) {
    xcache = pos.x; ycache = pos.y;
    // recompute everything:
    // generate some noise at each frequency (which we'll reuse several times):
    nlst = sxnoise_2d(pos.x * TR_FREQUENCY_LOWEST, pos.y * TR_FREQUENCY_LOWEST);
    nlow = sxnoise_2d(pos.x * TR_FREQUENCY_LOW, pos.y * TR_FREQUENCY_LOW);
    nmid = sxnoise_2d(pos.x * TR_FREQUENCY_MID, pos.y * TR_FREQUENCY_MID);
    nhig = sxnoise_2d(pos.x * TR_FREQUENCY_HIGH, pos.y * TR_FREQUENCY_HIGH);
    nhst = sxnoise_2d(pos.x * TR_FREQUENCY_HIGHEST, pos.y*TR_FREQUENCY_HIGHEST);
    // compute geoform mixing factors:
    compute_geoforms(nlst, &depths, &oceans, &plains, &hills, &mountains);
    // mix terrain height:
    terrain = (int) (
      (
        abscb(nlow)
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
    terrain += TR_DETAIL_LOW * abscb(nmid);
    terrain += TR_DETAIL_MID * abssq(nhig);
    roughness = 0.5 * abssq(nmid);
    terrain += roughness * TR_DETAIL_HIGH * nhst;
    // dirt depth:
    dirt = TR_DIRT_MID + (int) (
      nmid * TR_DIRT_VAR
    );
  }
  if (pos.z == terrain) {
    if (pos.z >= TR_SEA_LEVEL) {
      return B_GRASS;
    } else {
      return B_DIRT;
    }
  } else if (pos.z > terrain) {
    if (pos.z > TR_SEA_LEVEL) {
      return B_AIR;
    } else {
      return B_WATER;
    }
  } else {
    if (terrain - pos.z < dirt) {
      return B_DIRT;
    }
    return B_STONE;
  }
}
