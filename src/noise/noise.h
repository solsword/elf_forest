#ifndef NOISE_H
#define NOISE_H

#include <stdint.h>
#include <stddef.h>

// noise.h
// Noise functions (mainly simplex noise).

/**************
 * Structures *
 **************/

// A 2D grid neighborhood holds both integer indexes and floating point
// locations for each of the nine cells surrounding the cell containing the
// point in question.
struct grid_neighborhood_2d_s;
typedef struct grid_neighborhood_2d_s grid_neighborhood_2d;

/*********
 * Flags *
 *********/

// Flags to control the algorithm used by Worley noise:

// Don't use the distance to the nearest point as part of the base result.
#define WORLEY_FLAG_IGNORE_NEAREST 0x00000001
// Subtract the distance to the second-nearest point from the result and invert
// it. If WORLEY_FLAG_IGNORE_NEAREST is given, the result will be the distance
// to the second-nearest point.
#define WORLEY_FLAG_INCLUDE_NEXTBEST 0x00000002
// Disables normalization of the result.
#define WORLEY_FLAG_DONT_NORMALIZE 0x0000004

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

extern float const SURFLET_RADIUS_2D;
extern float const SURFLET_SQ_RADIUS_2D;

extern float const SURFLET_RADIUS_2D;
extern float const SURFLET_SQ_RADIUS_2D;

// The maximum distance in two dimensions that an arbitrary point might be from
// a random point within one of the nine nearest grid cells is the diagonal of
// a grid cell, which for unit cells is just sqrt(2). Because this is achieved
// when the sample point is at the corner of a cell and all four points in
// cells adjacent to that corner happen to be at extreme diagonals from it,
// this maximum distance is valid when searching for up to the 5th-nearest
// point.
extern float const MAX_WORLEY_DISTANCE_2D;
extern float const MAX_SQ_WORLEY_DISTANCE_2D;

/********
 * Data *
 ********/

// The numbers 0-255 shuffled and then copied twice two copies reduces the
// amount of %ing you have to do to keep indices within range.
static ptrdiff_t const HASH_MASK = 0xff;
static ptrdiff_t const HASH_BITS = 8;
#define HASH_OF(x) (((ptrdiff_t) x) & HASH_MASK)
#define UPPER_HASH_OF(x) ((((ptrdiff_t) x) >> 2) & HASH_MASK)
#define MIXED_HASH_OF(x) ( \
  ( \
    ((ptrdiff_t) x) ^ ((ptrdiff_t) x) >> HASH_BITS \
  ) & HASH_MASK \
)
static ptrdiff_t const HASH[512] = {
  248, 244, 209,  63, 108,  81,  67, 202,
  240, 140, 196, 217, 194,  48, 213, 234,
  216,  94, 160,  72, 200, 190, 126,  15,
  218, 233, 181, 171,  50, 242, 230, 147,
   60,  31, 231,  21,  53,  71,  84,  66,
   88, 203, 128, 163, 102, 138, 201, 232,
  250, 106, 123,  38,  86, 132, 238, 100,
  205,  73, 141, 253, 246,  43, 117,  26,
  162, 210, 249,  24, 137, 105, 124, 236,
  113, 239, 222,  55, 251, 214, 165, 254,
  131, 101, 168, 135, 188, 184, 134, 226,
  185,  75, 174,  37,  42, 204, 183, 235,
  107,  61, 161,  54, 179,  44, 170, 122,
   99,  18,   5,  97,  11,  79,  30, 187,
   27, 211,  80, 104,  17,  35, 228,  96,
   45, 153, 212, 112,  47,  56, 110, 114,
  148, 223, 189,  32,   3, 207, 121,  14,
   95,  78, 199, 150, 173, 220, 198,  98,
   76, 225,  16, 221,  46,  58,   0, 157,
  151,  36, 180, 176, 208, 182, 146, 130,
   23,  51,  77, 192, 252, 166,  12,  39,
   41,   2, 247, 215,  93, 175,   9, 197,
    7,  40, 159,  82, 133, 119, 241, 156,
  245, 143,  91,  34, 127,  57, 103, 167,
    4, 255, 243, 136, 195,  64,  87,   1,
  118, 177,   6,  62,  65,  68, 219, 227,
   92,  28,  49, 206,   8, 191, 154, 109,
  169, 129, 158,  19,  59,  29, 111,  25,
  139, 155,  10, 193,  83, 224,  74, 120,
   90,  20, 186, 116,  33, 145,  70,  52,
  229, 144, 149, 178,  22, 115,  89,  85,
  164,  69, 152, 125, 172, 142, 237, 13,

  248, 244, 209,  63, 108,  81,  67, 202,
  240, 140, 196, 217, 194,  48, 213, 234,
  216,  94, 160,  72, 200, 190, 126,  15,
  218, 233, 181, 171,  50, 242, 230, 147,
   60,  31, 231,  21,  53,  71,  84,  66,
   88, 203, 128, 163, 102, 138, 201, 232,
  250, 106, 123,  38,  86, 132, 238, 100,
  205,  73, 141, 253, 246,  43, 117,  26,
  162, 210, 249,  24, 137, 105, 124, 236,
  113, 239, 222,  55, 251, 214, 165, 254,
  131, 101, 168, 135, 188, 184, 134, 226,
  185,  75, 174,  37,  42, 204, 183, 235,
  107,  61, 161,  54, 179,  44, 170, 122,
   99,  18,   5,  97,  11,  79,  30, 187,
   27, 211,  80, 104,  17,  35, 228,  96,
   45, 153, 212, 112,  47,  56, 110, 114,
  148, 223, 189,  32,   3, 207, 121,  14,
   95,  78, 199, 150, 173, 220, 198,  98,
   76, 225,  16, 221,  46,  58,   0, 157,
  151,  36, 180, 176, 208, 182, 146, 130,
   23,  51,  77, 192, 252, 166,  12,  39,
   41,   2, 247, 215,  93, 175,   9, 197,
    7,  40, 159,  82, 133, 119, 241, 156,
  245, 143,  91,  34, 127,  57, 103, 167,
    4, 255, 243, 136, 195,  64,  87,   1,
  118, 177,   6,  62,  65,  68, 219, 227,
   92,  28,  49, 206,   8, 191, 154, 109,
  169, 129, 158,  19,  59,  29, 111,  25,
  139, 155,  10, 193,  83, 224,  74, 120,
   90,  20, 186, 116,  33, 145,  70,  52,
  229, 144, 149, 178,  22, 115,  89,  85,
  164,  69, 152, 125, 172, 142, 237,  13
};

/*************************
 * Structure Definitions *
 *************************/

struct grid_neighborhood_2d_s {
  ptrdiff_t i, j; // index of the center cell
  float x[9]; // x-coordinates of the 9 grid points in the neighborhood
  float y[9]; // y-coordinates of the same
};

/********************
 * Inline Functions *
 ********************/

// hash functions using the hash table:
static inline ptrdiff_t hash_2d(ptrdiff_t i, ptrdiff_t j) {
  return HASH[(i & HASH_MASK) + HASH[(j & HASH_MASK)]];
}
static inline ptrdiff_t hash_3d(ptrdiff_t i, ptrdiff_t j, ptrdiff_t k) {
  return HASH[(i & HASH_MASK) + HASH[(j & HASH_MASK) + HASH[k & HASH_MASK]]];
}

/*************
 * Functions *
 *************/

// 2D simplex noise:
float sxnoise_2d(float x, float y);

// 3D simplex noise:
float sxnoise_3d(float x, float y, float z);

// 2D Worley noise:
float wrnoise_2d(
  float x, float y,
  ptrdiff_t wrapx, ptrdiff_t wrapy,
  uint32_t flags
);

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

/*******************
 * Remix Functions *
 *******************/

static inline float wrnoise_2d_default(float x, float y) {
  return wrnoise_2d(x, y, 0, 0, 0);
}
#endif // ifndef NOISE_H
