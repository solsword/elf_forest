#ifndef FUNCTIONS_H
#define FUNCTIONS_H

// functions.h
// A toolbox of mathematical functions.

#include <stdint.h>

/*********
 * Enums *
 *********/

enum map_function_e {
  MFN_LINEAR, // x' = x
  MFN_QUADRATIC, // x' = x*x
  MFN_CUBIC, // x' = x*x*x
  MFN_SQRT, // x' = sqrt(x)
  MFN_EXPONENTIAL, // x' = (exp(x) / exp(1)) - 1
  MFN_SIGMOID, // exponential sigmoid shape
  MFN_HILL, // stacked quadratic gentler sigmoid
  MFN_TERRACE, // two quadratics stacked into a terrace
  MFN_SPREAD_UP, // quadratic; pushes things a bit higher
  MFN_SPREAD_DOWN, // quadratic; pushes things a bit lower
};
typedef enum map_function_e map_function;

/********************
 * Inline Functions *
 ********************/

// Maps the given value through the given function.
static float fmap(float x, map_function f) {
  switch (f) {
    case MFN_LINEAR:
      return x;
    case MFN_QUADRATIC:
      return x*x;
    case MFN_CUBIC:
      return x*x*x;
    case MFN_SQRT:
      return sqrtf(x);
    case MFN_EXPONENTIAL:
      return (exp(x) - 1)/exp(1);
    case MFN_SIGMOID:
      return 1.0 / (1.0 + exp(6.0-x*12.0));
    case MFN_HILL:
      if (x < 0.5) {
        return x*x*2;
      } else {
        float omx = 1 - x;
        return 1 - omx*omx*2;
      }
    case MFN_TERRACE:
      if (x < 0.5) {
        return x*x*2;
      } else {
        float xmh = x - 0.5;
        return 0.5 + xmh*xmh*2;
      }
    case MFN_SPREAD_UP:
      return x*((1 - x)*1.6 + x);
    case MFN_SPREAD_DOWN:
      return (1 - x)*(-0.6*x) + x;
  }
  // "bad function"
  return -1;
}

// Maps the given value through the given function, flipping negative values
// pre-mapping and flopping return values if the input was negative.
static inline float fmap_flipflop(float x, map_function f) {
  float sign = 1;
  if (x < 0) {
    sign = -1;
    x = -x;
  }
  float result = fmap(x, f);
  return sign*result;
}

/*************
 * Functions *
 *************/

#endif // ifndef FUNCTIONS_H
