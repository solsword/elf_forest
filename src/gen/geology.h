#ifndef GEOLOGY_H
#define GEOLOGY_H

// geology.h
// Stone types and strata generation.

#include <stdint.h>

#include "math/functions.h"

#include "txgen/txg_minerals.h"
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
#define MAX_STRATA_LAYERS 256

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
extern float const GN_DISTORTION_SCALE;
extern float const GN_LARGE_VAR_SCALE;
extern float const GN_MED_VAR_SCALE;

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
  float persistence; // how much this layer extends at the expense of others
   // Note that larger values are stronger; [0.5, 2] is reasonable.
  float scale_bias; // biases the noise scales

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

 // Derived species information:
 // ----------------------------------
  species base_species; // exact material type for main mass
  species vein_species[N_VEIN_TYPES]; // types for veins
  species inclusion_species[N_INCLUSION_TYPES]; // types for inclusions
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
    1 + st->radial_variance * sxnoise_2d(
      theta / st->radial_frequency,
      0,
      expanded_hash_1d(st->seed)
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
    expanded_hash_1d(st->seed+1)
  );
  *dy = st->gross_distortion * sxnoise_2d(
    x/scale, y/scale,
    expanded_hash_1d(st->seed+2)
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
    expanded_hash_1d(st->seed+5)
  );
  scale = GN_MED_VAR_SCALE * st->scale_bias;
  *noise += st->med_var * sxnoise_2d(
    x/scale, y/scale,
    expanded_hash_1d(st->seed+6)
  );
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
