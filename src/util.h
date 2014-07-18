#ifndef UTIL_H
#define UTIL_H

// util.h
// Various small utilities.

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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

/*************
 * Constants *
 *************/

// radians <-> degrees
static float const R2D = 180*M_1_PI;
static float const D2R = M_PI/180;

/********************
 * Inline Functions *
 ********************/

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
