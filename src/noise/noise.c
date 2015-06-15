// noise.c
// Noise functions (mainly simplex noise).

#include <math.h>
#include <stdlib.h>

#include "noise.h"

// DEBUG:
#include <stdio.h>

/************
 * Examples *
 ************/

// There are 5 parameters/octave.

float BASE__NORM__NORM__NORM[20] = {
  // scale    amplitude    offset x    offset y    offset z
      1.0,       1.0,         0.0,        0.0,        0.0,
      2.0,       0.5,         0.0,        0.0,        0.0,
      4.0,      0.25,         0.0,        0.0,        0.0,
      8.0,     0.125,         0.0,        0.0,        0.0,
};

float BASE__MULT[10] = {
  // scale    amplitude    offset x    offset y    offset z
      1.0,       1.0,         0.0,        0.0,        0.0,
      2.0,       0.9,         0.0,        0.0,        0.0,
};
uint32_t BASE__MULT_F[2] = {
  0,
  BLEND_METHOD_MULTIPLY,
};

float BASE__DIST[15] = {
  // scale    amplitude    offset x    offset y    offset z
      1.0,       0.1,         0.0,        0.0,        0.0,
      1.5,       0.2,         0.0,        0.0,        0.0,
      2.0,       1.0,         0.0,        0.0,        0.0,
};
uint32_t BASE__DIST_F[3] = {
  0,
  NOISE_FILTER_DISTORT,
  NOISE_FILTER_DISTORT_FULL | BLEND_METHOD_REPLACE,
};

float BASE__ABS__ABS__ABS[20] = {
  // scale    amplitude    offset x    offset y    offset z
      1.0,       1.0,         0.0,        0.0,        0.0,
      2.1,       0.6,         0.0,        0.0,        0.0,
      4.3,       0.3,         0.0,        0.0,        0.0,
      8.5,       0.1,         0.0,        0.0,        0.0,
};
uint32_t BASE__ABS__ABS__ABS_F[4] = {
  0,
  NOISE_FILTER_ABS,
  NOISE_FILTER_ABS,
  NOISE_FILTER_ABS,
};

float BASE__HABS__HABS__HABS[20] = {
  // scale    amplitude    offset x    offset y    offset z
      1.0,       1.0,         0.0,        0.0,        0.0,
      2.1,       0.6,         0.0,        0.0,        0.0,
      4.3,       0.3,         0.0,        0.0,        0.0,
      8.5,       0.1,         0.0,        0.0,        0.0,
};
uint32_t BASE__HABS__HABS__HABS_F[4] = {
  0,
  NOISE_FILTER_ABS | NOISE_FILTER_SQUARE | BLEND_METHOD_HYBRID_LAST,
  NOISE_FILTER_ABS | NOISE_FILTER_SQUARE | BLEND_METHOD_HYBRID_LAST,
  NOISE_FILTER_ABS | NOISE_FILTER_SQUARE | BLEND_METHOD_HYBRID_LAST,
};

float BASE__HFABS__HFABS__HFABS[20] = {
  // scale    amplitude    offset x    offset y    offset z
      1.0,       1.0,         0.0,        0.0,        0.0,
      2.1,       0.5,         0.0,        0.0,        0.0,
      4.3,      0.25,         0.0,        0.0,        0.0,
      8.5,     0.125,         0.0,        0.0,        0.0,
};
uint32_t BASE__HFABS__HFABS__HFABS_F[4] = {
  0,
  NOISE_FILTER_ABS | NOISE_FILTER_SQUARE | BLEND_METHOD_HYBRID_FULL,
  NOISE_FILTER_ABS | NOISE_FILTER_SQUARE | BLEND_METHOD_HYBRID_FULL,
  NOISE_FILTER_ABS | NOISE_FILTER_SQUARE | BLEND_METHOD_HYBRID_FULL,
};

float EX_TERRAIN[45] = {
  // scale    amplitude    offset x    offset y    offset z
  // Normal octaves:
      1.0,       1.3,         0.0,        0.0,        0.0,
      4.4,       0.1,         0.0,        0.0,        0.0,
      4.7,       0.1,         0.0,        0.0,        5.5,
      5.2,       0.1,         0.0,        0.0,        7.7,
      2.1,       0.6,         0.0,        0.0,        0.0,
      8.5,       0.1,         0.0,        0.0,        0.0,
  // Some low-frequency layers that we'll use 50/50 with the octaves above:
     0.17,       1.0,         0.0,        0.0,        3.3,
     0.35,       0.6,         0.0,        0.0,        3.3,
     0.93,       0.4,         0.0,        0.0,        3.3,
};
uint32_t EX_TERRAIN_F[9] = {
  0,
  NOISE_FILTER_ABS | NOISE_FILTER_SQUARE | BLEND_METHOD_HYBRID_FULL_SCALED,
  NOISE_FILTER_ABS | NOISE_FILTER_SQUARE | BLEND_METHOD_HYBRID_FULL_SCALED,
  NOISE_FILTER_ABS | NOISE_FILTER_SQUARE | BLEND_METHOD_HYBRID_FULL_SCALED,
  0,
  0,
  BLEND_METHOD_RENORM,
  0,
  0,
};

/********
 * Data *
 ********/

// A circle with radius r=25 has 20 lattice points on it's circumference, and
// they're fairly evenly distributed. We don't want to just truncate the 3d
// gradients 'cause the truncated versions have some overlap and don't maintain
// the equal-length property. To make the index wrapping a mask instead of a
// mod, we'll repeat the 20 vectors three times and add 4 by duplicating a
// symmetric set. We can exploit this duplication to add some magnitude
// variation to our vectors (which effectively adds a free lower-frequency
// component to the noise).
static float const GRADIENTS_2D[128] = {
// Three 20-entry columns -> 60 1x2 vectors (120 floats):
   25/25.0,   0/25.0,       20/25.0,   0/25.0,       16/25.0,   0/25.0,
   24/25.0,   7/25.0,       19/25.0,   5/25.0,       15/25.0,   4/25.0,
   20/25.0,  15/25.0,       16/25.0,  12/25.0,       12/25.0,  10/25.0,
   15/25.0,  20/25.0,       12/25.0,  16/25.0,       10/25.0,  12/25.0,
    7/25.0,  24/25.0,        5/25.0,  19/25.0,        4/25.0,  15/25.0,
    0/25.0,  25/25.0,        0/25.0,  20/25.0,        0/25.0,  16/25.0,
   -7/25.0,  24/25.0,       -5/25.0,  19/25.0,       -4/25.0,  15/25.0,
  -15/25.0,  20/25.0,      -12/25.0,  16/25.0,      -10/25.0,  12/25.0,
  -20/25.0,  15/25.0,      -16/25.0,  12/25.0,      -12/25.0,  10/25.0,
  -24/25.0,   7/25.0,      -19/25.0,   5/25.0,      -15/25.0,   4/25.0,
  -25/25.0,   0/25.0,      -20/25.0,   0/25.0,      -16/25.0,   0/25.0,
  -24/25.0,  -7/25.0,      -19/25.0,  -5/25.0,      -15/25.0,  -4/25.0,
  -20/25.0, -15/25.0,      -16/25.0, -12/25.0,      -12/25.0, -10/25.0,
  -15/25.0, -20/25.0,      -12/25.0, -16/25.0,      -10/25.0, -12/25.0,
   -7/25.0, -24/25.0,       -5/25.0, -19/25.0,       -4/25.0, -15/25.0,
   -0/25.0, -25/25.0,        0/25.0, -20/25.0,        0/25.0, -16/25.0,
    7/25.0, -24/25.0,        5/25.0, -19/25.0,        4/25.0, -15/25.0,
   15/25.0, -20/25.0,       12/25.0, -16/25.0,       10/25.0, -12/25.0,
   20/25.0, -15/25.0,       16/25.0, -12/25.0,       12/25.0, -10/25.0,
   24/25.0,  -7/25.0,       19/25.0,  -5/25.0,       15/25.0,  -4/25.0,
// Four extra 1x2 vectors makes 64 vectors (128 floats):
   20/25.0,  15/25.0,
  -20/25.0,  15/25.0,
  -20/25.0, -15/25.0,
   20/25.0, -15/25.0
};

// Perlin's suggestion to use the midpoints of the edges of a 2x2x2 cube
// centered at the origin is pretty smart: we get to compute dot products
// without multiplication, and the 12 pretty-well-distributed gradients all
// have equal lengths. We'll waste a little space here to turn a multiply (*3)
// into a shift (<< 2). We'll waste more space to turn a mod (%12) into a mask
// (& 0x3f). Unlike the 2D case, we won't worry about magnitude variation, as
// that would force us to compute a real dot product.
static float const GRLEN_3D = M_SQRT2;
static float const GRDIV_3D = 1.0/M_SQRT2;
static ptrdiff_t const GRADIENTS_3D[256] = {
// 12 1x3 vectors plus an extra padding column of zeroes to align the vectors.
// We repeat this 5 times to make 60 4-int entries, and then duplicate 4
// semi-symmetric entries to make 64 (this adds a slight bias since we'd need
// to duplicate 6 to get a full symmetric set).
// 12*4*5 = 240 ints:
   1,  0,  1,   0,
   1,  1,  0,   0,
   1,  0, -1,   0,
   1, -1,  0,   0,
  -1,  0,  1,   0,
  -1,  1,  0,   0,
  -1,  0, -1,   0,
  -1, -1,  0,   0,
   0,  1,  1,   0,
   0,  1, -1,   0,
   0, -1,  1,   0,
   0, -1, -1,   0,

   1,  0,  1,   0,
   1,  1,  0,   0,
   1,  0, -1,   0,
   1, -1,  0,   0,
  -1,  0,  1,   0,
  -1,  1,  0,   0,
  -1,  0, -1,   0,
  -1, -1,  0,   0,
   0,  1,  1,   0,
   0,  1, -1,   0,
   0, -1,  1,   0,
   0, -1, -1,   0,

   1,  0,  1,   0,
   1,  1,  0,   0,
   1,  0, -1,   0,
   1, -1,  0,   0,
  -1,  0,  1,   0,
  -1,  1,  0,   0,
  -1,  0, -1,   0,
  -1, -1,  0,   0,
   0,  1,  1,   0,
   0,  1, -1,   0,
   0, -1,  1,   0,
   0, -1, -1,   0,

   1,  0,  1,   0,
   1,  1,  0,   0,
   1,  0, -1,   0,
   1, -1,  0,   0,
  -1,  0,  1,   0,
  -1,  1,  0,   0,
  -1,  0, -1,   0,
  -1, -1,  0,   0,
   0,  1,  1,   0,
   0,  1, -1,   0,
   0, -1,  1,   0,
   0, -1, -1,   0,

   1,  0,  1,   0,
   1,  1,  0,   0,
   1,  0, -1,   0,
   1, -1,  0,   0,
  -1,  0,  1,   0,
  -1,  1,  0,   0,
  -1,  0, -1,   0,
  -1, -1,  0,   0,
   0,  1,  1,   0,
   0,  1, -1,   0,
   0, -1,  1,   0,
   0, -1, -1,   0,

// 4*4 = 16 ints (the semi-symmetric duplicate entries) for a total of 256:
   1,  0,  1,   0,
   1,  1,  0,   0,
  -1,  0, -1,   0,
  -1, -1,  0,   0
};

/*************
 * Constants *
 *************/

#define RT3 1.7320508075688772
#define RT3__TWO (RT3 / 2.0)
#define TWO__RT3 (2.0 / RT3)

// sqrt(3)/2 squared is 3/4:
float const SURFLET_RADIUS_2D = RT3__TWO;
float const SURFLET_SQ_RADIUS_2D = 0.75;

// The 3D simplices aren't quite regular (regular tetrahedrons don't tile
// properly), so this value is a bit of a fudge. A regular unit tetrahedron
// would have height 0.81, but we'll assume that our distorted tetrahedra will
// only have maximum allowable radius ~.75. Squaring that will yield 0.5625.
float const SURFLET_RADIUS_3D = 0.75;
float const SURFLET_SQ_RADIUS_3D = 0.5625;

// At worst the nearest point will be diagonally across a grid cell.
float const MAX_WORLEY_DISTANCE_2D = M_SQRT2;
float const MAX_SQ_WORLEY_DISTANCE_2D = 2.0;

// These scaling values are chosen based on trial-and-error.
// Because of this it might be possible for a value outside [-1,1] to be
// generated.
static float SCALE_2D = 14.7; // Mostly falls within [-0.9,0.9]
static float SCALE_3D = 7.0; // Mostly falls within [-0.9,0.9]
// Note that the 2D noise seems to have a slight positive bias (this is a
// bug?) with an average of ~50.5 over 65536 samples normalized to integers on
// [0,99] (expected average is 49.5).
// Note also that the noise min/max values over a large area will contract
// noticeably when the sampling grid approaches the size of the simplices.
// Ideally use at least 4 samples/simplex when values from the full range are
// desired.


// To transform a simplex grid into a square grid and vice versa, we can squash
// the square grid along the x=y diagonal. Consequently, the transformation
// equation has the following form:
//
//   x' = x + (x + y) * C
//   y' = y + (x + y) * C
//
// We know that the points [0, 0], [0, 1], and [1, 1] should be corners of a
// simplex after the transform, so the distances (and by extension the square
// of the distances) from [0, 0] to both [0, 1] and [1, 1] should be the same
// after the transformation. We can thus use the following system of equations
// to solve for C:
//
//   x01 = 0 + (0 + 1) * C
//   y01 = 1 + (0 + 1) * C
//   d2 = (x01 - 0)^2 + (y01 - 0)^2
//
//   x11 = 1 + (1 + 1) * C
//   y11 = 1 + (1 + 1) * C
//   d2 = (x11 - 0)^2 + (y11  - 0)^2
//
// -------------------------------------------------- (simplify)
//
//   x01 = C
//   y01 = 1 + C
//   d2 = x01^2 + y01^2
//
//   x11 = 1 + 2*C
//   y11 = 1 + 2*C
//   d2 = x11^2 + y11^2
//
// -------------------------------------------------- (d2 == d2)
//
//   x01^2 + y01^2 = x11^2 + y11^2
//
// -------------------------------------------------- (substitute values)
//
//   C^2 + (1 + C)^2 = (1 + 2*C)^2 + (1 + 2*C)^2
//
// -------------------------------------------------- (simplify terms)
//
//   2*C^2 + 2*C + 1 = 8*C^2 + 8*C + 2
//
// -------------------------------------------------- (subtract lhs from rhs)
//
//   6*C^2 + 6*C + 1 = 0
//
// -------------------------------------------------- (quadrtic formula)
//
//   C = (-b +/- sqrt(b^2 - 4*a*c)) / 2*a
//     a = 6
//     b = 6
//     c = 1
//
// -------------------------------------------------- (fill out a/b/c)
//
//   C = (-6 +/- sqrt(6^2 - 4*6*1)) / 2*6
//
// -------------------------------------------------- (simplify)
//
//   C = (-6 +/- sqrt(36 - 24)) / 12
//
// -------------------------------------------------- (simplify square root)
//
//   C = (-6 +/- 2*sqrt(3)) / 12
//
// -------------------------------------------------- (take out 2/2)
//
//   C = (-3 +/- sqrt(3)) / 6
//
// -------------------------------------------------- (two possible solutions)
//
//       {   (sqrt(3) - 3) / 6    }
//   C = {                        }
//       {   -(sqrt(3) + 3) / 6   }
//
// -------------------------------------------------- (pick arbitrarily)
//
//   C = (sqrt(3) - 3) / 6
//
// -------------------------------------------------- (calculate)
//
//   C = -0.21132486540518713
//
// -------------------------------------------------- (done)
//
// So if we use:
//
//   x' = x + (x + y)*C
//   y' = y + (x + y)*C
//
// with this value of C, if x and y come from a square grid, x' and y' will
// come from a corresponding simplex grid. This is the distorted -> true
// transform: the distorted (square grid) coordinates are easier to reason
// about; the true (simplex grid) coordinates can be computed as needed. The
// inverse transform is needed to figure out which simplex cell an arbitrary
// point lies within, however. For this we want a C' such that:
//
//   x = x' + (x' + y')*C'
//
// will hold true for all x/y. Substituting the transformations above yields:
//
//   x = (x + (x + y)*C) + (x + (x + y)*C + (y + (x + y)*C))*C'
//
// -------------------------------------------------- (distribute around x/y)
// 
//   x = (2*C'*C + C' + C + 1)*x + (2*C'*C + C' * C)*y
//
// -------------------------------------------------- (swap lhs/rhs + subtract)
//
//   (2*C'*C + C' + C)*x + (2*C'*C + C' * C)*y = 0
//
// -------------------------------------------------- (let x=1, y=0)
//
//   (2*C'*C + C' + C) = 0
//
// -------------------------------------------------- (distribute around C')
//
//   C' * (2*C + 1) = -C
//
// -------------------------------------------------- (divide to isolate C')
//
//   C' = -C / (2*C + 1)
//
// -------------------------------------------------- (substitute C)
//
//   C' = -((sqrt(3) - 3) / 6) / (2*((sqrt(3) - 3) / 6)) + 1)
//
// -------------------------------------------------- (simplify denominator)
//
//   C' = -((sqrt(3) - 3) / 6) / (2*(sqrt(3)/6 - 1/2) + 1)
//
// -------------------------------------------------- (simplify denominator)
//
//   C' = -((sqrt(3) - 3) / 6) / ((sqrt(3)/3 - 1) + 1)
//
// -------------------------------------------------- (simplify denominator)
//
//   C' = -((sqrt(3) - 3) / 6) / (1 / sqrt(3))
//
// -------------------------------------------------- (flip denominator)
//
//   C' = sqrt(3) * -((sqrt(3) - 3) / 6)
//
// -------------------------------------------------- (distribute)
//
//   C' = -(3 - 3*sqrt(3)) / 6
//
// -------------------------------------------------- (simplify)
//
//   C' = -1/2 * (1 - sqrt(3))
//
// -------------------------------------------------- (distribute -1)
//
//   C' = 1/2 * (sqrt(3) - 1)
//
// -------------------------------------------------- (calculate)
//
//   C' = 0.3660254037844386
//
// -------------------------------------------------- (done)
//
// So now we have both a distorted -> normal transform and a normal ->
// distorted transform and we can work with a square grid and pretend it's a
// simplex grid. The resulting simplices will have edge lengths equal to the
// distance between [0, 0] and [0, 1] after transformation:
//
//   [0, 0]   ->   [0, 0]
//   [0, 1]   ->   [0 + (0 + 1)*C, 1 + (0 + 1)*C]   =>   [C, 1 + C]
//
//   L = sqrt( (C - 0)^2 + ((1 + C) - 0)^2 )
//
// -------------------------------------------------- (simplify)
//
//   L = sqrt( 2*C^2 + 2*C + 1 )
//
// -------------------------------------------------- (substitute C)
//
//   L = sqrt( 2*((sqrt(3) - 3) / 6)^2 + 2*((sqrt(3) - 3) / 6) + 1 )
//
// -------------------------------------------------- (simplify 1st term)
//
//   L = sqrt( 2*((3 - 2*sqrt(3)*3 + 9) / 36) + 2*((sqrt(3) - 3) / 6) + 1 )
//
// -------------------------------------------------- (simplify 2nd/3rd terms)
//
//   L = sqrt( 2*((3 - 2*sqrt(3)*3 + 9) / 36) + sqrt(3)/3 )
//
// -------------------------------------------------- (simplify more)
//
//   L = sqrt( ((6 - 4*sqrt(3)*3 + 18) / 36) + sqrt(3)/3 )
//
// -------------------------------------------------- (equalize denominators)
//
//   L = sqrt( ((1/2 - sqrt(3) + 3/2) / 3) + sqrt(3)/3 )
//
// -------------------------------------------------- (combine fractions)
//
//   L = sqrt( (1/2 + 3/2 - sqrt(3) + sqrt(3) ) / 3 )
//
// -------------------------------------------------- (simplify)
//
//   L = sqrt(2/3)
//
// -------------------------------------------------- (simplify)
//
//   L = sqrt(2/3)
//
// -------------------------------------------------- (calculate)
//
//   L = 0.816496580927726
//
// -------------------------------------------------- (done)

#define SQUARE_TO_SIMPLEX_FACTOR -0.2113248654051871
#define SIMPLEX_TO_SQUARE_FACTOR  0.3660254037844386
#define SIMPLEX_EDGE_LENGTH 0.816496580927726

// The closest dendrite edge could be at the other end of a simplex.

float const MAX_DENDRITE_DISTANCE_2D = SIMPLEX_EDGE_LENGTH;


/********************
 * Inline Functions *
 ********************/

// Look up the value for 2D gradient i at (x, y) (along with derivatives):
static inline float grad_2d(ptrdiff_t g, float x, float y) {
  return GRADIENTS_2D[g]*x + GRADIENTS_2D[g + 1]*y;
}
// Note the full equation for the gradient is as follows (where ssqr is
// SURFLET_SQ_RADIUS_2D):
// 
//  (
//    0
//    + gx * x^9
//    + gy * y^9
//
//    + gy * x^8 * y
//    + gx * x * y^8
//
//    - 4 * ssqr * gx * x^7
//    + 4 * gx * x^7 * y^2
//    - 4 * ssqr * gy * y^7
//    + 4 * gy * x^2 * y^7
//
//    - 4 * ssqr * gy * x^6 * y
//    + 4 * gy * x^6 * y^3
//    - 4 * ssqr * gx * x * y^6
//    + 4 * gx * x^3 * y^6
//
//    + 6 * (ssqr ^ 2) * gx * x^5
//    - 12 * ssqr * gx * x^5 * y^2
//    + 6 * gx * x^5 * y^4
//    + 6 * (ssqr ^ 2) * gy * y^5
//    - 12 * ssqr * gy * x^2 * y^5
//    + 6 * gy * x^4 * y^5
//
//    + 6 * (ssqr ^ 2) * gy * x^4 * y
//    - 12 * ssqr * gy * x^4 * y^3
//    + 6 * (ssqr ^ 2) * gx * x * y^4
//    - 12 * ssqr * gx * x^3 * y^4
//
//    - 4 * (ssqr ^ 3) * gx * x^3
//    + 12 * (ssqr ^ 2) * gx * x^3 * y^2
//    - 4 * (ssqr ^ 3) * gy * y^3
//    + 12 * (ssqr ^ 2) * gy * x^2 * y^3
//
//    - 4 * (ssqr ^ 3) * gy * x^2 * y
//    - 4 * (ssqr ^ 3) * gx * x * y^2
//
//    + (ssqr ^ 4) * gx * x
//    + (ssqr ^ 4) * gy * y
//  );
// 
// The functions below are the derivatives of this with respect to x and y,
// simplified for the computer. Appendix A shows the derivation for the above
// full gradient equation.
static inline float grad_2d_dx(ptrdiff_t g, float x, float y) {
  float gx = GRADIENTS_2D[g];
  float gy = GRADIENTS_2D[g+1];
  float x2 = x*x;
  float x3 = x2*x;
  float x4 = x3*x;
  float x5 = x4*x;
  float x6 = x5*x;
  float x7 = x6*x;
  float x8 = x7*x;
  float y2 = y*y;
  float y3 = y2*y;
  float y4 = y3*y;
  float y5 = y4*y;
  float y6 = y5*y;
  float y7 = y6*y;
  float y8 = y7*y;
  float ssqr = SURFLET_SQ_RADIUS_2D;
  float ssqr2 = ssqr * ssqr;
  float ssqr3 = ssqr2 * ssqr;
  float ssqr4 = ssqr3 * ssqr;

  return (
    0
    + 9 * gx * x8

    + 8 * gy * x7 * y
    + gx * y8

    - 7*4 * ssqr * gx * x6
    + 7*4 * gx * x6 * y2
    + 2*4 * gy * x * y7

    - 6*4 * ssqr * gy * x5 * y
    + 6*4 * gy * x5 * y3
    - 4 * ssqr * gx * y6
    + 3*4 * gx * x2 * y6

    + 5*6 * ssqr2 * gx * x4
    - 5*12 * ssqr * gx * x4 * y2
    + 5*6 * gx * x4 * y4
    - 2*12 * ssqr * gy * x * y5
    + 4*6 * gy * x3 * y5

    + 4*6 * (ssqr2) * gy * x3 * y
    - 4*12 * ssqr * gy * x3 * y3
    + 6 * (ssqr2) * gx * y4
    - 3*12 * ssqr * gx * x2 * y4

    - 3*4 * (ssqr3) * gx * x2
    + 3*12 * (ssqr2) * gx * x2 * y2
    + 2*12 * (ssqr2) * gy * x * y3

    - 2*4 * (ssqr3) * gy * x * y
    - 4 * (ssqr3) * gx * y2

    + (ssqr4) * gx
  );
}
static inline float grad_2d_dy(ptrdiff_t g, float x, float y) {
  float gx = GRADIENTS_2D[g];
  float gy = GRADIENTS_2D[g+1];
  float x2 = x*x;
  float x3 = x2*x;
  float x4 = x3*x;
  float x5 = x4*x;
  float x6 = x5*x;
  float x7 = x6*x;
  float x8 = x7*x;
  float y2 = y*y;
  float y3 = y2*y;
  float y4 = y3*y;
  float y5 = y4*y;
  float y6 = y5*y;
  float y7 = y6*y;
  float y8 = y7*y;
  float ssqr = SURFLET_SQ_RADIUS_2D;
  float ssqr2 = ssqr * ssqr;
  float ssqr3 = ssqr2 * ssqr;
  float ssqr4 = ssqr3 * ssqr;

  return (
    0
    + 9 * gy * y8

    + gy * x8
    + 8 * gx * x * y7

    + 2*4 * gx * x7 * y
    - 7*4 * ssqr * gy * y6
    + 7*4 * gy * x2 * y6

    - 4 * ssqr * gy * x6
    + 3*4 * gy * x6 * y2
    - 6*4 * ssqr * gx * x * y5
    + 6*4 * gx * x3 * y5

    - 2*12 * ssqr * gx * x5 * y
    + 4*6 * gx * x5 * y3
    + 5*6 * (ssqr2) * gy * y4
    - 5*12 * ssqr * gy * x2 * y4
    + 5*6 * gy * x4 * y4

    + 6 * (ssqr2) * gy * x4
    - 3*12 * ssqr * gy * x4 * y2
    + 4*6 * (ssqr2) * gx * x * y3
    - 4*12 * ssqr * gx * x3 * y3

    + 2*12 * (ssqr2) * gx * x3 * y
    - 3*4 * (ssqr3) * gy * y2
    + 3*12 * (ssqr2) * gy * x2 * y2

    - 4 * (ssqr3) * gy * x2
    - 2*4 * (ssqr3) * gx * x * y

    + (ssqr4) * gy
  );
}

// Look up the value for 3D gradient i at (x, y, z):
static inline float grad_3d(ptrdiff_t i, float x, float y, float z) {
  ptrdiff_t g = (i & 0x3f) << 2;
  return (
    x * GRADIENTS_3D[g] +
    y * GRADIENTS_3D[g + 1] +
    z * GRADIENTS_3D[g + 2]
  )*GRDIV_3D;
}

// Given a gradient index g and a surflet position (x, y) from the center of a
// surflet, compute the surflet influence at the target position.
static inline float compute_surflet_value_2d(
  ptrdiff_t g,
  float x, float y
) {
  // attenuation
  float atten = (SURFLET_SQ_RADIUS_2D - (x*x + y*y));
  if (atten < 0) {
    return 0.0;
  } else {
    atten *= atten;
    atten *= atten;
  }

  return atten * grad_2d(g, x, y);
}

// Given surflet indices i and j, a surflet center position (sx, sy), and a
// target position (x, y), compute the surflet gradient at the target position,
// putting the results into the rx and ry and returning the surflet value as
// normal.
static inline float compute_surflet_gradient_2d(
  ptrdiff_t g,
  float x, float y,
  float *rx, float *ry
) {
  // attenuation
  float atten = (SURFLET_SQ_RADIUS_2D - (x*x + y*y));

  if (atten < 0) {
    *rx = 0;
    *ry = 0;
    return 0;
  } else {
    atten *= atten;
    atten *= atten;
    // TODO: FIX THIS!
  }

  *rx = grad_2d_dx(g, x, y);
  *ry = grad_2d_dy(g, x, y);
  return atten * grad_2d(g, x, y);
}

// Given surflet indices i, j, and k, a surflet center position (sx, sy, sz),
// and a target position (x, y, z), compute the surflet influence at the target
// position.
static inline float compute_surflet_value_3d(
  ptrdiff_t i, ptrdiff_t j, ptrdiff_t k,
  float sx, float sy, float sz,
  float x, float y, float z
) {
  float dx = x - sx;
  float dy = y - sy;
  float dz = z - sz;

  float atten = SURFLET_SQ_RADIUS_3D - (dx*dx + dy*dy + dz*dz);
  if (atten < 0) {
    return 0.0;
  } else {
    atten *= atten;
    return atten * grad_3d(
      hash_3d(i, j, k),
      dx, dy, dz
    );
  }
}

static inline void compute_offset_grid_point_2d(
  grid_neighborhood_2d *grn,
  ptrdiff_t i,
  ptrdiff_t j,
  size_t idx,
  ptrdiff_t salt
) {
  grn->x[idx] = (float) i + (
    hash_2d(
      i,
      j ^ salt
    ) / ((float) HASH_MASK)
  );
  grn->y[idx] = (float) j + (
    hash_2d(
      j ^ salt,
      i
    ) / ((float) HASH_MASK)
  );
}

static inline ptrdiff_t _posmod(ptrdiff_t n, ptrdiff_t modulus) {
  return ((n % modulus) + modulus) % modulus;
}

static inline void compute_offset_grid_point_2d_wrapped(
  grid_neighborhood_2d *grn,
  ptrdiff_t i,
  ptrdiff_t j,
  ptrdiff_t width,
  ptrdiff_t height,
  size_t idx,
  ptrdiff_t salt
) {
  ptrdiff_t iw = i, jw = j;
  if (width > 0) { iw = _posmod(iw, width); }
  if (height > 0) { jw = _posmod(jw, height); }
  grn->x[idx] = (float) i + (
    hash_2d(
      iw + salt,
      jw + salt
    ) / ((float) HASH_MASK)
  );
  grn->y[idx] = (float) j + (
    hash_3d(
      jw + salt,
      iw + salt,
      iw + salt
    ) / ((float) HASH_MASK)
  );
}

// Uses the SQUARE_TO_SIMPLEX_FACTOR constant to convert from square grid
// coordinates x/y to simplex grid coordinates. r_x and r_y may be the
// addresses of x and y if in-place conversion is desired.
static inline void sqr__spx(float x, float y, float *r_x, float *r_y) {
  float squash = (x + y) * SQUARE_TO_SIMPLEX_FACTOR;
  *r_x = x + squash;
  *r_y = y + squash;
}

// Uses SIMPLEX_TO_SQUARE_FACTOR for the inverse of the above transformation.
static inline void spx__sqr(float x, float y, float *r_x, float *r_y) {
  float squash = (x + y) * SIMPLEX_TO_SQUARE_FACTOR;
  *r_x = x + squash;
  *r_y = y + squash;
}

// From x and y values, computes the i/j indices and upper/lower bit in a
// simplex grid. This function uses a diagonal squash to transform a
// rectangular grid into a simplex grid and vice versa.
static inline void get_simplex_grid_cell(
  float x, float y,
  ptrdiff_t *r_i, ptrdiff_t *r_j, ptrdiff_t *r_upper
) {
  spx__sqr(x, y, &x, &y);
  *r_i = ffloor(x);
  *r_j = ffloor(y);
  *r_upper = (y - ((float) *r_j)) > (x - ((float) *r_i));
}

// These two functions standardize computing arbitrary hashes for simplex grid
// cells:
static ptrdiff_t simplex_cell_hash(ptrdiff_t i, ptrdiff_t j, ptrdiff_t upper) {
  return hash_3d(i ^ upper, i+j, (i+7)*j);
}

static ptrdiff_t simplex_alt_hash(ptrdiff_t i, ptrdiff_t j, ptrdiff_t upper) {
  return hash_3d(i-j, j ^ upper, i*(j+3));
}

// From i and j grid indices, compute the grid cell origin in a simplex grid.
static inline void simplex_grid_cell_origin(
  ptrdiff_t i, ptrdiff_t j,
  float *r_x, float *r_y
) {
  sqr__spx((float) i, (float) j, r_x, r_y);
}

// Given i and j indices and the upper/lower bit, compute the randomly offset
// grid point within the given simplex cell. Returns x/y coordinates in normal
// (simplex grid) space.
static inline void simplex_grid_offset_point(
  ptrdiff_t i, ptrdiff_t j, ptrdiff_t upper,
  float *r_x, float *r_y
) {
  // Get a random point within a unit square:
  float x = float_hash_1d(simplex_cell_hash(i, j, upper));
  float y = float_hash_1d(simplex_alt_hash(i, j, upper));
  float tmp;
  // Flip the point if it's in the wrong half (final distribution remains
  // uniform over the triangle):
  if (
    ( upper && x > y)
  ||
    (!upper && x < y)
  ) {
    tmp = x;
    x = y;
    y = tmp;
  }
  // Add our cell origin in square grid space:
  x += (float) i;
  y += (float) j;
  // Transform back to simplex grid space and we're done:
  sqr__spx(x, y, r_x, r_y);
}

/* This function fills in the x/y/z location information for a 22-cell simplex
 * grid neighborhood on a jittered grid (see simplex_grid_offset_point). Below
 * are the relative index patterns for both upper and lower cells:
 *
 *               +       +                                  +
 *              / \     / \                                / \
 *             / . \   / . \                              / . \
 *            / 0,2 \ / 1,2 \                            / 1,2 \
 *   +-------+-------+-------+-------+          +-------+-------+-------+
 *    \-1,1 / \ 0,1 / \ 1,1 / \ 2,1 /            \ 0,1 / \ 1,1 / \ 2,1 /
 *     \ ^ / . \ ^ / . \ ^ / . \ ^ /              \ ^ / . \ ^ / . \ ^ /
 *      \ /-1,1 \ / 0,1 \ / 1,1 \ /                \ / 0,1 \ / 1,1 \ /
 *       +-------+-------+-------+          +-------+-------+-------+-------+
 *      / \-1,0 / \ 0,0 / \ 1,0 / \          \-1,0 / \ 0,0 / \ 1,0 / \ 2,0 /
 *     / . \ ^ / . \ ^ / . \ ^ / . \          \ ^ / . \ ^ / . \ ^ / . \ ^ /
 *    /-2,0 \ /-1,0 \ / 0,0 \ / 1,0 \          \ /-1,0 \ / 0,0 \ / 1,0 \ /
 *   +-------+-------+-------+-------+          +-------+-------+-------+
 *          / \-1,-1/ \ 0,-1/ \                / \-1,-1/ \ 0,-1/ \ 1,-1/ \
 *         / . \ ^ / . \ ^ / . \              / . \ ^ / . \ ^ / . \ ^ / . \
 *        /-2,-1\ /-1,-1\ / 0,-1\            /-2,-1\ /-1,-1\ / 0,-1\ / 1,-1\
 *       +-------+-------+-------+          +-------+-------+-------+-------+
 *                \-1,-2/                            \-1,-2/ \ 0,-2/
 *                 \ ^ /                              \ ^ /   \ ^ /
 *                  \ /                                \ /     \ /
 *                   +                                  +       +
 *
 * The neighborhood triangles are indexed as follows:
 *
 *               +       +                                  +
 *              / \     / \                                / \
 *             /   \   /   \                              /   \
 *            /  0  \ /  1  \                            / 21  \
 *   +-------+-------+-------+-------+          +-------+-------+-------+
 *    \  2  / \  4  / \  6  / \  8  /            \ 16  / \ 18  / \ 20  /
 *     \   /   \   /   \   /   \   /              \   /   \   /   \   /
 *      \ /  3  \ /  5  \ /  7  \ /                \ / 17  \ / 19  \ /
 *       +-------+-------+-------+          +-------+-------+-------+-------+
 *      / \ 10  / \ 12  / \ 14  / \          \  9  / \ 11  / \ 13  / \ 15  /
 *     /   \   /   \   /   \   /   \          \   /   \   /   \   /   \   /
 *    /  9  \ / 11  \ / 13  \ / 15  \          \ / 10  \ / 12  \ / 14  \ /
 *   +-------+-------+-------+-------+          +-------+-------+-------+
 *          / \ 17  / \ 19  / \                / \  3  / \  5  / \  7  / \
 *         /   \   /   \   /   \              /   \   /   \   /   \   /   \
 *        / 16  \ / 18  \ / 20  \            /  2  \ /  4  \ /  6  \ /  8  \
 *       +-------+-------+-------+          +-------+-------+-------+-------+
 *                \ 21  /                            \  0  / \  1  /
 *                 \   /                              \   /   \   /
 *                  \ /                                \ /     \ /
 *                   +                                  +       +
 *
 */
static inline void fill_simplex_neighborhood_2d(
  simplex_neighborhood_2d *sxn,
  ptrdiff_t i,
  ptrdiff_t j,
  ptrdiff_t upper,
  float (*manifold)(float, float, ptrdiff_t),
  ptrdiff_t msalt
) {
  ptrdiff_t ii;
  if (upper) {
    simplex_grid_offset_point(i-1, j+1, 1, &(sxn->x[0]), &(sxn->y[0]));
    sxn->i[0] = i  ; sxn->j[0] = j+2; sxn->u[0] = 0;
    sxn->i[1] = i+1; sxn->j[1] = j+2; sxn->u[1] = 0;

    sxn->i[2] = i-1; sxn->j[2] = j+1; sxn->u[2] = 1;
    sxn->i[3] = i-1; sxn->j[3] = j+1; sxn->u[3] = 0;
    sxn->i[4] = i  ; sxn->j[4] = j+1; sxn->u[4] = 1;
    sxn->i[5] = i  ; sxn->j[5] = j+1; sxn->u[5] = 0;
    sxn->i[6] = i+1; sxn->j[6] = j+1; sxn->u[6] = 1;
    sxn->i[7] = i+1; sxn->j[7] = j+1; sxn->u[7] = 0;
    sxn->i[8] = i+2; sxn->j[8] = j+1; sxn->u[8] = 1;

    sxn->i[ 9] = i-2; sxn->j[ 9] = j  ; sxn->u[ 9] = 0;
    sxn->i[10] = i-1; sxn->j[10] = j  ; sxn->u[10] = 1;
    sxn->i[11] = i-1; sxn->j[11] = j  ; sxn->u[11] = 0;
    sxn->i[12] = i  ; sxn->j[12] = j  ; sxn->u[12] = 1;
    sxn->i[13] = i  ; sxn->j[13] = j  ; sxn->u[13] = 0;
    sxn->i[14] = i+1; sxn->j[14] = j  ; sxn->u[14] = 1;
    sxn->i[15] = i+1; sxn->j[15] = j  ; sxn->u[15] = 0;

    sxn->i[16] = i-2; sxn->j[16] = j-1; sxn->u[16] = 0;
    sxn->i[17] = i-1; sxn->j[17] = j-1; sxn->u[17] = 1;
    sxn->i[18] = i-1; sxn->j[18] = j-1; sxn->u[18] = 0;
    sxn->i[19] = i  ; sxn->j[19] = j-1; sxn->u[19] = 1;
    sxn->i[20] = i  ; sxn->j[20] = j-1; sxn->u[20] = 0;

    sxn->i[21] = i-1; sxn->j[21] = j-2; sxn->u[21] = 1;
  } else {
    sxn->i[0] = i-1; sxn->j[0] = j-2; sxn->u[0] = 1;
    sxn->i[1] = i  ; sxn->j[1] = j-2; sxn->u[1] = 1;

    sxn->i[2] = i-2; sxn->j[2] = j-1; sxn->u[2] = 0;
    sxn->i[3] = i-1; sxn->j[3] = j-1; sxn->u[3] = 1;
    sxn->i[4] = i-1; sxn->j[4] = j-1; sxn->u[4] = 0;
    sxn->i[5] = i  ; sxn->j[5] = j-1; sxn->u[5] = 1;
    sxn->i[6] = i  ; sxn->j[6] = j-1; sxn->u[6] = 0;
    sxn->i[7] = i+1; sxn->j[7] = j-1; sxn->u[7] = 1;
    sxn->i[8] = i+1; sxn->j[8] = j-1; sxn->u[8] = 0;

    sxn->i[ 9] = i-1; sxn->j[ 9] = j  ; sxn->u[ 9] = 1;
    sxn->i[10] = i-1; sxn->j[10] = j  ; sxn->u[10] = 0;
    sxn->i[11] = i  ; sxn->j[11] = j  ; sxn->u[11] = 1;
    sxn->i[12] = i  ; sxn->j[12] = j  ; sxn->u[12] = 0;
    sxn->i[13] = i+1; sxn->j[13] = j  ; sxn->u[13] = 1;
    sxn->i[14] = i+1; sxn->j[14] = j  ; sxn->u[14] = 0;
    sxn->i[15] = i+2; sxn->j[15] = j  ; sxn->u[15] = 1;

    sxn->i[16] = i  ; sxn->j[16] = j+1; sxn->u[16] = 1;
    sxn->i[17] = i  ; sxn->j[17] = j+1; sxn->u[17] = 0;
    sxn->i[18] = i+1; sxn->j[18] = j+1; sxn->u[18] = 1;
    sxn->i[19] = i+1; sxn->j[19] = j+1; sxn->u[19] = 0;
    sxn->i[20] = i+2; sxn->j[20] = j+1; sxn->u[20] = 1;

    sxn->i[21] = i+1; sxn->j[21] = j+2; sxn->u[21] = 0;
  }
  // Compute x/y and z values:
  for (ii = 0; ii < 22; ++ii) {
    simplex_grid_offset_point(
      sxn->i[ii],
      sxn->j[ii],
      sxn->u[ii],
      &(sxn->x[ii]),
      &(sxn->y[ii])
    );
    sxn->z[ii] = manifold(sxn->x[ii], sxn->y[ii], msalt);
  }
}

// Given a point [x, y], finds the shortest distance to the line segment
// between [fx, fy] and [tx, ty]. It works by projecting [fx, fy] -> [x, y]
// onto [fx, fy] -> [tx, ty], and returning the distance from [x, y] to either
// the resulting point or one of the endpoints of the segment.
static inline float dist_to_edge(
  float x, float y,
  float fx, float fy,
  float tx, float ty
) {
  float seg_x, seg_y, vec_x, vec_y, proj_x, proj_y;
  float dot, m_seg, m_proj;
  // The segment from [fx, fy] to [tx, ty]
  seg_x = tx - fx;
  seg_y = ty - fy;
  // The vector from [fx, fy] to [x, y]
  vec_x = x - fx;
  vec_y = y - fy;
  // compute the dot product:
  dot = seg_x * vec_x + seg_y * vec_y;
  // the squared magnitude of the line segment:
  m_seg = sqrtf(seg_x*seg_x + seg_y*seg_y);
  // We multiply the segment vector by the dot product with the vector to the
  // point in question and divide it by the magnitude of the segment twice. The
  // first division gives us a unit vector in the direction we want. Because
  // the dot product is |A|*|B|*cos(theta) and the projection should have
  // length |B|*cos(theta) in the direction of B, we want to scale our unit
  // vector by dot(A, B) / |A|.
  proj_x = seg_x * dot / (m_seg*m_seg);
  proj_y = seg_y * dot / (m_seg*m_seg);

  m_proj = sqrtf(proj_x*proj_x + proj_y*proj_y);

  if (dot < 0) {
    // We're behind the first point, return distance to it:
    return sqrtf(vec_x*vec_x + vec_y*vec_y);
  } else if (m_proj > m_seg) {
    // We're beyond the second point: return the distance to it:
    return sqrtf((x - tx)*(x - tx) + (y - ty)*(y - ty));
  } else {
    // The closest point is on the segment:
    proj_x += fx;
    proj_y += fy;
    return sqrtf((x - proj_x)*(x - proj_x) + (y - proj_y)*(y - proj_y));
  }
}

// Helper function for nearest_neighborhood_edge_distance that handles checking
// the distance to all three potential edges from a single simplex cell. If
// there are no downhill edges in a cell, it returns MAX_DENDRITE_DISTANCE_2D.
static inline float _check_edges_in_neighborhood(
  simplex_neighborhood_2d *sxn,
  float x,
  float y,
  ptrdiff_t center,
  ptrdiff_t a,
  ptrdiff_t b,
  ptrdiff_t c
) {
  ptrdiff_t dir, checked;
  // We'll check edges from here in a random order based on our coordinates:
  /*
  dir = _posmod(
    simplex_cell_hash(
      sxn->i[center],
      sxn->j[center],
      sxn->u[center]
    ),
    3
  );
  switch (dir) {
    case 0:
      goto lbl_check_a;
    case 1:
      goto lbl_check_b;
    case 2:
      goto lbl_check_c;
  }
  */
  // Check neighbors until we find one that's pointed downhill; that'll be the
  // official edge for this cell. We jump to one of the first 3 cases randomly
  // and continue until all edges have been checked:
  checked = 0;
lbl_check_a:
  if (sxn->z[center] > sxn->z[a]) {
    return dist_to_edge(
      x, y,
      sxn->x[center], sxn->y[center],
      sxn->x[a], sxn->y[a]
    );
  }
  checked += 1;
lbl_check_b:
  if (sxn->z[center] > sxn->z[b]) {
    return dist_to_edge(
      x, y,
      sxn->x[center], sxn->y[center],
      sxn->x[b], sxn->y[b]
    );
  }
  checked += 1;
lbl_check_c:
  if (sxn->z[center] > sxn->z[c]) {
    return dist_to_edge(
      x, y,
      sxn->x[center], sxn->y[center],
      sxn->x[c], sxn->y[c]
    );
  }
  checked += 1;
  if (checked == 3) { return 1+MAX_DENDRITE_DISTANCE_2D; }
  if (sxn->z[center] > sxn->z[a]) {
    return dist_to_edge(
      x, y,
      sxn->x[center], sxn->y[center],
      sxn->x[a], sxn->y[a]
    );
  }
  checked += 1;
  if (checked == 3) { return 1+MAX_DENDRITE_DISTANCE_2D; }
  if (sxn->z[center] > sxn->z[b]) {
    return dist_to_edge(
      x, y,
      sxn->x[center], sxn->y[center],
      sxn->x[b], sxn->y[b]
    );
  }
  return 1+MAX_DENDRITE_DISTANCE_2D;
}

// Given a filled-in simplex grid neighborhood for the given x/y location, find
// the distance from the given point to the nearest edge within that
// neighborhood.
static inline float nearest_neighborhood_edge_distance(
  simplex_neighborhood_2d *sxn,
  float x, float y
) {
  float ax, ay, bx, by, cx, cy;
  float min_so_far, dist;

  /* Distances to the corners of our triangle for culling purposes. The cells
   * are laid out as follows for consistent mapping between AB/BC/AC edges and
   * indices of the corresponding triangles as laid out for fill_simplex_
   * neighborhood_2d.
   *
   *    upper:     lower:
   *
   *  B-------C      A
   *   \     /      / \
   *    \   /      /   \
   *     \ /      /     \
   *      A      B-------C
   */
  float cd_a, cd_b, cd_c;
  if (sxn->u[12]) {
    sqr__spx((float) sxn->i[12]    , (float) sxn->j[12]    , &ax, &ay);
    sqr__spx((float) sxn->i[12]    , (float) sxn->j[12] + 1, &bx, &by);
    sqr__spx((float) sxn->i[12] + 1, (float) sxn->j[12] + 1, &cx, &cy);
  } else {
    sqr__spx((float) sxn->i[12] + 1, (float) sxn->j[12] + 1, &ax, &ay);
    sqr__spx((float) sxn->i[12]    , (float) sxn->j[12]    , &bx, &by);
    sqr__spx((float) sxn->i[12] + 1, (float) sxn->j[12]    , &cx, &cy);
  }
  cd_a = sqrtf((x - ax)*(x - ax) + (y - ay)*(y - ay));
  cd_b = sqrtf((x - bx)*(x - bx) + (y - by)*(y - by));
  cd_c = sqrtf((x - cx)*(x - cx) + (y - cy)*(y - cy));

  // DEBUG:
  if (cd_a < 0.04) { return MAX_DENDRITE_DISTANCE_2D; }
  if (cd_b < 0.04) { return MAX_DENDRITE_DISTANCE_2D; }
  if (cd_c < 0.04) { return MAX_DENDRITE_DISTANCE_2D; }

  ptrdiff_t i, j, u;
  float opx, opy;
  get_simplex_grid_cell(x, y, &i, &j, &u);
  simplex_grid_offset_point(i, j, u, &opx, &opy);
  if (sqrtf((x - opx)*(x - opx) + (y - opy)*(y - opy)) < 0.03) {
    return MAX_DENDRITE_DISTANCE_2D+1;
  }

  // There are 12 triangles adjacent to the neighborhood origin triangle, and
  // each of those (plus the origin itself) needs to find its downhill edge and
  // test distance to the given point. We're not going to worry too much about
  // re-checking edges as checking is cheap and the code to keep track of which
  // we've checked would be more expensive than just doing multiple checks
  // sometimes. We'll start with the distance to the edge in the same cell as
  // our point and do some culling based on that. Refer to the diagram above
  // the fill_simplex_neighborhood_2d function for index positions.
  dist = _check_edges_in_neighborhood(sxn, x, y, 12, 11, 5, 13);
  min_so_far = dist;
  // DEBUG:
  //*
  if (dist > MAX_DENDRITE_DISTANCE_2D) {
    printf("BAD: %zu, %zu:%zu -> %.3f\n", i, j, u, dist);
    printf(
      "  > %.3f :: %.3f, %.3f, %.3f\n",
      sxn->z[12], sxn->z[11], sxn->z[5], sxn->z[13]
    );
    printf(
      "  ?x %.3f :: %.3f, %.3f, %.3f\n",
      sxn->x[12], sxn->x[11], sxn->x[5], sxn->x[13]
    );
    printf(
      "  ?y %.3f :: %.3f, %.3f, %.3f\n",
      sxn->y[12], sxn->y[11], sxn->y[5], sxn->y[13]
    );
    printf(
      "  ?x+y %.3f :: %.3f, %.3f, %.3f\n",
      sxn->x[12] + sxn->y[12],
      sxn->x[11] + sxn->y[11],
      sxn->x[5] + sxn->y[5],
      sxn->x[13] + sxn->y[13]
    );
    return dist;
  }
  // */
  // check across side AB:
  dist = _check_edges_in_neighborhood(sxn, x, y, 11, 10, 17, 12);
  if (dist < min_so_far) { min_so_far = dist; }
  // check across side BC:
  dist = _check_edges_in_neighborhood(sxn, x, y, 5, 4, 12, 6);
  if (dist < min_so_far) { min_so_far = dist; }
  // check across side AC:
  dist = _check_edges_in_neighborhood(sxn, x, y, 13, 12, 19, 14);
  if (dist < min_so_far) { min_so_far = dist; }
  // check behind corner A:
  dist = _check_edges_in_neighborhood(sxn, x, y, 17, 16, 11, 18);
  if (dist < min_so_far) { min_so_far = dist; }
  dist = _check_edges_in_neighborhood(sxn, x, y, 19, 18, 13, 20);
  if (dist < min_so_far) { min_so_far = dist; }
  // We can rule out checking the pure diagonal based on distances sometimes:
  if (min_so_far > cd_a) {
    dist = _check_edges_in_neighborhood(sxn, x, y, 18, 17, 21, 19);
    if (dist < min_so_far) { min_so_far = dist; }
  }
  // check behind corner B:
  dist = _check_edges_in_neighborhood(sxn, x, y, 10, 9, 3, 11);
  if (dist < min_so_far) { min_so_far = dist; }
  dist = _check_edges_in_neighborhood(sxn, x, y, 4, 3, 0, 5);
  if (dist < min_so_far) { min_so_far = dist; }
  // Another pure diagonal:
  if (min_so_far > cd_b) {
    dist = _check_edges_in_neighborhood(sxn, x, y, 3, 2, 10, 4);
    if (dist < min_so_far) { min_so_far = dist; }
  }
  // check behind corner C:
  dist = _check_edges_in_neighborhood(sxn, x, y, 6, 5, 1, 7);
  if (dist < min_so_far) { min_so_far = dist; }
  dist = _check_edges_in_neighborhood(sxn, x, y, 14, 13, 7, 15);
  if (dist < min_so_far) { min_so_far = dist; }
  // The last pure diagonal
  if (min_so_far > cd_c) {
    dist = _check_edges_in_neighborhood(sxn, x, y, 7, 6, 14, 8);
    if (dist < min_so_far) { min_so_far = dist; }
  }
  return min_so_far;
}

/*************
 * Functions *
 *************/

// 2D simplex noise:
float sxnoise_2d(float x, float y, ptrdiff_t salt) {
  // Skew the input space.
  // Move -
  //   x values to the left depending on their height
  // and
  //   scale y values down
  // so that integer values lie on the edges of a simplex grid (tiled
  // equilateral triangles the first of which has the line segment (0,0)->(1,0)
  // as its base). Note that Perlin's original simplex method uses a different
  // simplex grid which is not at all axis-aligned (ours lines up with the x
  // axis) and which also saves a couple of multiplies when skewing and
  // unskewing coordinates. I'm using a partially-aligned grid because the skew
  // transformation is conceptually simpler. The choice of simplex grid also
  // affects how our skewed squares are subdivided: Perlin's are divided from
  // lower left to upper right, while ours are divided by the other diagonal.

  // Each equilateral triangle is sqrt(3)/2 tall, so we want to divide y by
  // sqrt(3)/2 and then subtract 1/2 from x for each unit of the new y.
  float sy = y * TWO__RT3;
  float sx = x - sy * 0.5;

  // Now we can get the integer coords of the bottom-left corner of the simplex
  // pair we're in:
  ptrdiff_t i = ffloor(sx);
  ptrdiff_t j = ffloor(sy);

  // And the fractional components in square-grid-space that will disambiguate
  // between the upper and lower simplices within this square:
  float fx = sx - i;
  float fy = sy - j;

  // Having found our simplex, we can compute the simplex corner indices,
  // the unskewed corner locations, and the surflet values:
  ptrdiff_t upper = (fx > (1 - fy));

  // Corner indices (without branching):
  //
  // (i, j+1)         (i+1, j+1)
  //    +----------------+
  //    |--              |
  //    |  --            |
  //    |    --          |
  //    |      --        |
  //    |        --      |
  //    |          --    |
  //    |            --  |
  //    |              --|
  //    +----------------+
  // (i, j)           (i+1, j)
  //
  // If upper, then indices are:
  // (i, j+1); (i+1, j); (i+1, j+1)
  //   otherwise:
  // (i, j); (i+1, j); (i, j+1)
  ptrdiff_t i0 = i;
  ptrdiff_t j0 = j + upper;

  ptrdiff_t i1 = i + 1;
  ptrdiff_t j1 = j;

  ptrdiff_t i2 = i + upper;
  ptrdiff_t j2 = j + 1;

  // Unskewed corner locations:
  float cx0 = i0 + j0 * 0.5;
  float cy0 = j0 * RT3__TWO;

  float cx1 = i1 + j1 * 0.5;
  float cy1 = j1 * RT3__TWO;

  float cx2 = i2 + j2 * 0.5;
  float cy2 = j2 * RT3__TWO;

  // Salt our indices:
  // Note that we don't change the underlying floating point space, because
  // that can easily lead to floating point imprecision when extremely
  // low-frequency noise is desired.
  i0 += salt;
  j0 += salt;

  i1 += salt;
  j1 += salt;

  i2 += salt;
  j2 += salt;

  // Compute gradient indices for each simplex corner:
  ptrdiff_t g0 = (((hash_1d(i0) ^ hash_1d(j0 + 3))+i0) & 0x3f) << 1;
  ptrdiff_t g1 = (((hash_1d(i1) ^ hash_1d(j1 + 3))+i1) & 0x3f) << 1;
  ptrdiff_t g2 = (((hash_1d(i2) ^ hash_1d(j2 + 3))+i2) & 0x3f) << 1;

  // Surflet values:
  float srf0 = compute_surflet_value_2d(g0, x - cx0, y - cy0);
  float srf1 = compute_surflet_value_2d(g1, x - cx1, y - cy1);
  float srf2 = compute_surflet_value_2d(g2, x - cx2, y - cy2);

  // Return the scaled sum:
  return SCALE_2D * (srf0 + srf1 + srf2);
}

// 2D simplex noise that also returns its gradient:
float sxnoise_grad_2d(float x, float y, ptrdiff_t salt, float *dx, float *dy) {
  // a copy of sxnoise_2d to start with
  float sy = y * TWO__RT3;
  float sx = x - sy * 0.5;

  ptrdiff_t i = ffloor(sx);
  ptrdiff_t j = ffloor(sy);

  float fx = sx - i;
  float fy = sy - j;

  ptrdiff_t upper = (fx > (1 - fy));

  ptrdiff_t i0 = i;
  ptrdiff_t j0 = j + upper;

  ptrdiff_t i1 = i + 1;
  ptrdiff_t j1 = j;

  ptrdiff_t i2 = i + upper;
  ptrdiff_t j2 = j + 1;

  float cx0 = i0 + j0 * 0.5;
  float cy0 = j0 * RT3__TWO;

  float cx1 = i1 + j1 * 0.5;
  float cy1 = j1 * RT3__TWO;

  float cx2 = i2 + j2 * 0.5;
  float cy2 = j2 * RT3__TWO;

  i0 += salt;
  j0 += salt;

  i1 += salt;
  j1 += salt;

  i2 += salt;
  j2 += salt;

  ptrdiff_t g0 = (((hash_1d(i0) ^ hash_1d(j0 + 3))+i0) & 0x3f) << 1;
  ptrdiff_t g1 = (((hash_1d(i1) ^ hash_1d(j1 + 3))+i1) & 0x3f) << 1;
  ptrdiff_t g2 = (((hash_1d(i2) ^ hash_1d(j2 + 3))+i2) & 0x3f) << 1;

  // Here we start getting gradient information:
  float tdx = 0, tdy = 0;

  float srf0 = compute_surflet_gradient_2d(g0, x - cx0, y - cy0, &tdx, &tdy);
  *dx = tdx;
  *dy = tdy;
  float srf1 = compute_surflet_gradient_2d(g1, x - cx1, y - cy1, &tdx, &tdy);
  *dx += tdx;
  *dy += tdy;
  float srf2 = compute_surflet_gradient_2d(g2, x - cx2, y - cy2, &tdx, &tdy);
  *dx += tdx;
  *dy += tdy;

  // Write out the scaled gradient values:
  *dx *= SCALE_2D;
  *dy *= SCALE_2D;

  // Return the scaled sum:
  return SCALE_2D * (srf0 + srf1 + srf2);
}

// 3D simplex noise:
// Note: Perlin uses the simple skew factor 1/3 applied to the sum of the
// coordinates to skew the grid. The idea here is that you want your orthogonal
// coordinate system to be aligned to the edges of several simplices which
// together make a hypercube (or at least hyperprism) (in the 2-dimensional
// case, a pair of equilateral triangles is the target; here we're trying to
// find a sextuple of tetrahedra). Unfortunately, unlike in the 2-dimensional
// case, regular 3D simplices (tetrahedra) don't tile the space. So there's
// going to be some distortion in the noise, but we'll choose our skew factor
// (or let Perlin choose it for us, in fact) to minimize this.
float sxnoise_3d(float x, float y, float z, ptrdiff_t salt) {
  float const skew = 1.0/3.0;
  float const unskew = -1.0/6.0;
  // Skew the input space (see note above).
  float sk = skew * (x + y + z);
  float sx = x + sk;
  float sy = y + sk;
  float sz = z + sk;

  // Now we can get the integer coords of the bottom-left corner of the simplex
  // sextuple we're in:
  ptrdiff_t i = ffloor(sx);
  ptrdiff_t j = ffloor(sy);
  ptrdiff_t k = ffloor(sz);

  // And the fractional components in cubic-grid-space that will disambiguate
  // between the six simplices within this cube:
  float fx = sx - i;
  float fy = sy - j;
  float fz = sz - k;

  // Having found our simplex, we can compute the simplex corner indices,
  // the unskewed corner locations, and the surflet values:

  // The packing of six tetrahedra into a cube is hard to visualize (at least
  // for me it is), but we can think of each tetrahedron as being defined by a
  // path along edges of the cube from the 0, 0, 0 corner of the cube to the
  // 1, 1, 1 corner that traverses exactly two cube vertices along the way (the
  // four points traversed including (0, 0, 0) and (1, 1, 1) define a
  // tetrahedron). We can use the ordering of the fractional x, y, z values to
  // determine which path encloses the point that we're at. We'll start by
  // dividing the cube three times which will give us three rules about which
  // axis must be traversed before which other axis. For example, if fx > fy,
  // we know we're in the south-east half of the cube, so we know that whatever
  // path we choose, it will have to traverse an x=0 -> x=1 edge before it
  // traverses a y=0 -> y=1 edge.
  ptrdiff_t x_before_y = (fx > fy);
  ptrdiff_t x_before_z = (fx > fz);
  ptrdiff_t y_before_z = (fy > fz);

  // Corner indices (without branching):
  // For the various tetrahedra, the corners are:
  //
  //                     x_before_y
  //        x_before_z                 x_after_z
  // y_before_z    y_after_z    y_before_z   y_after_z
  //   0  0  0      0  0  0    *impossible*   0  0  0
  //   1  0  0      1  0  0                   0  0  1
  //   1  1  0      1  0  1                   1  0  1
  //   1  1  1      1  1  1                   1  1  1
  //
  //                     x_after_y
  //        x_before_z                 x_after_z
  // y_before_z    y_after_z    y_before_z   y_after_z
  //   0  0  0    *impossible*    0  0  0     0  0  0
  //   0  1  0                    0  1  0     0  0  1
  //   1  1  0                    0  1  1     0  1  1
  //   1  1  1                    1  1  1     1  1  1
  //
  //
  // Since we don't care about ordering, we can reorder some of the simplexes
  // to avoid using conditionals when computing their integer vertices:
  //
  //                     x_before_y
  //        x_before_z                 x_after_z
  // y_before_z    y_after_z    y_before_z   y_after_z
  //   0  0  0      0  0  0    *impossible*   0  0  0
  //   1  0  0      1  0  0                   1  0  1
  //   1  1  0      1  0  1                   0  0  1
  //   1  1  1      1  1  1                   1  1  1
  //
  //                     x_after_y
  //        x_before_z                 x_after_z
  // y_before_z    y_after_z    y_before_z   y_after_z
  //   0  0  0    *impossible*    0  0  0     0  0  0
  //   0  1  0                    0  1  1     0  1  1
  //   1  1  0                    0  1  0     0  0  1
  //   1  1  1                    1  1  1     1  1  1

  ptrdiff_t i0 = i;
  ptrdiff_t j0 = j;
  ptrdiff_t k0 = k;

  ptrdiff_t i1 = i + x_before_y;
  ptrdiff_t j1 = j + 1 - x_before_y;
  ptrdiff_t k1 = k + 1 - x_before_z;

  ptrdiff_t i2 = i + x_before_z;
  ptrdiff_t j2 = j + y_before_z;
  ptrdiff_t k2 = k + 1 - y_before_z;

  ptrdiff_t i3 = i + 1;
  ptrdiff_t j3 = j + 1;
  ptrdiff_t k3 = k + 1;

  // Unskewed corner locations:
  float unsk = unskew * (i0 + j0 + k0);
  float cx0 = i0 + unsk;
  float cy0 = j0 + unsk;
  float cz0 = k0 + unsk;

  unsk = unskew * (i1 + j1 + k1);
  float cx1 = i1 + unsk;
  float cy1 = j1 + unsk;
  float cz1 = k1 + unsk;

  unsk = unskew * (i2 + j2 + k2);
  float cx2 = i2 + unsk;
  float cy2 = j2 + unsk;
  float cz2 = k2 + unsk;

  unsk = unskew * (i3 + j3 + k3);
  float cx3 = i3 + unsk;
  float cy3 = j3 + unsk;
  float cz3 = k3 + unsk;
  // Sadly, the backbeat ends here.

  // Salt our indices:
  i0 += salt;
  j0 += salt;
  k0 += salt;

  i1 += salt;
  j1 += salt;
  k1 += salt;

  i2 += salt;
  j2 += salt;
  k2 += salt;

  i3 += salt;
  j3 += salt;
  k3 += salt;

  // Surflet values:
  float srf0 = compute_surflet_value_3d(i0, j0, k0, cx0, cy0, cz0, x, y, z);
  float srf1 = compute_surflet_value_3d(i1, j1, k1, cx1, cy1, cz1, x, y, z);
  float srf2 = compute_surflet_value_3d(i2, j2, k2, cx2, cy2, cz2, x, y, z);
  float srf3 = compute_surflet_value_3d(i3, j3, k3, cx3, cy3, cz3, x, y, z);

  // Return the scaled sum:
  return SCALE_3D * (srf0 + srf1 + srf2 + srf3);
}

// 2D Worley noise:
float wrnoise_2d(float x, float y, ptrdiff_t salt) {
  grid_neighborhood_2d grn;
  ptrdiff_t i, j;
  float dx, dy;
  float d = 0;
  float best = MAX_SQ_WORLEY_DISTANCE_2D;
  float secondbest = MAX_SQ_WORLEY_DISTANCE_2D;

  grn.i = ffloor(x);
  grn.j = ffloor(y);

  // Point locations:
  for (i = 0; i < 3; ++i) {
    for (j = 0; j < 3; ++j) {
      compute_offset_grid_point_2d(
        &grn,
        grn.i + i - 1,
        grn.j + j - 1,
        i + j*3,
        salt
      );
    }
  }

  // Find the two closest points:
  for (i = 0; i < 9; ++i) {
    dx = grn.x[i] - x;
    dy = grn.y[i] - y;
    d = (dx * dx + dy * dy);
    if (d < best) {
      secondbest = best;
      best = d;
    } else if (d < secondbest) {
      secondbest = d;
    }
  }

  // Return the scaled distance difference:
  return (secondbest - best) / MAX_SQ_WORLEY_DISTANCE_2D;
}

// "fancy" 2D worley noise:
float wrnoise_2d_fancy(
  float x, float y, ptrdiff_t salt,
  ptrdiff_t wrapx, ptrdiff_t wrapy,
  float *dx, float *dy,
  uint32_t flags
) {
  grid_neighborhood_2d grn;
  ptrdiff_t i, j;
  float dist_x, dist_y;
  float d = 0;
  float dx_this, dy_this;
  float result;
  float best = MAX_SQ_WORLEY_DISTANCE_2D;
  float dx_best = 0, dy_best = 0;
  float secondbest= MAX_SQ_WORLEY_DISTANCE_2D;
  float dx_secondbest = 0, dy_secondbest = 0;
  float thirdbest= MAX_SQ_WORLEY_DISTANCE_2D;
  float dx_thirdbest = 0, dy_thirdbest = 0;

  grn.i = ffloor(x);
  grn.j = ffloor(y);

  // Point locations:
  for (i = 0; i < 3; ++i) {
    for (j = 0; j < 3; ++j) {
      compute_offset_grid_point_2d_wrapped(
        &grn,
        grn.i + i - 1,
        grn.j + j - 1,
        wrapx, wrapy,
        i + j*3,
        salt
      );
    }
  }

  // Find the three closest points:
  for (i = 0; i < 9; ++i) {
    dist_x = grn.x[i] - x;
    dist_y = grn.y[i] - y;
    d = (dist_x * dist_x + dist_y * dist_y);
    dx_this = 2 * dist_x;
    dy_this = 2 * dist_y;
    if (d < best) {
      thirdbest = secondbest;
      dx_thirdbest = dx_secondbest;
      dy_thirdbest = dy_secondbest;
      secondbest = best;
      dx_secondbest = dx_best;
      dy_secondbest = dy_best;
      best = d;
      dx_best = dx_this;
      dy_best = dy_this;
    } else if (d < secondbest) {
      thirdbest = secondbest;
      dx_thirdbest = dx_secondbest;
      dy_thirdbest = dy_secondbest;
      secondbest = d;
      dx_secondbest = dx_this;
      dy_secondbest = dy_this;
    } else if (d < thirdbest) {
      thirdbest = d;
      dx_thirdbest = dx_this;
      dy_thirdbest = dy_this;
    }
  }

  // Return the scaled distance:
  result = 0;
  if (!(flags & WORLEY_FLAG_IGNORE_NEAREST)) {
    if (flags & WORLEY_FLAG_SMOOTH_SIDES) {
      float interp = best / (best + secondbest);
      float dx_interp = (
        secondbest * dx_best - best * dx_secondbest
      ) / pow(best + secondbest, 2);
      float dy_interp = (
        secondbest * dy_best - best * dy_secondbest
      ) / pow(best + secondbest, 2);

      result += (1 - interp) * best + interp * secondbest;
      *dx = (
        -best * dx_interp
      +
        secondbest * dx_interp
      -
        (interp - 1) * dx_best
      +
        interp * dx_secondbest
      );
      *dy = (
        -best * dy_interp
      +
        secondbest * dy_interp
      -
        (interp - 1) * dy_best
      +
        interp * dy_secondbest
      );
    } else {
      result += best;
      *dx = dx_best;
      *dy = dy_best;
    }
  }
  if (flags & WORLEY_FLAG_INCLUDE_NEXTBEST) {
    if (flags & WORLEY_FLAG_SMOOTH_SIDES) {
      float secondthird = secondbest / thirdbest;
      result -= (
        (1 - secondthird) * secondbest +
        secondthird * (0.5 * secondbest + 0.5 * thirdbest)
      );
      *dx -= (
        dx_secondbest * (1.5 - secondthird)
      +
        0.5 * dx_thirdbest * secondthird * secondthird
      );
      *dy -= (
        dy_secondbest * (1.5 - secondthird)
      +
        0.5 * dy_thirdbest * secondthird * secondthird
      );
    } else {
      result -= secondbest;
      *dx -= dx_secondbest;
      *dy -= dy_secondbest;
    }
    result *= -1;
    *dx *= -1;
    *dy *= -1;
  }
  if (!(flags & WORLEY_FLAG_DONT_NORMALIZE)) {
    //result = sqrt(result / MAX_SQ_WORLEY_DISTANCE_2D);
    result /= MAX_SQ_WORLEY_DISTANCE_2D;
    *dx /= MAX_SQ_WORLEY_DISTANCE_2D;
    *dy /= MAX_SQ_WORLEY_DISTANCE_2D;
    //result = smooth(result, 4, 0.5);
  }
  return result;
}

// Dendritic noise:
// Dendritic noise uses the same core principle as Worley noise: a jittered
// grid of points as the basis for stateless noise. But instead of measuring
// distance to the points directly, it connects the points into a grid with
// line segments, uses an underlying manifold to determine which segments are
// pointing downhill, and ensures that at each grid point, all but one downhill
// segment is turned off. Noise values are then determined by the distance to
// the nearest line segment.
// TODO: Either don't pass in the salt or allow for custom scaling.
float dnnoise_2d(
  float x,
  float y,
  ptrdiff_t salt,
  float (*manifold)(float, float, ptrdiff_t),
  ptrdiff_t msalt
) {
  simplex_neighborhood_2d sxn;
  ptrdiff_t i, j, upper;
  float dist;

  // Get our simplex grid location:
  get_simplex_grid_cell(x, y, &i, &j, &upper);

  // Fill out the simplex neighborhood:
  fill_simplex_neighborhood_2d(&sxn, i, j, upper, manifold, msalt);

  // Compute the closest distance to an edge in the neighborhood:
  dist = nearest_neighborhood_edge_distance(&sxn, x, y);

  return dist / MAX_DENDRITE_DISTANCE_2D;
}

// Fractal noise:
// Generate simplex noise at multiple frequencies (by scaling the input
// coordinates) and sum it up. The scaling for each octave is given my the
// frequency ratio argument (relative to the previous octave) and the
// persistence value attenuates each successive octave. The offset values are
// used to shift the origin of each octave relative to the previous octave.
float fractal_sxnoise_2d(
  float x, float y,
  int octaves, 
  float frequency_ratio,
  float persistence,
  float offset_x, float offset_y
) {
  int oct;
  float n = 0;
  float power = 0;
  float scale = 1.0;
  float amplitude = 1.0;
  float orix = 0.0;
  float oriy = 0.0;
  for (oct = 0; oct < octaves; ++oct) {
    // Add offsets:
    orix += offset_x;
    oriy += offset_y;
    // Transform frequency and amplitude:
    scale *= frequency_ratio;
    amplitude *= persistence;
    // Compute transformed coordinates:
    float tx = (x - orix) * scale;
    float ty = (y - oriy) * scale;
    // Accumulate noise:
    n += amplitude * sxnoise_2d(tx, ty, 0); // TODO: Better salt
    // Keep track of total power:
    power += fabs(amplitude);
  }
  return n / power;
}

// 3D fractal noise works just like its 2D counterpart.
float fractal_sxnoise_3d(
  float x, float y, float z,
  int octaves, 
  float frequency_ratio,
  float persistence,
  float offset_x, float offset_y, float offset_z
) {
  int oct;
  float n = 0;
  float power = 0;
  float scale = 1.0;
  float amplitude = 1.0;
  float orix = 0.0;
  float oriy = 0.0;
  float oriz = 0.0;
  for (oct = 0; oct < octaves; ++oct) {
    // Add offsets:
    orix += offset_x;
    oriy += offset_y;
    oriz += offset_z;
    // Transform frequency and amplitude:
    scale *= frequency_ratio;
    amplitude *= persistence;
    // Compute transformed coordinates:
    float tx = (x - orix) * scale;
    float ty = (y - oriy) * scale;
    float tz = (z - oriz) * scale;
    // Accumulate noise:
    n += amplitude * sxnoise_3d(tx, ty, tz, 0); // TODO: Better salt
    // Keep track of total power:
    power += fabs(amplitude);
  }
  return n / power;
}

// The table versions of fractal noise read octave data from a table instead of
// generating it according to fixed parameters. For these, the code is complex
// enough that duplicating it for the 2d/3d cases would be bad, so we use some
// macros to automate the differences between the two cases.
#define FRACTAL_SXNOISE_VAR_TABLE \
  float DIM_FNAME( \
    DIM_ARGS \
    int octaves, \
    float *table, \
    uint32_t *flags \
  ) { \
    int oct; \
    DIM_TR_COORDS_DEF \
    float noise; \
    float n = 0; \
    float power = 0; \
    float scale = 1.0; \
    float amplitude = 1.0; \
    DIM_INIT_OFFSETS \
    float last = 0.0; \
    uint32_t oflags = 0; \
    for (oct = 0; oct < octaves; ++oct) { \
      if (flags != NULL) { \
        oflags = flags[oct]; \
      } \
      DIM_UNPACK_TABLE \
      /* Compute transformed coordinates: */ \
      if (oflags & NOISE_FILTER_DISTORT) { \
        DIM_DISTORT_OFFSETS \
      } \
      if (oflags & NOISE_FILTER_DISTORT_FULL) { \
        DIM_DISTORT_OFFSETS_FULL \
      } \
      DIM_COMPUTE_TR_COORDS \
      /* Construct noise (and remember the value generated): */ \
      DIM_GET_NOISE \
      if (oflags & NOISE_FILTER_ABS) { \
        noise = 0.999999 - fabs(noise); \
      } \
      if (oflags & NOISE_FILTER_SQUARE) { \
        if (noise < 0) { \
          noise = -noise*noise; \
        } else { \
          noise *= noise; \
        } \
      } \
      if (oflags & BLEND_METHOD_REPLACE) { \
        noise *= amplitude; \
        n = noise; \
        power = amplitude; \
      } else if (oflags & BLEND_METHOD_RENORM) { \
        noise *= amplitude; \
        n = n / power + noise; \
        power = 1 + amplitude; \
      } else if (oflags & BLEND_METHOD_MULTIPLY) { \
        /* Use amplitude as an exponent */ \
        if (noise < 0) { \
          noise = -powf(-noise, amplitude); \
        } else { \
          noise = powf(noise, amplitude); \
        } \
        n *= noise; \
        /* power remains unchanged */ \
      } else if (oflags & BLEND_METHOD_HYBRID_LAST) { \
        amplitude *= last; \
        noise *= amplitude; \
        n += noise; \
        power += fabs(amplitude); \
      } else if (oflags & BLEND_METHOD_HYBRID_FULL) { \
        amplitude *= n; \
        noise *= amplitude; \
        n += noise; \
        power += fabs(amplitude); \
      } else if (oflags & BLEND_METHOD_HYBRID_LAST_SCALED) { \
        amplitude *= (1 + last) / 2.0; \
        noise *= amplitude; \
        n += noise; \
        power += fabs(amplitude); \
      } else if (oflags & BLEND_METHOD_HYBRID_FULL_SCALED) { \
        amplitude *= (1 + (n/power)) / 2.0; \
        noise *= amplitude; \
        n += noise; \
        power += fabs(amplitude); \
      } else { \
        noise *= amplitude; \
        n += noise; \
        power += fabs(amplitude); \
      } \
      last = noise; \
    } \
    return n / power; \
  }

// Here are the varying parts for the 2D case:

#define DIM_FNAME fractal_sxnoise_2d_table
#define DIM_ARGS float x, float y,
#define DIM_TR_COORDS_DEF float tx, ty;
#define DIM_INIT_OFFSETS \
  float orix = 0.0; \
  float oriy = 0.0;

#define DIM_UNPACK_TABLE \
      /* Get frequency and amplitude: */ \
      scale = table[0 + (oct << 2)]; \
      amplitude = table[1 + (oct << 2)]; \
      /* Get offsets: */ \
      orix = table[2 + (oct << 2)]; \
      oriy = table[3 + (oct << 2)];

#define DIM_DISTORT_OFFSETS \
      orix += last; \
      oriy += last;

#define DIM_DISTORT_OFFSETS_FULL \
      orix += n; \
      oriy += n;

#define DIM_COMPUTE_TR_COORDS \
    tx = (x - orix) * scale; \
    ty = (y - oriy) * scale;

#define DIM_GET_NOISE \
    noise = sxnoise_2d(tx, ty, 0);

// TODO: Better salt value above

// And now we can create the function content:
FRACTAL_SXNOISE_VAR_TABLE

// Now undefine all those macros so that we can give them different definitions:

#undef DIM_FNAME
#undef DIM_ARGS
#undef DIM_TR_COORDS_DEF
#undef DIM_INIT_OFFSETS
#undef DIM_UNPACK_TABLE
#undef DIM_DISTORT_OFFSETS
#undef DIM_DISTORT_OFFSETS_FULL
#undef DIM_COMPUTE_TR_COORDS
#undef DIM_GET_NOISE

// Now we redefine everything for the 3D case:

#define DIM_FNAME fractal_sxnoise_3d_table
#define DIM_ARGS float x, float y, float z,
#define DIM_TR_COORDS_DEF float tx, ty, tz;
#define DIM_INIT_OFFSETS \
  float orix = 0.0; \
  float oriy = 0.0; \
  float oriz = 0.0;

#define DIM_UNPACK_TABLE \
      /* Get frequency and amplitude: */ \
      scale = table[0 + (oct * 5)]; \
      amplitude = table[1 + (oct * 5)]; \
      /* Get offsets: */ \
      orix = table[2 + (oct * 5)]; \
      oriy = table[3 + (oct * 5)]; \
      oriz = table[4 + (oct * 5)];

#define DIM_DISTORT_OFFSETS \
      orix += last; \
      oriy += last; \
      oriz += last;

#define DIM_DISTORT_OFFSETS_FULL \
      orix += n; \
      oriy += n; \
      oriz += n;

#define DIM_COMPUTE_TR_COORDS \
    tx = (x - orix) * scale; \
    ty = (y - oriy) * scale; \
    tz = (z - oriz) * scale;

#define DIM_GET_NOISE \
    noise = sxnoise_3d(tx, ty, tz, 0);

// TODO: Better salt above

// And now we can create the 3D version of the same function:
FRACTAL_SXNOISE_VAR_TABLE

/*
 * Appendices:

A: Derivation of the full equation for a 2D surflet:

  gx and gy are the gradient values for the surflet, and the dot product of
  these values and the surflet-center-relative x- and y-coordinates is the base
  surflet value. Base attenuation is:

    SURFLET_SQ_RADIUS_2D - (x*x + y*y)

  ...but it's taken to the fourth power before being applied.


  (ssqr - (x*x + y*y)) *
  (ssqr - (x*x + y*y)) *
  (ssqr - (x*x + y*y)) *
  (ssqr - (x*x + y*y)) *
  (gx*x + gy*y);


  (
    0
    + ssqr * ssqr 
    - 2 * ssqr * (x*x + y*y)
    + (x*x + y*y) * (x*x + y*y)
  ) * (
    0
    + ssqr * ssqr
    - 2 * ssqr * (x*x + y*y)
    + (x*x + y*y) * (x*x + y*y)
  ) * (
    gx*x + gy*y
  );


  (
    0
    + (ssqr ^ 4)
    - 4 * (ssqr ^ 3) * (x^2 + y^2)
    + 2 * (ssqr ^ 2) * (x^2 + y^2) * (x^2 + y^2)
    + 4 * (ssqr ^ 2) * (x^2 + y^2) * (x^2 + y^2)
    - 4 * ssqr * (x^2 + y^2) ^ 3
    + (x^2 + y^2) ^ 4
  ) * (
    gx*x + gy*y
  );


  (
    0
    + (ssqr ^ 4)
    - 4 * (ssqr ^ 3) * (x^2 + y^2)
    + 6 * (ssqr ^ 2) * (x^2 + y^2) * (x^2 + y^2)
    - 4 * ssqr * (x^2 + y^2) ^ 3
    + (x^2 + y^2) ^ 4
  ) * (
    gx*x + gy*y
  );


  (
    0
    + (ssqr ^ 4)
    - 4 * (ssqr ^ 3) * x^2
    - 4 * (ssqr ^ 3) * y^2
    + 6 * (ssqr ^ 2) * (x^4 + 2 * x^2 * y^2 + y^4)
    - 4 * ssqr * (x^4 + 2 * x^2 * y^2 + y^4) * (x^2 + y^2)
    + (x^4 + 2 * x^2 * y^2 + y^4) * (x^4 + 2 * x^2 * y^2 + y^4)
  ) * (
    gx*x + gy*y
  );


  (
    0
    + (ssqr ^ 4)
    - 4 * (ssqr ^ 3) * x^2
    - 4 * (ssqr ^ 3) * y^2
    + 6 * (ssqr ^ 2) * (x^4 + 2 * x^2 * y^2 + y^4)
    - 4 * ssqr * (
      x^6 + (x^4 * y^2) + (2 * x^4 * y^2) + (2 * x^2 * y^4) + (x^2 * y^4) + y^6
    )
    + (
      x^8 + (4 * x^6 * y^2) + (2 * x^4 * y^4) +
        (4 * x^4 * y^4) + (4 * x^2 * y^6) + y^8
    )
  ) * (
    gx*x + gy*y
  );


  (
    0
    + (ssqr ^ 4)
    - 4 * (ssqr ^ 3) * x^2
    - 4 * (ssqr ^ 3) * y^2
    + 6 * (ssqr ^ 2) * (x^4 + 2 * x^2 * y^2 + y^4)
    - 4 * ssqr * (
      x^6 + (x^4 * y^2) + (2 * x^4 * y^2) + (2 * x^2 * y^4) + (x^2 * y^4) + y^6
    )
    + (
      x^8 + (4 * x^6 * y^2) + (2 * x^4 * y^4) +
        (4 * x^4 * y^4) + (4 * x^2 * y^6) + y^8
    )
  ) * (
    gx*x + gy*y
  );


  (
    0
    + (ssqr ^ 4)
    - 4 * (ssqr ^ 3) * x^2
    - 4 * (ssqr ^ 3) * y^2
    + 6 * (ssqr ^ 2) * x^4
    + 12 * (ssqr ^ 2) * x^2 * y^2
    + 6 * (ssqr ^ 2) * y^4
    - 4 * ssqr * x^6
    - 12 * ssqr * x^4 * y^2
    - 12 * ssqr * x^2 * y^4
    - 4 * ssqr * y^6
    + (
      x^8 + (4 * x^6 * y^2) + (6 * x^4 * y^4) + (4 * x^2 * y^6) + y^8
    )
  ) * (
    gx*x + gy*y
  );


  (
    0
    + (ssqr ^ 4)
    - 4 * (ssqr ^ 3) * x^2
    - 4 * (ssqr ^ 3) * y^2
    + 6 * (ssqr ^ 2) * x^4
    + 12 * (ssqr ^ 2) * x^2 * y^2
    + 6 * (ssqr ^ 2) * y^4
    - 4 * ssqr * x^6
    - 12 * ssqr * x^4 * y^2
    - 12 * ssqr * x^2 * y^4
    - 4 * ssqr * y^6
    + x^8
    + 4 * x^6 * y^2
    + 6 * x^4 * y^4
    + 4 * x^2 * y^6
    + y^8
  ) * (
    gx*x + gy*y
  );


  (
    0
    + (ssqr ^ 4) * gx * x
    - 4 * (ssqr ^ 3) * gx * x^3
    - 4 * (ssqr ^ 3) * gx * x * y^2
    + 6 * (ssqr ^ 2) * gx * x^5
    + 12 * (ssqr ^ 2) * gx * x^3 * y^2
    + 6 * (ssqr ^ 2) * gx * x * y^4
    - 4 * ssqr * gx * x^7
    - 12 * ssqr * gx * x^5 * y^2
    - 12 * ssqr * gx * x^3 * y^4
    - 4 * ssqr * gx * x * y^6
    + gx * x^9
    + 4 * gx * x^7 * y^2
    + 6 * gx * x^5 * y^4
    + 4 * gx * x^3 * y^6
    + gx * x * y^8
  ) +
  (
    0
    + (ssqr ^ 4) * gy * y
    - 4 * (ssqr ^ 3) * gy * x^2 * y
    - 4 * (ssqr ^ 3) * gy * y^3
    + 6 * (ssqr ^ 2) * gy * x^4 * y
    + 12 * (ssqr ^ 2) * gy * x^2 * y^3
    + 6 * (ssqr ^ 2) * gy * y^5
    - 4 * ssqr * gy * x^6 * y
    - 12 * ssqr * gy * x^4 * y^3
    - 12 * ssqr * gy * x^2 * y^5
    - 4 * ssqr * gy * y^7
    + gy * x^8 * y
    + 4 * gy * x^6 * y^3
    + 6 * gy * x^4 * y^5
    + 4 * gy * x^2 * y^7
    + gy * y^9
  );


  (
    0
    + (ssqr ^ 4) * gx * x
    - 4 * (ssqr ^ 3) * gx * x^3
    - 4 * (ssqr ^ 3) * gx * x * y^2
    + 6 * (ssqr ^ 2) * gx * x^5
    + 12 * (ssqr ^ 2) * gx * x^3 * y^2
    + 6 * (ssqr ^ 2) * gx * x * y^4
    - 4 * ssqr * gx * x^7
    - 12 * ssqr * gx * x^5 * y^2
    - 12 * ssqr * gx * x^3 * y^4
    - 4 * ssqr * gx * x * y^6
    + gx * x^9
    + 4 * gx * x^7 * y^2
    + 6 * gx * x^5 * y^4
    + 4 * gx * x^3 * y^6
    + gx * x * y^8
    + (ssqr ^ 4) * gy * y
    - 4 * (ssqr ^ 3) * gy * x^2 * y
    - 4 * (ssqr ^ 3) * gy * y^3
    + 6 * (ssqr ^ 2) * gy * x^4 * y
    + 12 * (ssqr ^ 2) * gy * x^2 * y^3
    + 6 * (ssqr ^ 2) * gy * y^5
    - 4 * ssqr * gy * x^6 * y
    - 12 * ssqr * gy * x^4 * y^3
    - 12 * ssqr * gy * x^2 * y^5
    - 4 * ssqr * gy * y^7
    + gy * x^8 * y
    + 4 * gy * x^6 * y^3
    + 6 * gy * x^4 * y^5
    + 4 * gy * x^2 * y^7
    + gy * y^9
  );


  (
    0
    + gx * x^9
    + gy * y^9

    + gy * x^8 * y
    + gx * x * y^8

    - 4 * ssqr * gx * x^7
    + 4 * gx * x^7 * y^2
    - 4 * ssqr * gy * y^7
    + 4 * gy * x^2 * y^7

    - 4 * ssqr * gy * x^6 * y
    + 4 * gy * x^6 * y^3
    - 4 * ssqr * gx * x * y^6
    + 4 * gx * x^3 * y^6

    + 6 * (ssqr ^ 2) * gx * x^5
    - 12 * ssqr * gx * x^5 * y^2
    + 6 * gx * x^5 * y^4
    + 6 * (ssqr ^ 2) * gy * y^5
    - 12 * ssqr * gy * x^2 * y^5
    + 6 * gy * x^4 * y^5

    + 6 * (ssqr ^ 2) * gy * x^4 * y
    - 12 * ssqr * gy * x^4 * y^3
    + 6 * (ssqr ^ 2) * gx * x * y^4
    - 12 * ssqr * gx * x^3 * y^4

    - 4 * (ssqr ^ 3) * gx * x^3
    + 12 * (ssqr ^ 2) * gx * x^3 * y^2
    - 4 * (ssqr ^ 3) * gy * y^3
    + 12 * (ssqr ^ 2) * gy * x^2 * y^3

    - 4 * (ssqr ^ 3) * gy * x^2 * y
    - 4 * (ssqr ^ 3) * gx * x * y^2

    + (ssqr ^ 4) * gx * x
    + (ssqr ^ 4) * gy * y
  );

*/
