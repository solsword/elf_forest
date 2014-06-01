#ifndef GEOLOGY_H
#define GEOLOGY_H

// geology.h
// Stone types and strata generation.

#include <stdint.h>

#include "math/functions.h"

#include "world/materials.h"

/************************
 * Types and Structures *
 ************************/

// A layer of material that encompasses some region of the world.
struct stratum_s;
typedef struct stratum_s stratum;

// A layer of "erosion" which eats away at strata beneath it.
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
extern float GEOTHERMAL_SEED;

extern float GROSS_DISTORTION_SCALE;
extern float FINE_DISTORTION_SCALE;
extern float LARGE_VAR_SCALE;
extern float MED_VAR_SCALE;
extern float SMALL_VAR_SCALE;
extern float RIDGE_SCALE;

/*************************
 * Structure Definitions *
 *************************/

struct stratum_s {
  float seed; // seed for various noise sources

 // Base parameters:
 // ----------------
  float cx, cy; // center x/y
  float size; // base radius
  float thickness; // base thickness
  map_function profile; // base profile shape
  material_origin source; // where the material for this layer comes from

 // Derived noise parameters:
 // -------------------------
  float scale_bias; // biases the noise scales
  float infill; // how much to fill in noise from the layer below

  // distortion:
  float gross_distortion; // large-scale distortion
  float fine_distortion; // small-scale distortion

  // core variation (expressed as a fraction of the base thickness):
  float large_var; // large-scale variation
  float med_var; // medium-scale variation

  // positive detail (expressed in max blocks)
  float small_var; // small-scale variation
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
  float seed; // The seed for noise
  // TODO: HERE!
}

struct column_dynamics_s {
  float pressure;
  float erosion;
};

struct stratum_dynamics_s {
  float pressure; // pressure at the top of this stratum
  r_pos_t height; // height in blocks of the base of this stratum
  r_pos_t infill; // how many blocks of infill this stratum has in this column
};

/********************
 * Inline Functions *
 ********************/

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new stratum with the given parameters.
stratum *create_stratum(
  float seed,
  float cx, float cy,
  float size, float thickness,
  map_function profile,
  material_origin source
);

/*************
 * Functions *
 *************/

// Computes the height of a stratum at a given x/y position. Needs to know
// stratum dynamics information as well as the next-lower stratum (for infill).
float stratum_height(
  r_pos_t x, r_pos_t y,
  stratum *st,
  stratum *below,
  column_dynamics *cd,
  stratum_dynamics *sd
);

// Computes the block at the given position within a stratum, using integer
// height in cells from the bottom of the stratum. Needs to know the stratum
// dynamics computed during a call to stratum_height.
material stratum_material(
  r_pos_t x, r_pos_t y, r_pos_t height,
  stratum *st,
  stratum_dynamics *sd
);

#endif // ifndef GEOLOGY_H
