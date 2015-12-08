// terrain.c
// Code for determining terrain height.

#include <math.h>
#include <stdio.h>
#ifdef DEBUG
  #include <stdlib.h>
#endif

#include "world/blocks.h"
#include "world/world.h"
#include "world/world_map.h"
#include "datatypes/heightmap.h"
#include "noise/noise.h"

#ifdef DEBUG
  #include "txgen/cartography.h"
#endif

#include "util.h"

#include "geology.h"
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

// All possible permutations of the numbers 0, 1, 2, 3
uint8_t const TR_DIRECTION_PERMUTATIONS[24*4] = {
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
  return wr->pos.x + wr->world->width * wr->pos.y;
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

  fx = (x / (float) (hm->width - 1));
  fy = (y / (float) (hm->height - 1));

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
  dperm = 0;

  // Start our particle at a random point that's lower than it (give up after a
  // few tries though):
  i = 0;
  valid = 0;
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
    seed = prng((seed >> 4) ^ seed); // amp low-bits variability a little

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
      idx = hm_idx(hm, nx, ny);
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

void generate_topography(world_map *wm) {
  world_map_pos xy;
  world_region *wr;
  tectonic_sheet *ts;
  size_t i, j, pcount, pgrowth;
  float pth;
  heightmap *topo, *modulation, *precipitation, *flow, *tmp, *save;
  ptrdiff_t seed;
#ifdef DEBUG
  texture *tx;
#endif

  ts = wm->tectonics;
  seed = prng(wm->seed + 9164);

  topo = create_heightmap(wm->width, wm->height);
  modulation = create_heightmap(wm->width, wm->height);
  precipitation = create_heightmap(wm->width, wm->height);
  flow = create_heightmap(wm->width, wm->height);
  tmp = create_heightmap(wm->width, wm->height);
  save = create_heightmap(wm->width, wm->height);

  // First, render our tectonic sheet into our topo heightmap and normalize it:
  printf("    ...rendering tectonics...\n");
  hm_process(topo, (void*) ts, &_render_tectonics);
  printf("    ...done rendering tectonics...\n");
  hm_normalize(topo);

#ifdef DEBUG
  tx = create_texture(wm->width, wm->height);
  render_heightmap(topo, tx, 0);
  write_texture_to_png(tx, "out/topography_tectonics.png");
  cleanup_texture(tx);
#endif

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
    printf(
      "    ...%zu / %zu particle waves done...\r",
      i + 1,
      (size_t) TR_BUILD_WAVE_COUNT
    );
  }
  printf("\n");

#ifdef DEBUG
  tx = create_texture(wm->width, wm->height);
  render_heightmap(topo, tx, 0);
  write_texture_to_png(tx, "out/topography_with_particles.png");
  cleanup_texture(tx);
#endif

  // We want to slump things out, but at each step we mix back in a little of
  // the original topography.
  for (i = 0; i < TR_BUILD_SLUMP_STEPS; ++i) {
    hm_copy(topo, save);
    hm_reset(tmp);
    hm_slump(topo, tmp, TR_BUILD_SLUMP_MAX_SLOPE, TR_BUILD_SLUMP_RATE);
    hm_average(topo, save, TR_BUILD_SLUMP_SAVE_MIX);
  }

  // Finally do a bit of pure slumping:
  for (i = 0; i < TR_BUILD_FINAL_SLUMPS; ++i) {
    hm_reset(tmp);
    hm_slump(topo, tmp, TR_BUILD_SLUMP_MAX_SLOPE, TR_BUILD_SLUMP_RATE);
  }

#ifdef DEBUG
  tx = create_texture(wm->width, wm->height);
  render_heightmap(topo, tx, 0);
  write_texture_to_png(tx, "out/topography_slumped.png");
  cleanup_texture(tx);
#endif

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
  hm_normalize(topo);
  hm_normalize(flow);

#ifdef DEBUG
  tx = create_texture(wm->width, wm->height);
  render_heightmap(topo, tx, 0);
  write_texture_to_png(tx, "out/topography_eroded.png");

  render_heightmap(flow, tx, 0);
  write_texture_to_png(tx, "out/flow.png");
  cleanup_texture(tx);
#endif

  // At this point our heightmap is complete, we just need to copy values over
  // into our topography information:
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      wr->topography.terrain_height.z = hm_height(topo, xy.x, xy.y);
      wr->topography.flow_potential = hm_height(flow, xy.x, xy.y);
    }
  }

  // Finally, remap everything from [0, 1] onto full world heights:
  geomap_topography(wm);

  // Now that we've updated our height information, let's build an empirical
  // manifold with proper slope information (slope will be in blocks per
  // block):
  compute_manifold(wm);

  // Free our temporary processing arrays:
  cleanup_heightmap(topo);
  cleanup_heightmap(modulation);
  cleanup_heightmap(precipitation);
  cleanup_heightmap(flow);
  cleanup_heightmap(tmp);
  cleanup_heightmap(save);
}


void geomap_topography(world_map *wm) {
  world_map_pos xy;
  world_region *wr;
  manifold_point result, ignore;
  terrain_region dontcare;

  for (xy.x = 0; xy.x < wm->width; xy.x += 1) {
    for (xy.y = 0; xy.y < wm->height; xy.y += 1) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      geomap(&(wr->topography.terrain_height), &result, &dontcare, &ignore);
      mani_copy_as(&(wr->topography.terrain_height), &result);
      // TODO: Separate heights here?
      wr->topography.geologic_height = wr->topography.terrain_height.z;
    }
  }
}

void compute_terrain_height(
  world_map *wm,
  global_pos *pos,
  manifold_point *r_gross,
  manifold_point *r_rocks,
  manifold_point *r_dirt
) {
  static int xcache = 3, ycache = 7;
  static manifold_point hills, ridges, mounds;
  static manifold_point gross_height, rocks_height, dirt_height;
  static world_region* neighborhood[25];
  static manifold_point interp_values[25];

  world_map_pos wmpos;
  ptrdiff_t seed;
  size_t i;
  float divisor;

  manifold_point tmp;
  manifold_point dstx, dsty;

  if (xcache == pos->x && ycache == pos->y) {
    // no need to recompute everything:
    // printf("cached heights: %.2f, %.2f\n\n", rocks_height.z, dirt_height.z);
    mani_copy_as(r_gross, &gross_height);
    mani_copy_as(r_rocks, &rocks_height);
    mani_copy_as(r_dirt, &dirt_height);
  }
  // beyond this point we can't let multiple threads run at once as they'd mess
  // up each others' static variables.
  omp_set_lock(&TERRAIN_HEIGHT_LOCK);

  seed = prng(wm->seed + 54621);
  glpos__wmpos(pos, &wmpos);
  
  xcache = pos->x; ycache = pos->y;

  // Reset gross height:
  gross_height.z = 0;
  gross_height.dx = 0;
  gross_height.dy = 0;
  // Interpolate region terrain_height values to get a gross height:
  get_world_neighborhood(wm, &wmpos, neighborhood);
  compute_region_interpolation_values(wm, neighborhood, pos, interp_values);
  divisor = 0;
  for (i = 0; i < 25; ++i) {
    if (neighborhood[i] != NULL) {
      mani_copy_as(&tmp, &(neighborhood[i]->topography.terrain_height));
      mani_multiply(&tmp, &(interp_values[i]));
      mani_add(&gross_height, &tmp);
      divisor += interp_values[i].z;
    }
  }
#ifdef DEBUG
  if (divisor == 0) {
    fprintf(stderr, "Sum of interpolation values is zero!\n");
    exit(1);
  }
#endif
  // TODO: Should the divisor be a manifold?
  mani_scale_const(&gross_height, 1.0 / divisor);

  // gross_height now holds the interpolated terrain height

  // Compute hill, ridge, and mound manifolds:
  get_standard_distortion(
    pos->x, pos->y, &seed,
    TR_DFREQ_HILLS,
    TR_DSCALE_HILLS,
    &dstx,
    &dsty
  );
  simplex_component(
    &hills,
    pos->x + dstx.z, pos->y + dsty.z,
    dstx.dx, dstx.dy,
    dsty.dx, dsty.dy,
    TR_NFREQ_HILLS,
    &seed
  );

  get_standard_distortion(
    pos->x, pos->y, &seed,
    TR_DFREQ_RIDGES,
    TR_DSCALE_RIDGES,
    &dstx,
    &dsty
  );
  worley_component(
    &ridges,
    pos->x + dstx.z, pos->y + dsty.z,
    dstx.dx, dstx.dy,
    dsty.dx, dsty.dy,
    TR_NFREQ_RIDGES,
    &seed,
    WORLEY_FLAG_INCLUDE_NEXTBEST
  ); // TODO: Adjust flags here

  get_standard_distortion(
    pos->x, pos->y, &seed,
    TR_DFREQ_MOUNDS,
    TR_DSCALE_MOUNDS,
    &dstx,
    &dsty
  );
  simplex_component(
    &mounds,
    pos->x + dstx.z, pos->y + dsty.z,
    dstx.dx, dstx.dy,
    dsty.dx, dsty.dy,
    TR_NFREQ_MOUNDS,
    &seed
  );

  // Compute rocks height by adding hills, ridges, and mounds to gross height:
  // TODO: interpolate scaling factors between world regions
  mani_copy_as(&rocks_height, &gross_height);

  mani_scale_const(&hills, TR_SCALE_HILLS/2); // base is in [-1, 1]
  mani_add(&rocks_height, &hills);

  mani_scale_const(&ridges, TR_SCALE_RIDGES); // base is in [0, 1]
  mani_add(&rocks_height, &ridges);

  mani_scale_const(&mounds, TR_SCALE_MOUNDS/2); // base is in [-1, 1]
  mani_add(&rocks_height, &mounds);

  // Figure out our soil depth:
  compute_dirt_height(
    pos, &seed,
    &rocks_height,
    //&hills, &ridges, &mounds
    &dirt_height
  );

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
  mani_copy_as(r_gross, &gross_height);
  mani_copy_as(r_rocks, &rocks_height);
  mani_copy_as(r_dirt, &dirt_height);

  // Now we can release the lock and let another thread compute some terrain
  // height.
  omp_unset_lock(&TERRAIN_HEIGHT_LOCK);
}

void compute_dirt_height(
  global_pos *pos, ptrdiff_t *seed,
  manifold_point *rocks_height,
  // TODO: Do we want/need these?
  //manifold_point *hills, manifold_point *ridges, manifold_point *mounds,
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
    seed
  );
  simplex_component(
    &temp,
    pos->x, pos->y,
    1, 0,
    0, 1,
    TR_DIRT_NOISE_SCALE * 2.5,
    seed
  );
  mani_scale_const(&temp, 0.5);
  mani_add(&var, &temp);

  mani_scale_const(&var, 1.0/1.5);
  mani_offset_const(&var, 1);
  mani_scale_const(&var, 0.5 * 0.4);
  mani_offset_const(&var, 0.6);

  // Base soil depth:
  mani_copy_as(result, &var);
  mani_scale_const(result, TR_BASE_SOIL_DEPTH);

  // Mountains and hills have less dirt on them in general:
  // TODO: THIS!
  /*
  simplex_component(
    &temp,
    pos->x, pos->y,
    1, 0,
    0, 1,
    TR_DIRT_EROSION_SCALE,
    seed
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
    seed
  );
  mani_offset_const(&temp, 1);
  mani_scale_const(&temp, 0.5 * 0.5);
  mani_offset_const(&temp, 0.25);
  mani_scale_const(&temp, -TR_DIRT_HILL_EROSION);
  mani_multiply(&temp, hills);
  mani_scale_const(&temp, 1.0/TR_SCALE_HILLS);
  mani_add(result, &temp);
  */

  // Altitude above/below sea level contributes to soil depth due to wind
  // erosion and marine snow (this is a stupidly oversimplified model of
  // course):
  mani_copy_as(&temp, rocks_height);
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
  mani_copy_as(&temp, rocks_height);
  /*
   * TODO: THIS?
  mani_sub(&temp, details);
  mani_sub(&temp, bumps);
  */
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
  /*
   * TODO: THIS?
  mani_sub(result, details);
  mani_sub(result, bumps);
  */

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
  // TODO: Something about this!
  *region = TR_REGION_PLAINS;
  *tr_interp = 0.5;
  return;
  /*
  static int xcache = 3, ycache = 7;
  static manifold_point my_tr_interp;
  static terrain_region my_region;
  manifold_point dontcare;
  ptrdiff_t seed = TR_NOISE_SALT;

  if (xcache == pos->x && ycache == pos->y) {
    // no need to recompute everything:
    *region = my_region;
    *tr_interp = my_tr_interp.z;
    return;
  }
  xcache = pos->x; ycache = pos->y;

  compute_base_geoforms(
    pos, &seed,
    &dontcare, &dontcare, &dontcare,
    &dontcare, &dontcare,
    &dontcare, &dontcare,
    &dontcare, &dontcare, &dontcare,
    &dontcare, // &base,
    &my_region,
    &my_tr_interp,
    &dontcare
  );
  *region = my_region;
  *tr_interp = my_tr_interp.z;
  */
}

void terrain_cell(
  world_map *wm,
  world_region* neighborhood[],
  global_pos *glpos,
  cell *result
) {
  manifold_point gross_height, stone_height, dirt_height;
  float h;
  world_region *best, *secondbest; // best and second-best regions
  float strbest, strsecond; // their respective strengths

  // compute_terrain_height handles caching:
  compute_terrain_height(wm, glpos, &gross_height, &stone_height, &dirt_height);

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
    result->blocks[0] = b_make_block(B_AIR);
    result->blocks[1] = b_make_block(B_VOID);
    return;
  }
  // */

  // DEBUG: Caves to show things off more:
  //*
  if (
    sxnoise_3d(glpos->x*1/12.0, glpos->y*1/12.0, glpos->z*1/12.0, 17) >
      sxnoise_3d(glpos->x*1/52.0, glpos->y*1/52.0, glpos->z*1/52.0, 18)
  ) {
    result->blocks[0] = b_make_block(B_AIR);
    result->blocks[1] = b_make_block(B_VOID);
    return;
  }
  // */

  // Compute our fractional height:
  h = glpos->z / stone_height.z;

  // Figure out the nearest regions:
  compute_region_contenders(
    wm,
    neighborhood,
    glpos,
    1788111,
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
    result->blocks[0] = b_make_block(B_WATER);
    result->blocks[1] = b_make_block(B_VOID);
  } else { // we're above the ground
    // TODO: Add more here?
    result->blocks[0] = b_make_block(B_AIR);
    result->blocks[1] = b_make_block(B_VOID);
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
  // TODO: get geologic_height in here!
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
    result->blocks[0] = b_make_block(B_STONE);
  } else {
    st = get_stratum(best, h);
    // TODO: veins and inclusions here!
    result->blocks[0] = b_make_species(B_STONE, st->base_species);
  }
  result->blocks[1] = b_make_block(B_VOID);
}

void dirt_cell(
  world_map *wm,
  global_pos *glpos,
  float h,
  float elev,
  world_region *wr,
  cell *result
) {
  float beach_height, beach_top, beach_roots;
  int topsoil = 0;

  if (wr == NULL) {
    // TODO: better fallback for edges
    result->blocks[0] = b_make_block(B_DIRT);
    result->blocks[1] = b_make_block(B_VOID);
    return;
  }

  // compute beach height:
  beach_height = TR_BEACH_BASE_HEIGHT;
  beach_height += TR_BEACH_HEIGHT_VAR * sxnoise_2d(
    glpos->x * TR_BEACH_HEIGHT_NOISE_SCALE,
    glpos->y * TR_BEACH_HEIGHT_NOISE_SCALE,
    wr->seed + 18294
  );

  beach_top = TR_HEIGHT_SEA_LEVEL + beach_height;
  beach_roots = TR_HEIGHT_SEA_LEVEL - beach_height * 1.5;

  topsoil = elev - glpos->z <= 1.0;

  if (
    glpos->z > beach_top // above the top of the beach
  ||
    glpos->z < beach_roots
    // TODO: Test this!
  ) { // TODO: lakes and rivers!
    if (glpos->z < TR_HEIGHT_SEA_LEVEL) {
      if (topsoil) {
        pick_dirt_block(
          wr,
          glpos,
          h,
          &(wr->climate.soil.ocean_floor_topsoil),
          result
        );
      } else {
        pick_dirt_block(
          wr,
          glpos,
          h,
          &(wr->climate.soil.ocean_floor),
          result
        );
      }
    } else {
      if (topsoil) {
        pick_dirt_block(
          wr,
          glpos,
          h,
          &(wr->climate.soil.top_soil),
          result
        );
      } else {
        pick_dirt_block(
          wr,
          glpos,
          h,
          &(wr->climate.soil.base_soil),
          result
        );
      }
    }
  } else {
    if (topsoil) {
      pick_dirt_block(
        wr,
        glpos,
        h,
        &(wr->climate.soil.beach_topsoil),
        result
      );
    } else {
      pick_dirt_block(
        wr,
        glpos,
        h,
        &(wr->climate.soil.beaches),
        result
      );
    }
  }
}

void pick_dirt_block(
  world_region *wr,
  global_pos *glpos,
  float h,
  soil_type *soil,
  cell *result
) {
  size_t i;

  block soil_type = soil->main_block_type;
  species soil_species = soil->main_species;
  block *alt_blocks = soil->alt_block_types;
  species *alt_species = soil->alt_species;
  float *alt_strengths = soil->alt_strengths;
  float *alt_hdeps = soil->alt_hdeps;

  float beststr = TR_SOIL_ALT_THRESHOLD;
  float str = 0;

  for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
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
      str *= 1.0 + 0.4 * pow(h * alt_hdeps[i], 1 - (alt_hdeps[i]/3.0));
    } else if (alt_hdeps[i] < 0) {
      str *= 1.0 + 0.4 * pow((1 - h) * -alt_hdeps[i], 1 - (-alt_hdeps[i]/3.0));
    } // do nothing if alt_hdeps[i] == 0
    if (str > beststr) {
      beststr = str;
      soil_type = alt_blocks[i];
      soil_species = alt_species[i];
    }
  }
  result->blocks[0] = b_make_species(soil_type, soil_species);
  result->blocks[1] = b_make_block(B_VOID);
}
