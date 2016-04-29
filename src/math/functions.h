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
  MFN_EXPONENTIAL, // x' = (exp(x) - 1) / (exp(1) - 1)
  MFN_EX_EXPONENTIAL, // x' = (exp(x*5) - 1) / (exp(5) - 1)
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
static inline float fmap(float x, map_function f) {
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
      return (exp(x) - 1)/(exp(1) - 1);
    case MFN_EX_EXPONENTIAL:
      return (exp(x*5) - 1)/(exp(5) - 1);
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

// Miscellaneous useful functions:

// A logarithmic sigmoid with slope ~0.5 at the center. Doesn't exactly hit 0/1
// at the ends.
static inline float sigmoid(float t, float start, float end) {
  if (t < start) {
    return 0;
  } else if (t > end) {
    return 1;
  } else {
    float tprime = -1.0 + 2.0 * (t - start) / (end - start);
    return 1.0 / (1 + exp(-tprime*5));
  }
}

// An exponential distribution over [0, 1] with values in [0, 1]. Power should
// be strictly greater than zero; the equation approaches a line as power goes
// to zero, and becomes more eccentric as power goes to infinity.
static inline float expdist(float x, float power) {
  return (exp(x * power) - 1) / (exp(power) - 1);
}

// A logarithmic distribution over [0, 1] with values in [0, 1]. Offset should
// be strictly greater than zero; the equation approaches a line as offset goes
// to infinity, and becomes more eccentric as offset goes to zero.
static inline float logdist(float x, float offset) {
  return (log(x + offset) - log(offset)) / (log(1 + offset) - log(offset));
}

// A sigmoid on [0,1] built out of two parabolas that hits 0/1 at the ends and
// which has slope asymptotic to 0 at the ends. The inflection point is
// adjustable (it's y-value is always equal to its x-value).
static inline float strict_sigmoid(float x, float inflect) {
  if (x < inflect) {
    // TODO: is the slope pointing in the right direction?
    return (inflect / 0.25) * pow(x/(2*inflect), 2);
  } else {
    // TODO: Is this calculation correct?
    float omi = (1 - inflect);
    return 1 - (omi/0.25)*pow((1 - x)/(2*omi), 2);
  }
}

// Returns the analytical derivative of a strict_sigmoid with the given
// inflection point.
static inline float strict_sigmoid_slope(float x, float inflect) {
  if (x < inflect) {
    return (2 * 0.3 / (0.25 * (4*inflect*inflect))) * x;
  } else {
    float omi = (1 - inflect);
    return -(omi/(0.25 * (4*omi*omi)))*(-2.0 + 2.0*x);
  }
}

/*************
 * Functions *
 *************/

#endif // ifndef FUNCTIONS_H
