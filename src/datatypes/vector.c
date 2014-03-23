// vector.c
// 3D floating point vectors.

#include "vector.h"

/*************
 * Constants *
 *************/

const vector     V_UP = { .x=  0.0, .y=  0.0, .z=  1.0 };
const vector   V_DOWN = { .x=  0.0, .y=  0.0, .z= -1.0 };
const vector  V_NORTH = { .x=  0.0, .y=  1.0, .z=  0.0 };
const vector  V_SOUTH = { .x=  0.0, .y= -1.0, .z=  0.0 };
const vector   V_EAST = { .x=  1.0, .y=  0.0, .z=  0.0 };
const vector   V_WEST = { .x= -1.0, .y=  0.0, .z=  0.0 };

/*************
 * Functions *
 *************/
