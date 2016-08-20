#ifndef UTIL_H
#define UTIL_H

// util.h
// Various small utilities.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include <GL/gl.h>

/*********
 * Types *
 *********/

union fp_conv_u {
  void *p;
  float f;
};
typedef union fp_conv_u fp_conv;

union ip_conv_u {
  void *p;
  intptr_t i;
};
typedef union ip_conv_u ip_conv;

/**********
 * MACROS *
 **********/

// Bits in a type:
#define bits(t) (sizeof(t) * 8ULL)

// All 1s except the sign bit:
#define lull(t) ((0x1ULL << (bits(t) - 1ULL)) - 1ULL)

// Various fully-shifted bytes:
#define bull(t) (0x8ULL << (bits(t) - 4ULL))
#define full(t) (0xFULL << (bits(t) - 4ULL))
#define tull(t) (0x7ULL << (bits(t) - 4ULL))

// Min for unsigned is just 0:
#define uminof(t) ( (t) 0 )
// Max for unsigned is all 1s:
#define umaxof(t) ( (t) (full(t) | lull(t)) )

// Min for signed is just the sign bit:
#define sminof(t) ( (t) bull(t) )
// Max for signed is all 1s except the sign bit:
#define smaxof(t) ( (t) (tull(t) | lull(t)) )

#define HALF_PTR_BITS (bits(uintptr_t)/2)
#define HALF_PTR_TOP ((uintptr_t) (umaxof(uintptr_t)<<HALF_PTR_BITS))
#define HALF_PTR_BOT ((uintptr_t) (~((uintptr_t) HALF_PTR_TOP)))

#define IS_LITTLE_ENDIAN (*((uint8_t*) (&ENDIAN_DETECTOR)) == 1)

/*************
 * Constants *
 *************/

// radians <-> degrees
static float const R2D = 180*M_1_PI;
static float const D2R = M_PI/180;

// Endian detection
static uint16_t const ENDIAN_DETECTOR = 1;

/********************
 * Inline Functions *
 ********************/

// Takes the modulus of a ptrdiff_t but also ensures it's positive:
static inline ptrdiff_t posmod(ptrdiff_t n, ptrdiff_t base) {
  return ((n % base) + base) % base;
}

// Conversions from/to network byte order for 64-bit values:
static inline uint64_t hton64(uint64_t host_ordered) {
  uint8_t* bytes;
  if (IS_LITTLE_ENDIAN) { // host is little: convert to big
    bytes = (uint8_t*) (&host_ordered);
    return (
      (((uint64_t) bytes[0]) << 56)
    |    
      (((uint64_t) bytes[1]) << 48)
    |    
      (((uint64_t) bytes[2]) << 40)
    |    
      (((uint64_t) bytes[3]) << 32)
    |    
      (((uint64_t) bytes[4]) << 24)
    |    
      (((uint64_t) bytes[5]) << 16)
    |    
      (((uint64_t) bytes[6]) << 8)
    |
      ((uint64_t) bytes[7])
    );
  } else {
    return host_ordered;
  }
}

static inline uint64_t ntoh64(uint64_t net_ordered) {
  uint8_t* bytes;
  if (IS_LITTLE_ENDIAN) { // host is little: convert from big
    bytes = (uint8_t*) (&net_ordered);
    return (
      (((uint64_t) bytes[0]) << 56)
    |    
      (((uint64_t) bytes[1]) << 48)
    |    
      (((uint64_t) bytes[2]) << 40)
    |    
      (((uint64_t) bytes[3]) << 32)
    |    
      (((uint64_t) bytes[4]) << 24)
    |    
      (((uint64_t) bytes[5]) << 16)
    |    
      (((uint64_t) bytes[6]) << 8)
    |
      ((uint64_t) bytes[7])
    );
  } else {
    return net_ordered;
  }
}


// Faster floor function (mostly 'cause we're ignoring IEEE error stuff):
static inline int fastfloor(float x) {
  int ix = (int) x;
  return ix - (ix > x);
}
// Likewise for ceil():
static inline int fastceil(float x) {
  int ix = (int) x;
  return ix + (ix < x);
}

// Integer min/max
static inline int imin(int a, int b) {
  return a < b? a : b;
}

static inline int imax(int a, int b) {
  return a > b? a : b;
}

// Rounds a floating point value to the nearest fraction with the given integer
// denominator.
static inline float rounddenom(float x, int denom) {
  return roundf(x*denom)/((float) denom);
}

// Reinterprets the bits of a void* as an intptr_t. Useful when ints need to be
// stored in containers that accept void*s.
static inline intptr_t p_as_i(void *p) {
  ip_conv conv;
  conv.p = p;
  return conv.i;
}

// The inverse of the above, this reinterprets an intptr_t as a void*.
static inline void * i_as_p(intptr_t i) {
  ip_conv conv;
  conv.i = i;
  return conv.p;
}

// Reinterprets the bits of a void* as a float. Useful when floats need to be
// stored in containers that accept void*s.
static inline float p_as_f(void *p) {
  fp_conv conv;
  conv.p = p;
  return conv.f;
}

// The inverse of the above, this reinterprets a float as a void*.
static inline void * f_as_p(float f) {
  fp_conv conv;
  conv.f = f;
  return conv.p;
}

// Stupid-simple PRNG:
static inline ptrdiff_t prng(ptrdiff_t seed) {
  return (seed * 49103) + 2147483659; // both are prime
  // TODO: Would this be better in any way?
  // return (((seed * 49103) + 2147483659) * 78157) + 11665001; // all prime
}

// Simple ptrdiff_t->float
// Note that resolution is roughly 1/2^20, so don't expect too much.
// Returns a value in [0, 1)
static inline float ptrf(ptrdiff_t seed) {
  return (float) ((seed % 524309) + 524309) / 1048617.0;
}

// Simple random int within specific range:
static inline ptrdiff_t randi(
  ptrdiff_t seed,
  ptrdiff_t min,
  ptrdiff_t max
) {
  return min + posmod(prng(seed + 81882), ((max - min) + 1));
}

// Random floating point number in specific range:
static inline float randf(ptrdiff_t seed, float min, float max) {
  return min + ptrf(prng(seed + 45201))*(max - min);;
}

// Returns a random floating point number within the specified range drawn from
// a pseudo-normal distribution with variance of about 1/4 the given range and
// mean at the center of the given range.
static inline float randf_pnorm(ptrdiff_t seed, float min, float max) {
  float result = randf(seed, min, max);
  result += randf(prng(seed+7488), min, max);
  result += randf(prng(prng(seed+18712)+4657), min, max);
  return result / 3.0;
}

// Packs two floats (both of which should be in [0, 1]) into a single void*.
// Depending on the platform's pointer size, the resolution will be different.
// Floating point errors should be expected.
static inline void* fxy__ptr(float x, float y) {
#ifdef DEBUG
  if (x < 0 || x > 1 || y < 0 || y > 1) {
    printf("xyptr x/y out of range [0, 1]: (%.2f,%.2f)\n", x, y);
  }
#endif
  return (void*) (
    (((uintptr_t) (x * HALF_PTR_BOT)) << HALF_PTR_BITS)
  +
    ((uintptr_t) (y * HALF_PTR_BOT))
  );
}

// The inverse of fxy__ptr; unpacks just the x component. Don't expect the
// result to be equal to the value that was packed in, however, due to the
// conversion. The value returned will always be in [0, 1] however.
static inline float ptr__fx(void const * const ptr) {
  return (
    ((((uintptr_t) ptr) >> HALF_PTR_BITS) & HALF_PTR_BOT)
  /
    ((float) HALF_PTR_BOT)
  );
}

// See ptr__x: this just unpacks the y component instead of the x component.
static inline float ptr__fy(void const * const ptr) {
  return (
    (((uintptr_t) ptr) & HALF_PTR_BOT)
  /
    ((float) HALF_PTR_BOT)
  );
}

// These i[xy] pointer conversion functions work like the f- versions above but
// for unsigned integers. Precision is of course not a problem, but truncation
// could be.
static inline void* ixy__ptr(size_t x, size_t y) {
  return (void*) (((x & HALF_PTR_BOT) << HALF_PTR_BITS) + (y & HALF_PTR_BOT));
}

static inline size_t ptr__ix(void const * const ptr) {
  return (size_t) ((((uintptr_t) ptr) >> HALF_PTR_BITS) & HALF_PTR_BOT);
}

static inline size_t ptr__iy(void const * const ptr) {
  return (size_t) (((uintptr_t) ptr) & HALF_PTR_BOT);
}

// Normalizes the given angle to be between -M_PI and M_PI.
static inline void norm_angle(float *angle) {
  *angle = fmod(*angle, 2*M_PI);
  if (*angle > M_PI) {
    *angle -= 2*M_PI;
  }
  if (*angle < -M_PI) {
    *angle += 2*M_PI;
  }
}

// Returns the positive angle between the two given headings, which should each
// be within 2*M_PI of each other.
static inline float angle_between(float first, float second) {
  float result = fabs(second - first);
  if (result > M_PI) {
    return 2*M_PI - result;
  } else {
    return result;
  }
}

// Takes two headings in [-M_PI, M_PI] and returns a heading between them,
// governed by the interpolation constant interp which should be in [0, 1] (if
// interp == 0, the result is first, if interp == 1, the result is second).
static inline float mix_angles(float first, float second, float interp) {
  float between = second - first;
  if (between > M_PI) {
    between = -(2*M_PI - between);
  } else if (between < -M_PI) {
    between = 2*M_PI + between;
  }
  between = first + between * (1 - interp);
  norm_angle(&between);
  return between;
}

static inline void nap(int ms) {
  // TODO: cross-platform BS
  // TODO: check return value for error?
  (void) usleep(ms*1000);
}

// Looks for an OpenGL error and prints it to stderr, after the given reason.
// If it finds an error, it returns 1 after printing it, otherwise it returns 0.
static inline int report_opengl_error(char const * const reason) {
  GLenum e = glGetError();
  if (e == GL_NO_ERROR) {
    return 0;
  }
  // TODO: check error here?
  (void) fputs(reason, stderr);
  switch (e) {
    case GL_INVALID_ENUM:
      fprintf(stderr, "OpenGL: Invalid enumeration.\n");
      break;
    case GL_INVALID_VALUE:
      fprintf(stderr, "OpenGL: Invalid value.\n");
      break;
    case GL_INVALID_OPERATION:
      fprintf(stderr, "OpenGL: Invalid operation.\n");
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      fprintf(stderr, "OpenGL: Invalid framebuffer operation.\n");
      break;
    case GL_OUT_OF_MEMORY:
      fprintf(stderr, "OpenGL: Out of memory.\n");
      break;
    default:
      fprintf(stderr, "Unknown OpenGL error: %d.\n", (int) e);
  }
  return 1;
}

#endif // ifndef UTIL_H
