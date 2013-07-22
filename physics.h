#ifndef PHYSICS_H
#define PHYSICS_H

// physics.h
// Physical simulation.

/**************
 * Structures *
 **************/

// A vector of 3 floats.
struct vector_s;
typedef struct vector_s vector;

/*************************
 * Structure Definitions *
 *************************/

struct vector_s {
  float x, y, z;
};

#endif //ifndef PHYSICS_H
