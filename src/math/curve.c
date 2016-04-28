// curve.c
// Functions for representing Bezier curves.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include <math.h>

#include "curve.h"

#include "datatypes/vector.h"

/*************
 * Functions *
 *************/

void curve_foreach(curve *c, float spacing, void (*f)(float, vector*,vector*)) {
  vector p, dir;
  float inc = 0;
  inc = spacing / est_curve_length(c);
  float t = 0;
#ifdef DEBUG
  if (spacing <= 0) {
    printf(
      "Error: can't iterate over a curve with zero or negative spacing.\n"
    );
    exit(EXIT_FAILURE);
  }
#endif
  for (t = 0; t <= 1.0; t += inc) {
    point_on_curve(c, t, &p);
    direction_on_curve(c, t, &dir);
    f(t, &p, &dir);
  }
}

void curve_witheach(
  curve *c,
  float spacing,
  void *arg,
  void (*f)(float, vector *, vector *, void *)
) {
  vector p, dir;
  float inc = 0;
  float t = 0;
#ifdef DEBUG
  // check for bad spacing values
  if (spacing <= 0) {
    fprintf(
      stderr,
      "Error: can't iterate over a curve with zero or negative spacing.\n"
    );
    exit(EXIT_FAILURE);
  }
#endif
  // compute our increment
  inc = spacing / est_curve_length(c);
  // check for bad increment values
  if (inc <= 0 || !isfinite(inc)) {
    inc = 1.1; // just call the function once: at the curve starting point
  }
  for (t = 0; t <= 1.0; t += inc) {
    point_on_curve(c, t, &p);
    direction_on_curve(c, t, &dir);
    f(t, &p, &dir, arg);
  }
}
