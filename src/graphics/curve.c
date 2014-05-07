// curve.c
// Functions for representing Bezier curves.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include "curve.h"

#include "datatypes/vector.h"

/*************
 * Functions *
 *************/

void curve_foreach(curve *c, float spacing, void (*f)(vector *, vector *)) {
  vector p, dir;
  float inc = spacing / est_curve_length(c);
  float t = 0;
#ifdef DEBUG
  if (spacing <= 0) {
    printf(
      "Error: can't iterate over a curve with zero or negative spacing.\n"
    );
    exit(1);
  }
#endif
  for (t = 0; t <= 1.0; t += inc) {
    point_on_curve(c, t, &p);
    direction_on_curve(c, t, &dir);
    f(&p, &dir);
  }
}

void curve_witheach(
  curve *c,
  float spacing,
  void *arg,
  void (*f)(vector *, vector *, void *)
) {
  vector p, dir;
  float inc = spacing / est_curve_length(c);
  float t = 0;
#ifdef DEBUG
  if (spacing <= 0) {
    printf(
      "Error: can't iterate over a curve with zero or negative spacing.\n"
    );
    exit(1);
  }
#endif
  for (t = 0; t <= 1.0; t += inc) {
    point_on_curve(c, t, &p);
    direction_on_curve(c, t, &dir);
    f(&p, &dir, arg);
  }
}
