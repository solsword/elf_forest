// physics.c
// Physical simulation.

#include <math.h>

// DEBUG:
#include <stdio.h>

#include "datatypes/octree.h"
#include "datatypes/vector.h"
#include "tick/tick.h"
#include "physics/physics.h"
#include "control/ctl.h"
#include "world/world.h"
#include "world/entities.h"

/*************
 * Constants *
 *************/

float const PHYS_TARGET_RESOLUTION = 1.0/180.0;

float const IMPULSE_WIDTH = 1.0/30.0;

float const BOUNCE_DISTANCE = 0.0005;

float const JUMP_DETECTION_THRESHOLD = 1.3;

float const MIN_VELOCITY = 0.05;

move_flag const MF_ON_GROUND = 0x01;
move_flag const MF_IN_LIQUID = 0x02;
move_flag const   MF_IN_VOID = 0x04;
move_flag const MF_CROUCHING = 0x08;
move_flag const   MF_DO_JUMP = 0x10;
move_flag const   MF_DO_FLAP = 0x20;

/***********
 * Globals *
 ***********/

// TODO: Good value for this?
//float GRAVITY = 40.0;
float GRAVITY = 60.0;

// TODO: Good values for these?
/* Values at SUB_DT=1/60.0
float AIR_DRAG = 0.985;
float GROUND_DRAG = 0.90;
float LIQUID_DRAG = 0.87;
// */
float AIR_DRAG = 0.995;
float GROUND_DRAG = 0.965;
float LIQUID_DRAG = 0.92;

float NEUTRAL_CONTROL_DAMPING = 0.9;

float CROUCH_COEFFICIENT = 0.3;
float STRAFE_COEFFICIENT = 0.7;
float BACKUP_COEFFICIENT = 0.4;

float LEAP_BACK_RATIO = 3.0;
float LEAP_CUTOFF_SPEED_SQ = 36.0;

// Initially we're at 1:1 time.
float PHYS_DT = SECONDS_PER_TICK;
float PHYS_SUB_DT = 0;

int PHYS_SUBSTEPS = 1;

physics_callback PHYS_SUBSTEP_START_CALLBACK = NULL;
physics_callback PHYS_SUBSTEP_END_CALLBACK = NULL;

/********************
 * Inline Functions *
 ********************/

// "Resolves" entity collisions by jostling: each collision imparts some
// arbitrary force.
static inline void resolve_entity_collisions(entity *e) {
  // TODO: This function.
}

static inline void update_position_x (
  entity *e,
  region_pos *min,
  region_pos *max,
  vector *increment
) {
  r_pos_t next_block;
  region_pos pos;
  if (increment->x > 0) {
    next_block = fastfloor(e->box.max.x + increment->x);
    if (next_block != max->x) {
      pos.x = e->area->origin.x + next_block;
      for (pos.y = min->y; pos.y <= max->y; ++pos.y) {
        for (pos.z = min->z; pos.z <= max->z; ++pos.z) {
          if (is_solid(block_at(&pos))) {
            e->vel.x = 0;
            increment->x = 0;
            e->pos.x = next_block - (BOUNCE_DISTANCE + e->size.x / 2.0);
            goto done_x;
          }
        }
      }
      max->x = e->area->origin.x + next_block;
    }
  } else {
    next_block = fastfloor(e->box.min.x + increment->x);
    if (next_block != min->x) {
      pos.x = e->area->origin.x + next_block;
      for (pos.y = min->y; pos.y <= max->y; ++pos.y) {
        for (pos.z = min->z; pos.z <= max->z; ++pos.z) {
          if (is_solid(block_at(&pos))) {
            e->vel.x = 0;
            increment->x = 0;
            e->pos.x = next_block + 1 + BOUNCE_DISTANCE + e->size.x / 2.0;
            goto done_x;
          }
        }
      }
      min->x = e->area->origin.x + next_block;
    }
  }
done_x:
  e->pos.x += increment->x;
}

static inline void update_position_y(
  entity *e,
  region_pos *min,
  region_pos *max,
  vector *increment
) {
  r_pos_t next_block;
  region_pos pos;
  if (increment->y > 0) {
    next_block = fastfloor(e->box.max.y + increment->y);
    if (next_block != max->y) {
      pos.y = e->area->origin.y + next_block;
      for (pos.x = min->x; pos.x <= max->x; ++pos.x) {
        for (pos.z = min->z; pos.z <= max->z; ++pos.z) {
          if (is_solid(block_at(&pos))) {
            e->vel.y = 0;
            increment->y = 0;
            e->pos.y = next_block - (BOUNCE_DISTANCE + e->size.y / 2.0);
            goto done_y;
          }
        }
      }
      max->y = e->area->origin.y + next_block;
    }
  } else {
    next_block = fastfloor(e->box.min.y + increment->y);
    if (next_block != min->y) {
      pos.y = e->area->origin.y + next_block;
      for (pos.x = min->x; pos.x <= max->x; ++pos.x) {
        for (pos.z = min->z; pos.z <= max->z; ++pos.z) {
          if (is_solid(block_at(&pos))) {
            e->vel.y = 0;
            increment->y = 0;
            e->pos.y = next_block + 1 + BOUNCE_DISTANCE + e->size.y / 2.0;
            goto done_y;
          }
        }
      }
      min->y = e->area->origin.y + next_block;
    }
  }
done_y:
  e->pos.y += increment->y;
}

static inline void update_position_z(
  entity *e,
  region_pos *min,
  region_pos *max,
  vector *increment
) {
  r_pos_t next_block;
  region_pos pos;
  if (increment->z > 0) {
    next_block = fastfloor(e->box.max.z + increment->z);
    if (next_block != max->z) {
      pos.z = e->area->origin.z + next_block;
      for (pos.x = min->x; pos.x <= max->x; ++pos.x) {
        for (pos.y = min->y; pos.y <= max->y; ++pos.y) {
          if (is_solid(block_at(&pos))) {
            e->vel.z = 0;
            increment->z = 0;
            e->pos.z = next_block - (BOUNCE_DISTANCE + e->size.z / 2.0);
            goto done_z;
          }
        }
      }
      max->z = e->area->origin.z + next_block;
    }
  } else {
    next_block = fastfloor(e->box.min.z + increment->z);
    if (next_block != min->z) {
      pos.z = e->area->origin.z + next_block;
      for (pos.x = min->x; pos.x <= max->x; ++pos.x) {
        for (pos.y = min->y; pos.y <= max->y; ++pos.y) {
          if (is_solid(block_at(&pos))) {
            e->vel.z = 0;
            increment->z = 0;
            e->pos.z = next_block + 1 + (BOUNCE_DISTANCE + e->size.z / 2.0);
            goto done_z;
          }
        }
      }
      min->z = e->area->origin.z + next_block;
    }
  }
done_z:
  e->pos.z += increment->z;
}

// Edits the entity's velocity according to control inputs. Note that we only
// constrain result velocity in two dimensions. Also note that a lack of
// control inputs translates to active velocity damping.
static inline void integrate_control_inputs(entity *e) {
  float base = e->walk;
  float backup = 1.0;
  float leapscale = 1.0;
  vector forward, right, cdir;
  vector v = { .x = 0, .y = 0, .z = 0 };
  vector vjump = { .x = 0, .y = 0, .z = 0 };
  vector vleap = { .x = 0, .y = 0, .z = 0 };

  // Normal control inputs that accumulate every frame:
  vface(&forward, e->yaw, 0);
  vface(&right, e->yaw - M_PI_2, 0);
  vcopy(&cdir, &(e->control));
  vnorm(&cdir);
  if (in_liquid(e)) {
    base = e->swim;
  } else {
    if (!on_ground(e)) {
      base = e->fly;
    }
    if (is_crouching(e)) {
      base *= CROUCH_COEFFICIENT;
    }
  }
  vzero(&v);
  backup = (e->control.y < 0) * BACKUP_COEFFICIENT + (e->control.y >= 0);
  vadd_scaled(&v, &forward, e->control.y * backup);
  vadd_scaled(&v, &right, e->control.x * STRAFE_COEFFICIENT);
  vadd_scaled(&v, &V_UP, e->control.z);
  if (vmag2(&v) > 1) { vnorm(&v); }

  // Event control inputs that happen once in a while:
  vcopy(&vjump, &V_UP);
  vcopy(&vleap, &v);
  vleap.z = 0;
  // Jump or flap as appropriate:
  if (do_jump(e)) {
    vscale(&vjump, e->jump);
    vscale(&vleap, e->leap);
    clear_do_jump(e);
  } else if (do_flap(e)) {
    vscale(&vjump, e->flap);
    vscale(&vleap, e->flap);
    clear_do_flap(e);
  } else {
    vscale(&vjump, 0);
    vscale(&vleap, 0);
  }
  // If trying to leap backwards, leap farther to compensate for the backup
  // coefficient:
  if (e->control.y < 0) {
    leapscale = vdot(&cdir, &V_SOUTH);
    leapscale *= leapscale;
    leapscale *= leapscale;
    vscale(&vleap, leapscale * LEAP_BACK_RATIO);
  }
  // If our velocity is too high, don't apply the leap impulse:
  if (do_jump(e) && vmag2(&(e->vel)) > LEAP_CUTOFF_SPEED_SQ) {
    vscale(&vleap, 0);
  }

  // Scale our control vector and add impulses:
  if (vmag2(&v) == 0 && on_ground(e)) {
    e->vel.x *= NEUTRAL_CONTROL_DAMPING;
    e->vel.y *= NEUTRAL_CONTROL_DAMPING;
  } else {
    vscale(&v, base);
  }
  add_impulse(e, &v);
  add_impulse(e, &vleap);
  add_impulse(e, &vjump);
}

// Updates an entity's position while respecting solid blocks.
static inline void update_position_collide_blocks(entity *e) {
  region_pos min, max; // min/max block positions
  vector increment; // the increment vector
  // fill in min/max coords
  e_min__rpos(e, &min);
  e_max__rpos(e, &max);
  // compute increment
  vcopy(&increment, &(e->vel));
  vscale(&increment, PHYS_SUB_DT);
  // Make sure we're getting up-to-date block data from block_at:
  refresh_block_at_cache();

  // DEBUG:
#ifdef DEBUG_DETECT_JUMPS
  vector old_pos, new_pos;
  vcopy(&old_pos, &(e->pos));
#endif

  // Update x/y axes according to the magnitude of their increments:
  if (increment.y > increment.x) {
    update_position_y(e, &min, &max, &increment);
    update_position_x(e, &min, &max, &increment);
  } else {
    update_position_x(e, &min, &max, &increment);
    update_position_y(e, &min, &max, &increment);
  }
  // Update z last:
  update_position_z(e, &min, &max, &increment);

#ifdef DEBUG_DETECT_JUMPS
  // If enabled, check for massive position updates:
  vcopy(&new_pos, &(e->pos));
  double dist = sqrt(
    (new_pos.x - old_pos.x) * (new_pos.x - old_pos.x)
  +
    (new_pos.y - old_pos.y) * (new_pos.y - old_pos.y)
  +
    (new_pos.z - old_pos.z) * (new_pos.z - old_pos.z)
  );
  if (dist > JUMP_DETECTION_THRESHOLD) {
    printf("position jumped! %f\n", dist);
    printf("increment: %f,  %f,  %f\n", increment.x, increment.y, increment.z);
  }
#endif

  // Try to avoid poisoning anyone else who might use block_at:
  refresh_block_at_cache();
}

static inline void check_move_flags(entity *e) {
  region_pos pos;
  // Avoid getting stale block data:
  refresh_block_at_cache();
  // MF_ON_GROUND
  clear_on_ground(e);
  if (e->vel.z <= 0) {
    pos.z = e->area->origin.z + fastfloor(e->box.min.z - BOUNCE_DISTANCE*2.0);
    for (
      pos.x = e_rp_min_x(e);
      pos.x <= e_rp_max_x(e) && !on_ground(e);
      ++pos.x
    ) {
      for (
        pos.y = e_rp_min_y(e);
        pos.y <= e_rp_max_y(e) && !on_ground(e);
        ++pos.y
      ) {
        if (is_solid(block_at(&pos))) {
          set_on_ground(e);
        }
      }
    }
  }
  // MF_IN_LIQUID
  clear_in_liquid(e);
  for (
    pos.x = e_rp_min_x(e);
    pos.x <= e_rp_max_x(e) && !in_liquid(e);
    ++pos.x
  ) {
    for (
      pos.y = e_rp_min_y(e);
      pos.y <= e_rp_max_y(e) && !in_liquid(e);
      ++pos.y
    ) {
      for (
        pos.z = e_rp_min_z(e);
        pos.z <= e_rp_max_z(e) && !in_liquid(e);
        ++pos.z
      ) {
        if (is_liquid(block_at(&pos))) {
          set_in_liquid(e);
        }
      }
    }
  }
  // MF_IN_VOID
  clear_in_void(e);
  get_head_rpos(e, &pos);
  if (is_void(block_at(&pos))) {
    set_in_void(e);
  }
  // MF_CROUCHING, MF_DO_JUMP, and MF_DO_FLAP handled in ctl.c
  // Avoid letting others get stale block data:
  refresh_block_at_cache();
}

/*************
 * Functions *
 *************/

void adjust_physics_resolution(void) {
  PHYS_SUBSTEPS = (int) (PHYS_DT / PHYS_TARGET_RESOLUTION);
  PHYS_SUB_DT = PHYS_DT / PHYS_SUBSTEPS;
}

void tick_physics(entity *e) {
  float drag;
  vector acceleration;
  // Recompute our movement flags:
  check_move_flags(e);
  // Call the start callback after checking move flags so it can mess with
  // them:
  if (PHYS_SUBSTEP_START_CALLBACK != NULL) {
    (*PHYS_SUBSTEP_START_CALLBACK)(e);
  }
  // If we're in a void, don't move at all:
  /*
  if (in_void(e)) {
    vzero(&(e->impulse));
    return;
  }
  // */
  // Get control impulse:
  integrate_control_inputs(e);
  // Integrate kinetics:
  acceleration.x = e->impulse.x / e->mass;
  acceleration.y = e->impulse.y / e->mass;
  acceleration.z = e->impulse.z / e->mass;
  if (in_liquid(e)) {
    acceleration.z -= GRAVITY * (1 - e->buoyancy);
  } else if (is_airborne(e) && !is_crouching(e)) {
    acceleration.z -= GRAVITY * (1 - e->lift);
  } else {
    acceleration.z -= GRAVITY;
  }
  // Resolve entity collisions:
  resolve_entity_collisions(e);
  // Integrate acceleration into velocity:
  vadd_scaled(&(e->vel), &acceleration, PHYS_SUB_DT);
  // Drag:
  if (in_liquid(e)) {
    drag = LIQUID_DRAG;
  } else if (on_ground(e)) {
    drag = GROUND_DRAG;
  } else {
    drag = AIR_DRAG;
  }
  vscale(&(e->vel), drag);
  // Make sure that if we're accelerating we've got at least a minimum velocity:
  int mvx = fabs(e->vel.x) < MIN_VELOCITY;
  int mvy = fabs(e->vel.y) < MIN_VELOCITY;
  int mvz = fabs(e->vel.z) < MIN_VELOCITY;
  e->vel.x += (mvx && acceleration.x > 0)*MIN_VELOCITY;
  e->vel.x += (mvx && acceleration.x < 0)*-MIN_VELOCITY;
  e->vel.y += (mvy && acceleration.y > 0)*MIN_VELOCITY;
  e->vel.y += (mvy && acceleration.y < 0)*-MIN_VELOCITY;
  e->vel.z += (mvz && acceleration.z > 0)*MIN_VELOCITY;
  e->vel.z += (mvz && acceleration.z < 0)*-MIN_VELOCITY;
  // Update our position while respecting solid blocks:
  update_position_collide_blocks(e);
  // Recompute the entity's bounding box:
  compute_bb(e);
  // Reset impulse:
  vzero(&(e->impulse));
  if (PHYS_SUBSTEP_END_CALLBACK != NULL) {
    (*PHYS_SUBSTEP_END_CALLBACK)(e);
  }
}
