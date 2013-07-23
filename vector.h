#ifndef VECTOR_H
#define VECTOR_H

// vector.h
// 3D floating point vectors.

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

/*************
 * Functions *
 *************/

#endif //ifndef VECTOR_H
