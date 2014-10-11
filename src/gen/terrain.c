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
  manifold_point *r_rocks,
  manifold_point *r_dirt
) {
  static int xcache = 3, ycache = 7;
  static manifold_point continents, primary_geoforms, secondary_geoforms;
  static manifold_point geodetails, mountains;
  static manifold_point hills, ridges;
  static manifold_point mounds, details, bumps;
  static manifold_point rocks_height, dirt_height;
  manifold_point temp;

  terrain_region region;
  manifold_point tr_interp;
  manifold_point base;
  ptrdiff_t salt = TR_NOISE_SALT;

  if (xcache == pos->x && ycache == pos->y) {
    // no need to recompute everything:
    mani_copy(*r_rocks, rocks_height);
    mani_copy(*r_dirt, dirt_height);
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
    region, &tr_interp,
    &mountains, &hills, &ridges, &mounds, &details, &bumps
  );

  // TODO: attenuate mountains near the beach?
  mani_copy(temp, mountains);
  mani_scale_const(temp, TR_SCALE_MOUNTAINS);
  mani_add(rocks_height, temp);

  mani_copy(temp, hills);
  mani_scale_const(temp, TR_SCALE_HILLS);
  mani_add(rocks_height, temp);

  mani_copy(temp, ridges);
  mani_scale_const(temp, TR_SCALE_RIDGES);
  mani_add(rocks_height, temp);

  mani_copy(temp, mounds);
  mani_scale_const(temp, TR_SCALE_MOUNDS);
  mani_add(rocks_height, temp);

  mani_copy(temp, details);
  mani_scale_const(temp, TR_SCALE_DETAILS);
  mani_add(rocks_height, temp);

  mani_copy(temp, bumps);
  mani_scale_const(temp, TR_SCALE_BUMPS);
  mani_add(rocks_height, temp);

  // Figure out our soil depth:
  compute_dirt_height(
    pos, &salt,
    &rocks_height,
    &mountains, &hills,
    &details, &bumps,
    &dirt_height
  );

  // Write out our results:
  mani_copy(*r_rocks, rocks_height);
  mani_copy(*r_dirt, dirt_height);
}

// helper for alter_terrian_values
static inline void alter_value(
  manifold_point *value,
  manifold_point *interp,
  float this_str, float this_ctr,
  float next_str, float next_ctr,
  manifold_point *this_flatten,
  manifold_point *next_flatten
) {
  manifold_point result, temp, tinterp;
  mani_copy(temp, *value);
  mani_smooth(temp, this_str, this_ctr);
  mani_multiply(temp, *interp);
  mani_multiply(temp, *this_flatten);
  mani_copy(result, temp);

  mani_copy(temp, *value);
  mani_copy(tinterp, *interp);
  mani_smooth(temp, next_str, next_ctr);
  mani_scale_const(tinterp, -1);
  mani_offset_const(tinterp, 1);
  mani_multiply(temp, tinterp);
  mani_multiply(temp, *next_flatten);

  mani_add(result, temp);
  mani_copy(*value, result);
}

void alter_terrain_values(
  region_pos *pos, ptrdiff_t *salt,
  terrain_region region, manifold_point *tr_interp,
  manifold_point *mountains, manifold_point *hills, manifold_point *ridges,
  manifold_point *mounds, manifold_point *details, manifold_point *bumps
) {
  manifold_point flatten, nofl, temp;

  nofl.z = 1;
  nofl.dx = 0;
  nofl.dy = 0;

  // Superflat areas:
  simplex_component(
    &flatten,
    pos->x, pos->y,
    1, 1,
    TR_FREQUENCY_SGEOFORMS * 1.3,
    salt
  );
  simplex_component(
    &temp,
    pos->x, pos->y,
    1, 1,
    TR_FREQUENCY_SGEOFORMS * 2.4,
    salt
  );
  mani_scale_const(temp, 0.7);
  mani_add(flatten, temp);
  mani_scale_const(flatten, 1.0/1.7);
  mani_offset_const(flatten, 1);
  mani_scale_const(flatten, 0.5);

  if (region == TR_REGION_DEPTHS) {
    // amplify *bumps, *ridges, and *details; attenuate *hills and *mountains
    alter_value(mountains, tr_interp, 1.5, 0.6, 3, 0.75, &nofl, &nofl);
    alter_value(hills, tr_interp, 2, 0.7, 1.5, 0.65, &nofl, &nofl);
    alter_value(ridges, tr_interp, 4, 0.2, 3, 0.8, &nofl, &nofl);
    alter_value(mounds, tr_interp, 1, 1, 1.3, 0.6, &nofl, &nofl);
    alter_value(details, tr_interp, 2, 0.2, 2, 0.6, &nofl, &nofl);
    alter_value(bumps, tr_interp, 3, 0.3, 3, 0.8, &nofl, &nofl);
  } else if (region == TR_REGION_SHELF) {
    // gently attenuate most features; strongly attenuate *ridges:
    mani_scale_const(flatten, 0.8);

    alter_value(mountains, tr_interp, 3, 0.75, 1.7, 0.6, &nofl, &flatten);
    alter_value(hills, tr_interp, 1.5, 0.65, 1.5, 0.7, &nofl, &flatten);
    alter_value(ridges, tr_interp, 3, 0.8, 1.3, 0.6, &nofl, &flatten);
    alter_value(mounds, tr_interp, 1.3, 0.6, 1.3, 0.6, &nofl, &flatten);
    alter_value(details, tr_interp, 2, 0.6, 1.4, 0.6, &nofl, &flatten);
    alter_value(bumps, tr_interp, 3, 0.8, 3, 0.9, &nofl, &flatten);
  } else if (region == TR_REGION_PLAINS) {
    // attenuate everything; create some superflat regions:
    mani_scale_const(flatten, 0.8);

    alter_value(mountains, tr_interp, 1.7, 0.6, 1, 1, &flatten, &nofl);
    alter_value(hills, tr_interp, 1.5, 0.7, 1.8, 0.3, &flatten, &nofl);
    alter_value(ridges, tr_interp, 1.3, 0.6, 1.5, 0.35, &flatten, &nofl);
    alter_value(mounds, tr_interp, 1.3, 0.6, 1, 1, &flatten, &nofl);
    alter_value(details, tr_interp, 1.4, 0.6, 1, 1, &flatten, &nofl);
    alter_value(bumps, tr_interp, 3, 0.9, 1, 1, &flatten, &nofl);

  } else if (region == TR_REGION_HILLS) {
    // slightly accentuate *hills and *ridges:
    mani_smooth(flatten, 1.2, 0.5);
    mani_scale_const(flatten, 0.9);
    mani_offset_const(flatten, 0.1);

    alter_value(mountains, tr_interp, 1, 1, 0.6, 0.5, &nofl, &nofl);
    alter_value(hills, tr_interp, 1.8, 0.3, 1.3, 0.55, &nofl, &nofl);
    alter_value(ridges, tr_interp, 1.5, 0.35, 1.6, 0.4, &nofl, &flatten);
    alter_value(mounds, tr_interp, 1, 1, 1, 1, &nofl, &flatten);

    mani_scale_const(flatten, 0.5);
    mani_offset_const(flatten, 0.25);
    alter_value(details, tr_interp, 1, 1, 1, 1, &nofl, &flatten);
    alter_value(bumps, tr_interp, 1, 1, 1, 1, &nofl, &flatten);

  } else if (region == TR_REGION_HIGHLANDS) {
    // accentuate *mountains; attenuate *hills and *ridges slightly; create some
    // flatter regions:
    mani_smooth(flatten, 1.2, 0.5);
    mani_scale_const(flatten, 0.9);
    mani_offset_const(flatten, 0.1);

    alter_value(mountains, tr_interp, 0.6, 0.5, 0.6, 0.65, &nofl, &nofl);
    alter_value(hills, tr_interp, 1.3, 0.55, 1.5, 0.4, &nofl, &nofl);
    alter_value(ridges, tr_interp, 1.6, 0.4, 0.5, 0.7, &flatten, &nofl);
    alter_value(mounds, tr_interp, 1, 1, 1.3, 0.3, &flatten, &nofl);

    mani_scale_const(flatten, 0.5);
    mani_offset_const(flatten, 0.25);
    alter_value(details, tr_interp, 1, 1, 1.5, 0.4, &flatten, &nofl);
    alter_value(bumps, tr_interp, 1, 1, 1.5, 0.3, &flatten, &nofl);

  } else if (region == TR_REGION_MOUNTAINS) {
    // amplify most things:
    mani_smooth(*mountains, 0.6, 0.65);
    mani_smooth(*hills, 1.5, 0.4);
    mani_smooth(*ridges, 0.5, 0.7);
    mani_smooth(*mounds, 1.3, 0.3);
    mani_smooth(*details, 1.5, 0.4);
    mani_smooth(*bumps, 1.5, 0.3);
  }
}

void compute_dirt_height(
  region_pos *pos, ptrdiff_t *salt,
  manifold_point *rocks_height,
  manifold_point *mountains, manifold_point *hills,
  manifold_point *details, manifold_point *bumps,
  manifold_point *result
) {
  manifold_point var, temp;
  float steepness;
  // Base soil height varies a bit over large distances:
  simplex_component(
    &var,
    pos->x, pos->y,
    1, 1,
    TR_DIRT_NOISE_SCALE,
    salt
  );
  simplex_component(
    &temp,
    pos->x, pos->y,
    1, 1,
    TR_DIRT_NOISE_SCALE * 2.5,
    salt
  );
  mani_scale_const(temp, 0.5);
  mani_add(var, temp);

  mani_scale_const(var, 1.0/1.5);
  mani_offset_const(var, 1);
  mani_scale_const(var, 0.5 * 0.4);
  mani_offset_const(var, 0.6);

  // Base soil depth:
  mani_copy(*result, var);
  mani_scale_const(*result, TR_BASE_SOIL_DEPTH);

  // Mountains and hills have less dirt on them in general:
  simplex_component(
    &temp,
    pos->x, pos->y,
    1, 1,
    TR_DIRT_EROSION_SCALE,
    salt
  );
  mani_offset_const(temp, 1);
  mani_scale_const(temp, 0.5 * 0.5);
  mani_offset_const(temp, 0.25);
  mani_scale_const(temp, -TR_DIRT_MOUNTAIN_EROSION);
  mani_multiply(temp, *mountains);
  mani_add(*result, temp);

  simplex_component(
    &temp,
    pos->x, pos->y,
    1, 1,
    TR_DIRT_EROSION_SCALE * 1.6,
    salt
  );
  mani_offset_const(temp, 1);
  mani_scale_const(temp, 0.5 * 0.5);
  mani_offset_const(temp, 0.25);
  mani_scale_const(temp, -TR_DIRT_HILL_EROSION);
  mani_multiply(temp, *hills);
  mani_add(*result, temp);

  // Altitude above/below sea level contributes to soil depth due to wind
  // erosion and marine snow (this is a stupidly oversimplified model of
  // course):
  mani_copy(temp, *rocks_height);
  mani_offset_const(temp, -TR_HEIGHT_SEA_LEVEL);
  if (temp.z > 0) {
    mani_scale_const(temp, 1.0/(TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL));
    mani_smooth(temp, 2.3, 0.8);
  } else {
    mani_scale_const(temp, 1.0/TR_HEIGHT_SEA_LEVEL);
    mani_smooth(temp, 1.4, 0.5);
  }

  mani_scale_const(temp, -TR_ALTITUDE_EROSION_STRENGTH);
  mani_add(*result, temp);

  // Steeper places (especially at high altitudes) have less dirt, while
  // flatter places have more:
  steepness = mani_slope(*rocks_height);
  steepness = fmin(1.5, steepness); // slopes above 1.5 are treated identically
  steepness /= 1.5;
  smooth(steepness, 2.0, 0.25);
  steepness -= 0.25;

  mani_copy(temp, *rocks_height);
  mani_scale_const(temp, 1.0/TR_MAX_HEIGHT);
  mani_smooth(temp, 1.5, 0.65);
  mani_scale_const(temp, steepness * -TR_SLOPE_EROSION_STRENGTH);
    // Note: this is incorrect: for a correct erosion manifold you'd need to
    // know the second derivative of rocks_height, but we don't so we're
    // ignoring it. This means that the dirt_height won't have a completely
    // correct derivative, but for most applications, using the rocks_height
    // derivative should be satisfactory.
  mani_add(*result, temp);

  // Details and bumps in the stone aren't reflected in the dirt:
  // TODO: smoothness vs. roughness
  mani_copy(temp, *details);
  mani_scale_const(temp, -TR_SCALE_DETAILS);
  mani_add(*result, temp);

  mani_copy(temp, *bumps);
  mani_scale_const(temp, -TR_SCALE_BUMPS);
  mani_add(*result, temp);

  // Dirt depth can't be negative:
  if (result->z < 0) {
    result->z = 0;
    result->dx = 0;
    result->dy = 0;
  }

  // Convert to absolute dirt height:
  mani_add(*result, *rocks_height);
}

void geoform_info(
  region_pos *pos,
  terrain_region* region,
  float* tr_interp
) {
  static int xcache = 3, ycache = 7;
  /*
  static manifold_point continents, primary_geoforms;
  static manifold_point secondary_geoforms;
  static manifold_point geodetails;
  static manifold_point base;
  */
  static manifold_point my_tr_interp;
  static terrain_region my_region;
  manifold_point dontcare;
  ptrdiff_t salt = TR_NOISE_SALT;

  if (xcache == pos->x && ycache == pos->y) {
    // no need to recompute everything:
    *region = my_region;
    *tr_interp = my_tr_interp.z;
    return;
  }
  xcache = pos->x; ycache = pos->y;

  compute_base_geoforms(
    pos, &salt,
    &dontcare, &dontcare, &dontcare,
    &dontcare, &dontcare,
    /*
    &continents, &primary_geoforms, &secondary_geoforms,
    &geodetails, &dontcare,
    */
    &dontcare, &dontcare,
    &dontcare, &dontcare, &dontcare,
    &dontcare, // &base,
    &my_region,
    &my_tr_interp,
    &dontcare
  );
  *region = my_region;
  *tr_interp = my_tr_interp.z;
}
