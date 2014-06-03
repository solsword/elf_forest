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

// A layer of "erosion" which eats away at strata beneath it.
// TODO: Use these...
struct erosion_layer_s;
typedef struct erosion_layer_s erosion_layer;

// Records per-column dynamics (pressure and erosion) as stratum heights are
// being calculated within a column of cells.
struct column_dynamics_s;
typedef struct column_dynamics_s column_dynamics;

// Stratum dynamics are assigned to a strata for a particular column of rock,
// but are fixed specific to that strata rather than being updated during
// computation of the column and applying to all strata in the column like
// column dynamics.
struct stratum_dynamics_s;
typedef struct stratum_dynamics_s stratum_dynamics;

/*************
 * Constants *
 *************/

// Maximum number of stone layers per world region
#define MAX_STRATA_LAYERS 128

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
  float scraping; // amount of scraping (max blocks)
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

struct erosion_layer_s {
  ptrdiff_t seed; // The seed for noise
  // TODO: HERE!
};

struct column_dynamics_s {
  float pressure;
  float erosion;
};

struct stratum_dynamics_s {
  r_pos_t thickness; // thickness of this stratum in blocks
  r_pos_t infill; // how many blocks of infill this stratum has in this column
  float pressure; // pressure at the top of this stratum
  r_pos_t elevation; // height in blocks of the base of this stratum
};

/********************
 * Inline Functions *
 ********************/

static inline float geothermal_temperature(r_pos_t x, r_pos_t y, r_pos_t z) {
  return 100.0; // TODO: HERE!
}

static inline float stratum_core(r_pos_t x, r_pos_t y, stratum *st) {
  // compute distortion
  float scale = GN_GROSS_DISTORTION_SCALE * st->scale_bias;
  float dx = st->gross_distortion * managed_sxnoise_2d(
    x, y,
    scale, scale,
    4.5 * st->seed
  );
  float dy = st->gross_distortion * managed_sxnoise_2d(
    x, y,
    scale, scale,
    5.4 * st->seed
  );
  scale = GN_FINE_DISTORTION_SCALE * st->scale_bias;
  dx += st->fine_distortion * managed_sxnoise_2d(
    x, y,
    scale, scale,
    7.2 * st->seed
  );
  dy += st->fine_distortion * managed_sxnoise_2d(
    x, y,
    scale, scale,
    2.7 * st->seed
  );

  // find angle and compute radius, followed by base thickness
  vector v;
  v.x = x + dx - st->cx;
  v.y = y + dy - st->cy;
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
  float t = 1 - d/r; // base thickness (possibly < 0)

  // Compute noise:
  scale = GN_LARGE_VAR_SCALE * st->scale_bias;
  float n = st->large_var * managed_sxnoise_2d(
    v.x, v.y,
    scale, scale,
    st->seed*84.1
  );
  scale = GN_MED_VAR_SCALE * st->scale_bias;
  n += st->med_var * managed_sxnoise_2d(
    v.x, v.y,
    scale, scale,
    st->seed*14.8
  );
  scale = GN_SMALL_VAR_SCALE * st->scale_bias;
  n += st->small_var * managed_sxnoise_2d(
    v.x, v.y,
    scale, scale,
    st->seed*48.1
  );
  scale = GN_TINY_VAR_SCALE * st->scale_bias;
  n += st->tiny_var * managed_sxnoise_2d(
    v.x, v.y,
    scale, scale,
    st->seed*18.4
  );
  return st->thickness * fmap(t, st->profile) + n;
}

static inline float stratum_detail(r_pos_t x, r_pos_t y, stratum *st) {
  return 3.0; // TODO: HERE!
}

static inline float stratum_infill(
  r_pos_t x, r_pos_t y,
  stratum *st,
  stratum *below
) {
  if (below == NULL) {
    return 0;
  }
  return 3.0; // TODO: HERE!
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

// Computes the thickness, infill, and pressure of the given stratum at the
// given x/y position, storing that information into the given stratum_dynamics
// object and updating the given column_dynamics object. Needs to know the
// next-lower stratum (for infill). Does not compute the elevation (that can
// only be know after all of the strata in a stack have had their dynamics
// computed).
void compute_stratum_dynamics(
  r_pos_t x, r_pos_t y,
  stratum *st,
  stratum *below,
  column_dynamics *cd,
  stratum_dynamics *sd
);

// Computes the block at the given position (which is assumed to fall into the
// given stratum) Needs to know the stratum dynamics computed by
// compute_stratum_dynamics.
block stratum_material(
  region_pos *rpos,
  stratum *st,
  stratum_dynamics *sd
);

#endif // ifndef GEOLOGY_H
