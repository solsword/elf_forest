#ifndef PHYSICS_H
#define PHYSICS_H

// physics.h
// Physical simulation.

#include "octree.h"
#include "vector.h"
#include "world.h"
#include "entities.h"

/*************
 * Constants *
 *************/

// The desired time per integration tick.
extern const float TARGET_RESOLUTION;

// How far from a block an entity should be placed when sliding along it.
extern const float BOUNCE_DISTANCE;

// Minimum velocity: any nonzero velocity will be scaled to at least this
// number.
extern const float MIN_VELOCITY;

// Movement flags:
extern const move_flag MF_ON_GROUND;
extern const move_flag MF_IN_LIQUID;
extern const move_flag MF_CROUCHING;

/***********
 * Globals *
 ***********/

extern float GRAVITY;

// Drag forces:
extern float AIR_DRAG;
extern float GROUND_DRAG;
extern float LIQUID_DRAG;

// Motion coefficients:
extern float CROUCH_COEFFICIENT;
extern float STRAFE_COEFFICIENT;
extern float BACKUP_COEFFICIENT;

// The amount of time per timestep.
extern float DT;
// The amount of time per substep.
extern float SUB_DT;

// The number of full simulation substeps per tick.
extern int SUBSTEPS;

/*************************
 * Structure Definitions *
 *************************/

// TODO: Any of these?

/********************
 * Inline Functions *
 ********************/

static inline int on_ground(entity *e) {return e->move_flags & MF_ON_GROUND;}
static inline void set_on_ground(entity *e) {e->move_flags |= MF_ON_GROUND;}
static inline void clear_on_ground(entity *e) {e->move_flags &= ~MF_ON_GROUND;}

static inline int in_liquid(entity *e) {return e->move_flags & MF_IN_LIQUID;}
static inline void set_in_liquid(entity *e) {e->move_flags |= MF_IN_LIQUID;}
static inline void clear_in_liquid(entity *e) {e->move_flags &= ~MF_IN_LIQUID;}

static inline int is_crouching(entity *e) {return e->move_flags & MF_CROUCHING;}
static inline void set_crouching(entity *e) {e->move_flags |= MF_CROUCHING;}
static inline void clear_crouching(entity *e) {e->move_flags &= ~MF_CROUCHING;}

/*************
 * Functions *
 *************/

void adjust_resolution(void);

void tick_physics(entity *e);

#endif //ifndef PHYSICS_H
