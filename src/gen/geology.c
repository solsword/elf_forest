// geology.c
// Stone types and strata generation.

#include "noise/noise.h"
#include "math/functions.h"
#include "datatypes/vector.h"
#include "world/world.h"
#include "world/species.h"
#include "world/world_map.h"

#include "util.h"

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

  seed = prng(seed + hash_2d(seed, seed));

  switch (source) {
    case GEO_IGNEOUS:
      result->base_species = create_new_igneous_species(seed);

      result->persistence = 1.2 + 0.4 * float_hash_1d(seed);
      seed = prng(seed);
      result->scale_bias = 0.7 + 0.4 * float_hash_1d(seed);
      seed = prng(seed);

      result->radial_frequency = M_PI/(2.4 + 1.6*float_hash_1d(seed));
      seed = prng(seed);
      result->radial_variance = 0.1 + 0.3*float_hash_1d(seed);
      seed = prng(seed);

      result->gross_distortion = 900 + 500.0*float_hash_1d(seed);
      seed = prng(seed);
      result->fine_distortion = 110 + 40.0*float_hash_1d(seed);
      seed = prng(seed);

      result->large_var = result->thickness * (0.6 + 0.3*float_hash_1d(seed));
      seed = prng(seed);
      result->med_var = result->thickness * (0.4 + 0.25*float_hash_1d(seed));
      seed = prng(seed);
      result->small_var = result->thickness * (0.17 + 0.05*float_hash_1d(seed));
      seed = prng(seed);
      result->tiny_var = result->thickness * (0.04 + 0.06*float_hash_1d(seed));
      seed = prng(seed);

      result->detail_var = 1.0 + 2.0*float_hash_1d(seed);
      seed = prng(seed);
      result->ridges = 2.0 + 3.0*float_hash_1d(seed);
      seed = prng(seed);

      result->smoothing = 0.15 + 0.2*float_hash_1d(seed);
      seed = prng(seed);

      for (i = 0; i < WM_N_VEIN_TYPES; ++i) {
        result->vein_scale[i] = 0; // 23.4;
        result->vein_strength[i] = 0; // 0.5;
        result->vein_species[i] = 0; // TODO: Pick a material here!
      }

      for (i = 0; i < WM_N_INCLUSION_TYPES; ++i) {
        result->inclusion_frequency[i] = 0; // 0.01;
        result->inclusion_species[i] = 0; // TODO: Pick a material here!
      }
      break;

    case GEO_METAMORPHIC:
      result->base_species = create_new_metamorphic_species(seed);

      result->persistence = 0.8 + 0.5 * float_hash_1d(seed);
      seed = prng(seed);
      result->scale_bias = 0.8 + 0.4 * float_hash_1d(seed);
      seed = prng(seed);

      result->radial_frequency = M_PI/(2.8 + 2.0*float_hash_1d(seed));
      seed = prng(seed);
      result->radial_variance = 0.4 + 0.4*float_hash_1d(seed);
      seed = prng(seed);

      result->gross_distortion = 1200 + 900.0*float_hash_1d(seed);
      seed = prng(seed);
      result->fine_distortion = 180 + 110.0*float_hash_1d(seed);
      seed = prng(seed);

      result->large_var = result->thickness * (0.5 + 0.3*float_hash_1d(seed));
      seed = prng(seed);
      result->med_var = result->thickness * (0.3 + 0.25*float_hash_1d(seed));
      seed = prng(seed);
      result->small_var = result->thickness * (0.16 + 0.06*float_hash_1d(seed));
      seed = prng(seed);
      result->tiny_var = result->thickness * (0.02 + 0.03*float_hash_1d(seed));
      seed = prng(seed);

      result->detail_var = 0.3 + 1.8*float_hash_1d(seed);
      seed = prng(seed);
      result->ridges = 0.4 + 3.4*float_hash_1d(seed);
      seed = prng(seed);

      result->smoothing = 0.15 + 0.45*float_hash_1d(seed);
      seed = prng(seed);

      for (i = 0; i < WM_N_VEIN_TYPES; ++i) {
        result->vein_scale[i] = 0; // 23.4;
        result->vein_strength[i] = 0; // 0.5;
        result->vein_species[i] = 0; // TODO: Pick a material here!
      }

      for (i = 0; i < WM_N_INCLUSION_TYPES; ++i) {
        result->inclusion_frequency[i] = 0; // 0.01;
        result->inclusion_species[i] = 0; // TODO: Pick a material here!
      }
      break;

    case GEO_SEDIMENTAY:
    default:
      result->base_species = create_new_sedimentary_species(seed);

      result->persistence = 1.3 + 0.5 * float_hash_1d(seed);
      seed = prng(seed);
      result->scale_bias = 1.1 + 0.3 * float_hash_1d(seed);
      seed = prng(seed);

      result->radial_frequency = M_PI/(2.1 + 1.2*float_hash_1d(seed));
      seed = prng(seed);
      result->radial_variance = 0.05 + 0.2*float_hash_1d(seed);
      seed = prng(seed);

      result->gross_distortion = 700 + 400.0*float_hash_1d(seed);
      seed = prng(seed);
      result->fine_distortion = 30 + 30.0*float_hash_1d(seed);
      seed = prng(seed);

      result->large_var = result->thickness * (0.4 + 0.25*float_hash_1d(seed));
      seed = prng(seed);
      result->med_var = result->thickness * (0.2 + 0.15*float_hash_1d(seed));
      seed = prng(seed);
      result->small_var = result->thickness * (0.11 + 0.05*float_hash_1d(seed));
      seed = prng(seed);
      result->tiny_var = result->thickness * (0.03 + 0.07*float_hash_1d(seed));
      seed = prng(seed);

      result->detail_var = 0.7 + 3.2*float_hash_1d(seed);
      seed = prng(seed);
      result->ridges = 0.8 + 4.5*float_hash_1d(seed);
      seed = prng(seed);

      result->smoothing = 0.12 + 0.4*float_hash_1d(seed);
      seed = prng(seed);

      for (i = 0; i < WM_N_VEIN_TYPES; ++i) {
        result->vein_scale[i] = 0; // 23.4;
        result->vein_strength[i] = 0; // 0.5;
        result->vein_species[i] = 0; // TODO: Pick a material here!
      }

      for (i = 0; i < WM_N_INCLUSION_TYPES; ++i) {
        result->inclusion_frequency[i] = 0; // 0.01;
        result->inclusion_species[i] = 0; // TODO: Pick a material here!
      }
      break;
  }
  return result;
}

/*************
 * Functions *
 *************/

void generate_geology(world_map *wm) {
  size_t i, j;
  world_map_pos xy;
  global_pos anchor;
  gl_pos_t t;
  stratum *s;

  float avg_size = sqrtf(wm->width*wm->height);
  avg_size *= STRATA_AVG_SIZE;
  avg_size *= WORLD_REGION_BLOCKS;

  map_function profile = MFN_SPREAD_UP;
  geologic_source source = GEO_SEDIMENTAY;
  ptrdiff_t hash, h1, h2, h3, h4, h5;
  world_region *wr;
  for (i = 0; i < WM_MAX_STRATA_LAYERS * STRATA_COMPLEXITY; ++i) {
    // Create a stratum and append it to the list of all strata:
    hash = prng(wm->seed + 567*i);
    h1 = hash_1d(hash);
    h2 = hash_1d(h1);
    h3 = hash_1d(h2);
    h4 = hash_1d(h3);
    h5 = hash_1d(h4);
    switch (h4 % 3) {
      case 0:
        profile = MFN_SPREAD_UP;
        break;
      case 1:
        profile = MFN_TERRACE;
        break;
      case 2:
      default:
        profile = MFN_HILL;
        break;
    }
    switch (h5 % 3) {
      case 0:
        source = GEO_IGNEOUS;
        break;
      case 1:
        source = GEO_METAMORPHIC;
        break;
      case 2:
      default:
        source = GEO_SEDIMENTAY;
        break;
    }
    s = create_stratum(
      hash,
      float_hash_1d(hash)*wm->width, float_hash_1d(h1)*wm->height,
      avg_size * (0.6 + float_hash_1d(h2)*0.8), // size
      BASE_STRATUM_THICKNESS * exp(-0.5 + float_hash_1d(h3)*3.5), // thickness
      profile, // profile
      source
    );
    l_append_element(wm->all_strata, (void*) s);
    // Render the stratum into the various regions:
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = 0; xy.y < wm->height; ++xy.y) {
        // Compute thickness to determine if this region needs to be aware of
        // this stratum:
        compute_region_anchor(wm, &xy, &anchor);
        t = compute_stratum_height(s, &anchor);
        // If any corner has material, add this stratum to this region:
        if (t > 0) {
          //TODO: Real logging/debugging
          wr = get_world_region(wm, &xy); // no need to worry about NULL here
          if (wr->geology.stratum_count < WM_MAX_STRATA_LAYERS) {
            // adjust existing strata:
            for (j = 0; j < wr->geology.stratum_count; ++j) {
              wr->geology.bottoms[j] *= (
                wr->geology.total_height
              ) / (
                wr->geology.total_height + t
              );
            }
            wr->geology.total_height += t;
            wr->geology.bottoms[wr->geology.stratum_count] = 1 - (
              t / fmax(BASE_STRATUM_THICKNESS*6, wr->geology.total_height)
              // the higher of the new total height or approximately 6 strata
              // of height
            );
            wr->geology.strata[wr->geology.stratum_count] = s;
            wr->geology.stratum_count += 1;
          } // it's okay if some strata are zoned out by the layers limit
        }
      }
    }
    if (i % 10 == 0) {
      printf(
        "    ...%zu / %zu strata done...\r",
        i,
        (size_t) (WM_MAX_STRATA_LAYERS * STRATA_COMPLEXITY)
      );
    }
  }
  printf(
    "    ...%zu / %zu strata done...\r",
    (size_t) (WM_MAX_STRATA_LAYERS * STRATA_COMPLEXITY),
    (size_t) (WM_MAX_STRATA_LAYERS * STRATA_COMPLEXITY)
  );
  printf("\n");
}

gl_pos_t compute_stratum_height(stratum *st, global_pos *glpos) {
  // static variables:
  static stratum *pr_st = NULL;
  static global_chunk_pos pr_glcpos = { .x = -1, .y = -1, .z = -1 };
  // low- and high-frequency distortion:
  static float lfdx = 0; static float lfdy = 0;
  // low- and high-frequency noise:
  static float lfn = 0;
  // base thickness:
  static float base = 0;

  // normal variables:
  float fx;
  float fy;
  global_chunk_pos glcpos;

  // compute our chunk position:
  glpos__glcpos(glpos, &glcpos);

  if (pr_st != st || pr_glcpos.x != glcpos.x || pr_glcpos.y != glcpos.y) {
    // need to recompute low-frequency info:
    fx = (float) (glpos->x);
    fy = (float) (glpos->y);
    stratum_lf_distortion(st, fx, fy, &lfdx, &lfdy);
    stratum_lf_noise(st, fx+lfdx, fy+lfdy, &lfn);
    stratum_base_thickness(st, fx+lfdx, fy+lfdy, &base);
  }
  // set static variables:
  copy_glcpos(&glcpos, &pr_glcpos);
  pr_st = st;
  return (gl_pos_t) fastfloor(base + lfn);
}

species create_new_igneous_species(ptrdiff_t seed) {
  species result = create_stone_species();
  stone_species* ssp = get_stone_species(result);

  seed = prng(seed);

  float base_density = pow(norm_hash_1d(seed), 0.8);

  determine_new_igneous_material(&(ssp->material), seed, base_density);
  seed = prng(seed);
  determine_new_igneous_appearance(&(ssp->appearance), seed, base_density);

  return result;
}

species create_new_metamorphic_species(ptrdiff_t seed) {
  species result = create_stone_species();
  stone_species* ssp = get_stone_species(result);

  seed = prng(seed);

  float base_density = norm_hash_1d(seed);

  determine_new_metamorphic_material(&(ssp->material), seed, base_density);
  seed = prng(seed);
  determine_new_metamorphic_appearance(&(ssp->appearance), seed, base_density);

  return result;
}

species create_new_sedimentary_species(ptrdiff_t seed) {
  species result = create_stone_species();
  stone_species* ssp = get_stone_species(result);

  seed = prng(seed);

  float base_density = pow(norm_hash_1d(seed), 0.8);

  determine_new_sedimentary_material(&(ssp->material), seed, base_density);
  seed = prng(seed);
  determine_new_sedimentary_appearance(&(ssp->appearance), seed, base_density);

  return result;
}

void determine_new_igneous_material(
  material *target,
  ptrdiff_t seed,
  float base_density
) {
  target->origin = MO_IGNEOUS_MINERAL;

  seed = prng(seed);

  // Base density is taken from the external argument:
  target->solid_density = mat_density(0.25 + 4.75 * base_density);
  target->liquid_density = mat_density(2.5 + 0.5 * base_density);
  target->gas_density = mat_density(1.8 + 1.5 * base_density);

  // A much tighter distribution of specific heats that correlates a bit with
  // density:
  float base_specific_heat = (norm_hash_1d(seed) + norm_hash_1d(seed))/2.0;
  base_specific_heat = 0.7*base_specific_heat + 0.3*(1 - base_density);
  seed = prng(seed);

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
  seed = prng(seed);

  target->solidus = 550 + 700 * base_transition_temp;
  target->liquidus = target->solidus + 50 + norm_hash_1d(seed) * 200;
  seed = prng(seed);

  target->boiling_point = 1800 + base_transition_temp * 600;

  // igneous stone isn't known for combustion:
  target->ignition_point = smaxof(temperature);
  target->flash_point = smaxof(temperature);

  // it is generally not very malleable, and may be as brittle as glass:
  target->malleability = 80 * norm_hash_1d(seed);
  seed = prng(seed);

  // magma is extremely viscous:
  target->viscosity = pow(10.0, 6.0 + 10.0*float_hash_1d(seed));
  seed = prng(seed);

  // igneous rocks are pretty hard (and again this correlates with density):
  target->hardness = 100+120*(0.8*norm_hash_1d(seed) + 0.2*base_density);
}

void determine_new_metamorphic_material(
  material *target,
  ptrdiff_t seed,
  float base_density
) {
  target->origin = MO_METAMORPHIC_MINERAL;

  seed = prng(seed);

  // Base density is taken from the external argument:
  target->solid_density = mat_density(0.9 + 4.8 * base_density);
  target->liquid_density = mat_density(2.6 + 0.6 * base_density);
  target->gas_density = mat_density(1.8 + 1.5 * base_density);

  // A tighter distribution of specific heats that correlates a bit with
  // density:
  float base_specific_heat = norm_hash_1d(seed);
  base_specific_heat = 0.7*base_specific_heat + 0.3*(1 - base_density);
  seed = prng(seed);

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
  seed = prng(seed);

  target->solidus = 520 + 760 * base_transition_temp;
  target->liquidus = target->solidus + 20 + norm_hash_1d(seed) * 300;
  seed = prng(seed);

  target->boiling_point = 1700 + base_transition_temp * 800;

  // normal metamorphic stone isn't known for combustion:
  // TODO: Some high values here?
  target->ignition_point = smaxof(temperature);
  target->flash_point = smaxof(temperature);

  // it may be somewhat malleable, and is never completely brittle
  target->malleability = 10 + 90 * norm_hash_1d(seed);
  seed = prng(seed);

  // magma is extremely viscous:
  target->viscosity = pow(10.0, 6.0 + 10.0*float_hash_1d(seed));
  seed = prng(seed);

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

  seed = prng(seed);

  // Base density is taken from the external argument:
  target->solid_density = mat_density(1.2 + 3.7 * base_density);
  target->liquid_density = mat_density(2.6 + 0.5 * base_density);
  target->gas_density = mat_density(1.6 + 1.3 * base_density);

  // A tighter distribution of specific heats that correlates a bit with
  // density:
  float base_specific_heat = norm_hash_1d(seed);
  base_specific_heat = 0.7*base_specific_heat + 0.3*(1 - base_density);
  seed = prng(seed);

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
  seed = prng(seed);

  target->solidus = 440 + 780 * base_transition_temp;
  target->liquidus = target->solidus + 10 + norm_hash_1d(seed) * 180;
  seed = prng(seed);

  target->boiling_point = 1550 + base_transition_temp * 600;

  // normal sedimentary stone isn't known for combustion (see special fuel
  // stone generation methods):
  // TODO: Some high values here?
  target->ignition_point = smaxof(temperature);
  target->flash_point = smaxof(temperature);

  // it may be somewhat malleable, but it can also be quite brittle
  target->malleability = 5 + 110 * norm_hash_1d(seed);
  seed = prng(seed);

  // sedimentary magma is generally more viscous than other types (says I):
  target->viscosity = pow(10.0, 5.0 + 8.0*float_hash_1d(seed));
  seed = prng(seed);

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
  seed = prng(seed);
  target->seed = seed;

  // Lighter rocks tend to have smaller noise scales.
  target->scale = 0.1 + 0.08*(0.8*float_hash_1d(seed) + 0.2*base_density);
  seed = prng(seed);

  // Igneous rock types are relatively gritty. Denser rocks tend to be slightly
  // less gritty though.
  target->gritty = 1.4 + 2.3*(0.7*float_hash_1d(seed) + 0.3*(1-base_density));
  seed = prng(seed);

  // Denser rocks tend to be more contoured.
  target->contoured = 3.5 + 3.5*(0.8*float_hash_1d(seed) + 0.2*base_density);
  seed = prng(seed);

  // Lighter igneous rocks are far more porous.
  target->porous = 2.0 + 8.5*(0.5*float_hash_1d(seed) + 0.5*(1-base_density));
  seed = prng(seed);

  // Igneous rocks don't tend to be very bumpy.
  target->bumpy = 1.0 + 4.0*float_hash_1d(seed);
  seed = prng(seed);

  // Igneous rocks rarely have significant inclusions.
  target->inclusions = pow(norm_hash_1d(seed), 2.5);
  seed = prng(seed);

  // The distortion scale is within 10% of the base scale.
  target->dscale = target->scale * 1 + 0.2*(float_hash_1d(seed) - 0.5);
  seed = prng(seed);

  // Igneous rocks can have at most moderate distortion, and largely have very
  // little distortion.
  target->distortion = 4.4*pow(norm_hash_1d(seed), 2);
  seed = prng(seed);

  // Igneous rocks can be squashed in either direction
  target->squash = 0.7 + 0.6*norm_hash_1d(seed);
  seed = prng(seed);
  target->squash /= 0.7 + 0.6*norm_hash_1d(seed);
  seed = prng(seed);

  // Hues range from blue to orange:
  float hue = -0.2 + 0.3*float_hash_1d(seed);
  seed = prng(seed);
  if (hue < 0) {
    hue = 1 + hue;
  }

  // Saturation is usually negligible though:
  float sat = 0;
  if (float_hash_1d(seed) < 0.7) {
    seed = prng(seed);
    sat = 0.05*float_hash_1d(seed);
  } else {
    seed = prng(seed);
    sat = 0.3*float_hash_1d(seed);
  }
  seed = prng(seed);

  // Base values are correlated with density, and are mostly dark:
  float val = 0.6*float_hash_1d(seed) + 0.4*(1-base_density);
  seed = prng(seed);
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
  seed = prng(seed);
  target->seed = seed;

  // Metamorphic rocks exhibit a wide range of noise scales:
  target->scale = 0.08 + 0.12*float_hash_1d(seed);
  seed = prng(seed);

  // Metamorphic rock types are usually not very gritty, especially when
  // they're very dense.
  target->gritty = 0.8 + 2.5*(0.7*float_hash_1d(seed) + 0.3*(1-base_density));
  seed = prng(seed);

  // Denser rocks tend to be more contoured.
  target->contoured = 1.5 + 7.5*(0.8*float_hash_1d(seed) + 0.2*base_density);
  seed = prng(seed);

  // Lighter metamorphic rocks are a bit more porous.
  target->porous = 1.0 + 8.0*(0.7*float_hash_1d(seed) + 0.3*(1-base_density));
  seed = prng(seed);

  // Metamorphic rocks can be quite bumpy, especially when less dense.
  target->bumpy = 1.0 + 9.0*(0.6*float_hash_1d(seed) + 0.4*(1-base_density));
  seed = prng(seed);

  // Metamorphic rocks often have significant inclusions.
  target->inclusions = pow(float_hash_1d(seed), 1.3);
  seed = prng(seed);

  // The distortion scale is within 10% of the base scale.
  target->dscale = target->scale * 1 + 0.2*(float_hash_1d(seed) - 0.5);
  seed = prng(seed);

  // Metamorphic rocks can have quite a bit of distortion.
  target->distortion = 6.5*norm_hash_1d(seed);
  seed = prng(seed);

  // And they can be squashed quite a bit.
  target->squash = 0.6 + 0.8*norm_hash_1d(seed);
  seed = prng(seed);
  target->squash /= 0.6 + 0.8*norm_hash_1d(seed);
  seed = prng(seed);

  // All kinds of hues are possible.
  float hue = float_hash_1d(seed);
  seed = prng(seed);

  // Saturation is usually small.
  float sat = norm_hash_1d(seed);
  sat *= sat;
  sat *= 0.4;
  seed = prng(seed);

  // Base values have a wide range:
  float val = 0.2 + 0.6*float_hash_1d(seed);
  seed = prng(seed);

  // Construct the base color:
  pixel hsv = float_color(hue, sat, val, 1.0);
  hsv__rgb(hsv, &(target->base_color));

  // Metamorphic inclusions use the same distributions as the base rock, but
  // have a wider range of saturations:
  hue = float_hash_1d(seed);
  seed = prng(seed);
  sat = 0.6*pow(norm_hash_1d(seed), 1.5);
  seed = prng(seed);
  val = 0.2 + 0.6*float_hash_1d(seed);
  seed = prng(seed);

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
  seed = prng(seed);
  target->seed = seed;

  // Sedimentary rocks often have smaller scales than other rocks.
  target->scale = 0.07 + 0.07*float_hash_1d(seed);
  seed = prng(seed);

  // Sedimentary rock types are usually gritty.
  target->gritty = 3.1 + 2.5*float_hash_1d(seed);
  seed = prng(seed);

  // Denser rocks tend to be more contoured.
  target->contoured = 1.5 + 9.5*(0.7*float_hash_1d(seed) + 0.3*base_density);
  seed = prng(seed);

  // Lighter sedimentary rocks are a bit more porous.
  target->porous = 3.0 + 5.0*(0.7*float_hash_1d(seed) + 0.3*(1-base_density));
  seed = prng(seed);

  // Sedimentary rocks can be quite bumpy.
  target->bumpy = 3.0 + 6.0*float_hash_1d(seed);
  seed = prng(seed);

  // Sedimentary rocks usually don't have inclusions.
  target->inclusions = pow(norm_hash_1d(seed), 2.5);
  seed = prng(seed);

  // The distortion scale is within 20% of the base scale.
  target->dscale = target->scale * 1 + 0.4*(float_hash_1d(seed) - 0.5);
  seed = prng(seed);

  // Sedimentary rocks can have little to medium distortion.
  target->distortion = 4.5*pow(norm_hash_1d(seed), 1.4);
  seed = prng(seed);

  // Sedimentary rocks are usually squashed horizontally.
  target->squash = 0.6 + 0.6*norm_hash_1d(seed);
  seed = prng(seed);
  target->squash /= 0.8 + 0.6*norm_hash_1d(seed);
  seed = prng(seed);

  // Sedimentary rocks can be yellowish to bluish, or just gray:
  float hue = 0.15 + 0.5*float_hash_1d(seed);
  seed = prng(seed);

  // Saturation is usually small.
  float sat = norm_hash_1d(seed);
  seed = prng(seed);
  if (float_hash_1d(seed) < 0.4) {
    sat = 0;
  }
  sat *= sat;
  sat *= 0.3;
  seed = prng(seed);

  // Base values have a wide range:
  float val = 0.1 + 0.8*float_hash_1d(seed);
  seed = prng(seed);

  // Construct the base color:
  pixel hsv = float_color(hue, sat, val, 1.0);
  hsv__rgb(hsv, &(target->base_color));

  // Sedimentary inclusions use the same distributions as the base rock, but
  // have a wider range of saturations:
  hue = float_hash_1d(seed);
  seed = prng(seed);
  sat = 0.5*pow(norm_hash_1d(seed), 1.2);
  seed = prng(seed);
  val = 0.1 + 0.8*float_hash_1d(seed);
  seed = prng(seed);

  hsv = float_color(hue, sat, val, 1.0);
  hsv__rgb(hsv, &(target->alt_color));

  // Sedimentary rocks are generally bright.
  target->brightness = -0.05 + 0.35*norm_hash_1d(seed);
}
