// terrain.c
// Code for determining terrain height.

#include <math.h>

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

void compute_terrain_height(
  region_pos *pos,
  float *r_rocks,
  float *r_dirt,
  float *r_downhill
) {
  static int xcache = 3, ycache = 7;
  static float continents = 0, primary_geoforms = 0, secondary_geoforms = 0;
  float geodetails = 0, mountains = 0;
  static float hills = 0, ridges = 0, mounds = 0, details = 0, bumps = 0;
  terrain_region region;
  float tr_interp;
  static float rocks_height, dirt_height;
  float base;
  ptrdiff_t salt = TR_NOISE_SALT;

  if (xcache == pos->x && ycache == pos->y) {
    // no need to recompute everything:
    *r_rocks =  rocks_height;
    *r_dirt =  dirt_height;
  }
  xcache = pos->x; ycache = pos->y;

  compute_base_geoforms(
    pos, &salt,
    &continents, &primary_geoforms, &secondary_geoforms,
    &geodetails, &mountains,
    &hills, &ridges, &mounds, &details, &bumps,
    &base,
    &region,
    &tr_interp,
    &rocks_height
  );
  salt = expanded_hash_1d(salt);

  alter_terrain_values(
    pos, &salt,
    region, tr_interp,
    &mountains, &hills, &ridges, &mounds, &details, &bumps
  );

  // TODO: attenuate mountains near the beach?
  rocks_height += mountains * TR_SCALE_MOUNTAINS;
  rocks_height += hills * TR_SCALE_HILLS;
  rocks_height += ridges * TR_SCALE_RIDGES;
  rocks_height += mounds * TR_SCALE_MOUNDS;
  rocks_height += details * TR_SCALE_DETAILS;
  rocks_height += bumps * TR_SCALE_BUMPS;

  // Figure out our soil depth:
  compute_dirt_height(
    pos, &salt,
    rocks_height,
    mountains, hills, details, bumps,
    &dirt_height
  );

  // Write out our results:
  *r_rocks = rocks_height;
  *r_dirt = dirt_height;
  //*r_rocks = rocks_height;
  //*r_dirt = rocks_height;
}

void alter_terrain_values(
  region_pos *pos, ptrdiff_t *salt,
  terrain_region region, float tr_interp,
  float *mountains, float *hills, float *ridges,
  float *mounds, float *details, float *bumps
) {
  // Compute superflat areas:
  float flatten = sxnoise_2d(
    pos->x * TR_FREQUENCY_SGEOFORMS * 1.3,
    pos->y * TR_FREQUENCY_SGEOFORMS * 1.3,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  flatten += 0.7 * sxnoise_2d(
    pos->x * TR_FREQUENCY_SGEOFORMS * 2.4,
    pos->y * TR_FREQUENCY_SGEOFORMS * 2.4,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  flatten /= 1.7;
  flatten = (1 + flatten) / 2.0;
  if (region == TR_REGION_DEPTHS) {
    // amplify *bumps, *ridges, and *details; attenuate *hills and *mountains
    *mountains = (
      tr_interp * smooth(*mountains, 1.5, 0.6) +
      (1 - tr_interp) * smooth(*mountains, 3, 0.75)
    );
    *hills = (
      tr_interp * smooth(*hills, 2, 0.7) +
      (1 - tr_interp) * smooth(*hills, 1.5, 0.65)
    );
    *ridges = (
      tr_interp * smooth(*ridges, 4, 0.2) +
      (1 - tr_interp) * smooth(*ridges, 3, 0.8)
    );
    *mounds = (
      tr_interp * (*mounds) +
      (1 - tr_interp) * smooth(*mounds, 1.3, 0.6)
    );
    *details = (
      tr_interp * smooth(*details, 2, 0.2) +
      (1 - tr_interp) * smooth(*details, 2, 0.6)
    );
    *bumps = (
      tr_interp * smooth(*bumps, 3, 0.3) +
      (1 - tr_interp) * smooth(*bumps, 3, 0.8)
    );
  } else if (region == TR_REGION_SHELF) {
    // gently attenuate most features; strongly attenuate *ridges:
    flatten *= 0.8;
    *mountains = (
      tr_interp * smooth(*mountains, 3, 0.75) +
      (1 - tr_interp) * smooth(*mountains, 1.7, 0.6) * flatten
    );
    *hills = (
      tr_interp * smooth(*hills, 1.5, 0.65) +
      (1 - tr_interp) * smooth(*hills, 1.5, 0.7) * flatten
    );
    *ridges = (
      tr_interp * smooth(*ridges, 3, 0.8) +
      (1 - tr_interp) * smooth(*ridges, 1.3, 0.6) * flatten
    );
    *mounds = (
      tr_interp * smooth(*mounds, 1.3, 0.6) +
      (1 - tr_interp) * smooth(*mounds, 1.3, 0.6) * flatten
    );
    *details = (
      tr_interp * smooth(*details, 2, 0.6) +
      (1 - tr_interp) * smooth(*details, 1.4, 0.6) * flatten
    );
    *bumps = (
      tr_interp * smooth(*bumps, 3, 0.8) +
      (1 - tr_interp) * smooth(*bumps, 3, 0.9) * flatten
    );
  } else if (region == TR_REGION_PLAINS) {
    // attenuate everything; create some superflat regions:
    flatten *= 0.8;
    *mountains = (
      tr_interp * smooth(*mountains, 1.7, 0.6) * flatten +
      (1 - tr_interp) * (*mountains)
    );
    *hills = (
      tr_interp * smooth(*hills, 1.5, 0.7) * flatten +
      (1 - tr_interp) * smooth(*hills, 1.8, 0.3)
    );
    *ridges = (
      tr_interp * smooth(*ridges, 1.3, 0.6) * flatten +
      (1 - tr_interp) * smooth(*ridges, 1.5, 0.35)
    );
    *mounds = (
      tr_interp * smooth(*mounds, 1.3, 0.6) * flatten +
      (1 - tr_interp) * (*mounds)
    );
    *details = (
      tr_interp * smooth(*details, 1.4, 0.6) * flatten +
      (1 - tr_interp) * (*details)
    );
    *bumps = (
      tr_interp * smooth(*bumps, 3, 0.9) * flatten +
      (1 - tr_interp) * (*bumps)
    );
  } else if (region == TR_REGION_HILLS) {
    // slightly accentuate *hills and *ridges:
    flatten = smooth(flatten, 1.2, 0.5);
    flatten = 0.1 + 0.9 * flatten;
    *mountains = (
      tr_interp * (*mountains) +
      (1 - tr_interp) * smooth(*mountains, 0.6, 0.5)
    );
    *hills = (
      tr_interp * smooth(*hills, 1.8, 0.3) +
      (1 - tr_interp) * smooth(*hills, 1.3, 0.55)
    );
    *ridges = (
      tr_interp * smooth(*ridges, 1.5, 0.35) +
      (1 - tr_interp) * smooth(*ridges, 1.6, 0.4) * flatten
    );
    *mounds = (
      tr_interp * (*mounds) +
      (1 - tr_interp) * (*mounds) * flatten
    );
    *details = (
      tr_interp * (*details) +
      (1 - tr_interp) * (*details) * (0.25 + 0.5 * flatten)
    );
    *bumps = (
      tr_interp * (*bumps) +
      (1 - tr_interp) * (*bumps) * (0.25 + 0.5 * flatten)
    );
  } else if (region == TR_REGION_HIGHLANDS) {
    // accentuate *mountains; attenuate *hills and *ridges slightly; create some
    // flatter regions:
    flatten = smooth(flatten, 1.2, 0.5);
    flatten = 0.1 + 0.9 * flatten;
    *mountains = (
      tr_interp * smooth(*mountains, 0.6, 0.5) +
      (1 - tr_interp) * smooth(*mountains, 0.6, 0.65)
    );
    *hills = (
      tr_interp * smooth(*hills, 1.3, 0.55) +
      (1 - tr_interp) * smooth(*hills, 1.5, 0.4)
    );
    *ridges = (
      tr_interp * smooth(*ridges, 1.6, 0.4) * flatten +
      (1 - tr_interp) * smooth(*ridges, 0.5, 0.7)
    );
    *mounds = (
      tr_interp * (*mounds) * flatten +
      (1 - tr_interp) * smooth(*mounds, 1.3, 0.3)
    );
    *details = (
      tr_interp * (*details) * (0.25 + 0.5 * flatten) +
      (1 - tr_interp) * smooth(*details, 1.5, 0.4)
    );
    *bumps = (
      tr_interp * (*bumps) * (0.25 + 0.5 * flatten) +
      (1 - tr_interp) * smooth(*bumps, 1.5, 0.3)
    );
  } else if (region == TR_REGION_MOUNTAINS) {
    // amplify most things:
    *mountains = tr_interp * smooth(*mountains, 0.6, 0.65);
    *hills = tr_interp * smooth(*hills, 1.5, 0.4);
    *ridges = tr_interp * smooth(*ridges, 0.5, 0.7);
    *mounds = tr_interp * smooth(*mounds, 1.3, 0.3);
    *details = tr_interp * smooth(*details, 1.5, 0.4);
    *bumps = tr_interp * smooth(*bumps, 1.5, 0.3);
  }
}

void compute_dirt_height(
  region_pos *pos, ptrdiff_t *salt,
  float rocks_height,
  float mountains, float hills, float details, float bumps,
  float *result
) {
  float depth;
  float n, alt;
  // Base soil height varies a bit over large distances:
  n = sxnoise_2d(
    pos->x * TR_DIRT_NOISE_SCALE,
    pos->y * TR_DIRT_NOISE_SCALE,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  n += 0.5 * sxnoise_2d(
    pos->x * TR_DIRT_NOISE_SCALE * 2.5,
    pos->y * TR_DIRT_NOISE_SCALE * 2.5,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  n /= 1.5;
  n = (1 + n) / 2.0;
  n = 0.6 + 0.4 * n;

  // Base soil depth:
  depth = n * TR_BASE_SOIL_DEPTH;

  // Mountains and hills cause "erosion" which can lead to exposed rock:
  // TODO: gradient-based erosion? (this would be super-cool)
  n = sxnoise_2d(
    pos->x * TR_DIRT_EROSION_SCALE,
    pos->y * TR_DIRT_EROSION_SCALE,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  n = (1 + n) / 2.0;
  depth -= mountains * TR_DIRT_MOUNTAIN_EROSION * n;

  n = sxnoise_2d(
    pos->x * TR_DIRT_EROSION_SCALE * 1.6,
    pos->y * TR_DIRT_EROSION_SCALE * 1.6,
    *salt
  );
  *salt = expanded_hash_1d(*salt);
  n = (1 + n) / 2.0;
  depth -= hills * TR_DIRT_HILL_EROSION * n;

  // Altitude above/below sea level contributes to soil depth due to wind
  // erosion and marine snow (this is a stupidly oversimplified model of
  // course):
  alt = rocks_height - TR_HEIGHT_SEA_LEVEL;
  if (alt > 0) {
    alt /= TR_MAX_HEIGHT;
    alt = smooth(alt, 3, 0.9);
  } else {
    alt /= TR_HEIGHT_SEA_LEVEL;
    alt = smooth(alt, 1.4, 0.5);
  }

  depth -= alt * TR_ALTITUDE_EROSION_STRENGTH;

  // Details and bumps in the stone aren't reflected in the dirt:
  // TODO: smoothness vs. roughness
  depth -= details * TR_SCALE_DETAILS;
  depth -= bumps * TR_SCALE_BUMPS;

  // Dirt depth can't be negative:
  depth = depth < 0 ? 0 : depth;

  // Write out the result:
  *result = rocks_height + depth;
}

void geoform_info(region_pos *pos, terrain_region* region, float* tr_interp) {
  static int xcache = 3, ycache = 7;
  static float continents = 0, primary_geoforms = 0, secondary_geoforms = 0;
  static float geodetails = 0;
  static float base = 0;
  static float my_tr_interp = 0;
  static terrain_region my_region = 0;
  float dontcare = 0;
  ptrdiff_t salt = TR_NOISE_SALT;

  if (xcache == pos->x && ycache == pos->y) {
    // no need to recompute everything:
    *region = my_region;
    *tr_interp = my_tr_interp;
    return;
  }
  xcache = pos->x; ycache = pos->y;

  compute_base_geoforms(
    pos, &salt,
    &continents, &primary_geoforms, &secondary_geoforms,
    &geodetails, &dontcare,
    &dontcare, &dontcare, &dontcare, &dontcare, &dontcare,
    &base,
    &my_region,
    &my_tr_interp,
    &dontcare
  );
  *region = my_region;
  *tr_interp = my_tr_interp;
}
