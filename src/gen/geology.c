// geology.c
// Stone types and strata generation.

#include "noise/noise.h"
#include "math/functions.h"
#include "datatypes/vector.h"

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

/*************
 * Functions *
 *************/

void compute_stratum_dynamics(
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

  // store the end result:
  sd->thickness = (r_pos_t) result;
}

block stratum_material(
  region_pos *rpos,
  stratum *st,
  stratum_dynamics *sd
) {
  return b_make_species(B_STONE, 0);
  /*
  r_pos_t h = rpos->z - sd->elevation;
  // TODO: HERE!
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
  */
}
