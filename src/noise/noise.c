// noise.c
// Noise functions (mainly simplex noise).

#include <math.h>
#include <stdlib.h>

#include "noise.h"

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
static float const GRLEN_2D = 25.0;
static float const GRDIV_2D = 1.0/25.0;
static int const GRADIENTS_2D[128] = {
// Three 20-entry columns -> 60 1x2 vectors (120 ints):
   25,   0,       20,   0,       16,   0,
   24,   7,       19,   5,       15,   4,
   20,  15,       16,  12,       12,  10,
   15,  20,       12,  16,       10,  12,
    7,  24,        5,  19,        4,  15,
    0,  25,        0,  20,        0,  16,
   -7,  24,       -5,  19,       -4,  15,
  -15,  20,      -12,  16,      -10,  12,
  -20,  15,      -16,  12,      -12,  10,
  -24,   7,      -19,   5,      -15,   4,
  -25,   0,      -20,   0,      -16,   0,
  -24,  -7,      -19,  -5,      -15,  -4,
  -20, -15,      -16, -12,      -12, -10,
  -15, -20,      -12, -16,      -10, -12,
   -7, -24,       -5, -19,       -4, -15,
   -0, -25,        0, -20,        0, -16,
    7, -24,        5, -19,        4, -15,
   15, -20,       12, -16,       10, -12,
   20, -15,       16, -12,       12, -10,
   24,  -7,       19,  -5,       15,  -4,
// Four extra 1x2 vectors makes 64 vectors (128 ints):
   20,  15,
  -20,  15,
  -20, -15,
   20, -15
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
static int const GRADIENTS_3D[256] = {
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
#define RT3_TWO (RT3 / 2.0)
#define TWO_RT3 (2.0 / RT3)

// sqrt(3)/2 squared is 3/4:
float const SURFLET_RADIUS_2D = RT3_TWO;
float const SURFLET_SQ_RADIUS_2D = 0.75;

// The 3D simplices aren't quite regular (regular tetrahedrons don't tile
// properly), so this value is a bit of a fudge. A regular unit tetrahedron
// would have height 0.81, but we'll assume that our distorted tetrahedra will only have maximum allowable radius ~.75. Squaring that will yield 0.5625.
float const SURFLET_RADIUS_3D = 0.75;
float const SURFLET_SQ_RADIUS_3D = 0.5625;

// These scaling values are chosen based on trial-and-error.
// Because of this it might be possible for a value outside [-1,1] to be
// generated.
static float SCALE_2D = 3.25; // Mostly falls within [-0.9,0.9]
static float SCALE_3D = 7.0; // Mostly falls within [-0.9,0.9]
// Note that the 2D noise seems to have a slight positive bias (this is a
// bug?) with an average of ~50.5 over 65536 samples normalized to integers on
// [0,99] (expected average is 49.5).
// Note also that both the noise min/max values over a large area will contract
// noticeably when the sampling grid approaches the size of the simplices.
// Ideally use at least 4 samples/simplex when values from the full range are
// desired.

/********************
 * Inline Functions *
 ********************/

// Lookup the value for 2D gradient i at (x, y):
static inline float grad_2d(int i, float x, float y) {
  int g = (i & 0x3f) << 1;
  return (GRADIENTS_2D[g]*x + GRADIENTS_2D[g + 1]*y)*GRDIV_2D;
}

// Lookup the value for 3D gradient i at (x, y, z):
static inline float grad_3d(int i, float x, float y, float z) {
  int g = (i & 0x3f) << 2;
  return (
    x * GRADIENTS_3D[g] +
    y * GRADIENTS_3D[g + 1] +
    z * GRADIENTS_3D[g + 2]
  )*GRDIV_3D;
}

// Faster floor function (mostly 'cause we're ignoring IEEE error stuff):
static inline int fastfloor(float x) {
  int ix = (int) x;
  return ix - (ix > x);
}

// 2D hash function using the hash table:
static inline int hash_2d(int i, int j) {
  return HASH[(i & HASH_MASK) + HASH[(j & HASH_MASK)]];
}

// 3D hash function using the hash table:
static inline int hash_3d(int i, int j, int k) {
  return HASH[(i & HASH_MASK) + HASH[(j & HASH_MASK) + HASH[k & HASH_MASK]]];
}

// Given surflet indices i and j, a surflet center position (sx, sy), and a
// target position (x, y), compute the surflet influence at the target position.
static inline float compute_surflet_value_2d(
  int i, int j,
  float sx, float sy,
  float x, float y
) {
  float dx = x - sx;
  float dy = y - sy;

  float atten = SURFLET_SQ_RADIUS_2D - (dx*dx + dy*dy);
  if (atten < 0) {
    return 0.0;
  } else {
    atten *= atten;
    //return atten * ((hash_2d(i, j) & 0x2) - 1);
    return atten * grad_2d(hash_2d(i, j), dx, dy);
  }
}

// Given surflet indices i, j, and k, a surflet center position (sx, sy, sz),
// and a target position (x, y, z), compute the surflet influence at the target
// position.
static inline float compute_surflet_value_3d(
  int i, int j, int k,
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
    return atten * grad_3d(hash_3d(i, j, k), dx, dy, dz);
  }
}

/*************
 * Functions *
 *************/

// 2D simplex noise:
float sxnoise_2d(float x, float y) {
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
  // transformation is conceptually simpler. The choice of simples grid also
  // affects how our skewed squares are subdivided: Perlin's are divided from
  // lower left to upper right, while ours are divided by the other diagonal.

  // Each equilateral triangle is sqrt(3)/2 tall, so we want to divide y by
  // sqrt(3)/2 and then subtract 1/2 from x for each unit of the new y.
  float sy = y * TWO_RT3;
  float sx = x - sy * 0.5;

  // Now we can get the integer coords of the bottom-left corner of the simplex
  // pair we're in:
  int i = fastfloor(sx);
  int j = fastfloor(sy);

  // And the fractional components in square-grid-space that will disambiguate
  // between the upper and lower simplices within this square:
  float fx = sx - i;
  float fy = sy - j;

  // Having found our simplex, we can compute the simplex corner indices,
  // the unskewed corner locations, and the surflet values:
  int upper = (fx > (1 - fy));

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
  // (i+1, j); (i, j+1); (i+1, j+1)
  //   otherwise:
  // (i, j); (i, j+1); (i+1, j)
  int i0 = i + upper;
  int j0 = j;

  int i1 = i;
  int j1 = j + 1;

  int i2 = i + 1;
  int j2 = j + upper;

  // Unskewed corner locations:
  float cx0 = i0 + j0 * 0.5;
  float cy0 = j0 * RT3_TWO;

  float cx1 = i1 + j1 * 0.5;
  float cy1 = j1 * RT3_TWO;

  float cx2 = i2 + j2 * 0.5;
  float cy2 = j2 * RT3_TWO;

  // Surflet values:
  float srf0 = compute_surflet_value_2d(i0, j0, cx0, cy0, x, y);
  float srf1 = compute_surflet_value_2d(i1, j1, cx1, cy1, x, y);
  float srf2 = compute_surflet_value_2d(i2, j2, cx2, cy2, x, y);

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
float sxnoise_3d(float x, float y, float z) {
  float const skew = 1.0/3.0;
  float const unskew = -1.0/6.0;
  // Skew the input space (see note above).
  float sk = skew * (x + y + z);
  float sx = x + sk;
  float sy = y + sk;
  float sz = z + sk;

  // Now we can get the integer coords of the bottom-left corner of the simplex
  // sextuple we're in:
  int i = fastfloor(sx);
  int j = fastfloor(sy);
  int k = fastfloor(sz);

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
  int x_before_y = (fx > fy);
  int x_before_z = (fx > fz);
  int y_before_z = (fy > fz);

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
  // Since we don't care about ordering, we can reordered some of the simplexes
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

  int i0 = i;
  int j0 = j;
  int k0 = k;

  int i1 = i + x_before_y;
  int j1 = j + 1 - x_before_y;
  int k1 = k + 1 - x_before_z;

  int i2 = i + x_before_z;
  int j2 = j + y_before_z;
  int k2 = k + 1 - y_before_z;

  int i3 = i + 1;
  int j3 = j + 1;
  int k3 = k + 1;

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

  // Surflet values:
  float srf0 = compute_surflet_value_3d(i0, j0, k0, cx0, cy0, cz0, x, y, z);
  float srf1 = compute_surflet_value_3d(i1, j1, k1, cx1, cy1, cz1, x, y, z);
  float srf2 = compute_surflet_value_3d(i2, j2, k2, cx2, cy2, cz2, x, y, z);
  float srf3 = compute_surflet_value_3d(i3, j3, k3, cx3, cy3, cz3, x, y, z);

  // Return the scaled sum:
  return SCALE_3D * (srf0 + srf1 + srf2 + srf3);
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
    n += amplitude * sxnoise_2d(tx, ty);
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
    n += amplitude * sxnoise_3d(tx, ty, tz);
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
    noise = sxnoise_2d(tx, ty);

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
    noise = sxnoise_3d(tx, ty, tz);

// And now we can create the 3D version of the same function:
FRACTAL_SXNOISE_VAR_TABLE
