#ifndef PHYSICS_H
#define PHYSICS_H

// physics.h
// Physical simulation.

#include "octree.h"
#include "vector.h"
#include "world.h"
#include "entities.h"

/**************
 * Structures *
 **************/

// TODO: Any of these?

/*************
 * Constants *
 *************/

// The desired time per integration tick.
extern const float TARGET_RESOLUTION;

// How far entity bounding boxes should extend into blocks they're walking on.
extern const float TREAD_DEPTH;

// How far from a block an entity should be placed when sliding along it.
extern const float BOUNCE_DISTANCE;

// The size of the corner bevel for block bounding boxes.
extern const float CORNER_BEVEL;

/***********
 * Globals *
 ***********/

extern float GRAVITY;

// Fractional momentum loss:
extern float DRAG;

// The amount of time per timestep.
extern float DT;

// The number of simulation substeps per tick.
extern int SUBSTEPS;

/*************************
 * Structure Definitions *
 *************************/

// TODO: Any of these?

/*************
 * Functions *
 *************/

void tick_physics(entity *e);

#endif //ifndef PHYSICS_H
