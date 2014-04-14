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
extern float const PHYS_TARGET_RESOLUTION;

// The width of an "impulse" in seconds (very approximately used).
extern float const IMPULSE_WIDTH;

// How far from a cell an entity should be placed when sliding along it.
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
extern move_flag const MF_DO_JUMP;
extern move_flag const MF_DO_FLAP;

/***********
 * Globals *
 ***********/

extern float GRAVITY;

// Drag forces:
extern float AIR_DRAG;
extern float GROUND_DRAG;
extern float LIQUID_DRAG;

// Damping to apply when no control inputs are detected:
extern float NEUTRAL_CONTROL_DAMPING;

// Motion coefficients:
extern float CROUCH_COEFFICIENT;
extern float STRAFE_COEFFICIENT;
extern float BACKUP_COEFFICIENT;

// How much backward leaps are exaggerated:
extern float LEAP_BACK_RATIO;

// The square of the leap cutoff speed:
extern float LEAP_CUTOFF_SPEED_SQ;

// The amount of time per timestep.
extern float PHYS_DT;
// The amount of time per substep.
extern float PHYS_SUB_DT;

// The number of full simulation substeps per tick.
extern int PHYS_SUBSTEPS;

// Callbacks at the start and end of each physics tick for each entity:
typedef void (*physics_callback)(entity *);
extern physics_callback PHYS_SUBSTEP_START_CALLBACK;
extern physics_callback PHYS_SUBSTEP_END_CALLBACK;

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

static inline int do_jump(entity *e) {return e->move_flags & MF_DO_JUMP;}
static inline void set_do_jump(entity *e) {e->move_flags |= MF_DO_JUMP;}
static inline void clear_do_jump(entity *e) {e->move_flags &= ~MF_DO_JUMP;}

static inline int do_flap(entity *e) {return e->move_flags & MF_DO_FLAP;}
static inline void set_do_flap(entity *e) {e->move_flags |= MF_DO_FLAP;}
static inline void clear_do_flap(entity *e) {e->move_flags &= ~MF_DO_FLAP;}

static inline int is_airborne(entity *e) {
  return !on_ground(e) && !in_liquid(e);
}

static inline void add_impulse(entity *e, vector *impulse) {
  vscale(impulse, IMPULSE_WIDTH/(PHYS_SUB_DT));
  vadd(&(e->impulse), impulse);
}

/*************
 * Functions *
 *************/

// Compute the number of substeps and the time/substep (PHYS_SUBSTEPS and
// PHYS_SUB_DT) required to achieve the desired temporal resolution
// (PHYS_TARGET_RESOLUTION) given the time that each tick represents (PHYS_DT).
void adjust_physics_resolution(void);

// Update physics for the given entity:
void tick_physics(entity *e);

#endif //ifndef PHYSICS_H
