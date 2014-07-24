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

float TR_NOISE_SALT = 7300845;

float TR_TERRAIN_HEIGHT_AMP = 1.0;

char const * const TR_REGION_NAMES[] = {
  "ocean depths",
  "continental shelf",
  "coastal plains",
  "hills",
  "highlands",
  "mountains",
  "sky",
};

/*************
 * Functions *
 *************/

float terrain_height(region_pos *pos) {
  static int xcache = 3, ycache = 7;
  static float continents = 0, geoforms = 0, geodetails = 0, mountains = 0;
  static float hills = 0, ridges = 0, mounds = 0, details = 0, bumps = 0;
  terrain_region region;
  float tr_interp;
  static float height;
  float base;
  float flatten;
  ptrdiff_t salt = TR_NOISE_SALT;

  if (xcache == pos->x && ycache == pos->y) {
    // no need to recompute everything:
    return height;
  }
  xcache = pos->x; ycache = pos->y;

  compute_base_geoforms(
    pos, salt,
    &continents, &geoforms, &geodetails, &mountains,
    &hills, &ridges, &mounds, &details, &bumps,
    &base,
    &region,
    &tr_interp,
    &height
  );
  salt = expanded_hash_1d(salt);

  // DEBUG:
  //*
  if (region == TR_REGION_DEPTHS) {
    // amplify bumps, ridges, and details; attenuate hills
    hills = smooth(hills, 2, 0.7);
    ridges = smooth(ridges, 4, 0.2);
    details = smooth(details, 2, 0.2);
    bumps = smooth(bumps, 3, 0.3);
  } else if (region == TR_REGION_SHELF) {
    // gently attenuate most features; strongly attenuate ridges:
    hills = smooth(hills, 1.5, 0.65);
    ridges = smooth(ridges, 3, 0.8);
    mounds = smooth(mounds, 1.3, 0.6);
    details = smooth(details, 2, 0.6);
    bumps = smooth(bumps, 3, 0.8);
  } else if (region == TR_REGION_PLAINS) {
    // attenuate everything; create some superflat regions:
    flatten = sxnoise_2d(
      pos->x * TR_FREQUENCY_GEOFORMS * 1.8,
      pos->y * TR_FREQUENCY_GEOFORMS * 1.8,
      salt
    );
    salt = expanded_hash_1d(salt);
    flatten += 0.7 * sxnoise_2d(
      pos->x * TR_FREQUENCY_GEOFORMS * 1.5,
      pos->y * TR_FREQUENCY_GEOFORMS * 1.5,
      salt
    );
    flatten /= 1.7;
    flatten = (1 + flatten) / 2.0;
    flatten *= 0.8;

    hills = smooth(hills, 1.5, 0.7);
    hills *= flatten;
    ridges = smooth(ridges, 1.3, 0.6);
    ridges *= flatten;
    mounds = smooth(mounds, 1.3, 0.6);
    mounds *= flatten;
    details = smooth(details, 1.4, 0.6);
    details *= flatten;
    bumps = smooth(bumps, 3, 0.9);
    bumps *= flatten;
  } else if (region == TR_REGION_HILLS) {
    // slightly accentuate hills and ridges:
    hills = smooth(hills, 1.8, 0.3);
    ridges = smooth(ridges, 1.5, 0.35);
  } else if (region == TR_REGION_HIGHLANDS) {
    // attenuate hills and ridges slightly; create some flatter regions:
    flatten = sxnoise_2d(
      pos->x * TR_FREQUENCY_MOUNTAINS * 0.7,
      pos->y * TR_FREQUENCY_MOUNTAINS * 0.7,
      salt
    );
    salt = expanded_hash_1d(salt);
    flatten += 0.5 * sxnoise_2d(
      pos->x * TR_FREQUENCY_MOUNTAINS * 0.3,
      pos->y * TR_FREQUENCY_MOUNTAINS * 0.3,
      salt
    );
    salt = expanded_hash_1d(salt);
    flatten /= 1.5;
    flatten = (1 + flatten) / 2.0;
    flatten = smooth(flatten, 1.2, 0.5);
    flatten = 0.1 + 0.9 * flatten;

    hills = smooth(hills, 1.3, 0.55);
    hills *= flatten;
    ridges = smooth(ridges, 1.6, 0.4);
    ridges *= flatten;
    mounds *= flatten;
    details *= (0.25 + 0.5*flatten);
    bumps *= (0.25 + 0.5*flatten);
  } else if (region == TR_REGION_MOUNTAINS) {
    // amplify most things:
    hills = smooth(hills, 1.5, 0.4);
    ridges = smooth(ridges, 2, 0.2);
    mounds = smooth(mounds, 1.3, 0.3);
    details = smooth(details, 1.5, 0.4);
    bumps = smooth(bumps, 1.5, 0.3);
  }
  // */

  // TODO: attenuate mountains near the beach?
  //*
  height += mountains * TR_SCALE_MOUNTAINS;
  height += hills * TR_SCALE_HILLS;
  height += ridges * TR_SCALE_RIDGES;
  height += mounds * TR_SCALE_MOUNDS;
  height += details * TR_SCALE_DETAILS;
  height += bumps * TR_SCALE_BUMPS;
  // */

  return height;
}

void geoform_info(region_pos *pos, terrain_region* region, float* tr_interp) {
  static int xcache = 3, ycache = 7;
  static float continents = 0, geoforms = 0, geodetails = 0;
  static float base = 0;
  static float my_tr_interp = 0;
  static terrain_region my_region = 0;
  float dontcare = 0;

  if (xcache == pos->x && ycache == pos->y) {
    // no need to recompute everything:
    *region = my_region;
    *tr_interp = my_tr_interp;
    return;
  }
  xcache = pos->x; ycache = pos->y;

  compute_base_geoforms(
    pos, TR_NOISE_SALT,
    &continents, &geoforms, &geodetails, &dontcare,
    &dontcare, &dontcare, &dontcare, &dontcare, &dontcare,
    &base,
    &my_region,
    &my_tr_interp,
    &dontcare
  );
  *region = my_region;
  *tr_interp = my_tr_interp;
}
