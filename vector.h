#ifndef VECTOR_H
#define VECTOR_H

// vector.h
// 3D floating point vectors.

#include <math.h>

#include "util.h"

/**************
 * Structures *
 **************/

// A vector of 3 floats.
struct vector_s;
typedef struct vector_s vector;

/*************
 * Constants *
 *************/

extern const vector V_UP;
extern const vector V_DOWN;
extern const vector V_NORTH;
extern const vector V_SOUTH;
extern const vector V_EAST;
extern const vector V_WEST;

/*************************
 * Structure Definitions *
 *************************/

struct vector_s {
  float x, y, z;
};

/********************
 * Inline Functions *
 ********************/

static inline float vmag(const vector *v) {
  return sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
}

static inline float vmag2(const vector *v) {
  return v->x*v->x + v->y*v->y + v->z*v->z;
}

static inline void vscale(vector *v, float scale) {
  v->x *= scale;
  v->y *= scale;
  v->z *= scale;
}

// Rotates the vector around the z-axis by the given angle theta.
static inline void vyaw(vector *v, float yaw) {
  float ox = v->x;
  v->x = (v->x)*cosf(yaw) - (v->y)*sinf(yaw);
  v->y = (ox)*sinf(yaw) + (v->y)*cosf(yaw);
}

// Rotates the vector around its perpendicular in the x-y plane by the given
// angle phi.
static inline void vpitch(vector *v, float pitch) {
  float r = vmag(v);
  float rxy = sqrtf(v->x*v->x + v->y*v->y);
  float theta = atan2(v->y, v->x);
  float phi = atan2(v->z, rxy);
  v->x = (r * cosf(theta)) * cosf(phi);
  v->y = (r * sinf(theta)) * cosf(phi);
  v->z = r * sinf(phi);
}

// Overwrites the given vector with all zeroes.
static inline void vzero(vector *v) {
  v->x = 0;
  v->y = 0;
  v->z = 0;
}

// Overwrites the given vector with a normal vector facing in the given
// direction. North is yaw=0, pitch=0 is horizontal.
static inline void vface(vector *v, float yaw, float pitch) {
  v->x = -sinf(yaw) * cosf(pitch);
  v->y = cosf(yaw) * cosf(pitch);
  v->z = sinf(pitch);
}

static inline void vadd(vector *target, const vector *value) {
  target->x += value->x;
  target->y += value->y;
  target->z += value->z;
}

static inline void vadd_scaled(
  vector *target,
  const vector *value,
  float scale
) {
  target->x += value->x * scale;
  target->y += value->y * scale;
  target->z += value->z * scale;
}

static inline void vsub(vector *target, const vector *value) {
  target->x -= value->x;
  target->y -= value->y;
  target->z -= value->z;
}

static inline void vcopy(vector *target, const vector *value) {
  target->x = value->x;
  target->y = value->y;
  target->z = value->z;
}

static inline int vdot(const vector *first, const vector *second) {
  return (
    (first->x * second->x) +
    (first->y * second->y) +
    (first->z * second->z)
  );
}

/*************
 * Functions *
 *************/

#endif //ifndef VECTOR_H
