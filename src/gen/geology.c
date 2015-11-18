// geology.c
// Stone types and strata generation.

#include <stdio.h>

#include "noise/noise.h"
#include "math/functions.h"
#include "math/curve.h"
#include "datatypes/vector.h"
#include "world/world.h"
#include "world/species.h"
#include "world/world_map.h"
#include "tex/color.h"

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

static float const STONE_CONSTITUENT_AVERAGING_WEIGHTS[] = {
  1.5,
  1.0,
  0.7
};

/**********
 * Tables *
 **********/

rngtable const STONE_SOURCE_DISTRIBUTION = {
  .size = 3,
  .values = (void*[]) {
    (void*) GEO_IGNEOUS,
    (void*) GEO_METAMORPHIC,
    (void*) GEO_SEDIMENTARY
  },
  .weights = (float[]) { 0.5, 0.3, 0.2 }
  // skewed relative to Earth for extra sedimentary rocks
};

// Composition distributions:

rngtable const IGNEOUS_COMPOSITIONS = {
  .size = 7,
  .values = (void*[]) {
    (void*) MNRL_COMP_STONE,
    (void*) MNRL_COMP_STONE_STONE,

    (void*) MNRL_COMP_STONE_LIFE,

    (void*) MNRL_COMP_STONE_METAL,
    (void*) MNRL_COMP_STONE_METAL_METAL,

    (void*) MNRL_COMP_STONE_RARE,
    (void*) MNRL_COMP_LIFE
  },
  .weights = (float[]) {
    0.4, 0.35,  // 75% |  75%
    0.10,       // 10% |  85%
    0.05, 0.05, // 10% |  95%
    0.03, 0.02  //  5% | 100%
  }
};
rngtable const METAMORPHIC_COMPOSITIONS = {
  .size = 15,
  .values = (void*[]) {
    (void*) MNRL_COMP_STONE_WATER,
    (void*) MNRL_COMP_STONE_STONE,
    (void*) MNRL_COMP_STONE_METAL,

    (void*) MNRL_COMP_STONE_STONE_STONE,
    (void*) MNRL_COMP_STONE_STONE_METAL,
    (void*) MNRL_COMP_STONE_METAL_METAL,

    (void*) MNRL_COMP_STONE_STONE_LIFE,
    (void*) MNRL_COMP_STONE_AIR,
    (void*) MNRL_COMP_STONE_STONE,
    (void*) MNRL_COMP_RARE_RARE,

    (void*) MNRL_COMP_STONE_LIFE,
    (void*) MNRL_COMP_STONE_METAL_RARE,
    (void*) MNRL_COMP_STONE_RARE,

    (void*) MNRL_COMP_STONE_STONE_RARE,
    (void*) MNRL_COMP_STONE_RARE_RARE
  },
  .weights = (float[]) {
    0.17, 0.12, 0.12,        // 41% |  41%
    0.09, 0.09, 0.07,        // 25% |  66%
    0.06, 0.06, 0.06, 0.05,  // 23% |  89%
    0.04, 0.03, 0.02,        //  9% |  98%
    0.01, 0.01               //  2% | 100%
  }
};
rngtable const SEDIMENTARY_COMPOSITIONS = {
  .size = 7,
  .values = (void*[]) {
    (void*) MNRL_COMP_STONE_LIFE,
    (void*) MNRL_COMP_STONE_WATER,
    (void*) MNRL_COMP_STONE_AIR,

    (void*) MNRL_COMP_LIFE,

    (void*) MNRL_COMP_STONE,
    (void*) MNRL_COMP_STONE_STONE,

    (void*) MNRL_COMP_RARE_RARE
  },
  .weights = (float[]) {
    0.26, 0.23, 0.21,    // 70% |  70%
    0.12,                // 12% |  82%
    0.08, 0.06,          // 14% |  96%
    0.04                 //  4% | 100%
  }
};

rngtable const IGNEOUS_TRACES = {
  .size = 9,
  .values = (void*[]) {
    (void*) MNRL_TRACE_NONE,

    (void*) MNRL_TRACE_LIFE,
    (void*) MNRL_TRACE_METAL,
    (void*) MNRL_TRACE_METAL_METAL,

    (void*) MNRL_TRACE_RARE,
    (void*) MNRL_TRACE_METAL_RARE,

    (void*) MNRL_TRACE_RARE_RARE,
    (void*) MNRL_TRACE_STONE,
    (void*) MNRL_TRACE_STONE_METAL
  },
  .weights = (float[]) {
    0.5,               // 50% |  50%
    0.12, 0.11, 0.11,  // 34% |  84%
    0.06, 0.05,        // 11% |  95%
    0.02, 0.02, 0.01   //  5% | 100%
  }
};

rngtable const METAMORPHIC_TRACES = {
  .size = 11,
  .values = (void*[]) {
    (void*) MNRL_TRACE_NONE,

    (void*) MNRL_TRACE_RARE,
    (void*) MNRL_TRACE_METAL,

    (void*) MNRL_TRACE_WATER,
    (void*) MNRL_TRACE_AIR,

    (void*) MNRL_TRACE_METAL_RARE,
    (void*) MNRL_TRACE_RARE_RARE,
    (void*) MNRL_TRACE_METAL_METAL,

    (void*) MNRL_TRACE_LIFE,
    (void*) MNRL_TRACE_STONE_METAL,
    (void*) MNRL_TRACE_STONE
  },
  .weights = (float[]) {
    0.20,              // 20% |  20%
    0.15, 0.13,        // 28% |  48%
    0.10, 0.09,        // 19% |  67%
    0.07, 0.07, 0.07,  // 21% |  88%
    0.05, 0.04, 0.03   // 12% | 100%
  }
};

rngtable const SEDIMENTARY_TRACES = {
  .size = 8,
  .values = (void*[]) {
    (void*) MNRL_TRACE_NONE,

    (void*) MNRL_TRACE_STONE,

    (void*) MNRL_TRACE_LIFE,

    (void*) MNRL_TRACE_WATER,
    (void*) MNRL_TRACE_AIR,

    (void*) MNRL_TRACE_METAL,
    (void*) MNRL_TRACE_STONE_METAL,
    (void*) MNRL_TRACE_RARE
  },
  .weights = (float[]) {
    0.5,               // 50% |  50%
    0.17,              // 17% |  67%
    0.12,              // 12% |  79%
    0.08, 0.06,        // 14% |  93%
    0.03, 0.02, 0.02   //  7% | 100%
  }
};

/******************************
 * Constructors & Destructors *
 ******************************/
 
tectonic_sheet *create_tectonic_sheet(
  ptrdiff_t seed,
  size_t width,
  size_t height
) {
  tectonic_sheet *result = (tectonic_sheet *) malloc(sizeof(tectonic_sheet));
  result->seed = seed;
  result->width = width;
  result->height = height;
  result->points = (vector *) malloc(
    sizeof(vector) * sheet_pwidth(result) * sheet_pheight(result)
  );
  result->forces = (vector *) malloc(
    sizeof(vector) * sheet_pwidth(result) * sheet_pheight(result)
  );
  result->avgcounts = (uint8_t *) malloc(
    sizeof(uint8_t) * sheet_pwidth(result) * sheet_pheight(result)
  );
  reset_sheet(result);
  return result;
}

tectonic_sheet* copy_tectonic_sheet(tectonic_sheet *ts) {
  size_t i;
  tectonic_sheet *result = create_tectonic_sheet(
    ts->seed,
    ts->width,
    ts->height
  );
  for (i = 0; i < sheet_pwidth(ts) * sheet_pheight(ts); ++i) {
    vcopy_as(&(result->points[i]), &(ts->points[i]));
    vcopy_as(&(result->forces[i]), &(ts->forces[i]));
    result->avgcounts[i] = ts->avgcounts[i];
  }
  return result;
}

void cleanup_tectonic_sheet(tectonic_sheet *ts) {
  free(ts->points);
  free(ts->forces);
  free(ts);
}

stratum *create_stratum(
  world_map *wm,
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

  seed = prng(seed + 245161);

  result->base_species = create_new_stone_species(wm, source, seed);
  seed = prng(seed);

  switch (source) {
    case GEO_IGNEOUS:
      result->persistence = 1.2 + 0.4 * ptrf(seed);
      seed = prng(seed);
      result->scale_bias = 0.7 + 0.4 * ptrf(seed);
      seed = prng(seed);

      result->radial_frequency = M_PI/(2.4 + 1.6*ptrf(seed));
      seed = prng(seed);
      result->radial_variance = 0.1 + 0.3*ptrf(seed);
      seed = prng(seed);

      result->gross_distortion = 900 + 500.0*ptrf(seed);
      seed = prng(seed);
      result->fine_distortion = 110 + 40.0*ptrf(seed);
      seed = prng(seed);

      result->large_var = result->thickness * (0.6 + 0.3*ptrf(seed));
      seed = prng(seed);
      result->med_var = result->thickness * (0.4 + 0.25*ptrf(seed));
      seed = prng(seed);
      result->small_var = result->thickness * (0.17 + 0.05*ptrf(seed));
      seed = prng(seed);
      result->tiny_var = result->thickness * (0.04 + 0.06*ptrf(seed));
      seed = prng(seed);

      result->detail_var = 1.0 + 2.0*ptrf(seed);
      seed = prng(seed);
      result->ridges = 2.0 + 3.0*ptrf(seed);
      seed = prng(seed);

      result->smoothing = 0.15 + 0.2*ptrf(seed);
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
      result->persistence = 0.8 + 0.5 * ptrf(seed);
      seed = prng(seed);
      result->scale_bias = 0.8 + 0.4 * ptrf(seed);
      seed = prng(seed);

      result->radial_frequency = M_PI/(2.8 + 2.0*ptrf(seed));
      seed = prng(seed);
      result->radial_variance = 0.4 + 0.4*ptrf(seed);
      seed = prng(seed);

      result->gross_distortion = 1200 + 900.0*ptrf(seed);
      seed = prng(seed);
      result->fine_distortion = 180 + 110.0*ptrf(seed);
      seed = prng(seed);

      result->large_var = result->thickness * (0.5 + 0.3*ptrf(seed));
      seed = prng(seed);
      result->med_var = result->thickness * (0.3 + 0.25*ptrf(seed));
      seed = prng(seed);
      result->small_var = result->thickness * (0.16 + 0.06*ptrf(seed));
      seed = prng(seed);
      result->tiny_var = result->thickness * (0.02 + 0.03*ptrf(seed));
      seed = prng(seed);

      result->detail_var = 0.3 + 1.8*ptrf(seed);
      seed = prng(seed);
      result->ridges = 0.4 + 3.4*ptrf(seed);
      seed = prng(seed);

      result->smoothing = 0.15 + 0.45*ptrf(seed);
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

    case GEO_SEDIMENTARY:
    default:
      result->persistence = 1.3 + 0.5 * ptrf(seed);
      seed = prng(seed);
      result->scale_bias = 1.1 + 0.3 * ptrf(seed);
      seed = prng(seed);

      result->radial_frequency = M_PI/(2.1 + 1.2*ptrf(seed));
      seed = prng(seed);
      result->radial_variance = 0.05 + 0.2*ptrf(seed);
      seed = prng(seed);

      result->gross_distortion = 700 + 400.0*ptrf(seed);
      seed = prng(seed);
      result->fine_distortion = 30 + 30.0*ptrf(seed);
      seed = prng(seed);

      result->large_var = result->thickness * (0.4 + 0.25*ptrf(seed));
      seed = prng(seed);
      result->med_var = result->thickness * (0.2 + 0.15*ptrf(seed));
      seed = prng(seed);
      result->small_var = result->thickness * (0.11 + 0.05*ptrf(seed));
      seed = prng(seed);
      result->tiny_var = result->thickness * (0.03 + 0.07*ptrf(seed));
      seed = prng(seed);

      result->detail_var = 0.7 + 3.2*ptrf(seed);
      seed = prng(seed);
      result->ridges = 0.8 + 4.5*ptrf(seed);
      seed = prng(seed);

      result->smoothing = 0.12 + 0.4*ptrf(seed);
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

// Tectonic sheet functions:
// -------------------------

void reset_sheet(tectonic_sheet *ts) {
  size_t i, j, idx;
  for (i = 0; i < sheet_pwidth(ts); ++i) {
    for (j = 0; j < sheet_pheight(ts); ++j) {
      idx = sheet_pidx(ts, i, j);
      ts->points[idx].x = i;
      ts->points[idx].y = j * SX_GRID_HEIGHT;
      ts->points[idx].z = 0;
      if (j % 2 == 1) {
        ts->points[idx].x += SX_GRID_OFFSET;
      }

      vzero(&(ts->forces[idx]));
      ts->avgcounts[idx] = 0;
    }
  }
}

void stretch_sheet(tectonic_sheet *ts, float width, float height) {
  size_t i, j, idx;
  size_t pw, ph;
  float cx, cy;
  float min_x, max_x, min_y, max_y;
  pw = sheet_pwidth(ts);
  ph = sheet_pheight(ts);
  // Compute the center to find desired min/max x/y:
  cx = 0;
  cy = 0;
  for (i = 0; i < pw; ++i) {
    for (j = 0; j < ph; ++j) {
      idx = sheet_pidx(ts, i, j);
      cx += ts->points[idx].x;
      cy += ts->points[idx].y;
    }
  }
  cx /= (float) (pw * ph);
  cy /= (float) (pw * ph);

  min_x = cx - width / 2.0;
  max_x = cx + width / 2.0;

  min_y = cy - height / 2.0;
  max_y = cy + height / 2.0;

  // Pull all edge points in/outwards to the desired dimensions:
  for (i = 0; i < pw; ++i) {
    ts->points[sheet_pidx(ts, i, 0)].y = min_y;
    ts->points[sheet_pidx(ts, i, ph - 1)].y = max_y;
  }
  for (j = 0; j < ph; ++j) {
    ts->points[sheet_pidx(ts, 0, j)].x = min_x;
    ts->points[sheet_pidx(ts, pw - 1, j)].x = max_x;
  }
}

void rustle_sheet(
  tectonic_sheet *ts,
  float strength,
  float scale,
  ptrdiff_t seed
) {
  int i, j, idx;
  vector *p;
  ptrdiff_t oseed;
  seed = prng(seed + 71);
  oseed = prng(seed + 30);

  for (i = 0; i < sheet_pwidth(ts); ++i) {
    for (j = 0; j < sheet_pheight(ts); ++j) {
      idx = sheet_pidx(ts, i, j);
      p = &(ts->points[idx]);
      p->x += strength * sxnoise_2d(
        p->x * scale,
        p->y * scale,
        seed
      );
      p->y += strength * sxnoise_2d(
        p->x * scale,
        p->y * scale,
        oseed
      );
    }
  }
}

void settle_sheet(
  tectonic_sheet *ts,
  size_t iterations,
  float dt,
  float equilibrium_distance,
  float spring_constant,
  int hold_edges
) {
  size_t iter, i, j;
  size_t idx, idx_a, idx_b, idx_c;
  size_t pw, ph;
  pw = sheet_pwidth(ts);
  ph = sheet_pheight(ts);
  vector *a, *b, *c; // the points of the current triangle
  vector *f; // the force on the current point
  vector tmp; // vector used for intermediate calculations
  for (iter = 0; iter < iterations; ++iter) {
    // First pass: compute forces by iterating over triangles in the sheet:
    for (i = 0; i < ts->width; ++i) {
      for (j = 0; j < ts->height; ++j) {
        idx_a = sheet_pidx_a(ts, i, j);
        idx_b = sheet_pidx_b(ts, i, j);
        idx_c = sheet_pidx_c(ts, i, j);
        a = &(ts->points[idx_a]);
        b = &(ts->points[idx_b]);
        c = &(ts->points[idx_c]);
        if (i % 2 == j % 2) { // if this triangle points up
          // All six forces on all three corners of this triangle:
          // a <- b
          f = &(ts->forces[idx_a]);
          spring_force(b, a, &tmp, equilibrium_distance, spring_constant);
          vadd_to(f, &tmp);

          // a <- c
          spring_force(c, a, &tmp, equilibrium_distance, spring_constant);
          vadd_to(f, &tmp);

          // b <- a
          f = &(ts->forces[idx_b]);
          spring_force(a, b, &tmp, equilibrium_distance, spring_constant);
          vadd_to(f, &tmp);

          // b <- c
          spring_force(c, b, &tmp, equilibrium_distance, spring_constant);
          vadd_to(f, &tmp);

          // c <- a
          f = &(ts->forces[idx_c]);
          spring_force(a, c, &tmp, equilibrium_distance, spring_constant);
          vadd_to(f, &tmp);

          // c <- b
          spring_force(b, c, &tmp, equilibrium_distance, spring_constant);
          vadd_to(f, &tmp);
        } else if (i == 0 && (j % 2 == 1)) {
          // Both forces on our a <-> b edge:
          // a <- b
          f = &(ts->forces[idx_a]);
          spring_force(b, a, &tmp, equilibrium_distance, spring_constant);
          vadd_to(f, &tmp);

          // b <- a
          f = &(ts->forces[idx_b]);
          spring_force(a, b, &tmp, equilibrium_distance, spring_constant);
          vadd_to(f, &tmp);
        } else if ((i == ts->width - 1) && (j % 2 == 0) ) {
          // Both forces on our a <-> c edge:
          // a <- c
          f = &(ts->forces[idx_a]);
          spring_force(c, a, &tmp, equilibrium_distance, spring_constant);
          vadd_to(f, &tmp);

          // c <- a
          f = &(ts->forces[idx_c]);
          spring_force(a, c, &tmp, equilibrium_distance, spring_constant);
          vadd_to(f, &tmp);
        }
      }
    }
    // Second pass: apply forces (iterating over points this time)
    for (i = 0; i < pw; ++i) {
      for (j = 0; j < ph; ++j) {
        idx = sheet_pidx(ts, i, j);
        f = &(ts->forces[idx]);
        if (
          hold_edges
        &&
          (i == 0 || i == pw - 1 || j == 0 || j == ph - 1)
        ) {
          vzero(f);
          continue;
        }
        vscale(f, dt);
        vadd_to(&(ts->points[idx]), f);
        vzero(f);
      }
    }
  }
}

void untangle_sheet(
  tectonic_sheet *ts,
  size_t iterations,
  float dt,
  int hold_edges
) {
  size_t iter, i, j;
  size_t idx, idx_a, idx_b, idx_c;
  size_t pw, ph;
  pw = sheet_pwidth(ts);
  ph = sheet_pheight(ts);
  vector *a, *b, *c; // the points of the current triangle
  vector *f; // the "force" for the current point (used to store averages)
  vector *p; // The focused point
  vector tmp; // vector used for intermediate calculations
  for (iter = 0; iter < iterations; ++iter) {
    // First pass: compute averages by iterating over triangles:
    for (i = 0; i < ts->width; ++i) {
      for (j = 0; j < ts->height; ++j) {
        idx_a = sheet_pidx_a(ts, i, j);
        idx_b = sheet_pidx_b(ts, i, j);
        idx_c = sheet_pidx_c(ts, i, j);
        a = &(ts->points[idx_a]);
        b = &(ts->points[idx_b]);
        c = &(ts->points[idx_c]);
        if (i % 2 == j % 2) { // if this triangle points up
          // All six relations on all three corners of this triangle:
          // a <- b; a <- c
          f = &(ts->forces[idx_a]);
          f->x += b->x;
          f->y += b->y;
          f->x += c->x;
          f->y += c->y;
          ts->avgcounts[idx_a] += 2;

          // b <- a; b <- c
          f = &(ts->forces[idx_b]);
          f->x += a->x;
          f->y += a->y;
          f->x += c->x;
          f->y += c->y;
          ts->avgcounts[idx_b] += 2;

          // c <- a; c <- b
          f = &(ts->forces[idx_c]);
          f->x += a->x;
          f->y += a->y;
          f->x += b->x;
          f->y += b->y;
          ts->avgcounts[idx_c] += 2;
        } else if (i == 0 && (j % 2 == 1)) {
          // Both relations on our a <-> b edge:
          // a <- b
          f = &(ts->forces[idx_a]);
          f->x += b->x;
          f->y += b->y;
          ts->avgcounts[idx_a] += 1;

          // b <- a
          f = &(ts->forces[idx_b]);
          f->x += a->x;
          f->y += a->y;
          ts->avgcounts[idx_b] += 1;
        } else if ((i == ts->width - 1) && (j % 2 == 0) ) {
          // Both relations on our a <-> c edge:
          // a <- c
          f = &(ts->forces[idx_a]);
          f->x += c->x;
          f->y += c->y;
          ts->avgcounts[idx_a] += 1;

          // c <- a
          f = &(ts->forces[idx_c]);
          f->x += a->x;
          f->y += a->y;
          ts->avgcounts[idx_c] += 1;
        }
      }
    }
    // Second pass: move each point towards its respective average (iterating
    // over points this time)
    for (i = 0; i < pw; ++i) {
      for (j = 0; j < ph; ++j) {
        idx = sheet_pidx(ts, i, j);
        p = &(ts->points[idx]);
        f = &(ts->forces[idx]);
        if (
          hold_edges
        &&
          (i == 0 || i == pw - 1 || j == 0 || j == ph - 1)
        ) {
          vzero(f);
          ts->avgcounts[idx] = 0;
          continue;
        }
        f->x /= (float) ts->avgcounts[idx];
        f->y /= (float) ts->avgcounts[idx];

        tmp.x = f->x;
        tmp.y = f->y;

        tmp.x -= p->x;
        tmp.y -= p->y;

        tmp.x *= dt;
        tmp.y *= dt;

        p->x += tmp.x;
        p->y += tmp.y;

        vzero(f);
        ts->avgcounts[idx] = 0;
      }
    }
  }
}

void add_continents_to_sheet(
  tectonic_sheet *ts,
  float strength,
  float scale,
  float dstr,
  float dscale,
  ptrdiff_t seed
) {
  size_t i, j, idx;
  size_t pw, ph;
  float fx, fy;
  vector *p;
  ptrdiff_t dxseed, dyseed;
  float xphase, yphase;

  dxseed = prng(seed);
  dyseed = prng(dxseed);

  seed = prng(seed + 18291);
  xphase = ptrf(seed);
  seed = prng(seed + 6546);
  yphase = ptrf(seed);

  pw = sheet_pwidth(ts);
  ph = sheet_pwidth(ts);

  // Only pass: add continent height
  for (i = 0; i < pw; ++i) {
    for (j = 0; j < ph; ++j) {
      idx = sheet_pidx(ts, i, j);
      p = &(ts->points[idx]);
      fx = p->x;
      fy = p->y;

      // distortion
      fx += dstr * sxnoise_2d(
        p->x * dscale,
        p->y * dscale,
        dxseed
      );
      fy += dstr * sxnoise_2d(
        p->x * dscale,
        p->y * dscale,
        dyseed
      );

      // scaling:
      fx *= scale;
      fy *= scale;

      // sin/cos continents:
      p->z += strength * (
        sin(2.0 * M_PI * (fx + xphase))
      *
        cos(2.0 * M_PI * (fy + yphase))
      );
    }
  }
}

void add_ridges_to_sheet(
  tectonic_sheet *ts,
  float strength,
  float scale,
  float dstr,
  float dscale,
  float mscale,
  ptrdiff_t seed,
  ptrdiff_t mseed
) {
  size_t i, j, idx;
  size_t pw, ph;
  float fx, fy;
  float mx, my;
  float xphase, yphase;
  float modulation;
  float ignore;
  vector *p;
  ptrdiff_t dxseed, dyseed;

  dxseed = prng(seed + 13392);
  dyseed = prng(dxseed);

  pw = sheet_pwidth(ts);
  ph = sheet_pwidth(ts);

  seed = prng(seed + 5448);
  xphase = ptrf(seed);
  seed = prng(seed + 88166);
  yphase = ptrf(seed);

  // Only pass: add ridge height
  for (i = 0; i < pw; ++i) {
    for (j = 0; j < ph; ++j) {
      idx = sheet_pidx(ts, i, j);
      p = &(ts->points[idx]);
      fx = p->x;
      fy = p->y;
      mx = p->x;
      my = p->y;

      // distortion
      fx += dstr * sxnoise_2d(
        p->x * dscale,
        p->y * dscale,
        dxseed
      );
      fy += dstr * sxnoise_2d(
        p->x * dscale,
        p->y * dscale,
        dyseed
      );

      // scaling:
      fx *= scale;
      fy *= scale;
      mx *= mscale;
      my *= mscale;

      // phase offset:
      fx += xphase;
      fy += yphase;

      // modulation:
      modulation = sxnoise_2d(mx, my, mseed);
      modulation = 0.5 * (1 + modulation);
      modulation = strict_sigmoid(modulation, TECT_RMOD_SIGSHAPE);
      modulation = expdist(modulation, TECT_RMOD_EXPSHAPE);

      // worley ridges:
      p->z += strength * modulation * wrnoise_2d_fancy(
        fx, fy,
        seed,
        0, 0,
        &ignore, &ignore,
        0
      );
    }
  }
}

void seam_sheet(
  tectonic_sheet *ts,
  float dt,
  vector *from,
  vector *to,
  float width,
  int pull,
  int hold_edges
) {
  size_t i, j, idx;
  float str;
  size_t pw, ph;
  vector *p;
  vector tmp;

  pw = sheet_pwidth(ts);
  ph = sheet_pwidth(ts);

  // Only pass: compute and apply push/pull vectors
  for (i = 0; i < pw; ++i) {
    for (j = 0; j < ph; ++j) {
      idx = sheet_pidx(ts, i, j);
      p = &(ts->points[idx]);

      if (
        hold_edges
      &&
        (i == 0 || i == pw - 1 || j == 0 || j == ph - 1)
      ) {
        continue;
      }

      closest_point_on_line_segment(p, from, to, &tmp);
      tmp.z = 0;
      tmp.x = p->x - tmp.x;
      tmp.y = p->y - tmp.y;
      // tmp is now the shortest 2D vector from the line to this point

      str = sqrtf(tmp.x * tmp.x + tmp.y * tmp.y);
      if (str > width) {
        continue;
      }
      str = pow((width - str) / width, TECT_SEAM_SHAPE);
      str = strict_sigmoid(str, TECT_SEAM_SIGSHAPE);
      tmp.x *= str;
      tmp.y *= str;

      if (pull) {
        tmp.x *= -1;
        tmp.y *= -1;
      }
      p->x += tmp.x * dt;
      p->y += tmp.y * dt;
    }
  }
}

void squash_sheet(
  tectonic_sheet *ts,
  float lower_cutoff,
  float new_min,
  float upper_cutoff,
  float new_max
) {
  size_t i, j, idx;
  size_t pw, ph;
  float old_min, old_max;
  vector *p;
  float interp;
  curve lcurve, ucurve;
  vector bzr;

  pw = sheet_pwidth(ts);
  ph = sheet_pwidth(ts);

  old_min = sheet_min_z(ts);
  old_max = sheet_max_z(ts);

  // set up the lower and upper interpolation curves:
  lcurve.from.x = 0;
  lcurve.from.y = lower_cutoff;
  lcurve.from.z = 0;
  lcurve.go_towards.x = 0.5; // TODO: Be smarter here!
  lcurve.go_towards.y = new_min;
  lcurve.go_towards.z = 0;
  vcopy_as(&(lcurve.come_from), &(lcurve.go_towards));
  lcurve.to.x = 1.0;
  lcurve.to.y = new_min;
  lcurve.to.z = 0;

  ucurve.from.x = 0;
  ucurve.from.y = upper_cutoff;
  ucurve.from.z = 0;
  ucurve.go_towards.x = 0.5; // TODO: Be smarter here!
  ucurve.go_towards.y = new_max;
  ucurve.go_towards.z = 0;
  vcopy_as(&(ucurve.come_from), &(ucurve.go_towards));
  ucurve.to.x = 1.0;
  ucurve.to.y = new_max;
  ucurve.to.z = 0;

  // Only pass: squash lower and upper z values:
  for (i = 0; i < pw; ++i) {
    for (j = 0; j < ph; ++j) {
      idx = sheet_pidx(ts, i, j);
      p = &(ts->points[idx]);

      if (p->z < lower_cutoff) {
        interp = (lower_cutoff - p->z) / (lower_cutoff - old_min);
        point_on_curve(&lcurve, interp, &bzr);
        p->z = bzr.y;
      } else if (p->z > upper_cutoff) {
        interp = (p->z - upper_cutoff) / (old_max - upper_cutoff);
        point_on_curve(&ucurve, interp, &bzr);
        p->z = bzr.y;
      }
    }
  }
}

void generate_tectonics(world_map *wm) {
  size_t i;
  vector a, b;
  ptrdiff_t seed;
  float min_x, max_x, min_y, max_y, min_z, max_z;
  float squash_cutoff, squash_remap;
  tectonic_sheet *ts, *save;

  ts = wm->tectonics;
  seed = ts->seed;

  min_x = sheet_min_x(ts);
  max_x = sheet_max_x(ts);
  min_y = sheet_min_y(ts);
  max_y = sheet_max_y(ts);

  // Reset the sheet
  reset_sheet(ts);

  // Rustle the points a bit to undermine grid uniformity
  rustle_sheet(ts, TECT_SMALL_RUSTLE_STR, TECT_SMALL_RUSTLE_SCALE, seed);
  seed = prng(seed);
  rustle_sheet(ts, TECT_LARGE_RUSTLE_STR, TECT_LARGE_RUSTLE_SCALE, seed);
  seed = prng(seed);

  // Add continents and ridges to the sheet (will affect spring forces later)
  add_continents_to_sheet(
    ts,
    TECT_CONTINENTS_STR,
    TECT_CONTINENTS_SCALE,
    TECT_CONTINENTS_DSTR,
    TECT_CONTINENTS_DSCALE,
    seed
  );
  seed = prng(seed);
  add_continents_to_sheet(
    ts,
    TECT_CONTINENTS_STR * 0.8,
    TECT_CONTINENTS_SCALE * 1.4,
    TECT_CONTINENTS_DSTR,
    TECT_CONTINENTS_DSCALE * 1.4,
    seed
  );
  seed = prng(seed);

  add_ridges_to_sheet(
    ts,
    TECT_RIDGES_STR,
    TECT_RIDGES_SCALE,
    TECT_RIDGES_DSTR,
    TECT_RIDGES_DSCALE,
    TECT_RIDGES_MSCALE,
    seed,
    seed + 1817
  );
  seed = prng(seed);
  add_ridges_to_sheet(
    ts,
    TECT_RIDGES_STR,
    TECT_RIDGES_SCALE * 1.6,
    TECT_RIDGES_DSTR,
    TECT_RIDGES_DSCALE * 1.6,
    TECT_RIDGES_MSCALE,
    seed,
    seed + 1817
  );
  seed = prng(seed);

  // Save a copy of our base height (continents + ridges)
  save = copy_tectonic_sheet(ts); // allocates memory

  // Push/pull to/from some random seams
  a.z = 0;
  b.z = 0;
  for (i = 0; i < TECT_SEAM_COUNT; ++i) {
    // Get a random line segment:
    a.x = min_x + ptrf(seed) * (max_x - min_x);
    seed = prng(seed);
    a.y = min_y + ptrf(seed) * (max_y - min_y);
    seed = prng(seed);
    b.x = min_x + ptrf(seed) * (max_x - min_x);
    seed = prng(seed);
    b.y = min_y + ptrf(seed) * (max_y - min_y);
    seed = prng(seed);

    // Add the seam:
    seam_sheet(
      ts,
      TECT_SEAM_DT,
      &a,
      &b,
      TECT_SEAM_MIN_WIDTH + ptrf(seed) * TECT_SEAM_WIDTH_VAR,
      seed % 2 == 0,
      0
    );
    seed = prng(seed);
  }

  // Crumple up the sheet based on the seam results:
  for (i = 0; i < TECT_CRUMPLE_STEPS; ++i) {
    rustle_sheet(
      ts,
      TECT_CRUMPLE_RUSTLE_STR,
      TECT_CRUMPLE_RUSTLE_SCALE,
      seed
    );
    seed = prng(seed);
    settle_sheet(
      ts,
      TECT_CRUMPLE_SETTLE_STEPS,
      TECT_CRUMPLE_SETTLE_DT,
      TECT_CRUMPLE_EQ_DIST,
      TECT_CRUMPLE_K,
      0
    );
    untangle_sheet(
      ts,
      TECT_CRUMPLE_UNTANGLE_STEPS,
      TECT_CRUMPLE_UNTANGLE_DT,
      0
    );
  }

  // Stretch the sheet into a square shape:
  min_x = sheet_min_x(ts);
  max_x = sheet_max_x(ts);
  min_y = sheet_min_y(ts);
  max_y = sheet_max_y(ts);
  stretch_sheet(ts, max_x - min_x, max_y - min_y);
  settle_sheet(
    ts,
    TECT_STRETCH_RELAX_STEPS,
    TECT_STRETCH_RELAX_DT,
    TECT_STRETCH_RELAX_EQ_DIST,
    TECT_STRETCH_RELAX_K,
    1
  );
  untangle_sheet(
    ts,
    TECT_STRETCH_UNTANGLE_STEPS,
    TECT_STRETCH_UNTANGLE_DT,
    1
  );

  min_z = sheet_min_z(ts);
  max_z = sheet_max_z(ts);
  squash_cutoff = min_z + (max_z - min_z) * TECT_SQUASH_UPPER_FRACTION;
  squash_remap = squash_cutoff;
  squash_remap += (
    TECT_SQUASH_MAX_FACTOR
  * (1 - TECT_SQUASH_UPPER_FRACTION)
  * (max_z - min_z)
  );
  // Squash the sheet to make mountains stand out:
  squash_sheet(
    ts,
    TECT_SQUASH_LOWER_CUTOFF,
    TECT_SQUASH_NEW_MIN,
    squash_cutoff,
    squash_remap
  );

  // Merge our crumpled data with our pre-crumpling data (ignoring the fact
  // that grid positions have changed meanwhile).
  for (i = 0; i < sheet_pwidth(ts) * sheet_pheight(ts); ++i) {
    ts->points[i].z += save->points[i].z * TECT_RECOMBINE_STR;
    ts->points[i].z /= 1 + TECT_RECOMBINE_STR;
  }

  // Make sure to free our sheet copy:
  free(save);
}

float sheet_height(
  tectonic_sheet *ts,
  float fx,
  float fy
) {
  size_t i, j, idx_a, idx_b, idx_c;
  float px, py;
  float min_x, max_x, min_y, max_y;
  vector bc;

  min_x = sheet_min_x(ts);
  max_x = sheet_max_x(ts);
  min_y = sheet_min_y(ts);
  max_y = sheet_max_y(ts);

  px = min_x + (max_x - min_x) * fx;
  py = min_y + (max_y - min_y) * fy;

  // TODO: More efficient search here?
  for (i = 0; i < ts->width; ++i) {
    for (j = 0; j < ts->height; ++j) {
      idx_a = sheet_pidx_a(ts, i, j);
      idx_b = sheet_pidx_b(ts, i, j);
      idx_c = sheet_pidx_c(ts, i, j);

      bc.x = px;
      bc.y = py;
      bc.z = 0;
      xy__barycentric(
        &bc,
        &(ts->points[idx_a]),
        &(ts->points[idx_b]),
        &(ts->points[idx_c])
      );
      if (bc.x > 0 && bc.y > 0 && bc.z > 0) { // we're inside this triangle
        barycentric__xy(
          &bc,
          &(ts->points[idx_a]),
          &(ts->points[idx_b]),
          &(ts->points[idx_c])
        );
        return bc.z;
      }
    }
  }
#ifdef DEBUG
  fprintf(
    stderr,
    "WARNING: sheet height: no triangle at (%.3f, %.3f) -> (%.3f, %.3f).\n",
    fx, fy,
    px, py
  );
  return -100;
#endif
  return 0;
}

// General geology functions:
// --------------------------

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
  geologic_source source = GEO_SEDIMENTARY;
  ptrdiff_t hash, h1, h2, h3, h4, h5;
  world_region *wr;
  for (i = 0; i < WM_MAX_STRATA_LAYERS * STRATA_COMPLEXITY; ++i) {
    // Create a stratum and append it to the list of all strata:
    hash = prng(prng(wm->seed + 567*i));
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
    source = (geologic_source) rt_pick_result(&STONE_SOURCE_DISTRIBUTION, h5);
    s = create_stratum(
      wm,
      hash,
      ptrf(hash)*wm->width, ptrf(h1)*wm->height,
      avg_size * (0.6 + ptrf(h2)*0.8), // size
      BASE_STRATUM_THICKNESS * exp(-0.5 + ptrf(h3)*3.5), // thickness
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

species create_new_stone_species(
  world_map *wm,
  geologic_source source,
  ptrdiff_t seed
) {
  float base_density;
  species result = create_stone_species();
  stone_species* ssp = get_stone_species(result);

  ssp->source = source;

  switch (source) {
    case GEO_IGNEOUS:
    default:
      // composition
      determine_new_stone_composition(
        ssp,
        wm,
        &IGNEOUS_COMPOSITIONS,
        &IGNEOUS_TRACES,
        seed
      );

      // material
      base_density = determine_new_igneous_material(ssp, seed);
      seed = prng(seed);

      // appearance
      determine_new_igneous_appearance(
        ssp,
        base_density,
        seed
      );
      break;
    case GEO_METAMORPHIC:
      // composition
      determine_new_stone_composition(
        ssp,
        wm,
        &METAMORPHIC_COMPOSITIONS,
        &METAMORPHIC_TRACES,
        seed
      );

      // material
      base_density = determine_new_metamorphic_material(ssp, seed);
      seed = prng(seed);

      // appearance
      determine_new_metamorphic_appearance(
        ssp,
        base_density,
        seed
      );
      break;
    case GEO_SEDIMENTARY:
      // composition
      determine_new_stone_composition(
        ssp,
        wm,
        &SEDIMENTARY_COMPOSITIONS,
        &SEDIMENTARY_TRACES,
        seed
      );

      // material
      base_density = determine_new_sedimentary_material(ssp, seed);
      seed = prng(seed);

      // appearance
      determine_new_sedimentary_appearance(
        ssp,
        base_density,
        seed
      );
      break;
  }
  return result;
}

void determine_new_stone_composition(
  stone_species *ssp,
  world_map *wm,
  rngtable const * const composition,
  rngtable const * const trace_composition,
  ptrdiff_t seed
) {
  seed = prng(seed + 467541);

  // composition
  ssp->composition = (mineral_composition) rt_pick_result(
    composition,
    seed
  );
  seed = prng(seed);
  ssp->trace_composition = (mineral_trace_composition) rt_pick_result(
    trace_composition,
    seed
  );
  seed = prng(seed);

  determine_new_elemental_composition(
    wm,
    ssp->composition,
    ssp->constituents,
    seed
  );
  seed = prng(seed);

  determine_new_elemental_traces(
    wm,
    ssp->trace_composition,
    ssp->traces,
    seed
  );
}

void determine_new_elemental_composition(
  world_map *wm,
  mineral_composition comp,
  species *comp_array,
  ptrdiff_t seed
) {
  list* used = create_list();
  size_t i;
  element_species *esp;
  element_categorization first_category = 0;
  element_categorization second_category = 0;
  element_categorization third_category = 0;

  seed = prng(seed + 178818);

  switch (comp) {
    case MNRL_COMP_STONE:
    default:
      first_category = EL_CATEGORY_STONE;
      break;

    case MNRL_COMP_LIFE:
      first_category = EL_CATEGORY_LIFE;
      break;

    case MNRL_COMP_STONE_AIR:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_AIR;
      break;

    case MNRL_COMP_STONE_WATER:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_WATER;
      break;

    case MNRL_COMP_STONE_LIFE:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_LIFE;
      break;

    case MNRL_COMP_STONE_STONE:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_STONE;
      break;

    case MNRL_COMP_STONE_STONE_LIFE:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_STONE;
      third_category = EL_CATEGORY_LIFE;
      break;

    case MNRL_COMP_STONE_STONE_STONE:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_STONE;
      third_category = EL_CATEGORY_STONE;
      break;

    case MNRL_COMP_STONE_METAL:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_METAL;
      break;

    case MNRL_COMP_STONE_STONE_METAL:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_STONE;
      third_category = EL_CATEGORY_METAL;
      break;

    case MNRL_COMP_STONE_METAL_METAL:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_METAL;
      third_category = EL_CATEGORY_METAL;
      break;

    case MNRL_COMP_STONE_METAL_RARE:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_METAL;
      third_category = EL_CATEGORY_RARE;
      break;

    case MNRL_COMP_STONE_RARE:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_RARE;
      break;

    case MNRL_COMP_STONE_STONE_RARE:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_STONE;
      third_category = EL_CATEGORY_RARE;
      break;

    case MNRL_COMP_STONE_RARE_RARE:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_RARE;
      third_category = EL_CATEGORY_RARE;
      break;

    case MNRL_COMP_RARE_RARE:
      first_category = EL_CATEGORY_RARE;
      second_category = EL_CATEGORY_RARE;
      break;
  }
  i = 0;
  if (first_category != 0) {
    esp = pick_element(wm, first_category, used, seed);
    if (esp != NULL) {
      comp_array[i] = esp->id;
      l_append_element(used, (void*) esp);
      i += 1;
      seed = prng(seed);
    }
  }
  if (second_category != 0) {
    esp = pick_element(wm, second_category, used, seed);
    if (esp != NULL) {
      comp_array[i] = esp->id;
      l_append_element(used, (void*) esp);
      i += 1;
      seed = prng(seed);
    }
  }
  if (third_category != 0) {
    esp = pick_element(wm, third_category, used, seed);
    if (esp != NULL) {
      comp_array[i] = esp->id;
      // l_append_element(used, (void*) esp);
      i += 1;
      seed = prng(seed);
    }
  }
  for (; i < MAX_PRIMARY_CONSTITUENTS; ++i) {
    comp_array[i] = 0;
  }

  cleanup_list(used);
}

void determine_new_elemental_traces(
  world_map *wm,
  mineral_trace_composition comp,
  species *trace_array,
  ptrdiff_t seed
) {
  list* used = create_list();
  size_t i;
  element_species *esp;
  element_categorization first_category = 0;
  element_categorization second_category = 0;

  seed = prng(seed + 81771);

  switch (comp) {
    case MNRL_TRACE_NONE:
    default:
      break;

    case MNRL_TRACE_AIR:
      first_category = EL_CATEGORY_AIR;
      break;

    case MNRL_TRACE_WATER:
      first_category = EL_CATEGORY_WATER;
      break;

    case MNRL_TRACE_LIFE:
      first_category = EL_CATEGORY_LIFE;
      break;

    case MNRL_TRACE_STONE:
      first_category = EL_CATEGORY_STONE;
      break;

    case MNRL_TRACE_STONE_METAL:
      first_category = EL_CATEGORY_STONE;
      second_category = EL_CATEGORY_METAL;
      break;

    case MNRL_TRACE_METAL:
      first_category = EL_CATEGORY_METAL;
      break;

    case MNRL_TRACE_METAL_METAL:
      first_category = EL_CATEGORY_METAL;
      second_category = EL_CATEGORY_METAL;
      break;

    case MNRL_TRACE_METAL_RARE:
      first_category = EL_CATEGORY_METAL;
      second_category = EL_CATEGORY_RARE;
      break;

    case MNRL_TRACE_RARE:
      first_category = EL_CATEGORY_RARE;
      break;

    case MNRL_TRACE_RARE_RARE:
      first_category = EL_CATEGORY_RARE;
      second_category = EL_CATEGORY_RARE;
      break;
  }
  i = 0;
  if (first_category != 0) {
    esp = pick_element(wm, first_category, used, seed);
    if (esp != NULL) {
      trace_array[i] = esp->id;
      l_append_element(used, (void*) esp);
      i += 1;
      seed = prng(seed);
    }
  }
  if (second_category != 0) {
    esp = pick_element(wm, second_category, used, seed);
    if (esp != NULL) {
      trace_array[i] = esp->id;
      // l_append_element(used, (void*) esp);
      i += 1;
      seed = prng(seed);
    }
  }
  for (; i < MAX_TRACE_CONSTITUENTS; ++i) {
    trace_array[i] = 0;
  }

  cleanup_list(used);
}

float determine_new_igneous_material(
  stone_species *species,
  ptrdiff_t seed
) {
  material *target = &(species->material);
  size_t i;
  element_species *element;
  float denom;
  float avg;

  target->origin = MO_IGNEOUS_MINERAL;

  // The "base density" as influenced by the elemental composition of the
  // species determines standard material properties, each of which is then
  // further influenced a bit by the constituent elements.
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_density_tendency,
    avg
  );
  float base_density = (ptrf(seed) * 0.4  +  avg * 0.6);
  seed = prng(seed);

  // Base density directly controls density (element influences are already
  // rolled into base density).
  target->solid_density = mat_density(0.25  +  4.75 * base_density);
  target->liquid_density = mat_density(2.5  +  0.5 * base_density);
  target->gas_density = mat_density(1.8  +  1.5 * base_density);

  // A much tighter distribution of specific heats that correlates a bit with
  // density:
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_specific_heat_tendency,
    avg
  );
  float base_specific_heat = (
    randf_pnorm(seed, 0, 1)
  + randf_pnorm(prng(seed), 0, 1)
  )/2.0;
  base_specific_heat = 0.7 * base_specific_heat  +  0.3 * (1 - base_density);
  base_specific_heat = 0.6 * base_specific_heat  +  0.4 * avg;
  seed = prng(seed);

  target->solid_specific_heat = mat_specific_heat(
    0.3  +  1.1 * base_specific_heat
  );
  target->liquid_specific_heat = mat_specific_heat(
    0.8  +  1.4 * base_specific_heat
  );
  target->gas_specific_heat = mat_specific_heat(
    0.65  +  0.45 * base_specific_heat
  );

  target->cold_damage_temp = 0; // stones aren't vulnerable to cold

  // A flat distribution of melting points, slightly correlated with density:
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_transition_temp_tendency,
    avg
  );
  float base_transition_temp = ptrf(seed);
  base_transition_temp = 0.8 * base_transition_temp  +  0.2 * base_density;
  base_transition_temp = 0.6 * base_transition_temp  +  0.4 * avg;
  seed = prng(seed);

  target->solidus = 550 + 700 * base_transition_temp;
  target->liquidus = target->solidus + randf_pnorm(seed, 50, 250);
  seed = prng(seed);

  target->boiling_point = 1800 + base_transition_temp * 600;

  // igneous stone isn't known for combustion:
  target->ignition_point = smaxof(temperature);
  target->flash_point = smaxof(temperature);

  // igneous rock can begin to exhibit plasticity at temperatures as low as 20%
  // of its solidus, and can reach max plasticity by 60% of its solidus:
  float base_plastic_temp = pow(randf_pnorm(seed, 0, 1), 1.2);
  seed = prng(seed);
  target->cold_plastic_temp = target->solidus * (0.2 + base_plastic_temp * 0.5);
  target->warm_plastic_temp = target->solidus * (0.6 + base_plastic_temp * 0.4);

  // it is generally not very plastic, and may be as brittle as glass:
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_plasticity_tendency,
    avg
  );
  // using this distribution more than 75% of values fall into [0, 0.33]...
  float base_plasticity = expdist(ptrf(seed), 5);
  seed = prng(seed);
  base_plasticity = 0.8 * base_plasticity  +  0.2 * avg;

  // ...so since the first 1/3 of results are mapped to 0, >75% of igneous
  // rocks have 0 cold plasticity (ignoring elemental constituent effects):
  float tmp = (-5.0 + base_plasticity * 15.0);
  if (tmp < 0) { tmp = 0; }
  target->cold_plasticity = (plasticity) tmp;

  base_plasticity = expdist(ptrf(seed), 3);
  seed = prng(seed);
  base_plasticity = 0.6 * base_plasticity + 0.4 * avg;
  target->warm_plasticity = 5 + base_plasticity * 75;

  // Magma is extremely viscous (but also has a huge range of possible
  // viscosities). The composing elements' plasticity values are used.
  target->viscosity = pow(
    10.0,
    4.0
  + 2.0 * ptrf(seed)
  + 3.0 * base_density
  + 3.0 * avg
  );
  seed = prng(seed);

  // igneous rocks are pretty hard (this correlates with density):
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_hardness_tendency,
    avg
  );
  float base_hardness = randf_pnorm(seed, 0, 1);
  seed = prng(seed);
  base_hardness = 0.8 * base_hardness  +  0.2 * base_density;
  base_hardness = 0.6 * base_hardness  +  0.4 * avg;
  target->hardness = 100  +  120 * base_hardness;

  return base_density;
}

float determine_new_metamorphic_material(
  stone_species *species,
  ptrdiff_t seed
) {
  material *target = &(species->material);
  size_t i;
  element_species *element;
  float denom;
  float avg;

  target->origin = MO_METAMORPHIC_MINERAL;

  // The "base density" as influenced by the elemental composition of the
  // species determines standard material properties, each of which is then
  // further influenced a bit by the constituent elements.
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_density_tendency,
    avg
  );
  float base_density = (ptrf(seed) * 0.4  +  avg * 0.6);
  seed = prng(seed);

  // Base density is taken from the external argument:
  target->solid_density = mat_density(0.9 + 4.8 * base_density);
  target->liquid_density = mat_density(2.6 + 0.6 * base_density);
  target->gas_density = mat_density(1.8 + 1.5 * base_density);

  // A tighter distribution of specific heats that correlates a bit with
  // density:
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_specific_heat_tendency,
    avg
  );
  float base_specific_heat = randf_pnorm(seed, 0, 1);
  base_specific_heat = 0.7 * base_specific_heat  +  0.3 * (1 - base_density);
  base_specific_heat = 0.6 * base_specific_heat  +  0.4 * avg;
  seed = prng(seed);

  target->solid_specific_heat = mat_specific_heat(
    0.2  +  1.3 * base_specific_heat
  );
  target->liquid_specific_heat = mat_specific_heat(
    0.7  +  1.7 * base_specific_heat
  );
  target->gas_specific_heat = mat_specific_heat(
    0.6  +  0.55 * base_specific_heat
  );

  target->cold_damage_temp = 0; // stones aren't vulnerable to cold

  // A flat distribution of melting points, slightly correlated with density:
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_transition_temp_tendency,
    avg
  );
  float base_transition_temp = ptrf(seed);
  base_transition_temp = 0.8 * base_transition_temp  +  0.2 * base_density;
  base_transition_temp = 0.6 * base_transition_temp  +  0.4 * avg;
  seed = prng(seed);

  target->solidus = 520 + 760 * base_transition_temp;
  target->liquidus = target->solidus + randf_pnorm(seed, 20, 320);
  seed = prng(seed);

  target->boiling_point = 1700 + base_transition_temp * 800;

  // normal metamorphic stone isn't known for combustion:
  // TODO: Some high values here?
  target->ignition_point = smaxof(temperature);
  target->flash_point = smaxof(temperature);


  // metamorphic rock can begin to exhibit plasticity at temperatures as low as
  // 30% of its solidus, and can reach max plasticity by 50% of its solidus:
  float base_plastic_temp = pow(randf_pnorm(seed, 0, 1), 1.2);
  seed = prng(seed);
  target->cold_plastic_temp = target->solidus * (0.3 + base_plastic_temp * 0.4);
  target->warm_plastic_temp = target->solidus * (0.5 + base_plastic_temp * 0.5);

  // it is generally not very plastic, and may be quite brittle 
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_plasticity_tendency,
    avg
  );
  float base_plasticity = expdist(ptrf(seed), 3);
  seed = prng(seed);
  base_plasticity = 0.5 * base_plasticity  +  0.5 * avg;

  float tmp = (-3.0 + base_plasticity * 18.0);
  if (tmp < 0) { tmp = 0; }
  target->cold_plasticity = (plasticity) tmp;

  base_plasticity = expdist(ptrf(seed), 2);
  seed = prng(seed);
  base_plasticity = 0.5 * base_plasticity + 0.5 * avg;
  target->warm_plasticity = 10 + base_plasticity * 90;

  // Magma is extremely viscous (but also has a huge range of possible
  // viscosities). The composing elements' plasticity values are used.
  target->viscosity = pow(
    10.0,
    6.0
  + 4.0 * ptrf(seed)
  + 3.0 * base_density
  + 3.0 * avg
  );
  seed = prng(seed);

  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_hardness_tendency,
    avg
  );
  float base_hardness = sqrtf(randf_pnorm(seed, 0, 1));
  seed = prng(seed);
  base_hardness = 0.8 * base_hardness  +  0.2 * base_density;
  base_hardness = 0.6 * base_hardness  +  0.4 * avg;
  target->hardness = 40  +  190 * base_hardness;

  return base_density;
}

float determine_new_sedimentary_material(
  stone_species *species,
  ptrdiff_t seed
) {
  material *target = &(species->material);
  size_t i;
  element_species *element;
  float denom;
  float avg;

  target->origin = MO_SEDIMENTARY_MINERAL;

  // The "base density" as influenced by the elemental composition of the
  // species determines standard material properties, each of which is then
  // further influenced a bit by the constituent elements.
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_density_tendency,
    avg
  );
  float base_density = (ptrf(seed) * 0.4  +  avg * 0.6);
  seed = prng(seed);

  // Base density is taken from the external argument:
  target->solid_density = mat_density(1.2 + 3.7 * base_density);
  target->liquid_density = mat_density(2.6 + 0.5 * base_density);
  target->gas_density = mat_density(1.6 + 1.3 * base_density);

  // A tighter distribution of specific heats that correlates a bit with
  // density:
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_specific_heat_tendency,
    avg
  );
  float base_specific_heat = randf_pnorm(seed, 0, 1);
  base_specific_heat = 0.7 * base_specific_heat  +  0.3 * (1 - base_density);
  base_specific_heat = 0.6 * base_specific_heat  +  0.4 * avg;
  seed = prng(seed);

  target->solid_specific_heat = mat_specific_heat(
    0.4  +  1.4 * base_specific_heat
  );
  target->liquid_specific_heat = mat_specific_heat(
    0.9  +  1.4 * base_specific_heat
  );
  target->gas_specific_heat = mat_specific_heat(
    0.7  +  0.5 * base_specific_heat
  );

  target->cold_damage_temp = 0; // stones aren't vulnerable to cold

  // A flat distribution of melting points, slightly correlated with density:
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_transition_temp_tendency,
    avg
  );
  float base_transition_temp = ptrf(seed);
  base_transition_temp = 0.8 * base_transition_temp  +  0.2 * base_density;
  base_transition_temp = 0.6 * base_transition_temp  +  0.4 * avg;
  seed = prng(seed);

  target->solidus = 440 + 780 * base_transition_temp;
  target->liquidus = target->solidus + randf_pnorm(seed, 10, 190);
  seed = prng(seed);

  target->boiling_point = 1550 + base_transition_temp * 600;

  // normal sedimentary stone isn't known for combustion (see special fuel
  // stone generation methods):
  // TODO: Said methods!
  // TODO: Some high values here?
  target->ignition_point = smaxof(temperature);
  target->flash_point = smaxof(temperature);

  // sedimentary rock can begin to exhibit plasticity at temperatures as low as
  // 20% of its solidus, and can reach max plasticity by 50% of its solidus:
  float base_plastic_temp = pow(randf_pnorm(seed, 0, 1), 1.3);
  seed = prng(seed);
  target->cold_plastic_temp = target->solidus * (0.2 + base_plastic_temp * 0.5);
  target->warm_plastic_temp = target->solidus * (0.5 + base_plastic_temp * 0.5);

  // it is generally quite brittle:
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_plasticity_tendency,
    avg
  );
  float base_plasticity = expdist(ptrf(seed), 5.3);
  seed = prng(seed);
  base_plasticity = 0.8 * base_plasticity  +  0.2 * avg;

  float tmp = (-7.0 + base_plasticity * 12.0);
  if (tmp < 0) { tmp = 0; } // a large fraction will have 0 cold plasticity
  target->cold_plasticity = (plasticity) tmp;

  base_plasticity = expdist(ptrf(seed), 4);
  seed = prng(seed);
  base_plasticity = 0.6 * base_plasticity + 0.4 * avg;
  target->warm_plasticity = base_plasticity * 60;

  // sedimentary magma is generally more viscous than other types (says I):
  target->viscosity = pow(
    10.0,
    5.0
  + 2.0 * ptrf(seed)
  + 3.0 * base_density
  + 3.0 * avg
  );
  seed = prng(seed);

  // sedimentary rocks are generally a bit softer than other types:
  WEIGHTED_ELEMENT_PROPERTY(
    species->constituents,
    STONE_CONSTITUENT_AVERAGING_WEIGHTS,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_hardness_tendency,
    avg
  );
  float base_hardness = pow(randf_pnorm(seed, 0, 1), 0.8);
  seed = prng(seed);
  base_hardness = 0.8 * base_hardness  +  0.2 * base_density;
  base_hardness = 0.6 * base_hardness  +  0.4 * avg;
  target->hardness = 30  +  150 * base_hardness;

  return base_density;
}


void determine_new_igneous_appearance(
  stone_species *species,
  float base_density,
  ptrdiff_t seed
) {
  stone_filter_args *target = &(species->appearance);
  seed = prng(seed);
  target->seed = seed;

  // Lighter rocks tend to have smaller noise scales.
  target->scale = 0.1 + 0.08*(0.8*ptrf(seed) + 0.2*base_density);
  seed = prng(seed);

  // Igneous rock types are relatively gritty. Denser rocks tend to be slightly
  // less gritty though.
  target->gritty = 1.4 + 2.3*(0.7*ptrf(seed) + 0.3*(1-base_density));
  seed = prng(seed);

  // Denser rocks tend to be more contoured.
  target->contoured = 3.5 + 3.5*(0.8*ptrf(seed) + 0.2*base_density);
  seed = prng(seed);

  // Lighter igneous rocks are far more porous.
  target->porous = 2.0 + 8.5*(0.5*ptrf(seed) + 0.5*(1-base_density));
  seed = prng(seed);

  // Igneous rocks don't tend to be very bumpy.
  target->bumpy = 1.0 + 4.0*ptrf(seed);
  seed = prng(seed);

  // Igneous rocks rarely have significant inclusions.
  target->inclusions = pow(randf_pnorm(seed, 0, 1), 2.5);
  seed = prng(seed);

  // The distortion scale is within 10% of the base scale.
  target->dscale = target->scale * 1 + 0.2*(ptrf(seed) - 0.5);
  seed = prng(seed);

  // Igneous rocks can have major distortion, but largely have little to
  // moderate distortion.
  target->distortion = 7*pow(randf_pnorm(seed, 0, 1), 1.5);
  seed = prng(seed);

  // Igneous rocks can be squashed in either direction
  target->squash = randf_pnorm(seed, 0.7, 1.3);
  seed = prng(seed);
  target->squash /= randf_pnorm(seed, 0.7, 1.3);
  seed = prng(seed);

  // We define our color in L*c*h*, and convert to RGB later, potentially
  // clipping some values to keep them in-gamut.
  precise_color color;
  compute_combined_color(species, base_density, &color, seed);
  seed = prng(seed);

  // Construct the base color:
  lch__lab(&color);
  lab__xyz(&color);

  target->base_color = xyz__rgb(&color);

  // Igneous inclusions mostly just have contrasting brightness:
  color.x = (100.0 - color.x) + randf_pnorm(seed, -15, 15);
  seed = prng(seed);
  color.y += randf_pnorm(seed, -4, 12);
  color.z += randf_pnorm(seed, -M_PI/12.0, M_PI/12.0);

  target->alt_color = xyz__rgb(&color);

  // Not biased towards either brightness or darkness:
  target->brightness = randf_pnorm(seed, -0.2, 0.2);
}

void determine_new_metamorphic_appearance(
  stone_species *species,
  float base_density,
  ptrdiff_t seed
) {
  stone_filter_args *target = &(species->appearance);
  seed = prng(seed);
  target->seed = seed;

  // Metamorphic rocks exhibit a wide range of noise scales:
  target->scale = 0.08 + 0.12*ptrf(seed);
  seed = prng(seed);

  // Metamorphic rock types are usually not very gritty, especially when
  // they're very dense.
  target->gritty = 0.8 + 2.5*(0.7*ptrf(seed) + 0.3*(1-base_density));
  seed = prng(seed);

  // Denser rocks tend to be more contoured.
  target->contoured = 1.5 + 7.5*(0.8*ptrf(seed) + 0.2*base_density);
  seed = prng(seed);

  // Lighter metamorphic rocks are a bit more porous.
  target->porous = 1.0 + 8.0*(0.7*ptrf(seed) + 0.3*(1-base_density));
  seed = prng(seed);

  // Metamorphic rocks can be quite bumpy, especially when less dense.
  target->bumpy = 1.0 + 9.0*(0.6*ptrf(seed) + 0.4*(1-base_density));
  seed = prng(seed);

  // Metamorphic rocks often have significant inclusions.
  target->inclusions = pow(ptrf(seed), 1.3);
  seed = prng(seed);

  // The distortion scale is within 10% of the base scale.
  target->dscale = target->scale * 1 + 0.2*(ptrf(seed) - 0.5);
  seed = prng(seed);

  // Metamorphic rocks can have quite a bit of distortion.
  target->distortion = randf_pnorm(seed, 0, 6.5);
  seed = prng(seed);

  // And they can be squashed quite a bit.
  target->squash = randf_pnorm(seed, 0.6, 1.4);
  seed = prng(seed);
  target->squash /= randf_pnorm(seed, 0.6, 1.4);
  seed = prng(seed);

  // We define our color in L*c*h*, and convert to RGB later, potentially
  // clipping some values to keep them in-gamut.
  precise_color color;
  compute_combined_color(species, base_density, &color, seed);
  seed = prng(seed);

  // Construct the base color:
  lch__lab(&color);
  lab__xyz(&color);

  target->base_color = xyz__rgb(&color);

  // Metamorphic inclusions use the same distributions as the base rock, but
  // have a wider range of saturations:
  compute_combined_color(species, base_density, &color, seed);
  seed = prng(seed);
  color.y *= randf_pnorm(seed, 1.0, 1.5);
  seed = prng(seed);

  lch__lab(&color);
  lab__xyz(&color);

  target->alt_color = xyz__rgb(&color);

  // Metamorphic rocks have a slight bias towards brightness:
  target->brightness = randf_pnorm(seed, -0.25, 0.35);
}

void determine_new_sedimentary_appearance(
  stone_species *species,
  float base_density,
  ptrdiff_t seed
) {
  stone_filter_args *target = &(species->appearance);
  seed = prng(seed);
  target->seed = seed;

  // Sedimentary rocks often have smaller scales than other rocks.
  target->scale = 0.07 + 0.07*ptrf(seed);
  seed = prng(seed);

  // Sedimentary rock types are usually gritty.
  target->gritty = 3.1 + 2.5*ptrf(seed);
  seed = prng(seed);

  // Denser rocks tend to be more contoured.
  target->contoured = 1.5 + 9.5*(0.7*ptrf(seed) + 0.3*base_density);
  seed = prng(seed);

  // Lighter sedimentary rocks are a bit more porous.
  target->porous = 3.0 + 5.0*(0.7*ptrf(seed) + 0.3*(1-base_density));
  seed = prng(seed);

  // Sedimentary rocks can be quite bumpy.
  target->bumpy = 3.0 + 6.0*ptrf(seed);
  seed = prng(seed);

  // Sedimentary rocks usually don't have inclusions.
  target->inclusions = pow(randf_pnorm(seed, 0, 1), 2.5);
  seed = prng(seed);

  // The distortion scale is within 20% of the base scale.
  target->dscale = target->scale * 1 + 0.4*(ptrf(seed) - 0.5);
  seed = prng(seed);

  // Sedimentary rocks can have little to medium distortion.
  target->distortion = 4.5*pow(randf_pnorm(seed, 0, 1), 1.4);
  seed = prng(seed);

  // Sedimentary rocks are usually squashed horizontally.
  target->squash = randf_pnorm(seed, 0.6, 1.2);
  seed = prng(seed);
  target->squash /= randf_pnorm(seed, 0.8, 1.4);
  seed = prng(seed);

  // We define our color in L*c*h*, and convert to RGB later, potentially
  // clipping some values to keep them in-gamut.
  precise_color color;
  compute_combined_color(species, base_density, &color, seed);
  seed = prng(seed);

  // Construct the base color:
  lch__lab(&color);
  lab__xyz(&color);

  target->base_color = xyz__rgb(&color);

  // Sedimentary inclusions use the same distributions as the base rock, but
  // are often darker with less saturation.
  compute_combined_color(species, base_density, &color, seed);
  seed = prng(seed);
  color.x *= randf_pnorm(seed, 0.4, 1.2);
  color.y *= randf_pnorm(seed, 0.6, 1.1);
  seed = prng(seed);

  lch__lab(&color);
  lab__xyz(&color);

  target->alt_color = xyz__rgb(&color);

  // Sedimentary rocks are generally bright.
  target->brightness = -0.05 + 0.35*randf_pnorm(seed, 0, 1);
}

void compute_combined_color(
  stone_species *species,
  float base_density,
  precise_color *color,
  ptrdiff_t seed
) {
  size_t i;
  element_species *element;
  float denom = 0.0;
  float avg;
  float weight;

  // Hue
  // TODO: Tint & oxide colors!
  for (i = 0; i < MAX_PRIMARY_CONSTITUENTS; ++i) {
    if (species->constituents[i] != 0) {
      element = get_element_species(species->constituents[i]);
      switch (species->source) {
        case GEO_IGNEOUS:
        case GEO_SEDIMENTARY:
        default:
          weight = expdist(ptrf(seed), 4);
          seed = prng(seed);
          weight = 0.5 * weight + 0.5 * randf_pnorm(seed, 0.4, 1.0);
          seed = prng(seed);
          break;
        case GEO_METAMORPHIC: // considerably more variation in weights
          weight = expdist(ptrf(seed), 1.5);
          seed = prng(seed);
          weight = 0.5 * weight + 0.5 * ptrf(seed);
          seed = prng(seed);
          break;
      }
      // TODO: Better circular averaging technique here?
      color->z += weight * element->stone_chroma;
      denom += weight;
    }
  }
  if (denom != 0) {
    color->z /= denom;
  }

  // Saturation
  color->y = expdist(ptrf(seed), 3) * 60.0;
  seed = prng(seed);

  // Lightness
  AVERAGE_ELEMENT_PROPERTY(
    species->constituents,
    MAX_PRIMARY_CONSTITUENTS,
    i,
    element,
    denom,
    stone_light_dark_tendency,
    avg
  );
  color->x = 100.0 * (ptrf(seed) + ptrf(prng(seed + 18291))) / 2.0;
  seed = prng(seed);
  color->x = 0.3 * color->x + 0.7 * avg;

  // Customization according to geologic source:
  switch (species->source) {
    case GEO_IGNEOUS:
    default:
      // Small deviation in hue:
      color->z += randf_pnorm(seed, -M_PI/16.0, M_PI/16.0);
      seed = prng(seed);

      // Saturation is reduced:
      if (ptrf(seed) < 0.7) {
        seed = prng(seed);
        color->y *= randf_pnorm(seed, 0.05, 0.3);
        seed = prng(seed);
      } else {
        seed = prng(seed);
        color->y *= randf_pnorm(seed, 0.6, 0.8);
        seed = prng(seed);
      }

      // Lightness is correlated with base density and generally low:
      color->x = 0.6 * color->x + 0.4 * 100 * (1 - base_density);
      color->x = pow(color->x, 0.8);
      break;
    case GEO_METAMORPHIC:
      // Medium deviation in hue:
      color->z += randf_pnorm(seed, -M_PI/6.0, M_PI/6.0);
      seed = prng(seed);

      // Saturation may be boosted a bit:
      weight = randf_pnorm(seed, 0, 0.6);
      seed = prng(seed);
      color->y = (1 - weight) * color->y + weight * (30.0 + ptrf(seed) * 70.0);
      seed = prng(seed);

      // Lightness is unchanged.
      break;
    case GEO_SEDIMENTARY:
      // Small deviation in hue:
      color->z += randf_pnorm(seed, -M_PI/12.0, M_PI/12.0);
      seed = prng(seed);

      // Saturation is diminished:
      color->y *= expdist(randf_pnorm(seed, 0.3, 1.0), 2);
      seed = prng(seed);

      // Lightness may be increased:
      color->x *= randf_pnorm(seed, 1.0, 1.4);
      break;
  }

  // The modified color value is the result; no need to return anything.
}
