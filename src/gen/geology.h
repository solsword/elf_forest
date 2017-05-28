#ifndef GEOLOGY_H
#define GEOLOGY_H

// geology.h
// Stone types and strata generation.

#include <stdint.h>
#include <math.h>

#include "math/functions.h"
#include "datatypes/rngtable.h"

#include "txgen/txg_minerals.h"
#include "world/materials.h"
#include "world/world.h"
#include "world/world_map.h"

#include "util.h"

/*************
 * Constants *
 *************/

// Height and offset for a simplex grid:
#define SX_GRID_HEIGHT (sin(M_PI/3.0))
#define SX_GRID_OFFSET (cos(M_PI/3.0))

// Exponents defining the shapes of tectonic seams & squashing:
#define TECT_SEAM_SHAPE 0.6
#define TECT_SEAM_SIGSHAPE 0.4
#define TECT_RMOD_SIGSHAPE 0.75
#define TECT_RMOD_EXPSHAPE 4.4

// Tectonics sheet generation parameters:
// --------------------------------------

#define TECT_SMALL_RUSTLE_STR 0.4
#define TECT_SMALL_RUSTLE_SCALE (14.2/100.0)
#define TECT_LARGE_RUSTLE_STR 0.6
#define TECT_LARGE_RUSTLE_SCALE (7.5/100.0)

#define TECT_CONTINENTS_STR 0.06
#define TECT_CONTINENTS_SCALE (1.5/100.0)
#define TECT_CONTINENTS_DSTR 9.3
#define TECT_CONTINENTS_DSCALE (3.4/100.0)

#define TECT_RIDGES_STR (6.0 * 0.32) // the 6.0 offsets various biases
#define TECT_RIDGES_SCALE (5.4/100.0)
#define TECT_RIDGES_DSTR 2.4
#define TECT_RIDGES_DSCALE (10.7/100.0)
#define TECT_RIDGES_MSCALE (2.2/100.0)

#define TECT_SEAM_COUNT (((WORLD_WIDTH + WORLD_HEIGHT) / 2) / 5)
#define TECT_SEAM_DT 0.12
#define TECT_SEAM_MIN_WIDTH 6.3
#define TECT_SEAM_WIDTH_VAR 12.8

#define TECT_CRUMPLE_STEPS 4

#define TECT_CRUMPLE_RUSTLE_STR 0.3
#define TECT_CRUMPLE_RUSTLE_SCALE (12.7/100.0)

#define TECT_CRUMPLE_SETTLE_STEPS 15
#define TECT_CRUMPLE_SETTLE_DT 0.01
#define TECT_CRUMPLE_EQ_DIST 1.3
#define TECT_CRUMPLE_K 1.5

#define TECT_CRUMPLE_UNTANGLE_STEPS 6
#define TECT_CRUMPLE_UNTANGLE_DT 0.1

#define TECT_STRETCH_RELAX_STEPS 15
#define TECT_STRETCH_RELAX_DT 0.01
#define TECT_STRETCH_RELAX_EQ_DIST 1.3
#define TECT_STRETCH_RELAX_K 1.5

#define TECT_STRETCH_UNTANGLE_STEPS 4
#define TECT_STRETCH_UNTANGLE_DT 0.1

#define TECT_SQUASH_LOWER_CUTOFF 0.0
#define TECT_SQUASH_NEW_MIN -0.1
#define TECT_SQUASH_UPPER_FRACTION 0.75
#define TECT_SQUASH_MAX_FACTOR 0.2

#define TECT_RECOMBINE_STR 0.3

// Strata parameters:
// ------------------

// Controls the size of strata relative to the world map size.
static float const STRATA_AVG_SIZE = 0.25;
ELFSCRIPT_GL(n, STRATA_AVG_SIZE)

// Controls how many strata to generate (a multiple of MAX_STRATA_LAYERS).
static float const STRATA_COMPLEXITY = 3.0;
//static float const STRATA_COMPLEXITY = (1.0/18.0);
ELFSCRIPT_GL(n, STRATA_COMPLEXITY)

// The base stratum thickness (before an exponential distribution).
static float const BASE_STRATUM_THICKNESS = 10.0;
ELFSCRIPT_GL(n, BASE_STRATUM_THICKNESS);

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

// Weights when averaging the properties of stone constituent elements:
static float const STONE_CONSTITUENT_AVERAGING_WEIGHTS[];

/********************
 * Inline Functions *
 ********************/

// Finds the force exerted on point 'to' by a spring between 'from' and 'to'
// with equilibrium length 'eq' and spring constant 'k'. Stores the result in
// the 'result' vector.
static inline void spring_force(
  vector const * const from,
  vector const * const to,
  vector *result,
  float eq,
  float k
) {
  vcopy_as(result, to);
  vsub_from(result, from);
  float m = vmag(result);
  vnorm(result);
  vscale(result, (eq - m) * k);
}

// Finds the closest point to the given point 'p' on the line segment from 'a'
// to 'b'. Stores the result in 'result'.
static inline void closest_point_on_line_segment(
  vector const * const p,
  vector const * const a, vector const * const b,
  vector *result
) {
  vector segment;
  float len, plen;
  vcopy_as(result, p);
  vsub_from(result, a);
  vcopy_as(&segment, b);
  vsub_from(&segment, a);
  vproject(result, &segment);
  len = vmag(&segment);
  plen = vmag(result);
  if (vdot(result, &segment) < 0) {
    vcopy_as(result, a);
  } else if (len > plen) {
    vcopy_as(result, b);
  } else {
    vadd_to(result, a);
  }
}

// Takes a point 'p' and converts it to barycentric coordinates in the xy plane
// using the triangle 'a'-'b'-'c' as reference. z-values are completely ignored
// in the calculation. The input point 'p' is modified.
static inline void xy__barycentric(
  vector *p,
  vector const * const a,
  vector const * const b,
  vector const * const c
) {
  float det, b1, b2, b3;
  det = (
    (b->y - c->y) * (a->x - c->x)
  +
    (c->x - b->x) * (a->y - c->y)
  );

  b1 = (
    (b->y - c->y) * (p->x - c->x)
  +
    (c->x - b->x) * (p->y - c->y)
  );
  b1 /= det;

  b2 = (
    (c->y - a->y) * (p->x - c->x)
  +
    (a->x - c->x) * (p->y - c->y)
  );
  b2 /= det;

  b3 = 1 - b1 - b2;
  p->x = b1;
  p->y = b2;
  p->z = b3;
}

// Takes barycentric coordinates 'bc' in terms of the triangle 'a'-'b'-'c' and
// computes a 3D point on the surface of the triangle 'a'-'b'-'c' at the given
// barycentric coordinates. The result is stored in the input point 'bc'.
static inline void barycentric__xy(
  vector *bc,
  vector const * const a,
  vector const * const b,
  vector const * const c
){
  float x, y, z;
  x = bc->x * a->x + bc->y * b->x + bc->z * c->x;
  y = bc->x * a->y + bc->y * b->y + bc->z * c->y;
  z = bc->x * a->z + bc->y * b->z + bc->z * c->z;
  bc->x = x;
  bc->y = y;
  bc->z = z;
}

// The width in points (rather than triangles) of the given tectonic sheet.
static inline size_t sheet_pwidth(tectonic_sheet *ts) {
  return (ts->width / 2) + 1;
}

// The height in points (rather than triangles) of the given tectonic sheet.
static inline size_t sheet_pheight(tectonic_sheet *ts) {
  return ts->height + 1;
}

// The index of a point within the sheet's points array, based on (i, j) point
// coordinates (not triangle coordinates).
static inline size_t sheet_pidx(tectonic_sheet *ts, size_t i, size_t j) {
  return j * sheet_pwidth(ts) + i;
}

// The index of the first point of the triangle at triangle coordinates (i, j)
static inline size_t sheet_pidx_a(tectonic_sheet *ts, size_t i, size_t j) {
  if (j % 2 == 0) {
    return sheet_pidx(ts, (i+1)/2, j);
  } else {
    return sheet_pidx(ts, i/2, j);
  }
}

// The index of the second point of the triangle at triangle coordinates (i, j)
static inline size_t sheet_pidx_b(tectonic_sheet *ts, size_t i, size_t j) {
  if (j % 2 == 0) {
    return sheet_pidx(ts, i/2, j+1);
  } else {
    return sheet_pidx(ts, (i+1)/2, j+1);
  }
}

// The index of the third point of the triangle at triangle coordinates (i, j)
static inline size_t sheet_pidx_c(tectonic_sheet *ts, size_t i, size_t j) {
  if (i % 2 == j % 2) { // if this triangle points up instead of down
    return sheet_pidx(ts, i/2+1, j);
  } else {
    return sheet_pidx(ts, i/2+1, j+1);
  }
}

static inline float sheet_min_x(tectonic_sheet *ts) {
  float result = ts->points[0].x;
  size_t i, j, idx;
  for (i = 0; i < sheet_pwidth(ts); ++i) {
    for (j = 0; j < sheet_pheight(ts); ++j) {
      idx = sheet_pidx(ts, i, j);
      if (ts->points[idx].x < result) {
        result = ts->points[idx].x;
      }
    }
  }
  return result;
}

static inline float sheet_min_y(tectonic_sheet *ts) {
  float result = ts->points[0].y;
  size_t i, j, idx;
  for (i = 0; i < sheet_pwidth(ts); ++i) {
    for (j = 0; j < sheet_pheight(ts); ++j) {
      idx = sheet_pidx(ts, i, j);
      if (ts->points[idx].y < result) {
        result = ts->points[idx].y;
      }
    }
  }
  return result;
}

static inline float sheet_min_z(tectonic_sheet *ts) {
  float result = ts->points[0].z;
  size_t i, j, idx;
  for (i = 0; i < sheet_pwidth(ts); ++i) {
    for (j = 0; j < sheet_pheight(ts); ++j) {
      idx = sheet_pidx(ts, i, j);
      if (ts->points[idx].z < result) {
        result = ts->points[idx].z;
      }
    }
  }
  return result;
}

static inline float sheet_max_x(tectonic_sheet *ts) {
  float result = ts->points[0].x;
  size_t i, j, idx;
  for (i = 0; i < sheet_pwidth(ts); ++i) {
    for (j = 0; j < sheet_pheight(ts); ++j) {
      idx = sheet_pidx(ts, i, j);
      if (ts->points[idx].x > result) {
        result = ts->points[idx].x;
      }
    }
  }
  return result;
}

static inline float sheet_max_y(tectonic_sheet *ts) {
  float result = ts->points[0].y;
  size_t i, j, idx;
  for (i = 0; i < sheet_pwidth(ts); ++i) {
    for (j = 0; j < sheet_pheight(ts); ++j) {
      idx = sheet_pidx(ts, i, j);
      if (ts->points[idx].y > result) {
        result = ts->points[idx].y;
      }
    }
  }
  return result;
}

static inline float sheet_max_z(tectonic_sheet *ts) {
  float result = ts->points[0].z;
  size_t i, j, idx;
  for (i = 0; i < sheet_pwidth(ts); ++i) {
    for (j = 0; j < sheet_pheight(ts); ++j) {
      idx = sheet_pidx(ts, i, j);
      if (ts->points[idx].z > result) {
        result = ts->points[idx].z;
      }
    }
  }
  return result;
}

static inline float geothermal_temperature(global_pos *glpos) {
  return 100.0; // TODO: Geothermal temperature?
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
// returns the top stratum. It returns NULL if the given world region doesn't
// have any strata.
static inline stratum* get_stratum(
  world_region* wr,
  float h
) {
  int i;
  if (wr->geology.stratum_count == 0) {
    return NULL;
  }
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

static inline float get_base_density(stone_species *ssp, ptrdiff_t seed) {
  size_t i;
  float result = 2 * ptrf(seed);
  float denom = 2;
  element_species *element;
  for (i = 0; i < MN_MAX_PRIMARY_CONSTITUENTS; ++i) {
    element = get_element_species(ssp->constituents[i]);
    if (element != NULL) {
      result += element->stone_density_tendency;
      denom += 1;
    }
  }
  return result / denom;
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new tectonic sheet with the given parameters.
tectonic_sheet *create_tectonic_sheet(
  ptrdiff_t seed,
  size_t width,
  size_t height
);

// Allocates and returns a new tectonic sheet that's a copy of the given sheet,
// including its seed.
tectonic_sheet* copy_tectonic_sheet(tectonic_sheet *ts);

void cleanup_tectonic_sheet(tectonic_sheet *ts);

// Allocates and returns a new stratum with the given parameters.
stratum *create_stratum(
  world_map *wm,
  ptrdiff_t seed,
  float cx, float cy,
  float size, float thickness,
  map_function profile,
  geologic_source source
);

/*************
 * Functions *
 *************/

// Tectonic sheet functions:
// -------------------------

// Resets the given sheet, setting most things to 0 and remapping the points of
// the sheet to a simplex grid.
void reset_sheet(tectonic_sheet *ts);

// Stretches the sheet to fit the given height/width dimensions. This just
// moves the edge points outwards/inwards to match the given dimensions. It is
// recommended that some settling/untangling be applied afterwards with edges
// held to sort out interior points. This function only operates on x/y values
// and ignores z values.
void stretch_sheet(tectonic_sheet *ts, float width, float height);

// Randomly moves each grid point a little bit in the x/y plane. Strength
// represents the max perturbation, while scale controls the size of the noise.
void rustle_sheet(
  tectonic_sheet *ts,
  float strength,
  float scale,
  ptrdiff_t seed
);

// Settles the given sheet over the given number of iterations, moving points
// based on spring forces between them. The spring forces are determined by the
// given equilibrium distance and spring constant. Each iteration updates
// positions as if dt time had passed.
void settle_sheet(
  tectonic_sheet *ts,
  size_t iterations,
  float dt,
  float equilibrium_distance,
  float spring_constant,
  int hold_edges
);

// Untangles the given sheet over the given number of iterations, moving each
// point towards the average of its neighbors in the graph (might not be its
// neighbors is x/y/z space, hence the name). The process works only with x/y
// values and ignores z values. The 'dt' value determines the rate at which
// points move towards their neighbors' average position at each iteration.
void untangle_sheet(
  tectonic_sheet *ts,
  size_t iterations,
  float dt,
  int hold_edges
);

// Adds some continents to the tectonic sheet by changing z values. Doesn't
// affect x/y values at all. The strength parameters scales the height added,
// while the scale and dscale parameters control the scale of the continents
// and the scale of underlying distortion respectively. The dstr parameter
// controls the strength of distortion applied.
void add_continents_to_sheet(
  tectonic_sheet *ts,
  float strength,
  float scale,
  float dstr,
  float dscale,
  ptrdiff_t seed
);

// Adds some ridges to the tectonic sheet by changing z values. Doesn't affect
// x/y values at all. The strength parameter scales the height added, while
// the scale and dscale parameters control the scale of the ridges and the
// scale of underlying distortion respectively. The dstr parameter controls the
// strength of distortion applied. The mscale parameter controls the scale of
// modulation, while the mseed paramter provides a separate modulation seed.
void add_ridges_to_sheet(
  tectonic_sheet *ts,
  float strength,
  float scale,
  float dstr,
  float dscale,
  float mscale,
  ptrdiff_t seed,
  ptrdiff_t mseed
);

// Either expands or contracts the tectonic sheet around the given line
// segment, holding edge points still if instructed to do so. Operates only in
// the x/y plane, ignoring z values. If 'pull' is true, it will contract
// towards the line, otherwise it will expand away from it. The 'dt' parameter
// determines how far the seam pushes/pulls.
void seam_sheet(
  tectonic_sheet *ts,
  float dt,
  vector *from,
  vector *to,
  float width,
  int pull,
  int hold_edges
);

// Takes a tectonic sheet and flattens outlying values. Basically, takes the
// bottom lower_fraction of the distribution and compresses it into
// lower_strength of its former range, and does the same using the upper_
// parameters. This only affects z values.
void squash_sheet(
  tectonic_sheet *ts,
  float lower_fraction,
  float lower_strength,
  float upper_fraction,
  float upper_strength
);

// This is the core tectonics generation function that applies various
// operations to come up with an interesting tectonics sheet for a world map.
void generate_tectonics(world_map *wm);

// Returns the height of the given tectonic sheet at a point fx, fy expressed
// in fractions of the sheet's width and height.
float sheet_height(
  tectonic_sheet *ts,
  float fx,
  float fy
);

// General geology functions:
// --------------------------

// Generates and registers a new stone species for the given world map.
stone_species* generate_stone_species(world_map *wm, ptrdiff_t seed);

// Generates geology for the given world.
void generate_geology(world_map *wm);

// Computes the height of the given stratum at the given position (ignores z):
gl_pos_t compute_stratum_height(stratum *st, global_pos *glpos);

#endif // ifndef GEOLOGY_H
