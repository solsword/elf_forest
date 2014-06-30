#ifndef GEOLOGY_H
#define GEOLOGY_H

// geology.h
// Stone types and strata generation.

#include <stdint.h>

#include "math/functions.h"

#include "world/materials.h"
#include "world/world.h"

/*********
 * Enums *
 *********/

// Geologic sources influence how material types and other stratum parameters
// are chosen.
enum geologic_source_e {
  GEO_IGNEOUS,
  GEO_METAMORPHIC,
  GEO_SEDIMENTAY
};
typedef enum geologic_source_e geologic_source;

/************************
 * Types and Structures *
 ************************/

// A layer of material that encompasses some region of the world.
struct stratum_s;
typedef struct stratum_s stratum;

/*************
 * Constants *
 *************/

// Maximum number of stone layers per world region
//#define MAX_STRATA_LAYERS 32
#define MAX_STRATA_LAYERS 64

// Maximum number of material types present in other layers as veins
#define N_VEIN_TYPES 2

// Maximum number of material types included in other layers
#define N_INCLUSION_TYPES 8

/***********
 * Globals *
 ***********/

// The seed for geothermal information, which both helps determine strata
// placement and contributes to metamorphosis.
extern ptrdiff_t const GEOTHERMAL_SEED;

// Various GN_ (geology noise) constants used for defining default noise
// parameters during strata generation:
extern float const GN_GROSS_DISTORTION_SCALE;
extern float const GN_FINE_DISTORTION_SCALE;
extern float const GN_LARGE_VAR_SCALE;
extern float const GN_MED_VAR_SCALE;
extern float const GN_SMALL_VAR_SCALE;
extern float const GN_TINY_VAR_SCALE;
extern float const GN_DETAIL_VAR_SCALE;
extern float const GN_RIDGE_SCALE;

/*************************
 * Structure Definitions *
 *************************/

struct stratum_s {
  ptrdiff_t seed; // seed for various noise sources

 // Base parameters:
 // ----------------
  float cx, cy; // center x/y
  float size; // base radius
  float thickness; // base thickness
  map_function profile; // base profile shape
  geologic_source source; // where the material for this layer comes from

 // Derived noise parameters:
 // -------------------------
  float scale_bias; // biases the noise scales
  float infill; // how much to fill in noise from the layer below

  // radial variance:
  float radial_frequency;
  float radial_variance;

  // distortion:
  float gross_distortion; // large-scale distortion
  float fine_distortion; // small-scale distortion

  // core variation (expressed in max blocks)
  float large_var; // large-scale variation
  float med_var; // medium-scale variation
  float small_var; // small-scale variation
  float tiny_var; // tiny-scale variation

  // positive detail (expressed in max blocks)
  float detail_var; // detail-scale variation
  float ridges; // amplitude of ridges

  // negative detail:
  float smoothing; // amount of smooth weathering (fraction of detail removed)

 // Derived vein and inclusion information:
 // ---------------------------------------
  float vein_scale[N_VEIN_TYPES]; // scale of different veins (in blocks)
  float vein_strength[N_VEIN_TYPES]; // thickness and frequency of veins (0-1)
  float inclusion_frequency[N_INCLUSION_TYPES]; // frequency of inclusions (0-1)

 // Derived material type information:
 // ----------------------------------
  material base_material; // exact material type for main mass
  material vein_material[N_VEIN_TYPES]; // types for veins
  material inclusion_material[N_INCLUSION_TYPES]; // types for inclusions

 // Dynamic factors are erosion and pressure which influence compression and
 // metamorphosis.
};

/********************
 * Inline Functions *
 ********************/

static inline float geothermal_temperature(region_pos *rpos) {
  return 100.0; // TODO: HERE!
}

// Computes stratum base thickness at the given region position.
static inline void stratum_base_thickness(
  stratum *st,
  float x, float y,
  float *thickness
) {
  // find angle and compute radius, followed by base thickness
  vector v;
  v.x = x - st->cx;
  v.y = y - st->cy;
  v.z = 0;
  float theta = atan2(v.y, v.x);
  float d = vmag(&v);
  float r = st->size * (
    (1 - st->radial_variance) +
    2 * st->radial_variance * managed_sxnoise_2d(
      theta, st->seed,
      st->radial_frequency, 1,
      57.3
    )
  );
  *thickness = fmap(1 - d/r, st->profile); // base thickness (possibly < 0)
}

// Computes low-frequency distortion dx and dy at the given region position.
static inline void stratum_lf_distortion(
  stratum *st,
  float x, float y,
  float *dx, float *dy
) {
  // compute distortion
  float scale = GN_GROSS_DISTORTION_SCALE * st->scale_bias;
  *dx = st->gross_distortion * managed_sxnoise_2d(
    x, y,
    scale, scale,
    4.5 * st->seed
  );
  *dy = st->gross_distortion * managed_sxnoise_2d(
    x, y,
    scale, scale,
    5.4 * st->seed
  );
}

// Computes higher-frequency distortion dx and dy at the given region position.
static inline void stratum_hf_distortion(
  stratum *st,
  float x, float y,
  float *dx, float *dy
) {
  float scale = GN_FINE_DISTORTION_SCALE * st->scale_bias;
  *dx += st->fine_distortion * managed_sxnoise_2d(
    x, y,
    scale, scale,
    7.2 * st->seed
  );
  *dy += st->fine_distortion * managed_sxnoise_2d(
    x, y,
    scale, scale,
    2.7 * st->seed
  );
}

// Computes stratum low-frequency noise.
static inline void stratum_lf_noise(
  stratum *st,
  float x, float y,
  float *noise
) {
  // Compute noise:
  float scale = GN_LARGE_VAR_SCALE * st->scale_bias;
  *noise = st->large_var * managed_sxnoise_2d(
    x, y,
    scale, scale,
    st->seed*84.1
  );
  scale = GN_MED_VAR_SCALE * st->scale_bias;
  *noise += st->med_var * managed_sxnoise_2d(
    x, y,
    scale, scale,
    st->seed*14.8
  );
}

// Computes stratum high-frequency noise.
static inline void stratum_hf_noise(
  stratum *st,
  float x, float y,
  float *noise
) {
  float scale = GN_SMALL_VAR_SCALE * st->scale_bias;
  *noise += st->small_var * managed_sxnoise_2d(
    x, y,
    scale, scale,
    st->seed*48.1
  );
  scale = GN_TINY_VAR_SCALE * st->scale_bias;
  *noise += st->tiny_var * managed_sxnoise_2d(
    x, y,
    scale, scale,
    st->seed*18.4
  );
  scale = GN_RIDGE_SCALE * st->scale_bias;
  *noise += st->ridges * managed_wrnoise_2d(
    x, y,
    scale, scale,
    st->seed*84.1
  );
  scale = GN_DETAIL_VAR_SCALE * st->scale_bias;
  *noise += st->detail_var * managed_sxnoise_2d(
    x, y,
    scale, scale,
    st->seed*41.8
  );
  *noise *= (1 - st->smoothing);
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new stratum with the given parameters.
stratum *create_stratum(
  ptrdiff_t seed,
  float cx, float cy,
  float size, float thickness,
  map_function profile,
  geologic_source source
);

/*************
 * Functions *
 *************/

// Computes the heigh of the given stratum at the given coordinates (ignores z):
r_pos_t compute_stratum_height(stratum *st, region_pos *rpos);

#endif // ifndef GEOLOGY_H
