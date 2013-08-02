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

static inline void get_noise(
  float x, float y,
  float *lowest, float *low, float *medium, float *high, float *highest
) {
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

block terrain_block(region_pos pos) {
  static int xcache = 3, ycache = 7;
  static int terrain = 0;
  static int dirt = 1;
  static int cave_layer_1_b = 0, cave_layer_1_t = 0;
  static int cave_layer_2_b = 0, cave_layer_2_t = 0;
  static int cave_layer_3_b = 0, cave_layer_3_t = 0;
  static float nlst = 0, nlow = 0, nmid = 0, nhig = 0, nhst = 0;
  static float depths = 0, oceans = 0, plains = 0, hills = 0, mountains = 0;
  static int sandy = 0;
  int tunnel = 0;
  if (xcache != pos.x || ycache != pos.y) {
    xcache = pos.x; ycache = pos.y;
    // recompute everything:
    // generate some noise at each frequency (which we'll reuse several times):
    get_noise(pos.x, pos.y, &nlst, &nlow, &nmid, &nhig, &nhst);
    // compute geoform mixing factors:
    depths = 0;
    oceans = 0;
    plains = 0;
    hills = 0;
    mountains = 0;
    compute_geoforms(nlst, &depths, &oceans, &plains, &hills, &mountains);
    // compute terrain height:
    terrain = get_terrain_height(
      nlst, nlow, nmid, nhig, nhst,
      depths, oceans, plains, hills, mountains
    );
    // compute cave layers:
    get_cave_layers(
      nlst, nlow, nmid, nhig, nhst,
      depths, oceans, plains, hills, mountains,
      &cave_layer_1_b, &cave_layer_1_t,
      &cave_layer_2_b, &cave_layer_2_t,
      &cave_layer_3_b, &cave_layer_3_t
    );
    // dirt depth:
    dirt = TR_DIRT_MID + (int) (
      nmid * TR_DIRT_VAR
    );
    // sandiness:
    sandy =
      oceans * oceans > (TR_BEACH_THRESHOLD + (0.03 * terrain - TR_SEA_LEVEL));
  }
  // DEBUG: (tunnels are expensive)
  tunnel = 0;
  /*
  tunnel = get_tunnel(
    &pos,
    depths, oceans, plains, hills, mountains // TODO: Use these arguments!
  );
  // */
  if (
    tunnel
  &&
    pos.z <= terrain
  &&
    (
      terrain > TR_SEA_LEVEL
    ||
      pos.z < (terrain - TR_TUNNEL_UNDERSEA_OFFSET)
    )
  ) {
    return B_AIR;
  }
  if (pos.z == terrain) {
    if (sandy) {
      return B_SAND;
    } else if (pos.z >= TR_SEA_LEVEL) {
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
      if (sandy) {
        return B_SAND;
      } else {
        return B_DIRT;
      }
    }
    return B_STONE;
  }
}

void get_geoforms(
  int x, int y,
  float *depths, float *oceans, float *plains, float *hills, float *mountains
) {
  float noise = 0, ignore = 0;
  get_noise(x, y, &noise, &ignore, &ignore, &ignore, &ignore);
  compute_geoforms(noise, depths, oceans, plains, hills, mountains);
}
