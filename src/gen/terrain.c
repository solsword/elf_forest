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
  height = (
    continents + geoforms + mountains + hills +
    ridges + mounds + details + bumps
  );
  height = (height + 1) / 2.0;
  return height;
}
