// terrain.c
// Code for determining terrain height.

#include <math.h>

#include "world/blocks.h"
#include "world/world.h"
#include "world/world_map.h"
#include "noise/noise.h"

#include "util.h"

#include "geology.h"

#include "terrain.h"

/***********
 * Globals *
 ***********/

ptrdiff_t TR_NOISE_SALT = 7300845;

char const * const TR_REGION_NAMES[] = {
  "ocean depths",
  "continental shelf",
  "coastal plains",
  "hills",
  "highlands",
  "mountains",
  "sky",
};

/*******************
 * Private Globals *
 *******************/

omp_lock_t TERRAIN_HEIGHT_LOCK;

/*************
 * Functions *
 *************/

void setup_terrain_gen(ptrdiff_t seed) {
  omp_init_lock(&TERRAIN_HEIGHT_LOCK);
  TR_NOISE_SALT = prng(prng(seed) + 719102);
}

void compute_terrain_height(
  region_pos *pos,
  manifold_point *r_gross,
  manifold_point *r_rocks,
  manifold_point *r_dirt
) {
  static int xcache = 3, ycache = 7;
  static manifold_point continents, primary_geoforms, secondary_geoforms;
  static manifold_point geodetails, mountains;
  static manifold_point hills, ridges;
  static manifold_point mounds, details, bumps;
  static manifold_point gross_height, rocks_height, dirt_height;

  terrain_region region;
  manifold_point tr_interp;
  manifold_point base;
  ptrdiff_t salt = TR_NOISE_SALT;

  if (xcache == pos->x && ycache == pos->y) {
    // no need to recompute everything:
    // printf("cached heights: %.2f, %.2f\n\n", rocks_height.z, dirt_height.z);
    mani_copy(r_gross, &gross_height);
    mani_copy(r_rocks, &rocks_height);
    mani_copy(r_dirt, &dirt_height);
  }
  // beyond this point we can't let multiple threads run at once as they'd mess
  // up each others' static variables.
  omp_set_lock(&TERRAIN_HEIGHT_LOCK);
  
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
  salt = prng(salt);
#ifdef DEBUG
  if (isnan(rocks_height.dx) || isnan(rocks_height.dy)) {
    printf("nan base rocks_height\n");
    exit(1);
  }
#endif

  // DEBUG: print base geoforms:
  /*
  // printf("Base geoforms!\n");
  if (continents.z < -1 || continents.z > 1) {
    printf("BAD!\n");
    printf("continents: %.2f\n", continents.z);
    exit(17);
  }
  if (primary_geoforms.z < -1 || primary_geoforms.z > 1) {
    printf("BAD!\n");
    printf("primary_geoforms: %.2f\n", primary_geoforms.z);
    exit(17);
  }
  if (secondary_geoforms.z < -1 || secondary_geoforms.z > 1) {
    printf("BAD!\n");
    printf("secondary_geoforms: %.2f\n", secondary_geoforms.z);
    exit(17);
  }
  if (geodetails.z < -1 || geodetails.z > 1) {
    printf("BAD!\n");
    printf("geodetails: %.2f\n", geodetails.z);
    exit(17);
  }
  if (mountains.z < -1 || mountains.z > 1) {
    printf("BAD!\n");
    printf("mountains: %.2f\n", mountains.z);
    exit(17);
  }
  if (hills.z < -1 || hills.z > 1) {
    printf("BAD!\n");
    printf("hills: %.2f\n", hills.z);
    exit(17);
  }
  if (ridges.z < -1 || ridges.z > 1) {
    printf("BAD!\n");
    printf("ridges: %.2f\n", ridges.z);
    exit(17);
  }
  if (mounds.z < -1 || mounds.z > 1) {
    printf("BAD!\n");
    printf("mounds: %.2f\n", mounds.z);
    exit(17);
  }
  if (details.z < -1 || details.z > 1) {
    printf("BAD!\n");
    printf("details: %.2f\n", details.z);
    exit(17);
  }
  if (bumps.z < -1 || bumps.z > 1) {
    printf("BAD!\n");
    printf("bumps: %.2f\n", bumps.z);
    exit(17);
  }
  if (base.z < -1 || base.z > 1) {
    printf("BAD!\n");
    printf("base: %.2f\n", base.z);
    exit(17);
  }
  if (tr_interp.z < -1 || tr_interp.z > 1) {
    printf("BAD!\n");
    printf("interp: %.2f\n", tr_interp.z);
    exit(17);
  }
  // printf("rocks: %.2f\n", rocks_height.z);
  // printf("\n");
  // */

  alter_terrain_values(
    pos, &salt,
    region, &tr_interp,
    &mountains, &hills, &ridges, &mounds, &details, &bumps
  );

  // DEBUG:
  /*
  printf("altered values!\n");
  printf(
    "mountains: %.2f :: %.2f %.2f\n",
    mountains.z,
    mountains.dx,
    mountains.dy
  );
  printf(
    "hills: %.2f :: %.2f %.2f\n",
    hills.z,
    hills.dx,
    hills.dy
  );
  printf(
    "ridges: %.2f :: %.2f %.2f\n",
    ridges.z,
    ridges.dx,
    ridges.dy
  );
  printf(
    "mounds: %.2f :: %.2f %.2f\n",
    mounds.z,
    mounds.dx,
    mounds.dy
  );
  printf(
    "details: %.2f :: %.2f %.2f\n",
    details.z,
    details.dx,
    details.dy
  );
  printf(
    "bumps: %.2f :: %.2f %.2f\n",
    bumps.z,
    bumps.dx,
    bumps.dy
  );
  printf("\n");
  // */

  // TODO: attenuate mountains near the beach?
  //* DEBUG
  mani_scale_const(&mountains, TR_SCALE_MOUNTAINS);
  mani_add(&rocks_height, &mountains);

  mani_scale_const(&hills, TR_SCALE_HILLS);
  mani_add(&rocks_height, &hills);

  // Gross height includes hills and mountains but no smaller details.
  mani_copy(&gross_height, &rocks_height);

  mani_scale_const(&ridges, TR_SCALE_RIDGES);
  mani_add(&rocks_height, &ridges);

  mani_scale_const(&mounds, TR_SCALE_MOUNDS);
  mani_add(&rocks_height, &mounds);

  mani_scale_const(&details, TR_SCALE_DETAILS);
  mani_add(&rocks_height, &details);

  mani_scale_const(&bumps, TR_SCALE_BUMPS);
  mani_add(&rocks_height, &bumps);
  // */

  // DEBUG:
  //*
  // printf("summed rocks: %.2f\n", rocks_height.z);
  // printf("\n");
  // */

  // Figure out our soil depth:
  compute_dirt_height(
    pos, &salt,
    &rocks_height,
    &mountains, &hills,
    &details, &bumps,
    &dirt_height
  );

  // DEBUG:
  //*
  // printf("rocks post-dirt: %.2f\n", rocks_height.z);
  // printf("dirt: %.2f\n", dirt_height.z);
  // printf("\n");
  // */

#ifdef DEBUG
  if (isnan(rocks_height.z)||isnan(rocks_height.dx)||isnan(rocks_height.dy)) {
    printf("nan final rocks_height\n");
    exit(1);
  }
  if (isnan(dirt_height.z) || isnan(dirt_height.dx) || isnan(dirt_height.dy)) {
    printf("nan final dirt_height\n");
    exit(1);
  }
#endif

  // Write out our results:
  mani_copy(r_gross, &gross_height);
  mani_copy(r_rocks, &rocks_height);
  mani_copy(r_dirt, &dirt_height);

  // Now we can release the lock and let another thread compute some terrain
  // height.
  omp_unset_lock(&TERRAIN_HEIGHT_LOCK);

  // DEBUG:
  //*
  // printf("result rocks: %.2f\n", r_rocks->z);
  // printf("result dirt: %.2f\n", r_dirt->z);
  // printf("\n");
  // */
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
  mani_copy(&temp, value);
  mani_smooth(&temp, this_str, this_ctr);
  mani_multiply(&temp, interp);
  mani_multiply(&temp, this_flatten);
  mani_copy(&result, &temp);

  mani_copy(&temp, value);
  mani_copy(&tinterp, interp);
  mani_smooth(&temp, next_str, next_ctr);
  mani_scale_const(&tinterp, -1);
  mani_offset_const(&tinterp, 1);
  mani_multiply(&temp, &tinterp);
  mani_multiply(&temp, next_flatten);

  mani_add(&result, &temp);
  mani_copy(value, &result);
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
    1, 0,
    0, 1,
    TR_FREQUENCY_SGEOFORMS * 1.3,
    salt
  );
  simplex_component(
    &temp,
    pos->x, pos->y,
    1, 0,
    0, 1,
    TR_FREQUENCY_SGEOFORMS * 2.4,
    salt
  );
  mani_scale_const(&temp, 0.7);
  mani_add(&flatten, &temp);
  mani_scale_const(&flatten, 1.0/1.7);
  mani_offset_const(&flatten, 1);
  mani_scale_const(&flatten, 0.5);

  if (region == TR_REGION_DEPTHS) {
    // amplify *bumps, *ridges, and *details; attenuate *hills and *mountains
    alter_value(mountains, tr_interp, 2.3, 0.6, 3.8, 0.75, &nofl, &nofl);
    alter_value(hills, tr_interp, 2.8, 0.7, 2.3, 0.65, &nofl, &nofl);
    alter_value(ridges, tr_interp, 4.9, 0.2, 3.7, 0.8, &nofl, &nofl);
    alter_value(mounds, tr_interp, 0, 1, 1.9, 0.6, &nofl, &nofl);
    alter_value(details, tr_interp, 2.8, 0.2, 2.8, 0.6, &nofl, &nofl);
    alter_value(bumps, tr_interp, 3.7, 0.3, 3.7, 0.8, &nofl, &nofl);
  } else if (region == TR_REGION_SHELF) {
    // gently attenuate most features; strongly attenuate *ridges:
    mani_scale_const(&flatten, 0.8);

    alter_value(mountains, tr_interp, 3.8, 0.75, 2.9, 0.6, &nofl, &flatten);
    alter_value(hills, tr_interp, 2.3, 0.65, 2.3, 0.7, &nofl, &flatten);
    alter_value(ridges, tr_interp, 3.7, 0.8, 1.9, 0.6, &nofl, &flatten);
    alter_value(mounds, tr_interp, 1.9, 0.6, 1.9, 0.6, &nofl, &flatten);
    alter_value(details, tr_interp, 2.8, 0.6, 2.0, 0.6, &nofl, &flatten);
    alter_value(bumps, tr_interp, 3.7, 0.8, 3.7, 0.9, &nofl, &flatten);
  } else if (region == TR_REGION_PLAINS) {
    // attenuate everything; create some superflat regions:
    mani_scale_const(&flatten, 0.8);

    alter_value(mountains, tr_interp, 2.9, 0.6, 0, 1, &flatten, &nofl);
    alter_value(hills, tr_interp, 2.3, 0.7, 2.8, 0.3, &flatten, &nofl);
    alter_value(ridges, tr_interp, 1.9, 0.6, 2.1, 0.35, &flatten, &nofl);
    alter_value(mounds, tr_interp, 1.9, 0.6, 0, 1, &flatten, &nofl);
    alter_value(details, tr_interp, 2.0, 0.6, 0, 1, &flatten, &nofl);
    alter_value(bumps, tr_interp, 3.7, 0.9, 0, 1, &flatten, &nofl);

  } else if (region == TR_REGION_HILLS) {
    // slightly accentuate *hills and *ridges:
    mani_smooth(&flatten, 1.9, 0.5);
    mani_scale_const(&flatten, 0.9);
    mani_offset_const(&flatten, 0.1);

    alter_value(mountains, tr_interp, 0, 1, 1.8, 0.3, &nofl, &nofl);
    alter_value(hills, tr_interp, 2.7, 0.3, 1.9, 0.55, &nofl, &nofl);
    alter_value(ridges, tr_interp, 2.1, 0.35, 2.4, 0.4, &nofl, &flatten);
    alter_value(mounds, tr_interp, 0, 1, 0, 1, &nofl, &flatten);

    mani_scale_const(&flatten, 0.5);
    mani_offset_const(&flatten, 0.25);
    alter_value(details, tr_interp, 0, 1, 0, 1, &nofl, &flatten);
    alter_value(bumps, tr_interp, 0, 1, 0, 1, &nofl, &flatten);

  } else if (region == TR_REGION_HIGHLANDS) {
    // accentuate *mountains; attenuate *hills and *ridges slightly; create some
    // flatter regions:
    mani_smooth(&flatten, 1.9, 0.5);
    mani_scale_const(&flatten, 0.9);
    mani_offset_const(&flatten, 0.1);

    alter_value(mountains, tr_interp, 1.8, 0.3, 2.9, 0.2, &nofl, &nofl);
    alter_value(hills, tr_interp, 1.9, 0.55, 2.6, 0.4, &nofl, &nofl);
    alter_value(ridges, tr_interp, 2.4, 0.4, 2.4, 0.15, &flatten, &nofl);
    alter_value(mounds, tr_interp, 0, 1, 1.9, 0.3, &flatten, &nofl);

    mani_scale_const(&flatten, 0.5);
    mani_offset_const(&flatten, 0.25);
    alter_value(details, tr_interp, 0, 1, 2.3, 0.4, &flatten, &nofl);
    alter_value(bumps, tr_interp, 0, 1, 2.3, 0.3, &flatten, &nofl);

  } else if (region == TR_REGION_MOUNTAINS) {
    // amplify most things:
    mani_smooth(mountains, 2.9, 0.2);
    mani_smooth(hills, 2.6, 0.4);
    mani_smooth(ridges, 2.4, 0.15);
    mani_smooth(mounds, 1.9, 0.3);
    mani_smooth(details, 2.3, 0.4);
    mani_smooth(bumps, 2.3, 0.3);
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
    1, 0,
    0, 1,
    TR_DIRT_NOISE_SCALE,
    salt
  );
  simplex_component(
    &temp,
    pos->x, pos->y,
    1, 0,
    0, 1,
    TR_DIRT_NOISE_SCALE * 2.5,
    salt
  );
  mani_scale_const(&temp, 0.5);
  mani_add(&var, &temp);

  mani_scale_const(&var, 1.0/1.5);
  mani_offset_const(&var, 1);
  mani_scale_const(&var, 0.5 * 0.4);
  mani_offset_const(&var, 0.6);

  // Base soil depth:
  mani_copy(result, &var);
  mani_scale_const(result, TR_BASE_SOIL_DEPTH);

  // Mountains and hills have less dirt on them in general:
  simplex_component(
    &temp,
    pos->x, pos->y,
    1, 0,
    0, 1,
    TR_DIRT_EROSION_SCALE,
    salt
  );
  mani_offset_const(&temp, 1);
  mani_scale_const(&temp, 0.5 * 0.5);
  mani_offset_const(&temp, 0.25);
  mani_scale_const(&temp, -TR_DIRT_MOUNTAIN_EROSION);
  mani_multiply(&temp, mountains);
  mani_scale_const(&temp, 1.0/TR_SCALE_MOUNTAINS);
  mani_add(result, &temp);

  simplex_component(
    &temp,
    pos->x, pos->y,
    1, 0,
    0, 1,
    TR_DIRT_EROSION_SCALE * 1.6,
    salt
  );
  mani_offset_const(&temp, 1);
  mani_scale_const(&temp, 0.5 * 0.5);
  mani_offset_const(&temp, 0.25);
  mani_scale_const(&temp, -TR_DIRT_HILL_EROSION);
  mani_multiply(&temp, hills);
  mani_scale_const(&temp, 1.0/TR_SCALE_HILLS);
  mani_add(result, &temp);

  // Altitude above/below sea level contributes to soil depth due to wind
  // erosion and marine snow (this is a stupidly oversimplified model of
  // course):
  mani_copy(&temp, rocks_height);
  mani_offset_const(&temp, -TR_HEIGHT_SEA_LEVEL);
  if (temp.z > 0) {
    mani_scale_const(&temp, 1.0/(TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL));
    mani_smooth(&temp, 3.1, 0.8);
  } else {
    mani_scale_const(&temp, 1.0/TR_HEIGHT_SEA_LEVEL);
    mani_smooth(&temp, 2.0, 0.5);
  }

  mani_scale_const(&temp, -TR_ALTITUDE_EROSION_STRENGTH);
  mani_add(result, &temp);

  // Steeper places (especially at high altitudes) have less dirt, while
  // flatter places have more (but small-scale slope components are ignored):
  mani_copy(&temp, rocks_height);
  mani_sub(&temp, details);
  mani_sub(&temp, bumps);
  steepness = mani_slope(&temp);
  steepness = fmin(1.5, steepness); // slopes above 1.5 are treated identically
  steepness /= 1.5;
  smooth(steepness, 2.8, 0.25);
  steepness -= 0.25;

  mani_scale_const(&temp, 1.0/TR_MAX_HEIGHT);
  mani_smooth(&temp, 2.4, 0.65);
  mani_scale_const(&temp, steepness * -TR_SLOPE_EROSION_STRENGTH);
    // Note: this is incorrect: for a correct erosion manifold you'd need to
    // know the second derivative of rocks_height, but we don't so we're
    // ignoring it. This means that the dirt_height won't have a completely
    // correct derivative, but for most applications, using the rocks_height
    // derivative should be satisfactory.
  mani_add(result, &temp);

  // Details and bumps in the stone aren't reflected in the dirt:
  // TODO: smoothness vs. roughness
  mani_sub(result, details);
  mani_sub(result, bumps);

  // Dirt depth can't be negative:
  if (result->z < 0) {
    result->z = 0;
    result->dx = 0;
    result->dy = 0;
  }

  // Convert to absolute dirt height:
  mani_add(result, rocks_height);
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

void terrain_cell(
  world_map *wm,
  world_region* neighborhood[],
  region_pos *rpos,
  cell *result
) {
  static manifold_point dontcare, stone_height, dirt_height;
  static region_pos pr_rpos = { .x = -1, .y = -1, .z = -1 };
  float h;
  world_region *best, *secondbest; // best and second-best regions
  float strbest, strsecond; // their respective strengths

  // DEBUG: (to show the strata)
  /*
  if (
    (
      abs(
        rpos->x -
        ((WORLD_WIDTH / 2.0) * WORLD_REGION_BLOCKS + 2*CHUNK_SIZE)
      ) < CHUNK_SIZE
    ) && (
      rpos->z > (
        rpos->y - (WORLD_HEIGHT/2.0) * WORLD_REGION_BLOCKS
      ) + 8000
      //rpos->z > (rpos->y - (WORLD_HEIGHT*WORLD_REGION_BLOCKS)/2)
    )
  ) {
  //if (abs(rpos->x - 32770) < CHUNK_SIZE) {
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
    return;
  }
  // */

  // DEBUG: Caves to show things off more:
  /*
  if (
    sxnoise_3d(rpos->x*1/12.0, rpos->y*1/12.0, rpos->z*1/12.0, 17) >
      sxnoise_3d(rpos->x*1/52.0, rpos->y*1/52.0, rpos->z*1/52.0, 18)
  ) {
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
    return;
  }
  // */

  if (pr_rpos.x != rpos->x || pr_rpos.y != rpos->y) {
    // No need to recompute the surface height if we're at the same x/y.
    // TODO: Get rid of double-caching here?
    compute_terrain_height(rpos, &dontcare, &stone_height, &dirt_height);
  }

  // Keep track of our previous position:
  copy_rpos(rpos, &pr_rpos);

  // Compute our fractional height:
  h = rpos->z / stone_height.z;

  // Figure out the nearest regions:
  compute_region_contenders(
    wm,
    neighborhood,
    rpos,
    &best, &secondbest, 
    &strbest, &strsecond
  );

  if (h <= 1.0) { // we're in the stone layers
    stone_cell(
      wm, rpos,
      h, stone_height.z,
      best, secondbest, strbest, strsecond,
      result
    );
  } else if (h <= dirt_height.z / stone_height.z) { // we're in dirt
    dirt_cell(
      wm, rpos,
      (rpos->z - stone_height.z) / (dirt_height.z - stone_height.z),
      dirt_height.z,
      best,
      result
    );
  } else if (rpos->z <= TR_HEIGHT_SEA_LEVEL) { // under the ocean
    result->primary = b_make_block(B_WATER);
    result->secondary = b_make_block(B_VOID);
  } else { // we're above the ground
    // TODO: HERE!
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
  }
}

void stone_cell(
  world_map *wm, region_pos *rpos,
  float h, float ceiling,
  world_region *best, world_region *secondbest, float strbest, float strsecond,
  cell *result
) {
  stratum *st;
  // Add some noise to distort the base height:
  // TODO: more spatial variance in noise strength?
  h += (TR_STRATA_FRACTION_NOISE_STRENGTH / ceiling) * (
    sxnoise_3d(
      rpos->x * TR_STRATA_FRACTION_NOISE_SCALE,
      rpos->y * TR_STRATA_FRACTION_NOISE_SCALE,
      rpos->z * TR_STRATA_FRACTION_NOISE_SCALE,
      7193
    ) + 
    0.5 * sxnoise_3d(
      rpos->x * TR_STRATA_FRACTION_NOISE_SCALE * 1.7,
      rpos->y * TR_STRATA_FRACTION_NOISE_SCALE * 1.7,
      rpos->z * TR_STRATA_FRACTION_NOISE_SCALE * 1.7,
      7194
    )
  ) / 1.5;
  // Clamp out-of-range values after noise:
  // TODO: some low-frequency un-clamped noise?
  if (h > 1.0) { h = 1.0; } else if (h < 0.0) { h = 0.0; }

  // Where available, persistence values are also a factor:
  if (best != NULL) {
    st = get_stratum(best, h);
    strbest *= st->persistence;
  }
  if (secondbest != NULL) {
    st = get_stratum(secondbest, h);
    strsecond *= st->persistence;
  }

 // Now that we know which stratum to use, set the cell's block data:
  if (strsecond > strbest) {
    best = secondbest;
  }
  if (best == NULL) {
    // TODO: Various edge types here?
    result->primary = b_make_block(B_STONE);
  } else {
    st = get_stratum(best, h);
    // TODO: veins and inclusions here!
    result->primary = b_make_species(B_STONE, st->base_species);
  }
}

void dirt_cell(
  world_map *wm, region_pos *rpos,
  float h,
  float elev,
  world_region *wr,
  cell *result
) {
  size_t i, max_alts;
  block soil_type;
  species soil_species;
  float beach_height, rel_h, str, beststr;
  float *alt_strengths;
  float *alt_hdeps;
  block *alt_blocks;
  species *alt_species;

  // compute beach height:
  beach_height = TR_BEACH_BASE_HEIGHT;
  beach_height += TR_BEACH_HEIGHT_VAR * sxnoise_2d(
    rpos->x * TR_BEACH_HEIGHT_NOISE_SCALE,
    rpos->y * TR_BEACH_HEIGHT_NOISE_SCALE,
    wr->seed + 18294
  );

  rel_h = elev - TR_HEIGHT_SEA_LEVEL + beach_height;
  beststr = TR_SOIL_ALT_THRESHOLD;
  if (
    rel_h > 0
  ||
    h < 1 - (-rel_h / TR_BEACH_HEIGHT_VAR)
    // TODO: Test this!
  ) { // TODO: lakes!
    soil_type = B_DIRT;
    soil_species = wr->climate.soil.base_dirt;
    alt_strengths = wr->climate.soil.alt_dirt_strengths;
    alt_hdeps = wr->climate.soil.alt_dirt_hdeps;
    alt_blocks = wr->climate.soil.alt_dirt_blocks;
    alt_species = wr->climate.soil.alt_dirt_species;
    max_alts = MAX_ALT_DIRTS;
  } else {
    soil_type = B_SAND;
    soil_species = wr->climate.soil.base_sand;
    alt_strengths = wr->climate.soil.alt_sand_strengths;
    alt_hdeps = wr->climate.soil.alt_sand_hdeps;
    alt_blocks = wr->climate.soil.alt_sand_blocks;
    alt_species = wr->climate.soil.alt_sand_species;
    max_alts = MAX_ALT_SANDS;
  }
  for (i = 0; i < max_alts; ++i) {
    // TODO: Moisture dependence?
    str = sxnoise_3d(
      rpos->x * TR_SOIL_ALT_NOISE_SCALE,
      rpos->y * TR_SOIL_ALT_NOISE_SCALE,
      rpos->z * TR_SOIL_ALT_NOISE_SCALE,
      wr->seed + 4920 * i
    ) + 0.7 * sxnoise_3d(
      rpos->x * TR_SOIL_ALT_NOISE_SCALE * 2.4,
      rpos->y * TR_SOIL_ALT_NOISE_SCALE * 2.4,
      rpos->z * TR_SOIL_ALT_NOISE_SCALE * 2.4,
      wr->seed + 7482 * i
    ) + 0.3 * sxnoise_3d(
      rpos->x * TR_SOIL_ALT_NOISE_SCALE * 3.8,
      rpos->y * TR_SOIL_ALT_NOISE_SCALE * 3.8,
      rpos->z * TR_SOIL_ALT_NOISE_SCALE * 3.8,
      wr->seed + 3194 * i
    );
    str = (2 + str)/4.0; // [0, 1]
    str *= alt_strengths[i];
    if (alt_hdeps[i] > 0) {
      str *= pow(h, alt_hdeps[i]);
    } else {
      str *= pow((1 - h), -alt_hdeps[i]);
    }
    if (str > beststr) {
      beststr = str;
      soil_type = alt_blocks[i];
      soil_species = alt_species[i];
    }
  }
  result->primary = b_make_species(soil_type, soil_species);
}
