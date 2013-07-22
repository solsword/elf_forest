#ifndef CONV_H
#define CONV_H

// conv.h
// Conversion utilities.

#include <math.h>

/*************
 * Constants *
 *************/

// radians <-> degrees
static const float R2D = 180*M_1_PI;
static const float D2R = M_PI/180;

/********************
 * Inline Functions *
 ********************/

// Faster floor function (mostly 'cause we're ignoring IEEE error stuff):
static inline int fastfloor(float x) {
  int ix = (int) x;
  return ix - (ix > x);
}

#endif // ifndef CONV_H
