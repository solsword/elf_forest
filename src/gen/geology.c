// geology.c
// Stone types and strata generation.

#include "noise/noise.h"
#include "math/functions.h"
#include "datatypes/vector.h"
#include "world/world.h"

#include "worldgen.h"

#include "geology.h"


/***********
 * Globals *
 ***********/

ptrdiff_t const GEOTHERMAL_SEED = 397548;

// Note: these constants are all expressed in terms of blocks.
float const GN_GROSS_DISTORTION_SCALE = 784;
float const GN_FINE_DISTORTION_SCALE = 211;
float const GN_LARGE_VAR_SCALE = 2563;
float const GN_MED_VAR_SCALE = 1345;
float const GN_SMALL_VAR_SCALE = 547;
float const GN_TINY_VAR_SCALE = 43;
float const GN_DETAIL_VAR_SCALE = 5.4;
float const GN_RIDGE_SCALE = 67;

/******************************
 * Constructors & Destructors *
 ******************************/

stratum *create_stratum(
  ptrdiff_t seed,
  float cx, float cy,
  float size, float thickness,
  map_function profile,
  geologic_source source
) {
  int i;
  stratum *result = (stratum *) malloc(sizeof(stratum));
  result->seed = seed;
  result->cx = cx;
  result->cy = cy;
  result->size = size;
  result->thickness = thickness;
  result->profile = profile;
  result->source = source;

  result->base_material = 0; // TODO: Pick a material here!

  // TODO: Derive/randomize parameters here!
  result->scale_bias = 1.0;
  result->infill = 0.0;

  result->radial_frequency = M_PI/3.2;
  result->radial_variance = 0.4;

  result->gross_distortion = 1010.0;
  result->fine_distortion = 108.0;

  result->large_var = result->thickness*0.7;
  result->med_var = result->thickness*0.46;
  result->small_var = result->thickness*0.19;
  result->tiny_var = result->thickness*0.07;

  result->detail_var = 2.3;
  result->ridges = 2.5;

  result->smoothing = 0.2;

  for (i = 0; i < N_VEIN_TYPES; ++i) {
    result->vein_scale[i] = 0; // 23.4;
    result->vein_strength[i] = 0; // 0.5;
    result->vein_material[i] = 0; // TODO: Pick a material here!
  }

  for (i = 0; i < N_INCLUSION_TYPES; ++i) {
    result->inclusion_frequency[i] = 0; // 0.01;
    result->inclusion_material[i] = 0; // TODO: Pick a material here!
  }

  return result;
}

/********************
 * Inline Functions *
 ********************/

/*************
 * Functions *
 *************/

r_pos_t compute_stratum_height(stratum *st, region_pos *rpos) {
  // static variables:
  static stratum *pr_st = NULL;
  static region_pos pr_rpos = { .x = -1, .y = -1, .z = -1 };
  static region_chunk_pos pr_rcpos = { .x = -1, .y = -1, .z = -1 };
  // low- and high-frequency distortion:
  static float lfdx = 0; static float lfdy = 0;
  static float hfdx = 0; static float hfdy = 0;
  // low- and high-frequency noise:
  static float lfn = 0; static float hfn = 0;
  // base thickness:
  static float base = 0;

  // normal variables:
  float fx;
  float fy;
  region_chunk_pos rcpos;
  region_pos rounded_rpos;

  // compute our chunk position:
  rpos__rcpos(rpos, &rcpos);

  if (pr_st != st || pr_rcpos.x != rcpos.x || pr_rcpos.y != rcpos.y) {
    rcpos__rpos(&rcpos, &rounded_rpos);
    // need to recompute low-frequency info:
    fx = (float) rounded_rpos.x;
    fy = (float) rounded_rpos.y;
    stratum_lf_distortion(st, fx, fy, &lfdx, &lfdy);
    stratum_lf_noise(st, fx+lfdx, fy+lfdy, &lfn);
    stratum_base_thickness(st, fx+lfdx, fy+lfdy, &base);
  }
  if (pr_st != st || pr_rpos.x != rpos->x || pr_rpos.y != rpos->y) {
    // need to recompute high-frequency info:
    fx = (float) rpos->x;
    fy = (float) rpos->y;
    stratum_hf_distortion(st, fx, fy, &hfdx, &hfdy);
    stratum_hf_noise(st, fx+hfdx, fy+hfdy, &hfn);
  }
  return (r_pos_t) (base + (lfn - 0.5) + hfn);
  // set static variables:
  copy_rpos(rpos, &pr_rpos);
  copy_rcpos(&rcpos, &pr_rcpos);
}
