#ifndef PHYSICS_H
#define PHYSICS_H

// physics.h
// Physical simulation.

#include "datatypes/octree.h"
#include "datatypes/vector.h"
#include "world/world.h"
#include "world/entities.h"

/*************
 * Constants *
 *************/

// The desired time per integration tick.
extern float const TARGET_RESOLUTION;

// How far from a block an entity should be placed when sliding along it.
extern float const BOUNCE_DISTANCE;

// How far the player can move in one tick before the movement is considered an
// aberrant "jump" (turn on DEBUG_DETECT_JUMPS to check for these).
extern float const JUMP_DETECTION_THRESHOLD;

// Minimum velocity: any nonzero velocity will be scaled to at least this
// number.
extern float const MIN_VELOCITY;

// Movement flags:
extern move_flag const MF_ON_GROUND;
extern move_flag const MF_IN_LIQUID;
extern move_flag const MF_IN_VOID;
extern move_flag const MF_CROUCHING;

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

static inline int in_void(entity *e) {return e->move_flags & MF_IN_VOID;}
static inline void set_in_void(entity *e) {e->move_flags |= MF_IN_VOID;}
static inline void clear_in_void(entity *e) {e->move_flags &= ~MF_IN_VOID;}

static inline int is_crouching(entity *e) {return e->move_flags & MF_CROUCHING;}
static inline void set_crouching(entity *e) {e->move_flags |= MF_CROUCHING;}
static inline void clear_crouching(entity *e) {e->move_flags &= ~MF_CROUCHING;}

/*************
 * Functions *
 *************/

void adjust_physics_resolution(void);

void tick_physics(entity *e);

#endif //ifndef PHYSICS_H
