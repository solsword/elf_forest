#ifndef NOISE_H
#define NOISE_H

#include <stdint.h>

// noise.h
// Noise functions (mainly simplex noise).

/*********
 * Flags *
 *********/

// These flags control the noise generation when given to the table form of
// the fractal sum noise generators (see fractal_sxnoise_*d_table).

// Perlin's absolute value filter: use 1 - abs(noise) instead of noise.
#define NOISE_FILTER_ABS 0x00000001
// Square the noise. Note that Musgrave used abs + square.
#define NOISE_FILTER_SQUARE 0x00000002
// Use the previous octave of noise to distort the coordinates at this octave.
#define NOISE_FILTER_DISTORT 0x00000004
// Use the noise-so-far to distort coordinates at this octave (without dividing
// by the power-so-far).
#define NOISE_FILTER_DISTORT_FULL 0x00000008

// Note that unlike the noise filters, the blend methods are mutually
// exclusive. If more than one is set, the lowest-numbered will take
// precedence.

// Replaces all prior octaves of noise with this octave. Useful with filters
// like NOISE_FILTER_DISTORT which use information from the noise so far.
#define BLEND_METHOD_REPLACE 0x00000100
// Uses the normal blending method (additive blending) but gives a weight of
// 1.0 to the noise-so-far and blends that directly with the current octave,
// normalizing the noise-so-far so that the total power ends up being 1 + the
// amplitude of the current octave.
#define BLEND_METHOD_RENORM 0x00000200
// Multiply this octave with the total instead of adding it. If this flag is
// given the amplitude value will be treated as an exponent.
#define BLEND_METHOD_MULTIPLY 0x00000400
// Use the previous octave to scale this one, effectively a hybrid between
// multiplicative and additive combination. Amplitude is also multiplied in.
#define BLEND_METHOD_HYBRID_LAST 0x00000800
// Like HYBRID_LAST, but use the entire noise sum to scale our contribution
// rather than just the last octave's value. Amplitude is also multiplied in.
#define BLEND_METHOD_HYBRID_FULL 0x00001000
// Like HYBRID_LAST, but use a scaled version of the previous result, so that
// -1 -- 1 is mapped to 0 -- 1 to compute the amplitude influence.
#define BLEND_METHOD_HYBRID_LAST_SCALED 0x00002000
// Like HYBRID_LAST_SCALED, but uses the full noise-so-far.
#define BLEND_METHOD_HYBRID_FULL_SCALED 0x00004000

/************
 * Examples *
 ************/

// These are example tables to pass to the fractal_sxnoise_3d_table functions.
// Make sure to pass the right number of octaves as well. Note that all of
// these are designed to work with the 3D noise function since they use 3
// offset values per octave. Generally, 2D noise can be gotten by just slicing
// the 3D noise function.

// Standard 4-octave noise (no flags needed):
extern float BASE__NORM__NORM__NORM[];
// Two octaves, the second is multiplicative:
extern    float BASE__MULT[];
extern uint32_t BASE__MULT_F[];
// Two octaves, the second is distorted by the first:
extern    float BASE__DIST[];
extern uint32_t BASE__DIST_F[];
// Four octaves, the last three use absolute value:
extern    float BASE__ABS__ABS__ABS[];
extern uint32_t BASE__ABS__ABS__ABS_F[];
// Four octaves, the last three use hybrid squared absolute value:
extern    float BASE__HABS__HABS__HABS[];
extern uint32_t BASE__HABS__HABS__HABS_F[];
// Four octaves, the last three use full hybrid squared absolute value:
extern    float BASE__HFABS__HFABS__HFABS[];
extern uint32_t BASE__HFABS__HFABS__HFABS_F[];
// Example presets for terrain:
extern    float EX_TERRAIN[];
extern uint32_t EX_TERRAIN_F[];

/***********
 * Globals *
 ***********/

// The surflet radii used for simplex noise can be useful to know the structure
// of the noise. For example, if you're using slices of 3D noise as 2D noise
// with different "seeds", you need to space your slices at least
// 2*SURFLET_RADIUS_3D apart to guarantee that they're not correlated. The
// squared values are mainly provided because they're used in the algorithm for
// a squared distance comparison.

extern const float SURFLET_RADIUS_2D;
extern const float SURFLET_SQ_RADIUS_2D;

extern const float SURFLET_RADIUS_2D;
extern const float SURFLET_SQ_RADIUS_2D;

/*************
 * Functions *
 *************/

// 2D simplex noise:
float sxnoise_2d(float x, float y);

// 3D simplex noise:
float sxnoise_3d(float x, float y, float z);

// Multiple octaves of 2D simplex noise combined using the given number of
// octaves, frequency ratio, persistence, and offset values.
float fractal_sxnoise_2d(
  float x, float y,
  int octaves, 
  float frequency_ratio,
  float persistence,
  float offset_x, float offset_y
);

// Multiple octaves of 3D simplex noise combined using the given number of
// octaves, frequency ratio, persistence, and offset values.
float fractal_sxnoise_3d(
  float x, float y, float z,
  int octaves, 
  float frequency_ratio,
  float persistence,
  float offset_x, float offset_y, float offset_z
);

// The table versions of fractal noise take the frequency, amplitude, and
// offset values for each octave from a table rather than computing them
// according to a fixed formula. The table can also include flags to control
// how summation is performed. Obviously the number of octaves passed should
// agree with the size of the table, otherwise bad things will result, but NULL
// may be passed as the flags argument if the use of flags is not desired. See
// the Flags section above for details of flag options available.
float fractal_sxnoise_2d_table(
  float x, float y,
  int octaves, 
  float *table,
  uint32_t *flags
);

float fractal_sxnoise_3d_table(
  float x, float y, float z,
  int octaves, 
  float *table,
  uint32_t *flags
);

#endif // ifndef NOISE_H
