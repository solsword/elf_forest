// terrain.c
// Code for determining terrain height.

#include <math.h>

//#include "trees.h"
#include "terrain.h"

#include "world/blocks.h"
#include "world/world.h"
#include "noise/noise.h"

/***********
 * Globals *
 ***********/

float TR_NOISE_OFFSET = 7300;

float TR_TERRAIN_HEIGHT_AMP = 1.0;

/*************
 * Functions *
 *************/

float terrain_height(region_pos *pos) {
  static int xcache = 3, ycache = 7;
  static float continents = 0, geoforms = 0, mountains = 0, hills = 0;
  static float ridges = 0, mounds = 0, details = 0, bumps = 0;
  float height;
  if (xcache != pos->x || ycache != pos->y) {
    xcache = pos->x; ycache = pos->y;
    // recompute everything:
    // make some noise:
    get_noise(
      pos->x, pos->y,
      &continents, &geoforms, &mountains, &hills,
      &ridges, &mounds, &details, &bumps
    );
  }
  float base = (continents + geoforms + mountains) / 3.0;
  base = (1 + base)/2.0; // squash into [0, 1]
  base = smooth(base, 0.5, 0.5); // pull things in
  // remap everything:
  height = geomap(base);

  height += hills * TR_SCALE_HILLS;
  height += ridges * TR_SCALE_RIDGES;
  height += mounds * TR_SCALE_MOUNDS;
  height += details * TR_SCALE_DETAILS;
  height += bumps * TR_SCALE_BUMPS;

  return height;
}
