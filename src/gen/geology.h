#ifndef GEOLOGY_H
#define GEOLOGY_H

// geology.h
// Stone types and strata generation.

#include <stdint.h>

#include "math/functions.h"

#include "txgen/txg_minerals.h"
#include "world/materials.h"
#include "world/world.h"
#include "world/world_map.h"

#include "util.h"

/*************
 * Constants *
 *************/

// Controls the size of strata relative to the world map size.
#define STRATA_AVG_SIZE 0.25

// Controls how many strata to generate (a multiple of MAX_STRATA_LAYERS).
//#define STRATA_COMPLEXITY 3.0
#define STRATA_COMPLEXITY (1/32.0)

// The base stratum thickness (before an exponential distribution).
#define BASE_STRATUM_THICKNESS 10.0

/***********
 * Globals *
 ***********/

// The seed for geothermal information, which both helps determine strata
// placement and contributes to metamorphosis.
// TODO: Actualize this!
extern ptrdiff_t const GEOTHERMAL_SEED;

// Various GN_ (geology noise) constants used for defining default noise
// parameters during strata generation:
extern float const GN_DISTORTION_SCALE;
extern float const GN_LARGE_VAR_SCALE;
extern float const GN_MED_VAR_SCALE;

/********************
 * Inline Functions *
 ********************/

static inline float geothermal_temperature(global_pos *glpos) {
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
    1 + st->radial_variance * sxnoise_2d(
      theta / st->radial_frequency,
      0,
      prng(st->seed)
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
  float scale = GN_DISTORTION_SCALE * st->scale_bias;
  *dx = st->gross_distortion * sxnoise_2d(
    x/scale, y/scale,
    prng(st->seed+1)
  );
  *dy = st->gross_distortion * sxnoise_2d(
    x/scale, y/scale,
    prng(st->seed+2)
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
  *noise = st->large_var * sxnoise_2d(
    x/scale, y/scale,
    prng(st->seed+5)
  );
  scale = GN_MED_VAR_SCALE * st->scale_bias;
  *noise += st->med_var * sxnoise_2d(
    x/scale, y/scale,
    prng(st->seed+6)
  );
}

// Given a world region and a fractional height between 0 and 1, returns the
// stratum in that region at that height. If h is less than 0, it returns the
// bottom stratum in the given region, and likewise if h is greater than 1, it
// returns the top stratum.
static inline stratum* get_stratum(
  world_region* wr,
  float h
) {
  int i;
  stratum *result = wr->geology.strata[0];
  // Find out which layer we're in:
  for (i = 1; i < wr->geology.stratum_count; i += 1) {
    if (h < wr->geology.bottoms[i]) { // might not happen at all
      break;
    }
    result = wr->geology.strata[i];
  }
  return result;
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

// Generates geology for the given world.
void generate_geology(world_map *wm);

// Computes the heigh of the given stratum at the given coordinates (ignores z):
gl_pos_t compute_stratum_height(stratum *st, global_pos *glpos);

// Functions that create new types of stone:
species create_new_igneous_species(ptrdiff_t seed);
species create_new_metamorphic_species(ptrdiff_t seed);
species create_new_sedimentary_species(ptrdiff_t seed);

// Helper functions for creating materials:
void determine_new_igneous_material(
  material *target,
  ptrdiff_t seed,
  float base_density
);
void determine_new_metamorphic_material(
  material *target,
  ptrdiff_t seed,
  float base_density
);
void determine_new_sedimentary_material(
  material *target,
  ptrdiff_t seed,
  float base_density
);

// Helper functions for creating appearances:
void determine_new_igneous_appearance(
  stone_filter_args *target,
  ptrdiff_t seed,
  float base_density
);

void determine_new_metamorphic_appearance(
  stone_filter_args *target,
  ptrdiff_t seed,
  float base_density
);

void determine_new_sedimentary_appearance(
  stone_filter_args *target,
  ptrdiff_t seed,
  float base_density
);

#endif // ifndef GEOLOGY_H
