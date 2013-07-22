#ifndef VECTOR_H
#define VECTOR_H

// vector.h
// 3D floating point vectors.

#include "world.h"
#include "list.h"
#include "conv.h"

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

// Converts a vector v to a frame position using the floor of the vector's
// coords.
static inline void vec__fpos(vector *v, frame_pos *pos) {
  pos->x = fastfloor(v->x);
  pos->y = fastfloor(v->y);
  pos->z = fastfloor(v->z);
}

// Converts a frame position to a vector.
static inline void fpos__vec(frame_pos *pos, vector *v) {
  v->x = (float) pos->x;
  v->y = (float) pos->y;
  v->z = (float) pos->z;
}

/*************
 * Functions *
 *************/

#endif //ifndef VECTOR_H
