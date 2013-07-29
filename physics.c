// physics.c
// Physical simulation.

#include <math.h>

// DEBUG:
#include <stdio.h>

#include "octree.h"
#include "vector.h"
#include "tick.h"
#include "world.h"
#include "entities.h"
#include "physics.h"
#include "ctl.h"

/*************
 * Constants *
 *************/

const float TARGET_RESOLUTION = 1.0/180.0;

const float BOUNCE_DISTANCE = 0.0005;

const float MIN_VELOCITY = 0.05;

const move_flag MF_ON_GROUND = 0x01;
const move_flag MF_IN_LIQUID = 0x02;
const move_flag MF_CROUCHING = 0x04;

/***********
 * Globals *
 ***********/

float GRAVITY = 20.0;

float AIR_DRAG = 0.985;
float GROUND_DRAG = 0.90;
float LIQUID_DRAG = 0.84;

float CROUCH_COEFFICIENT = 0.3;
float STRAFE_COEFFICIENT = 0.7;
float BACKUP_COEFFICIENT = 0.4;

// Initially we're at 1:1 time.
float DT = SECONDS_PER_TICK;
float SUB_DT = 0;

int SUBSTEPS = 2;

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
  frame_pos *min,
  frame_pos *max,
  vector *increment
) {
  int next_block;
  frame_pos pos;
  if (increment->x > 0) {
    next_block = fastfloor(e->box.max.x + increment->x);
    if (next_block != max->x) {
      pos.x = next_block;
      for (pos.y = min->y; pos.y <= max->y; ++pos.y) {
        for (pos.z = min->z; pos.z <= max->z; ++pos.z) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.x = 0;
            increment->x = 0;
            e->pos.x = next_block - (BOUNCE_DISTANCE + e->size.x / 2.0);
            goto done_x;
          }
        }
      }
      max->x = next_block;
    }
  } else {
    next_block = fastfloor(e->box.min.x + increment->x);
    if (next_block != min->x) {
      pos.x = next_block;
      for (pos.y = min->y; pos.y <= max->y; ++pos.y) {
        for (pos.z = min->z; pos.z <= max->z; ++pos.z) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.x = 0;
            increment->x = 0;
            e->pos.x = next_block + 1 + BOUNCE_DISTANCE + e->size.x / 2.0;
            goto done_x;
          }
        }
      }
      min->x = next_block;
    }
  }
done_x:
  e->pos.x += increment->x;
}

static inline void update_position_y(
  entity *e,
  frame_pos *min,
  frame_pos *max,
  vector *increment
) {
  int next_block;
  frame_pos pos;
  if (increment->y > 0) {
    next_block = fastfloor(e->box.max.y + increment->y);
    if (next_block != max->y) {
      pos.y = next_block;
      for (pos.x = min->x; pos.x <= max->x; ++pos.x) {
        for (pos.z = min->z; pos.z <= max->z; ++pos.z) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.y = 0;
            increment->y = 0;
            e->pos.y = next_block - (BOUNCE_DISTANCE + e->size.y / 2.0);
            goto done_y;
          }
        }
      }
      max->y = next_block;
    }
  } else {
    next_block = fastfloor(e->box.min.y + increment->y);
    if (next_block != min->y) {
      pos.y = next_block;
      for (pos.x = min->x; pos.x <= max->x; ++pos.x) {
        for (pos.z = min->z; pos.z <= max->z; ++pos.z) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.y = 0;
            increment->y = 0;
            e->pos.y = next_block + 1 + BOUNCE_DISTANCE + e->size.y / 2.0;
            goto done_y;
          }
        }
      }
      min->y = next_block;
    }
  }
done_y:
  e->pos.y += increment->y;
}

static inline void update_position_z(
  entity *e,
  frame_pos *min,
  frame_pos *max,
  vector *increment
) {
  int next_block;
  frame_pos pos;
  if (increment->z > 0) {
    next_block = fastfloor(e->box.max.z + increment->z);
    if (next_block != max->z) {
      pos.z = next_block;
      for (pos.x = min->x; pos.x <= max->x; ++pos.x) {
        for (pos.y = min->y; pos.y <= max->y; ++pos.y) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.z = 0;
            increment->z = 0;
            e->pos.z = next_block - (BOUNCE_DISTANCE + e->size.z / 2.0);
            goto done_z;
          }
        }
      }
      max->z = next_block;
    }
  } else {
    next_block = fastfloor(e->box.min.z + increment->z);
    if (next_block != min->z) {
      pos.z = next_block;
      for (pos.x = min->x; pos.x <= max->x; ++pos.x) {
        for (pos.y = min->y; pos.y <= max->y; ++pos.y) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.z = 0;
            increment->z = 0;
            e->pos.z = min->z + BOUNCE_DISTANCE + e->size.z / 2.0;
            goto done_z;
          }
        }
      }
      min->z = next_block;
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
  vector forward, right, v;
  vface(&forward, e->yaw, 0);
  vface(&right, e->yaw - M_PI_2, 0);
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
  backup = (
    (e->control.y < 0) * BACKUP_COEFFICIENT +
    (1 - e->control.y >= 0)
  );
  vadd_scaled(&v, &forward, e->control.y * backup);
  vadd_scaled(&v, &right, e->control.x * STRAFE_COEFFICIENT);
  vadd_scaled(&v, &V_UP, e->control.z);
  vnorm(&v);
  vadd_scaled(&(e->impulse), &v, base);
}

// Updates an entity's position while respecting solid blocks.
static inline void update_position_collide_blocks(entity *e) {
  frame_pos min, max; // min/max block positions
  vector increment; // the increment vector
  // fill in min/max coords
  min.x = b_i_min_x(e->box); max.x = b_i_max_x(e->box);
  min.y = b_i_min_y(e->box); max.y = b_i_max_y(e->box);
  min.z = b_i_min_z(e->box); max.z = b_i_max_z(e->box);
  // compute increment
  vcopy(&increment, &(e->vel));
  vscale(&increment, SUB_DT);
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
}

static inline void check_move_flags(entity *e) {
  frame_pos pos;
  // MF_ON_GROUND
  clear_on_ground(e);
  if (e->vel.z <= 0) {
    pos.z = fastfloor(e->box.min.z - BOUNCE_DISTANCE*2.0);
    for (
      pos.x = b_i_min_x(e->box);
      pos.x <= b_i_max_x(e->box) && !on_ground(e);
      ++pos.x
    ) {
      for (
        pos.y = b_i_min_y(e->box);
        pos.y <= b_i_max_y(e->box) && !on_ground(e);
        ++pos.y
      ) {
        if (is_solid(block_at(e->fr, pos))) {
          set_on_ground(e);
        }
      }
    }
  }
  // MF_IN_LIQUID
  clear_in_liquid(e);
  for (
    pos.x = b_i_min_x(e->box);
    pos.x <= b_i_max_x(e->box) && !in_liquid(e);
    ++pos.x
  ) {
    for (
      pos.y = b_i_min_y(e->box);
      pos.y <= b_i_max_y(e->box) && !in_liquid(e);
      ++pos.y
    ) {
      for (
        pos.z = b_i_min_z(e->box);
        pos.z <= b_i_max_z(e->box) && !in_liquid(e);
        ++pos.z
      ) {
        if (is_liquid(block_at(e->fr, pos))) {
          set_in_liquid(e);
        }
      }
    }
  }
  // MF_CROUCHING handled in ctl.c
}

/*************
 * Functions *
 *************/

void adjust_resolution(void) {
  SUBSTEPS = (int) (DT / TARGET_RESOLUTION) & 1;
  SUB_DT = DT / SUBSTEPS;
}

void tick_physics(entity *e) {
  float drag;
  vector acceleration;
  // Recompute our movement flags:
  check_move_flags(e);
  // Get control impulse:
  integrate_control_inputs(e);
  // Integrate kinetics:
  acceleration.x = e->impulse.x / e->mass;
  acceleration.y = e->impulse.y / e->mass;
  acceleration.z = e->impulse.z / e->mass;
  if (in_liquid(e)) {
    acceleration.z -= GRAVITY * (1 - e->buoyancy);
  } else {
    acceleration.z -= GRAVITY;
  }
  // Resolve entity collisions:
  resolve_entity_collisions(e);
  // Integrate acceleration into velocity:
  vadd_scaled(&(e->vel), &acceleration, SUB_DT);
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
}
