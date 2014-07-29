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
float const GN_DISTORTION_SCALE = 784;
float const GN_LARGE_VAR_SCALE = 2563;
float const GN_MED_VAR_SCALE = 1345;

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

  seed = expanded_hash_1d(seed + hash_2d(seed, seed));

  switch (source) {
    case GEO_IGNEOUS:
      result->base_species = create_new_igneous_species(seed);

      result->persistence = 1.2 + 0.4 * float_hash_1d(seed);
      seed = expanded_hash_1d(seed);
      result->scale_bias = 0.7 + 0.4 * float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->radial_frequency = M_PI/(2.4 + 1.6*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->radial_variance = 0.1 + 0.3*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->gross_distortion = 900 + 500.0*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);
      result->fine_distortion = 110 + 40.0*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->large_var = result->thickness * (0.6 + 0.3*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->med_var = result->thickness * (0.4 + 0.25*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->small_var = result->thickness * (0.17 + 0.05*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->tiny_var = result->thickness * (0.04 + 0.06*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);

      result->detail_var = 1.0 + 2.0*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);
      result->ridges = 2.0 + 3.0*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->smoothing = 0.15 + 0.2*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

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

      result->persistence = 0.8 + 0.5 * float_hash_1d(seed);
      seed = expanded_hash_1d(seed);
      result->scale_bias = 0.8 + 0.4 * float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->radial_frequency = M_PI/(2.8 + 2.0*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->radial_variance = 0.4 + 0.4*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->gross_distortion = 1200 + 900.0*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);
      result->fine_distortion = 180 + 110.0*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->large_var = result->thickness * (0.5 + 0.3*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->med_var = result->thickness * (0.3 + 0.25*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->small_var = result->thickness * (0.16 + 0.06*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->tiny_var = result->thickness * (0.02 + 0.03*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);

      result->detail_var = 0.3 + 1.8*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);
      result->ridges = 0.4 + 3.4*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->smoothing = 0.15 + 0.45*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

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

      result->persistence = 1.3 + 0.5 * float_hash_1d(seed);
      seed = expanded_hash_1d(seed);
      result->scale_bias = 1.1 + 0.3 * float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->radial_frequency = M_PI/(2.1 + 1.2*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->radial_variance = 0.05 + 0.2*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->gross_distortion = 700 + 400.0*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);
      result->fine_distortion = 30 + 30.0*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->large_var = result->thickness * (0.4 + 0.25*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->med_var = result->thickness * (0.2 + 0.15*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->small_var = result->thickness * (0.11 + 0.05*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);
      result->tiny_var = result->thickness * (0.03 + 0.07*float_hash_1d(seed));
      seed = expanded_hash_1d(seed);

      result->detail_var = 0.7 + 3.2*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);
      result->ridges = 0.8 + 4.5*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

      result->smoothing = 0.12 + 0.4*float_hash_1d(seed);
      seed = expanded_hash_1d(seed);

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
  static region_chunk_pos pr_rcpos = { .x = -1, .y = -1, .z = -1 };
  // low- and high-frequency distortion:
  static float lfdx = 0; static float lfdy = 0;
  // low- and high-frequency noise:
  static float lfn = 0;
  // base thickness:
  static float base = 0;

  // normal variables:
  float fx;
  float fy;
  region_chunk_pos rcpos;

  // compute our chunk position:
  rpos__rcpos(rpos, &rcpos);

  if (pr_st != st || pr_rcpos.x != rcpos.x || pr_rcpos.y != rcpos.y) {
    // need to recompute low-frequency info:
    fx = (float) (rpos->x);
    fy = (float) (rpos->y);
    stratum_lf_distortion(st, fx, fy, &lfdx, &lfdy);
    stratum_lf_noise(st, fx+lfdx, fy+lfdy, &lfn);
    stratum_base_thickness(st, fx+lfdx, fy+lfdy, &base);
  }
  // set static variables:
  copy_rcpos(&rcpos, &pr_rcpos);
  pr_st = st;
  return (r_pos_t) fastfloor(base + lfn);
}

species create_new_igneous_species(ptrdiff_t seed) {
  species result = create_stone_species();
  stone_species* ssp = get_stone_species(result);

  seed = expanded_hash_1d(seed);

  float base_density = pow(norm_hash_1d(seed), 0.8);

  determine_new_igneous_material(&(ssp->material), seed, base_density);
  seed = expanded_hash_1d(seed);
  determine_new_igneous_appearance(&(ssp->appearance), seed, base_density);

  return result;
}

species create_new_metamorphic_species(ptrdiff_t seed) {
  species result = create_stone_species();
  stone_species* ssp = get_stone_species(result);

  seed = expanded_hash_1d(seed);

  float base_density = norm_hash_1d(seed);

  determine_new_metamorphic_material(&(ssp->material), seed, base_density);
  seed = expanded_hash_1d(seed);
  determine_new_metamorphic_appearance(&(ssp->appearance), seed, base_density);

  return result;
}

species create_new_sedimentary_species(ptrdiff_t seed) {
  species result = create_stone_species();
  stone_species* ssp = get_stone_species(result);

  seed = expanded_hash_1d(seed);

  float base_density = pow(norm_hash_1d(seed), 0.8);

  determine_new_sedimentary_material(&(ssp->material), seed, base_density);
  seed = expanded_hash_1d(seed);
  determine_new_sedimentary_appearance(&(ssp->appearance), seed, base_density);

  return result;
}

void determine_new_igneous_material(
  material *target,
  ptrdiff_t seed,
  float base_density
) {
  target->origin = MO_IGNEOUS_MINERAL;

  seed = expanded_hash_1d(seed);

  // Base density is taken from the external argument:
  target->solid_density = mat_density(0.25 + 4.75 * base_density);
  target->liquid_density = mat_density(2.5 + 0.5 * base_density);
  target->gas_density = mat_density(1.8 + 1.5 * base_density);

  // A much tighter distribution of specific heats that correlates a bit with
  // density:
  float base_specific_heat = (norm_hash_1d(seed) + norm_hash_1d(seed))/2.0;
  base_specific_heat = 0.7*base_specific_heat + 0.3*(1 - base_density);
  seed = expanded_hash_1d(seed);

  target->solid_specific_heat = mat_specific_heat(
    0.3 + 1.1*base_specific_heat
  );
  target->liquid_specific_heat = mat_specific_heat(
    0.8 + 1.4*base_specific_heat
  );
  target->gas_specific_heat = mat_specific_heat(
    0.65 + 0.45*base_specific_heat
  );

  target->cold_damage_temp = 0; // stones aren't vulnerable to cold

  // A flat distribution of melting points, slightly correlated with density:
  float base_transition_temp = float_hash_1d(seed);
  base_transition_temp = 0.8*base_transition_temp + 0.2*base_density;
  seed = expanded_hash_1d(seed);

  target->solidus = 550 + 700 * base_transition_temp;
  target->liquidus = target->solidus + 50 + norm_hash_1d(seed) * 200;
  seed = expanded_hash_1d(seed);

  target->boiling_point = 1800 + base_transition_temp * 600;

  // igneous stone isn't known for combustion:
  target->ignition_point = smaxof(temperature);
  target->flash_point = smaxof(temperature);

  // it is generally not very malleable, and may be as brittle as glass:
  target->malleability = 80 * norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // magma is extremely viscous:
  target->viscosity = pow(10.0, 6.0 + 10.0*float_hash_1d(seed));
  seed = expanded_hash_1d(seed);

  // igneous rocks are pretty hard (and again this correlates with density):
  target->hardness = 100+120*(0.8*norm_hash_1d(seed) + 0.2*base_density);
}

void determine_new_metamorphic_material(
  material *target,
  ptrdiff_t seed,
  float base_density
) {
  target->origin = MO_METAMORPHIC_MINERAL;

  seed = expanded_hash_1d(seed);

  // Base density is taken from the external argument:
  target->solid_density = mat_density(0.9 + 4.8 * base_density);
  target->liquid_density = mat_density(2.6 + 0.6 * base_density);
  target->gas_density = mat_density(1.8 + 1.5 * base_density);

  // A tighter distribution of specific heats that correlates a bit with
  // density:
  float base_specific_heat = norm_hash_1d(seed);
  base_specific_heat = 0.7*base_specific_heat + 0.3*(1 - base_density);
  seed = expanded_hash_1d(seed);

  target->solid_specific_heat = mat_specific_heat(
    0.2 + 1.3*base_specific_heat
  );
  target->liquid_specific_heat = mat_specific_heat(
    0.7 + 1.7*base_specific_heat
  );
  target->gas_specific_heat = mat_specific_heat(
    0.6 + 0.55*base_specific_heat
  );

  target->cold_damage_temp = 0; // stones aren't vulnerable to cold

  // A flat distribution of melting points, slightly correlated with density:
  float base_transition_temp = float_hash_1d(seed);
  base_transition_temp = 0.8*base_transition_temp + 0.2*base_density;
  seed = expanded_hash_1d(seed);

  target->solidus = 520 + 760 * base_transition_temp;
  target->liquidus = target->solidus + 20 + norm_hash_1d(seed) * 300;
  seed = expanded_hash_1d(seed);

  target->boiling_point = 1700 + base_transition_temp * 800;

  // normal metamorphic stone isn't known for combustion:
  // TODO: Some high values here?
  target->ignition_point = smaxof(temperature);
  target->flash_point = smaxof(temperature);

  // it may be somewhat malleable, and is never completely brittle
  target->malleability = 10 + 90 * norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // magma is extremely viscous:
  target->viscosity = pow(10.0, 6.0 + 10.0*float_hash_1d(seed));
  seed = expanded_hash_1d(seed);

  // metamorphic rocks can be pretty soft, but are mostly hard (again this
  // correlates with density):
  target->hardness = 40 + 190*(
    0.8*sqrtf(norm_hash_1d(seed)) + 0.2*base_density
  );
}

void determine_new_sedimentary_material(
  material *target,
  ptrdiff_t seed,
  float base_density
) {
  target->origin = MO_SEDIMENTARY_MINERAL;

  seed = expanded_hash_1d(seed);

  // Base density is taken from the external argument:
  target->solid_density = mat_density(1.2 + 3.7 * base_density);
  target->liquid_density = mat_density(2.6 + 0.5 * base_density);
  target->gas_density = mat_density(1.6 + 1.3 * base_density);

  // A tighter distribution of specific heats that correlates a bit with
  // density:
  float base_specific_heat = norm_hash_1d(seed);
  base_specific_heat = 0.7*base_specific_heat + 0.3*(1 - base_density);
  seed = expanded_hash_1d(seed);

  target->solid_specific_heat = mat_specific_heat(
    0.4 + 1.4*base_specific_heat
  );
  target->liquid_specific_heat = mat_specific_heat(
    0.9 + 1.4*base_specific_heat
  );
  target->gas_specific_heat = mat_specific_heat(
    0.7 + 0.5*base_specific_heat
  );

  target->cold_damage_temp = 0; // stones aren't vulnerable to cold

  // A flat distribution of melting points, slightly correlated with density:
  float base_transition_temp = float_hash_1d(seed);
  base_transition_temp = 0.8*base_transition_temp + 0.2*base_density;
  seed = expanded_hash_1d(seed);

  target->solidus = 440 + 780 * base_transition_temp;
  target->liquidus = target->solidus + 10 + norm_hash_1d(seed) * 180;
  seed = expanded_hash_1d(seed);

  target->boiling_point = 1550 + base_transition_temp * 600;

  // normal sedimentary stone isn't known for combustion (see special fuel
  // stone generation methods):
  // TODO: Some high values here?
  target->ignition_point = smaxof(temperature);
  target->flash_point = smaxof(temperature);

  // it may be somewhat malleable, but it can also be quite brittle
  target->malleability = 5 + 110 * norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // sedimentary magma is generally more viscous than other types (says I):
  target->viscosity = pow(10.0, 5.0 + 8.0*float_hash_1d(seed));
  seed = expanded_hash_1d(seed);

  // sedimentary rocks are generally a bit softer than other types:
  target->hardness = 30 + 150*(
    0.8*pow(norm_hash_1d(seed), 0.8) + 0.2*base_density
  );
}


void determine_new_igneous_appearance(
  stone_filter_args *target,
  ptrdiff_t seed,
  float base_density
) {
  seed = expanded_hash_1d(seed);
  target->seed = seed;

  // Lighter rocks tend to have smaller noise scales.
  target->scale = 0.1 + 0.08*(0.8*float_hash_1d(seed) + 0.2*base_density);
  seed = expanded_hash_1d(seed);

  // Igneous rock types are relatively gritty. Denser rocks tend to be slightly
  // less gritty though.
  target->gritty = 1.4 + 2.3*(0.7*float_hash_1d(seed) + 0.3*(1-base_density));
  seed = expanded_hash_1d(seed);

  // Denser rocks tend to be more contoured.
  target->contoured = 3.5 + 3.5*(0.8*float_hash_1d(seed) + 0.2*base_density);
  seed = expanded_hash_1d(seed);

  // Lighter igneous rocks are far more porous.
  target->porous = 2.0 + 8.5*(0.5*float_hash_1d(seed) + 0.5*(1-base_density));
  seed = expanded_hash_1d(seed);

  // Igneous rocks don't tend to be very bumpy.
  target->bumpy = 1.0 + 4.0*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Igneous rocks rarely have significant inclusions.
  target->inclusions = pow(norm_hash_1d(seed), 2.5);
  seed = expanded_hash_1d(seed);

  // The distortion scale is within 10% of the base scale.
  target->dscale = target->scale * 1 + 0.2*(float_hash_1d(seed) - 0.5);
  seed = expanded_hash_1d(seed);

  // Igneous rocks can have at most moderate distortion, and largely have very
  // little distortion.
  target->distortion = 4.4*pow(norm_hash_1d(seed), 2);
  seed = expanded_hash_1d(seed);

  // Igneous rocks can be squashed in either direction
  target->squash = 0.7 + 0.6*norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);
  target->squash /= 0.7 + 0.6*norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Hues range from blue to orange:
  float hue = -0.2 + 0.3*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);
  if (hue < 0) {
    hue = 1 + hue;
  }

  // Saturation is usually negligible though:
  float sat = 0;
  if (float_hash_1d(seed) < 0.7) {
    seed = expanded_hash_1d(seed);
    sat = 0.05*float_hash_1d(seed);
  } else {
    seed = expanded_hash_1d(seed);
    sat = 0.3*float_hash_1d(seed);
  }
  seed = expanded_hash_1d(seed);

  // Base values are correlated with density, and are mostly dark:
  float val = 0.6*float_hash_1d(seed) + 0.4*(1-base_density);
  seed = expanded_hash_1d(seed);
  val *= val;

  // Construct the base color:
  pixel hsv = float_color(hue, sat, val, 1.0);
  hsv__rgb(hsv, &(target->base_color));

  // Igneous inclusions just have contrasting brightness:
  hsv = float_color(hue, sat, 1 - val, 1.0);
  hsv__rgb(hsv, &(target->alt_color));

  // TODO: A skewed binary distribution here!
  target->brightness = -0.2 + 0.4*norm_hash_1d(seed);
}

void determine_new_metamorphic_appearance(
  stone_filter_args *target,
  ptrdiff_t seed,
  float base_density
) {
  seed = expanded_hash_1d(seed);
  target->seed = seed;

  // Metamorphic rocks exhibit a wide range of noise scales:
  target->scale = 0.08 + 0.12*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Metamorphic rock types are usually not very gritty, especially when
  // they're very dense.
  target->gritty = 0.8 + 2.5*(0.7*float_hash_1d(seed) + 0.3*(1-base_density));
  seed = expanded_hash_1d(seed);

  // Denser rocks tend to be more contoured.
  target->contoured = 1.5 + 7.5*(0.8*float_hash_1d(seed) + 0.2*base_density);
  seed = expanded_hash_1d(seed);

  // Lighter metamorphic rocks are a bit more porous.
  target->porous = 1.0 + 8.0*(0.7*float_hash_1d(seed) + 0.3*(1-base_density));
  seed = expanded_hash_1d(seed);

  // Metamorphic rocks can be quite bumpy, especially when less dense.
  target->bumpy = 1.0 + 9.0*(0.6*float_hash_1d(seed) + 0.4*(1-base_density));
  seed = expanded_hash_1d(seed);

  // Metamorphic rocks often have significant inclusions.
  target->inclusions = pow(float_hash_1d(seed), 1.3);
  seed = expanded_hash_1d(seed);

  // The distortion scale is within 10% of the base scale.
  target->dscale = target->scale * 1 + 0.2*(float_hash_1d(seed) - 0.5);
  seed = expanded_hash_1d(seed);

  // Metamorphic rocks can have quite a bit of distortion.
  target->distortion = 6.5*norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // And they can be squashed quite a bit.
  target->squash = 0.6 + 0.8*norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);
  target->squash /= 0.6 + 0.8*norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // All kinds of hues are possible.
  float hue = float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Saturation is usually small.
  float sat = norm_hash_1d(seed);
  sat *= sat;
  sat *= 0.4;
  seed = expanded_hash_1d(seed);

  // Base values have a wide range:
  float val = 0.2 + 0.6*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Construct the base color:
  pixel hsv = float_color(hue, sat, val, 1.0);
  hsv__rgb(hsv, &(target->base_color));

  // Metamorphic inclusions use the same distributions as the base rock, but
  // have a wider range of saturations:
  hue = float_hash_1d(seed);
  seed = expanded_hash_1d(seed);
  sat = 0.6*pow(norm_hash_1d(seed), 1.5);
  seed = expanded_hash_1d(seed);
  val = 0.2 + 0.6*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  hsv = float_color(hue, sat, val, 1.0);
  hsv__rgb(hsv, &(target->alt_color));

  // Metamorphic rocks have a slight bias towards brightness:
  target->brightness = -0.25 + 0.6*norm_hash_1d(seed);
}

void determine_new_sedimentary_appearance(
  stone_filter_args *target,
  ptrdiff_t seed,
  float base_density
) {
  seed = expanded_hash_1d(seed);
  target->seed = seed;

  // Sedimentary rocks often have smaller scales than other rocks.
  target->scale = 0.07 + 0.07*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Sedimentary rock types are usually gritty.
  target->gritty = 3.1 + 2.5*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Denser rocks tend to be more contoured.
  target->contoured = 1.5 + 9.5*(0.7*float_hash_1d(seed) + 0.3*base_density);
  seed = expanded_hash_1d(seed);

  // Lighter sedimentary rocks are a bit more porous.
  target->porous = 3.0 + 5.0*(0.7*float_hash_1d(seed) + 0.3*(1-base_density));
  seed = expanded_hash_1d(seed);

  // Sedimentary rocks can be quite bumpy.
  target->bumpy = 3.0 + 6.0*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Sedimentary rocks usually don't have inclusions.
  target->inclusions = pow(norm_hash_1d(seed), 2.5);
  seed = expanded_hash_1d(seed);

  // The distortion scale is within 20% of the base scale.
  target->dscale = target->scale * 1 + 0.4*(float_hash_1d(seed) - 0.5);
  seed = expanded_hash_1d(seed);

  // Sedimentary rocks can have little to medium distortion.
  target->distortion = 4.5*pow(norm_hash_1d(seed), 1.4);
  seed = expanded_hash_1d(seed);

  // Sedimentary rocks are usually squashed horizontally.
  target->squash = 0.6 + 0.6*norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);
  target->squash /= 0.8 + 0.6*norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Sedimentary rocks can be yellowish to bluish, or just gray:
  float hue = 0.15 + 0.5*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Saturation is usually small.
  float sat = norm_hash_1d(seed);
  seed = expanded_hash_1d(seed);
  if (float_hash_1d(seed) < 0.4) {
    sat = 0;
  }
  sat *= sat;
  sat *= 0.3;
  seed = expanded_hash_1d(seed);

  // Base values have a wide range:
  float val = 0.1 + 0.8*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  // Construct the base color:
  pixel hsv = float_color(hue, sat, val, 1.0);
  hsv__rgb(hsv, &(target->base_color));

  // Sedimentary inclusions use the same distributions as the base rock, but
  // have a wider range of saturations:
  hue = float_hash_1d(seed);
  seed = expanded_hash_1d(seed);
  sat = 0.5*pow(norm_hash_1d(seed), 1.2);
  seed = expanded_hash_1d(seed);
  val = 0.1 + 0.8*float_hash_1d(seed);
  seed = expanded_hash_1d(seed);

  hsv = float_color(hue, sat, val, 1.0);
  hsv__rgb(hsv, &(target->alt_color));

  // Sedimentary rocks are generally bright.
  target->brightness = -0.05 + 0.35*norm_hash_1d(seed);
}
