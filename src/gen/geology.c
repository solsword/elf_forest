// geology.c
// Stone types and strata generation.

#include "noise/noise.h"
#include "math/functions.h"

#include "geology.h"

/***********
 * Globals *
 ***********/

float GEOTHERMAL_SEED = 3975.48;

/******************************
 * Constructors & Destructors *
 ******************************/

stratum *create_stratum(
  float seed,
  float cx, float cy,
  float size, float thickness,
  map_function profile,
  material_origin source
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

  result->gross_distortion = 120.0;
  result->fine_distortion = 40.0;

  result->large_var = 0.2;
  result->med_var = 0.12;

  result->small_var = 2.3;
  result->ridges = 2.5;

  result->scraping = 0.7;
  result->smoothing = 0.3;

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

static inline float stratum_core(r_pos_t x, r_pos_t y, stratum *st) {
  return 12.0; // TODO: HERE!
}

static inline float stratum_detail(r_pos_t x, r_pos_t y, stratum *st) {
  return 3.0; // TODO: HERE!
}

static inline float stratum_infill(
  r_pos_t x, r_pos_t y,
  stratum *st,
  stratum *below
) {
  return 3.0; // TODO: HERE!
}

/*************
 * Functions *
 *************/

float stratum_height(
  r_pos_t x, r_pos_t y,
  stratum *st,
  stratum *below,
  column_dynamics *cd,
  stratum_dynamics *sd
) {
  float result = 0;

  // base height
  float core = stratum_core(x, y, st);
  float detail = stratum_detail(x, y, st);
  float infill = stratum_infill(x, y, st, below);
  sd->infill = (r_pos_t) infill;
  result = core + detail + infill;

  // erosion
  float erosion_rate = mt_erosion_rate(st->base_material);
  float erosion = erosion_rate * cd->erosion;
  cd->erosion = cd->erosion - (result/erosion_rate);
  if (cd->erosion < 0) { cd->erosion = 0; }
  result = result - erosion;
  if (result < 0) { result = 0; }

  // add our weight to pressure pre-compression:
  cd->pressure += mt_weight(st->base_material)*result;

  // compression
  float compression = mt_compression(st->base_material, cd->pressure);
  result *= compression;
  if (result < 1) { result = 1; }
  sd->pressure = cd->pressure;

  return result;
}

material stratum_material(
  r_pos_t x, r_pos_t y, r_pos_t height,
  stratum *st,
  stratum_dynamics *sd
) {
  // pick a material:
  material mat = st->base_material;
  // TODO: veins and inclusions here!
  //if (height < sd->infill) {
  //  return st->base_material;
  //}

  // metamorphosis:
  float temperature = geothermal_temperature(x, y, sd->height);
  float metamorphosis_probability = mt_metamorphosis_rate(
    mat,
    sd->pressure,
    temperature
  );
  if (
    float_hash_1d(
      hash_2d(
        mixed_hash_1d(x) + mixed_hash_1d(y) + mixed_hash_1d(height),
        mixed_hash_1d(st->seed)
      )
    ) < metamorphosis_probability
  ) { // metamorphose
    mat = mt_metamorphic_product(mat, sd->pressure, temperature);
  }

  return mat;
}
