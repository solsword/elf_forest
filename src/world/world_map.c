// world_map.c
// World map structure definition.

#include "noise/noise.h"

#include "gen/terrain.h"
#include "gen/geology.h"
#include "gen/climate.h"
#include "gen/biology.h"

#include "world_map.h"

/***********
 * Globals *
 ***********/

// The globally-accessible world:
world_map* THE_WORLD;

/******************************
 * Constructors & Destructors *
 ******************************/

world_map *create_world_map(ptrdiff_t seed, wm_pos_t width, wm_pos_t height) {
  world_map *result = (world_map*) malloc(sizeof(world_map));

  result->seed = prng(seed);
  result->width = width;
  result->height = height;
  result->regions = (world_region *) calloc(width*height, sizeof(world_region));
  result->all_strata = create_list();
  result->all_water = create_list();
  result->all_biomes = create_list();
  result->all_civs = create_list();

  return result;
}

void cleanup_world_map(world_map *wm) {
  // none of these need special cleanup beyond a free()
  destroy_list(wm->all_strata);
  destroy_list(wm->all_water);
  destroy_list(wm->all_biomes);
  destroy_list(wm->all_civs);
  free(wm->regions);
  free(wm);
}

/*************
 * Functions *
 *************/

void compute_region_contenders(
  world_map *wm,
  world_region* neighborhood[],
  region_pos *rpos,
  world_region **best, world_region **secondbest,
  float *strbest, float *strsecond
) {
  world_region *wr; // best and second-best regions
  int i;
  region_pos anchor;
  vector v, vbest, vsecond;
  float str, noise;
  ptrdiff_t salt;
  world_map_pos wmpos;

  rpos__wmpos(rpos, &wmpos);

 // Figure out the two nearest world regions:
  // Setup worst-case defaults:
  vbest.x = WORLD_REGION_BLOCKS;
  vbest.y = WORLD_REGION_BLOCKS;
  vbest.z = 0;
  *strbest = 0;
  vsecond.x = WORLD_REGION_BLOCKS;
  vsecond.y = WORLD_REGION_BLOCKS;
  vsecond.z = 0;
  *strsecond = 0;

  *secondbest = NULL;
  *best = NULL;

  // Figure out which of our neighbors are closest:
  wmpos.x -= 1;
  wmpos.y -= 1;
  for (i = 0; i < 9; i += 1) {
    wr = neighborhood[i];
    if (wr != NULL) {
      copy_rpos(&(wr->anchor), &anchor);
      salt = prng(wr->seed + 172841);
    } else {
      compute_region_anchor(wm, &wmpos, &anchor);
      salt = prng(prng(prng(wmpos.x) + wmpos.y) + 51923);
    }
    v.x = rpos->x - anchor.x;
    v.y = rpos->y - anchor.y;
    v.z = rpos->z - anchor.z;
    str = 1 - (vmag(&v) / MAX_REGION_ANCHOR_DISTANCE);
    // 3D base noise:
    noise = (
      sxnoise_3d(
        v.x * REGION_CONTENTION_NOISE_SCALE,
        v.y * REGION_CONTENTION_NOISE_SCALE,
        v.z * REGION_CONTENTION_NOISE_SCALE,
        salt * 576 + 9123
      ) + 
      0.6 * sxnoise_3d(
        v.x * REGION_CONTENTION_NOISE_SCALE * 2.1,
        v.y * REGION_CONTENTION_NOISE_SCALE * 2.1,
        v.z * REGION_CONTENTION_NOISE_SCALE * 2.1,
        salt * 577 + 9124
      )
    ) / 1.6;
    salt = prng(salt);
    noise = (1 + noise * REGION_CONTENTION_NOISE_STRENGTH) / 2.0;
    // [
    //   0.5 - REGION_CONTENTION_NOISE_STRENGTH/2,
    //   0.5 + REGION_CONTENTION_NOISE_STRENGTH/2
    // ]
    str *= noise;
    // Polar noise:
    vxyz__polar(&v, &v);
    noise = tiled_func(
      &sxnoise_2d,
      v.y * REGION_CONTENTION_POLAR_SCALE,
      v.z * REGION_CONTENTION_POLAR_SCALE,
      2*M_PI*REGION_CONTENTION_POLAR_SCALE,
      2*M_PI*REGION_CONTENTION_POLAR_SCALE,
      salt
    );
    vpolar__xyz(&v, &v);
    noise = (1 + noise * REGION_CONTENTION_POLAR_STRENGTH) / 2.0;
    // [
    //   0.5 - REGION_CONTENTION_POLAR_STRENGTH/2,
    //   0.5 + REGION_CONTENTION_POLAR_STRENGTH/2
    // ]
    str *= noise;
    if (str > *strbest) {
      vcopy(&vsecond, &vbest);
      *strsecond = *strbest;
      *secondbest = *best;

      vcopy(&vbest, &v);
      *strbest = str;
      *best = wr;
    } else if (str > *strsecond) {
      vcopy(&vsecond, &v);
      *strsecond = str;
      *secondbest = wr;
    }
    // Update wmpos based on i:
    if (i == 2 || i == 5) {
      wmpos.x += 1;
      wmpos.y -= 2;
    } else {
      wmpos.y += 1;
    }
  }
}

void find_valley(world_map *wm, world_map_pos *pos) {
  world_region *wr;
  wr = get_world_region(wm, pos);
  if (wr == NULL) {
    return;
  }

  while (wr->downhill != NULL) {
    wr = wr->downhill;
  }
  copy_wmpos(&(wr->pos), pos);
}
