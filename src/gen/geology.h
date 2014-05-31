#ifndef GEOLOGY_H
#define GEOLOGY_H

// geology.h
// Stone types and strata generation.

#include <stdint.h>

#include "math/functions.h"

/*********
 * Enums *
 *********/

enum material_origin_e {
  MO_IGNEOUS,
  MO_METAMORPHIC,
  MO_SEDIMENTARY,
  MO_EROSION,
  MO_ORGANIC,
};
typedef enum material_origin_e material_origin;

/************************
 * Types and Structures *
 ************************/

// A layer of stone that encompasses some region of the world.
struct stratum_s;
typedef struct stratum_s stratum;

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

// Maximum number of stone types included in other layers
#define N_INCLUSION_TYPES 8

/***********
 * Globals *
 ***********/

// The seed for geothermal information, which both helps determine strata
// placement and contributes to metamorphosis.
extern float GEOTHERMAL_SEED;

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

  // core variation:
  float large_var; // large-scale variation
  float med_var; // medium-scale variation

  // positive detail
  float small_var; // small-scale variation
  float ridges; // amplitude of ridges

  // negative detail:
  float scraping; // amount of scraping
  float cracks; // amplitude of cracks
  float weathering; // amount of smooth weathering

 // Derived vein and inclusion information:
 // ---------------------------------------
  float vein_scale[N_VEIN_TYPES]; // scale of different veins
  float vein_strength[N_VEIN_TYPES]; // thickness and frequency of veins
  float inclusion_frequency[N_INCLUSION_TYPES]; // frequency of inclusions

 // Derived material type information:
 // ----------------------------------
  block_variant base_variant; // exact material type for main mass
  block_variant vein_variants[N_VEIN_TYPES]; // types for veins
  block_variant inclusion_variants[N_INCLUSION_TYPES]; // types for inclusions

 // Dynamic factors are erosion and pressure which influence compression and
 // metamorphosis.
};

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
block stratum_block(
  r_pos_t x, r_pos_t y, r_pos_t height,
  stratum *st,
  stratum_dynamics *sd
);

#endif // ifndef GEOLOGY_H
