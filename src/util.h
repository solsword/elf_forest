#ifndef UTIL_H
#define UTIL_H

// util.h
// Various small utilities.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <GL/gl.h>

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

// Rounds a floating point value to the nearest fraction with the given integer
// denominator.
static inline float rounddenom(float x, int denom) {
  return roundf(x*denom)/((float) denom);
}

// (Poor quality) floating point rand:
static inline float randf(float min, float max) {
  float result = rand();
  result /= RAND_MAX;
  return min + result*(max - min);;
}

// Stupid-simple PRNG:
static inline ptrdiff_t prng(ptrdiff_t seed) {
  return (seed * 49103) + 2147483659; // both are primes
}

// Simple ptrdiff_t->float
// Note that resolution is roughly 1/2^20, so don't expect too much.
// Returns a value in [0, 1)
static inline float ptrf(ptrdiff_t seed) {
  return ((seed % 524309) + 524309) / 1048617.0;
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
    while (*angle > M_PI) {
      *angle -= M_PI*2;
    }
    while (*angle < -M_PI) {
      *angle += M_PI*2;
    }
}

static inline void nap(ms) {
  // TODO: cross-platform BS
  usleep(ms*1000);
}

// Looks for an OpenGL error and prints it to stderr, after the given reason.
// If it finds an error, it returns 1 after printing it, otherwise it returns 0.
static inline int report_opengl_error(char const * const reason) {
  GLenum e = glGetError();
  if (e == GL_NO_ERROR) {
    return 0;
  }
  fprintf(stderr, reason);
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
      fprintf(stderr, "Unknown OpenGL error: %d.\n", e);
  }
  return 1;
}

// Reads a file and returns a malloc'd char *:
static inline char * load_file(char const * const filename, size_t *size) {
  char * buffer = NULL;
  FILE * f = fopen(filename, "rb");

  if (f == NULL) {
    fprintf(
      stderr,
      "Error: unable to open file '%s'.\n",
      filename
    );
    exit(1);
  }
  fseek (f, 0, SEEK_END);
  *size = ftell(f);
  fseek (f, 0, SEEK_SET);
  buffer = malloc(*size);
  if (buffer == NULL) {
    fprintf(
      stderr,
      "Error: unable to allocate memory to read file '%s'.\n",
      filename
    );
    fclose(f);
    exit(1);
  }
  fread(buffer, 1, *size, f);
  fclose(f);
  return buffer;
}

#endif // ifndef UTIL_H
