#ifndef MANIFOLD_H
#define MANIFOLD_H

// manifold.h
// Math for surfaces.

#include <math.h>

#include "noise/noise.h"

/************************
 * Types and Structures *
 ************************/

struct manifold_point_s;
typedef struct manifold_point_s manifold_point;

/*************
 * Constants *
 *************/

/***********
 * Globals *
 ***********/

/*************************
 * Structure Definitions *
 *************************/

struct manifold_point_s {
  float z;
  float dx, dy;
};

/********************
 * Inline Functions *
 ********************/

// Basic manifold properties:
// --------------------------

// Returns the uphill direction (in radians) at the given manifold point.
static inline float mani_uphill(manifold_point point) {
  return atan2(point.dy, point.dx);
}

// Returns the downhill direction (in radians) at the given manifold point.
static inline float mani_downhill(manifold_point point) {
  return atan2(-point.dy, -point.dx);
}

// Returns the greatest slope value in any direction at the given manifold
// point.
static inline float mani_slope(manifold_point point) {
  return sqrtf(point.dx * point.dx + point.dy * point.dy);
}

// Returns the contour direction (arbitrarily for counter-clockwise contours)
// at the given manifold point. This is just the uphill direction minus pi/2.
static inline float mani_contour(manifold_point point) {
  return mani_uphill(point) - M_PI/2.0;
}

// Manipulation functions for changing manifolds:
// ----------------------------------------------

// Copies the source manifold to the target manifold.
static inline void mani_copy(manifold_point target, manifold_point source) {
  target.z = source.z;
  target.dx = source.dx;
  target.dy = source.dy;
}

// Adds a constant offset to the given manifold.
static inline void mani_offset_const(manifold_point point, float offset) {
  point.z += offset;
}

// Scales the given manifold by a constant.
static inline void mani_scale_const(manifold_point point, float factor) {
  point.z *= factor;
  point.dx *= factor;
  point.dy *= factor;
}

// Raises the manifold to the given constant exponent.
static inline void mani_pow_const(manifold_point point, float exponent) {
  point.dx = exponent * pow(point.z, exponent - 1) * point.dx;
  point.dy = exponent * pow(point.z, exponent - 1) * point.dy;
  point.z = pow(point.z, exponent);
}

// Adds two manifolds, storing the result in the first.
static inline void mani_add(
  manifold_point target,
  manifold_point addend
) {
  target.dx += addend.dx;
  target.dy += addend.dy;
  target.z += addend.z;
}

// Multiplies two manifolds, storing the result in the first.
static inline void mani_multiply(
  manifold_point target,
  manifold_point multiplicand
) {
  float mz, mdx, mdy;
  // temp variables in case target and multiplicand are the same
  mz = multiplicand.z;
  mdx = multiplicand.dx;
  mdy = multiplicand.dy;
  target.dx = target.z * mdx + mz * target.dx;
  target.dy = target.z * mdy + mz * target.dy;
  target.z *= mz;
}

// Takes two manifold points: one representing a function value computed using
// f(g(x)), but whose derivatives are just f'(g(x)), and another representing
// g(x) accurately. Edits the f(g(x)) manifold to correctly represent the
// derivatives of the composition.
static inline void mani_compose(manifold_point outer, manifold_point inner) {
  outer.dx *= inner.dx;
  outer.dy *= inner.dy;
}

// Works like mani_compose, but just takes floating point dx/dy values for the
// inner function instead of a whole manifold point.
static inline void mani_compose_simple(
  manifold_point outer,
  float dx, float dy
) {
  outer.dx *= dx;
  outer.dy *= dy;
}

// Smooths a manifold using the smooth function from noise.h.
static inline void mani_smooth(
  manifold_point point,
  float strength,
  float center
) {
  point.z = smooth(point.z, strength, center);
  smooth_grad(point.z, strength, center, &(point.dx), &(point.dy));
}

// Helper functions for putting noise values into manifold points:
// ---------------------------------------------------------------

// simplex noise
static inline void mani_get_sxnoise_2d(
  manifold_point point,
  float x, float y,
  ptrdiff_t salt
) {
  point.z = sxnoise_grad_2d(
    x, y,
    salt,
    &(point.dx), &(point.dy)
  );
}

// worley noise
static inline void mani_get_wrnoise_2d(
  manifold_point point,
  float x, float y,
  ptrdiff_t salt,
  float wrapx, float wrapy,
  uint32_t flags
) {
  point.z = wrnoise_2d_fancy(
    x, y,
    salt,
    wrapx, wrapy,
    &(point.dx), &(point.dy),
    flags
  );
}

/******************************
 * Constructors & Destructors *
 ******************************/

/*************
 * Functions *
 *************/

#endif // ifndef MANIFOLD_H
