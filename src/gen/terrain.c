// terrain.c
// Code for determining terrain height.

#include <math.h>

#include "world/blocks.h"
#include "world/world.h"
#include "world/world_map.h"
#include "datatypes/heightmap.h"
#include "noise/noise.h"

#include "util.h"

#include "geology.h"
// DEBUG:
#include "worldgen.h"

#include "terrain.h"

/***********
 * Globals *
 ***********/

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

uint8_t const TR_DIRECTION_PERMUTATIONS[24] = {
  0, 1, 2, 3,
  0, 1, 3, 2,
  0, 2, 1, 3,
  0, 2, 3, 1,
  0, 3, 1, 2,
  0, 3, 2, 1,

  1, 0, 2, 3,
  1, 0, 3, 2,
  1, 2, 0, 3,
  1, 2, 3, 0,
  1, 3, 0, 2,
  1, 3, 2, 0,

  2, 0, 1, 3,
  2, 0, 3, 1,
  2, 1, 0, 3,
  2, 1, 3, 0,
  2, 3, 0, 1,
  2, 3, 1, 0,

  3, 0, 1, 2,
  3, 0, 2, 1,
  3, 1, 0, 2,
  3, 1, 2, 0,
  3, 2, 0, 1,
  3, 2, 1, 0
};

/*********************
 * Private Functions *
 *********************/

// A helper function for computing the flat array index of a world region.
static inline wm_pos_t _ar_idx(world_region *wr) {
  return wr->pos.x + wr->world.width * wr->pos.y;
}

float _render_tectonics(
  heightmap *hm,
  size_t x,
  size_t y,
  float ignore,
  void *v_ts
) {
  float fx, fy;
  tectonic_sheet *ts = (tectonic_sheet*) v_ts;

  fx = (x / (float) (ts->width - 1));
  fy = (y / (float) (ts->height - 1));

  // Crop the edges of the tectonic sheet to avoid the distorted edges.
  fx = 0.5 + (0.5 - fx) * TR_TECT_CROP;
  fy = 0.5 + (0.5 - fy) * TR_TECT_CROP;

  // Get tectonic sheet height at this point:
  return sheet_height(ts, fx, fy);
}

float _get_modulation(
  heightmap *hm,
  size_t x,
  size_t y,
  float ignore,
  void *v_seed
) {
  ptrdiff_t seed = (ptrdiff_t) v_seed;
  float result;
  float fx, fy;

  fx = ((float) x) / ((float) hm->width);
  fy = ((float) y) / ((float) hm->width); // intentional

  return (
    sxnoise_2d(
      TR_BUILD_MODULATION_LARGE_SCALE * fx,
      TR_BUILD_MODULATION_LARGE_SCALE * fy,
      seed
    )
  +
    sxnoise_2d(
      TR_BUILD_MODULATION_SMALL_SCALE * fx,
      TR_BUILD_MODULATION_SMALL_SCALE * fy,
      seed
    ) * TR_BUILD_MODULATION_SMALL_STR
  );
}

/*************
 * Functions *
 *************/

void setup_terrain_gen() {
  omp_init_lock(&TERRAIN_HEIGHT_LOCK);
}

uint8_t run_particle(
  heightmap *hm,
  float height,
  size_t slip,
  size_t max_steps,
  ptrdiff_t seed
) {
  size_t px, py;
  size_t i, j, idx, ndir, nx, ny;
  ptrdiff_t dperm; // the order in which we'll check the different directions
  uint8_t valid;

  seed = prng(seed + 3451);

  // Start our particle at a random point that's lower than it (give up after a
  // few tries though):
  i = 0;
  while (!valid && i < TR_PARTICLE_START_TRIES) {
    px = posmod(seed, hm->width);
    seed = prng(seed);
    py = posmod(seed, hm->height);
    seed = prng(seed);
    if (hm_height(hm, px, py) < height) { valid = 1; }
    i += 1;
  }
  if (!valid) { // We didn't find a valid place to start this particle.
    return 0;
  }

  for (i = 0; i < max_steps; ++i) {
    idx = hm_idx(hm, px, py);
    if (px > 0 && hm->data[idx - 1] >= height) {
      if (slip <= 0) { break; }
      slip -= 1;
    }
    if (px < hm->width - 1 && hm->data[idx + 1] >= height) {
      if (slip <= 0) { break; }
      slip -= 1;
    }
    if (py > 0 && hm->data[idx - hm->width] >= height) {
      if (slip <= 0) { break; }
      slip -= 1;
    }
    if (py < hm->height - 1 && hm->data[idx + hm->width] >= height) {
      if (slip <= 0) { break; }
      slip -= 1;
    }

    nx = px;
    ny = py;
    valid = 0;
    // there are 24 orders to check directions in:
    dperm = posmod(seed + dperm, 24);
    seed = prng(seed);

    for (j = 0; j < 4; ++j) {
      ndir = TR_DIRECTION_PERMUTATIONS[dperm * 4 + j];
      if (ndir == 0) {
        nx = px - 1;
        ny = py;
      } else if (ndir == 1) {
        nx = px + 1;
        ny = py;
      } else if (ndir == 2) {
        nx = px;
        ny = py - 1;
      } else if (ndir == 3) {
        nx = px;
        ny = py + 1;
#ifdef DEBUG
      } else {
        fprintf(stderr, "Bad direction!");
        exit(1);
#endif
      }
      if (nx < 0 || nx > hm->width - 1 || ny < 0 || ny > hm->height - 1) {
        continue;
      }
      idx = hm_idx(nx, ny);
      if (hm->data[idx] < height) {
        valid = 1;
        break;
      }
    }

    if (!valid) {
      break;
    }
    px = nx;
    py = ny;
  }
  idx = hm_idx(hm, px, py);
  if (hm->data[idx] < height) {
    hm->data[idx] = height;
    return 1;
  } else {
    return 0;
  }
}

void generate_topology(world_map *wm) {
  world_map_pos xy;
  world_region *wr;
  tectonic_sheet *ts;
  size_t i, j;
  float fx, fy;
  float tmax, tmin;
  float z;
  float pth, pcount, pgrowth;
  heightmap *topo, *modulation, *precipitation, *flow, *tmp, *save;
  ptrdiff_t seed;

  ts = wm->tectonics;
  seed = prng(wm->seed + 9164);

  topo = create_heightmap(wm->width, wm->height);
  modulation = create_heightmap(wm->width, wm->height);
  precipitation = create_heightmap(wm->width, wm->height);
  flow = create_heightmap(wm->width, wm->height);
  tmp = create_heightmap(wm->width, wm->height);
  save = create_heightmap(wm->width, wm->height);

  tmax = sheet_height(ts, 0, 0);
  tmin = tmax;

  // First, render our tectonic sheet into our topo heightmap and normalize it:
  hm_process(topo, (void*) ts, &_render_tectonics);
  hm_normalize(topo);

  // Now we start adding particles:
  pcount = TR_BUILD_WAVE_SIZE;
  pgrowth = TR_BUILD_WAVE_GROWTH;
  pth = TR_BUILD_STARTING_HEIGHT;
  for (i = 0; i < TR_BUILD_WAVE_COUNT; ++i) {
    for (j = 0; j < pcount; ++j) {
      run_particle(topo, pth, TR_BUILD_SLIP, TR_BUILD_MAX_WANDER, seed);
      seed = prng(seed);
    }
    pth *= TR_BUILD_HEIGHT_FALLOFF;
    pcount += pgrowth;
    pgrowth += TR_BUILD_WAVE_COMPOUND;
  }

  // We want to slump things out, but at each step we mix back in a little of
  // the original topology.
  for (i = 0; i < TR_BUILD_SLUMP_STEPS; ++i) {
    hm_copy(topo, save);
    hm_slump(topo, tmp, TR_BUILD_SLUMP_MAX_SLOPE, TR_BUILD_SLUMP_RATE);
    hm_average(topo, save, TR_BUILD_SLUMP_SAVE_MIX);
  }

  // Finally do a bit of pure slumping:
  for (i = 0; i < TR_BUILD_FINAL_SLUMPS; ++i) {
    hm_slump(topo, tmp, TR_BUILD_SLUMP_MAX_SLOPE, TR_BUILD_SLUMP_RATE);
  }

  // TODO: Is this fine?
  // We invent completely uniform precipitation to drive erosion:
  hm_offset(precipitation, 0.5);

  // Next we get some modulation to create differential erosion in different
  // areas of the world:
  hm_process(modulation, (void*) seed, &_get_modulation);
  seed = prng(seed);
  hm_normalize(modulation);

  // And we do a round of erosion:
  for (i = 0; i < TR_BUILD_EROSION_STEPS; ++i) {
    hm_reset(flow);
    hm_reset(save);
    hm_reset(tmp);
    hm_erode(
      topo,
      modulation,
      precipitation,
      flow,
      save,
      tmp,
      TR_BUILD_EROSION_FLOW_STEPS,
      TR_BUILD_EROSION_FLOW_SLUMP_STEPS,
      TR_BUILD_EROSION_FLOW_MAXSLOPE,
      TR_BUILD_EROSION_FLOW_SLUMP_RATE,
      TR_BUILD_EROSION_STR,
      TR_BUILD_EROSION_MODULATION
    );
  }

  // Now do another round of erosion with a different modulation array:
  hm_process(modulation, (void*) seed, &_get_modulation);
  seed = prng(seed);
  hm_normalize(modulation);
  for (i = 0; i < TR_BUILD_EROSION_STEPS; ++i) {
    hm_reset(flow);
    hm_reset(save);
    hm_reset(tmp);
    hm_erode(
      topo,
      modulation,
      precipitation,
      flow,
      save,
      tmp,
      TR_BUILD_EROSION_FLOW_STEPS,
      TR_BUILD_EROSION_FLOW_SLUMP_STEPS,
      TR_BUILD_EROSION_FLOW_MAXSLOPE,
      TR_BUILD_EROSION_FLOW_SLUMP_RATE,
      TR_BUILD_EROSION_STR,
      TR_BUILD_EROSION_MODULATION
    );
  }

  // At this point our heightmap is complete, we just need to copy values over
  // into our topography information:
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      wr->topology.terrain_height.z = hm_height(topo, xy.x, xy.y);
    }
  }

  // Now that we've updated our height information, let's build an empirical
  // manifold with proper slope information:
  compute_manifold(wm);

  // Free our temporary processing arrays:
  cleanup_heightmap(topo);
  cleanup_heightmap(modulation);
  cleanup_heightmap(precipitation);
  cleanup_heightmap(flow);
  cleanup_heightmap(tmp);
  cleanup_heightmap(save);
}


void compute_terrain_height(
  global_pos *pos,
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

void compute_dirt_height(
  global_pos *pos, ptrdiff_t *salt,
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
  global_pos *pos,
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
  global_pos *glpos,
  cell *result
) {
  static manifold_point dontcare, stone_height, dirt_height;
  static global_pos pr_glpos = { .x = -1, .y = -1, .z = -1 };
  float h;
  world_region *best, *secondbest; // best and second-best regions
  float strbest, strsecond; // their respective strengths

  // DEBUG: (to show the strata)
  //*
  if (
    (
      abs(
        glpos->x -
        ((WORLD_WIDTH / 2.0) * WORLD_REGION_BLOCKS + 2*CHUNK_SIZE)
      ) < CHUNK_SIZE
    ) && (
      glpos->z > (
        glpos->y - (WORLD_HEIGHT/2.0) * WORLD_REGION_BLOCKS
      ) + 8000
      //glpos->z > (glpos->y - (WORLD_HEIGHT*WORLD_REGION_BLOCKS)/2)
    )
  ) {
  //if (abs(glpos->x - 32770) < CHUNK_SIZE) {
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
    return;
  }
  // */

  // DEBUG: Caves to show things off more:
  /*
  if (
    sxnoise_3d(glpos->x*1/12.0, glpos->y*1/12.0, glpos->z*1/12.0, 17) >
      sxnoise_3d(glpos->x*1/52.0, glpos->y*1/52.0, glpos->z*1/52.0, 18)
  ) {
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
    return;
  }
  // */

  if (pr_glpos.x != glpos->x || pr_glpos.y != glpos->y) {
    // No need to recompute the surface height if we're at the same x/y.
    // TODO: Get rid of double-caching here?
    compute_terrain_height(glpos, &dontcare, &stone_height, &dirt_height);
  }

  // Keep track of our previous position:
  copy_glpos(glpos, &pr_glpos);

  // Compute our fractional height:
  h = glpos->z / stone_height.z;

  // Figure out the nearest regions:
  compute_region_contenders(
    wm,
    neighborhood,
    glpos,
    &best, &secondbest, 
    &strbest, &strsecond
  );

  if (h <= 1.0) { // we're in the stone layers
    stone_cell(
      wm, glpos,
      h, stone_height.z,
      best, secondbest, strbest, strsecond,
      result
    );
  } else if (h <= dirt_height.z / stone_height.z) { // we're in dirt
    dirt_cell(
      wm, glpos,
      (glpos->z - stone_height.z) / (dirt_height.z - stone_height.z),
      dirt_height.z,
      best,
      result
    );
  } else if (glpos->z <= TR_HEIGHT_SEA_LEVEL) { // under the ocean
    result->primary = b_make_block(B_WATER);
    result->secondary = b_make_block(B_VOID);
  } else { // we're above the ground
    // TODO: HERE!
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
  }
}

void stone_cell(
  world_map *wm, global_pos *glpos,
  float h, float ceiling,
  world_region *best, world_region *secondbest, float strbest, float strsecond,
  cell *result
) {
  stratum *st;
  // Add some noise to distort the base height:
  // TODO: more spatial variance in noise strength?
  h += (TR_STRATA_FRACTION_NOISE_STRENGTH / ceiling) * (
    sxnoise_3d(
      glpos->x * TR_STRATA_FRACTION_NOISE_SCALE,
      glpos->y * TR_STRATA_FRACTION_NOISE_SCALE,
      glpos->z * TR_STRATA_FRACTION_NOISE_SCALE,
      7193
    ) + 
    0.5 * sxnoise_3d(
      glpos->x * TR_STRATA_FRACTION_NOISE_SCALE * 1.7,
      glpos->y * TR_STRATA_FRACTION_NOISE_SCALE * 1.7,
      glpos->z * TR_STRATA_FRACTION_NOISE_SCALE * 1.7,
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
  world_map *wm, global_pos *glpos,
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
    glpos->x * TR_BEACH_HEIGHT_NOISE_SCALE,
    glpos->y * TR_BEACH_HEIGHT_NOISE_SCALE,
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
    max_alts = WM_MAX_ALT_DIRTS;
  } else {
    soil_type = B_SAND;
    soil_species = wr->climate.soil.base_sand;
    alt_strengths = wr->climate.soil.alt_sand_strengths;
    alt_hdeps = wr->climate.soil.alt_sand_hdeps;
    alt_blocks = wr->climate.soil.alt_sand_blocks;
    alt_species = wr->climate.soil.alt_sand_species;
    max_alts = WM_MAX_ALT_SANDS;
  }
  for (i = 0; i < max_alts; ++i) {
    // TODO: Moisture dependence?
    str = sxnoise_3d(
      glpos->x * TR_SOIL_ALT_NOISE_SCALE,
      glpos->y * TR_SOIL_ALT_NOISE_SCALE,
      glpos->z * TR_SOIL_ALT_NOISE_SCALE,
      wr->seed + 4920 * i
    ) + 0.7 * sxnoise_3d(
      glpos->x * TR_SOIL_ALT_NOISE_SCALE * 2.4,
      glpos->y * TR_SOIL_ALT_NOISE_SCALE * 2.4,
      glpos->z * TR_SOIL_ALT_NOISE_SCALE * 2.4,
      wr->seed + 7482 * i
    ) + 0.3 * sxnoise_3d(
      glpos->x * TR_SOIL_ALT_NOISE_SCALE * 3.8,
      glpos->y * TR_SOIL_ALT_NOISE_SCALE * 3.8,
      glpos->z * TR_SOIL_ALT_NOISE_SCALE * 3.8,
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
