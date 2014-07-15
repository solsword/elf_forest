// geology.c
// Stone types and strata generation.

#include "noise/noise.h"
#include "math/functions.h"
#include "datatypes/vector.h"
#include "world/world.h"
#include "world/species.h"

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
float const GN_DETAIL_VAR_SCALE = 23.4;
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

  ptrdiff_t hash;
  hash = hash_3d(seed, seed, seed);

  switch (source) {
    case GEO_IGNEOUS:
      result->base_species = create_new_igneous_species(seed);

      result->scale_bias = 0.7 + 0.4 * float_hash_1d(hash);
      hash = hash_1d(hash);

      result->radial_frequency = M_PI/(2.4 + 1.6*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->radial_variance = 0.1 + 0.3*float_hash_1d(hash);
      hash = hash_1d(hash);

      result->gross_distortion = 900 + 500.0*float_hash_1d(hash);
      hash = hash_1d(hash);
      result->fine_distortion = 110 + 40.0*float_hash_1d(hash);
      hash = hash_1d(hash);

      result->large_var = result->thickness * (0.5 + 0.3*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->med_var = result->thickness * (0.3 + 0.2*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->small_var = result->thickness * (0.17 + 0.05*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->tiny_var = result->thickness * (0.04 + 0.06*float_hash_1d(hash));
      hash = hash_1d(hash);

      result->detail_var = 1.0 + 2.0*float_hash_1d(hash);
      hash = hash_1d(hash);
      result->ridges = 2.0 + 3.0*float_hash_1d(hash);
      hash = hash_1d(hash);

      result->smoothing = 0.15 + 0.2*float_hash_1d(hash);
      hash = hash_1d(hash);

      for (i = 0; i < N_VEIN_TYPES; ++i) {
        result->vein_scale[i] = 0; // 23.4;
        result->vein_strength[i] = 0; // 0.5;
        result->vein_species[i] = 0; // TODO: Pick a material here!
      }

      for (i = 0; i < N_INCLUSION_TYPES; ++i) {
        result->inclusion_frequency[i] = 0; // 0.01;
        result->inclusion_species[i] = 0; // TODO: Pick a material here!
      }
      break;

    case GEO_METAMORPHIC:
      result->base_species = create_new_metamorphic_species(seed);

      result->scale_bias = 0.9 + 0.4 * float_hash_1d(hash);
      hash = hash_1d(hash);

      result->radial_frequency = M_PI/(2.8 + 1.0*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->radial_variance = 0.3 + 0.3*float_hash_1d(hash);
      hash = hash_1d(hash);

      result->gross_distortion = 1100 + 600.0*float_hash_1d(hash);
      hash = hash_1d(hash);
      result->fine_distortion = 150 + 50.0*float_hash_1d(hash);
      hash = hash_1d(hash);

      result->large_var = result->thickness * (0.4 + 0.2*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->med_var = result->thickness * (0.2 + 0.2*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->small_var = result->thickness * (0.14 + 0.04*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->tiny_var = result->thickness * (0.03 + 0.04*float_hash_1d(hash));
      hash = hash_1d(hash);

      result->detail_var = 0.4 + 2.0*float_hash_1d(hash);
      hash = hash_1d(hash);
      result->ridges = 0.7 + 2.4*float_hash_1d(hash);
      hash = hash_1d(hash);

      result->smoothing = 0.2 + 0.25*float_hash_1d(hash);
      hash = hash_1d(hash);

      for (i = 0; i < N_VEIN_TYPES; ++i) {
        result->vein_scale[i] = 0; // 23.4;
        result->vein_strength[i] = 0; // 0.5;
        result->vein_species[i] = 0; // TODO: Pick a material here!
      }

      for (i = 0; i < N_INCLUSION_TYPES; ++i) {
        result->inclusion_frequency[i] = 0; // 0.01;
        result->inclusion_species[i] = 0; // TODO: Pick a material here!
      }
      break;

    case GEO_SEDIMENTAY:
    default:
      result->base_species = create_new_sedimentary_species(seed);

      result->scale_bias = 1.1 + 0.3 * float_hash_1d(hash);
      hash = hash_1d(hash);

      result->radial_frequency = M_PI/(2.1 + 1.2*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->radial_variance = 0.05 + 0.2*float_hash_1d(hash);
      hash = hash_1d(hash);

      result->gross_distortion = 700 + 400.0*float_hash_1d(hash);
      hash = hash_1d(hash);
      result->fine_distortion = 30 + 30.0*float_hash_1d(hash);
      hash = hash_1d(hash);

      result->large_var = result->thickness * (0.4 + 0.25*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->med_var = result->thickness * (0.2 + 0.15*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->small_var = result->thickness * (0.11 + 0.05*float_hash_1d(hash));
      hash = hash_1d(hash);
      result->tiny_var = result->thickness * (0.03 + 0.07*float_hash_1d(hash));
      hash = hash_1d(hash);

      result->detail_var = 0.7 + 3.2*float_hash_1d(hash);
      hash = hash_1d(hash);
      result->ridges = 0.8 + 4.5*float_hash_1d(hash);
      hash = hash_1d(hash);

      result->smoothing = 0.12 + 0.4*float_hash_1d(hash);
      hash = hash_1d(hash);

      for (i = 0; i < N_VEIN_TYPES; ++i) {
        result->vein_scale[i] = 0; // 23.4;
        result->vein_strength[i] = 0; // 0.5;
        result->vein_species[i] = 0; // TODO: Pick a material here!
      }

      for (i = 0; i < N_INCLUSION_TYPES; ++i) {
        result->inclusion_frequency[i] = 0; // 0.01;
        result->inclusion_species[i] = 0; // TODO: Pick a material here!
      }
      break;
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
    fx = (float) (rounded_rpos.x);
    fy = (float) (rounded_rpos.y);
    stratum_lf_distortion(st, fx, fy, &lfdx, &lfdy);
    stratum_lf_noise(st, fx+lfdx, fy+lfdy, &lfn);
    stratum_base_thickness(st, fx+lfdx, fy+lfdy, &base);
  }
  if (pr_st != st || pr_rpos.x != rpos->x || pr_rpos.y != rpos->y) {
    // need to recompute high-frequency info:
    fx = (float) (rpos->x);
    fy = (float) (rpos->y);
    stratum_hf_distortion(st, fx, fy, &hfdx, &hfdy);
    stratum_hf_noise(st, fx+hfdx, fy+hfdy, &hfn);
  }
  // set static variables:
  copy_rpos(rpos, &pr_rpos);
  copy_rcpos(&rcpos, &pr_rcpos);
  pr_st = st;
  //printf("geo: %.2f, %.2f, %.2f\n", base, lfn, hfn);
  //printf("geo: %d\n", (r_pos_t) (base + (lfn - 0.5) + hfn));
  return (r_pos_t) fastfloor(base + (lfn - 0.5) + hfn);
}

species create_new_igneous_species(ptrdiff_t seed) {
  species result = create_stone_species();
  stone_species* ssp = get_stone_species(result);
  ssp->material.origin = MO_IGNEOUS_MINERAL;

  ptrdiff_t hash = hash_2d(seed, seed);

  // A skewed normal-like distribution of densities:
  float base_density = pow(norm_hash_1d(hash), 0.8);
  hash = hash_1d(hash);

  ssp->material.solid_density = mat_density(0.25 + 4.75 * base_density);
  ssp->material.liquid_density = mat_density(2.5 + 0.5 * base_density);
  ssp->material.gas_density = mat_density(1.8 + 1.5 * base_density);

  // A much tighter distribution of specific heats that correlates a bit with
  // density:
  float base_specific_heat = (norm_hash_1d(hash) + norm_hash_1d(hash))/2.0;
  base_specific_heat = 0.7*base_specific_heat + 0.3*(1 - base_density);
  hash = hash_1d(hash);

  ssp->material.solid_specific_heat = mat_specific_heat(
    0.3 + 1.1*base_specific_heat
  );
  ssp->material.liquid_specific_heat = mat_specific_heat(
    0.8 + 1.4*base_specific_heat
  );
  ssp->material.gas_specific_heat = mat_specific_heat(
    0.65 + 0.45*base_specific_heat
  );

  ssp->material.cold_damage_temp = 0; // stones aren't vulnerable to cold

  // A flat distribution of melting points, slightly correlated with density:
  float base_transition_temp = float_hash_1d(hash);
  base_transition_temp = 0.8*base_transition_temp + 0.2*base_density;
  hash = hash_1d(hash);

  ssp->material.solidus = 550 + 700 * base_transition_temp;
  ssp->material.liquidus = ssp->material.solidus + 50 +
                           norm_hash_1d(hash) * 200;
  hash = hash_1d(hash);

  ssp->material.boiling_point = 1800 + base_transition_temp * 600;

  // igneous stone isn't known for combustion:
  ssp->material.ignition_point = smaxof(temperature);
  ssp->material.flash_point = smaxof(temperature);

  // it is generally not very malleable, and may be as brittle as glass:
  ssp->material.malleability = 80 * norm_hash_1d(hash);
  hash = hash_1d(hash);

  // magma is extremely viscous:
  ssp->material.viscosity = pow(10.0, 6.0 + 10.0*float_hash_1d(hash));
  hash = hash_1d(hash);

  // igneous rocks are pretty hard (and again this correlates with density):
  ssp->material.hardness = 100+120*(0.8*norm_hash_1d(hash) + 0.2*base_density);
  hash = hash_1d(hash);

  return result;
}

species create_new_metamorphic_species(ptrdiff_t seed) {
  species result = create_stone_species();
  stone_species* ssp = get_stone_species(result);
  ssp->material.origin = MO_METAMORPHIC_MINERAL;
  return result;
}

species create_new_sedimentary_species(ptrdiff_t seed) {
  species result = create_stone_species();
  stone_species* ssp = get_stone_species(result);
  ssp->material.origin = MO_SEDIMENTARY_MINERAL;
  return result;
}
