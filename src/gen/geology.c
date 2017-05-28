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
#include "elfscript/elfscript.h"

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

CSTR(GEO_GEN_KEY, "gen.geo", 7);
CSTR(GEO_GEN_KEY_SPECIES, "%gen_stone_species", 18);
CSTR(GEO_GEN_KEY_STRATA_PARAMS, "%gen_strata_params", 18);
CSTR(GEO_GEN_KEY_STRATUM, "%gen_stratum", 12);

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

stone_species* generate_stone_species(world_map *wm, ptrdiff_t seed) {
  stone_species *result = NULL;

  /*
   * TODO: Fix this!
  efd_node *gen_node, *gen_stone_species_node, *args_node, *species_node;

  SSTR(vn_world_map, "~world_map", 10);
  SSTR(f_singleton, "SINGLETON", 9);
  SSTR(vn_seed, "~seed", 5);
  SSTR(s_stone_species, "stone_species", 13);

  gen_node = efdx(ELFSCRIPT_ROOT, GEO_GEN_KEY);
  if (gen_node == NULL) {
    fprintf(
      stderr,
      "ERROR: Required node missing for stone species generation:\n"
    );
    s_fprintln(stderr, GEO_GEN_KEY);
  }
  gen_stone_species_node = efd_lookup_expected(gen_node, GEO_GEN_KEY_SPECIES);

  // Arguments to gen_stone_species:
  args_node = create_efd_node(ELFSCRIPT_NT_SCOPE, ELFSCRIPT_ANON_NAME, NULL);
  efd_add_child(
    args_node,
    construct_efd_obj_node(vn_world_map, NULL, f_singleton, wm)
  );
  efd_add_child(args_node, construct_efd_int_node(vn_seed, NULL, seed));

  // Generate and unpack the species (calling the unpacker directly ensures the
  // returned species is newly-allocated and won't be cleaned up along with the
  // underlying node):
  // TODO: Use an object node & copy its value instead?
  species_node = efd_call_function(gen_stone_species_node, args_node);
  result = (stone_species*) efd_lookup_unpacker(s_stone_species)(species_node);

  // Now we can cleanup the species node:
  cleanup_efd_node(species_node);

  // Add the new species to the global registry (thereby assigning its ID):
  (void) add_stone_species(result);
  */

  return result;
}

void generate_geology(world_map *wm) {
  size_t i, j;
  world_map_pos xy;
  global_pos anchor;
  gl_pos_t t;
  stratum *s;
  stone_species *st_sp;
  ptrdiff_t hash;

  /*
   * TODO: Fix this!
  efd_node *gen_node;
  efd_node *gen_strata_params_node, *gen_stratum_node;
  efd_node *args_node;
  efd_node *params_node, *stratum_node;

  SSTR(vn_i, "~i", 2);
  SSTR(vn_wm_seed, "~wm_seed", 8);
  SSTR(vn_wm_width, "~wm_width", 9);
  SSTR(vn_wm_height, "~wm_height", 10);
  SSTR(vn_species, "~species", 8);

  SSTR(vn_source, "~source", 6);

  SSTR(s_stratum, "stratum", 7);

  // collect ELFSCRIPT nodes
  gen_node = efdx(ELFSCRIPT_ROOT, GEO_GEN_KEY);
  if (gen_node == NULL) {
    fprintf(
      stderr,
      "ERROR: Required node missing for stone species generation:\n"
    );
    s_fprintln(stderr, GEO_GEN_KEY);
  }
  gen_strata_params_node = efd_lookup_expected(
    gen_node,
    GEO_GEN_KEY_STRATA_PARAMS
  );
  gen_stratum_node = efd_lookup_expected(gen_node, GEO_GEN_KEY_STRATUM);

  printf("    ...generating strata...\n");
  world_region *wr;
  for (i = 0; i < WM_MAX_STRATA_LAYERS * STRATA_COMPLEXITY; ++i) {
    hash = prng(prng(wm->seed + 567*i));

    // Create a stratum and append it to the list of all strata...
    st_sp = generate_stone_species(wm, hash);

    // Arguments to gen_strata_params:
    args_node = create_efd_node(ELFSCRIPT_NT_SCOPE, ELFSCRIPT_ANON_NAME, NULL);
    efd_add_child(args_node, construct_efd_int_node(vn_i, NULL, i));
    efd_add_child(
      args_node,
      construct_efd_int_node(vn_wm_seed, NULL, wm->seed)
    );
    efd_add_child(
      args_node,
      construct_efd_int_node(vn_source, NULL, (efd_int_t) st_sp->source)
    );
    efd_add_child(
      args_node,
      construct_efd_int_node(vn_wm_width, NULL, wm->width)
    );
    efd_add_child(
      args_node,
      construct_efd_int_node(vn_wm_height, NULL, wm->height)
    );
    efd_add_child(
      args_node,
      construct_efd_int_node(vn_species, NULL, (efd_int_t) st_sp->id)
    );

    // Generate parameters for this stratum
    params_node = efd_call_function(gen_strata_params_node, args_node);

    // Generate the stratum:
    stratum_node = efd_call_function(gen_stratum_node, params_node);

    // Unpack it into a stratum_s struct.
    s = (stratum*) efd_lookup_unpacker(s_stratum)(stratum_node);
    cleanup_efd_node(stratum_node);

    l_append_element(wm->all_strata, (void*) s);
    // Render the stratum into the various regions:
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = 0; xy.y < wm->height; ++xy.y) {
        // Compute thickness to determine if this region needs to be aware of
        // this stratum:
        compute_region_anchor(wm, &xy, &anchor);
        t = compute_stratum_height(s, &anchor);
        // If the anchor has material, add this stratum to this region:
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
    if (i % 20 == 0) {
      printf(
        "      ...%zu / %zu strata done...\r",
        i,
        (size_t) (WM_MAX_STRATA_LAYERS * STRATA_COMPLEXITY)
      );
      fflush(stdout);
    }
  }
  // TODO: REMOVE (PROF/DEBUG)
  printf(
    "      ...%zu / %zu strata done...\n",
    (size_t) (WM_MAX_STRATA_LAYERS * STRATA_COMPLEXITY),
    (size_t) (WM_MAX_STRATA_LAYERS * STRATA_COMPLEXITY)
  );
  */
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
