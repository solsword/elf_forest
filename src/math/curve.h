#ifndef CURVE_H
#define CURVE_H

// curve.h
// Functions for representing cubic Bezier curves.

#include "datatypes/vector.h"

/*************
 * Constants *
 *************/

static int const DEFAULT_CURVE_RESOLUTION = 0.01;

/**************
 * Structures *
 **************/

struct curve_s;
typedef struct curve_s curve;

/*************************
 * Structure Definitions *
 *************************/

struct curve_s {
  vector from;
  vector go_towards;
  vector to;
  vector come_from;
};

/********************
 * Inline Functions *
 ********************/

// Computes a point on the given curve which is somewhere along its length as
// given by the parameter t (0 <= t <= 1).
static inline void point_on_curve(curve *c, float t, vector *result) {
  float ti = 1 - t;
  float ti3 = ti*ti*ti;
  float ttiti = t*ti*ti;
  float ttti = t*t*ti;
  float t3 = t*t*t;
  result->x =
    ti3*c->from.x +
    3*ttiti*c->go_towards.x +
    3*ttti*c->come_from.x +
    t3*c->to.x;
  result->y =
    ti3*c->from.y +
    3*ttiti*c->go_towards.y +
    3*ttti*c->come_from.y +
    t3*c->to.y;
  result->z =
    ti3*c->from.z +
    3*ttiti*c->go_towards.z +
    3*ttti*c->come_from.z +
    t3*c->to.z;
}

// Works like point_on_curve, but computes the forward direction of the curve
// as a vector instead. (Used equation for first derivative from Wikipedia).
static inline void direction_on_curve(curve *c, float t, vector *result) {
  float ti = 1 - t;
  float ti2 = ti*ti;
  float tti = t*ti;
  float t2 = t*t;
  result->x =
    3*ti2*(c->go_towards.x - c->from.x) +
    6*tti*(c->come_from.x - c->go_towards.x) +
    3*t2*(c->to.x - c->come_from.x);
  result->y =
    3*ti2*(c->go_towards.y - c->from.y) +
    6*tti*(c->come_from.y - c->go_towards.y) +
    3*t2*(c->to.y - c->come_from.y);
  result->z =
    3*ti2*(c->go_towards.z - c->from.z) +
    6*tti*(c->come_from.z - c->go_towards.z) +
    3*t2*(c->to.z - c->come_from.z);
}

// Estimates the length of the given curve as the average of a linear
// approximation and a polyline approximation using the control points. Note
// that a better approximation would use the differance between the two
// approximations averaged here to estimate the error and subdivide the curve
// to get a guaranteed maximum error, but we don't do that here.
static inline float est_curve_length(curve *c) {
  vector line;
  vector p1;
  vector p2;
  vector p3;
  float lest = 0, pest = 0;

  // the linear estimate:
  vcopy_as(&line, &(c->to));
  vsub_from(&line, &(c->from));

  lest = vmag(&line);

  // the polyline estimate:
  vcopy_as(&p3, &(c->to));
  vsub_from(&p3, &(c->come_from));
  vcopy_as(&p2, &(c->come_from));
  vsub_from(&p2, &(c->go_towards));
  vcopy_as(&p1, &(c->go_towards));
  vsub_from(&p1, &(c->from));

  pest = vmag(&p1) + vmag(&p2) + vmag(&p3);

  return 0.5 * lest + 0.5 * pest;
}

/*************
 * Functions *
 *************/

// Calls the given function at each step along the curve, where steps are
// approximately the given distance apart. The function gets a parameterization
// variable between 0.0 and 1.0, a position vector, and a direction vector as
// arguments.
void curve_foreach(curve *c, float spacing, void (*f)(float, vector*, vector*));

// Works like curve_foreach, but passes an extra argument to each invocation of
// the iteration function.
void curve_witheach(
  curve *c,
  float spacing,
  void *arg,
  void (*f)(float, vector *, vector *, void *)
);

#endif // ifndef CURVE_H
