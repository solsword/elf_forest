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

/***********
 * Globals *
 ***********/

float GRAVITY = 20.0;

float AIR_DRAG = 0.9995;
float GROUND_DRAG = 0.85;

float AIR_CONTROL = 0.5;

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
  float prx, pry; // proposed x/y
  float vm2, pm2, w, w2; // squared velocity/proposed/walk magnitudes
  float limit; // the velocity limit
  float scale; // the scaling factor
  prx = e->vel.x; pry = e->vel.y;
  prx += e->control.x*SUB_DT; pry += e->control.y*SUB_DT;
  vm2 = (e->vel.x * e->vel.x) + (e->vel.y * e->vel.y);
  pm2 = prx*prx + pry*pry;
  w = e->walk * AIR_CONTROL;
  w2 = w * w;
  if (pm2 < vm2 || pm2 < w2) {
    e->vel.x = prx;
    e->vel.y = pry;
    e->vel.z = e->vel.z + e->control.z*SUB_DT;
  } else {
    limit = vm2 > w2 ? sqrtf(vm2) : w;
    scale = limit/sqrtf(pm2);
    prx *= scale;
    pry *= scale;
    e->vel.x = prx;
    e->vel.y = pry;
    e->vel.z = e->vel.z + e->control.z*SUB_DT;
  }
  vzero(&(e->control));
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
  // first update z:
  update_position_z(e, &min, &max, &increment);
}

static inline void check_on_ground(entity *e) {
  frame_pos pos;
  e->on_ground = 0;
  if (e->vel.z > 0) {
    return;
  }
  pos.z = fastfloor(e->box.min.z - BOUNCE_DISTANCE*2.0);
  for (
    pos.x = b_i_min_x(e->box);
    pos.x <= b_i_max_x(e->box) && !e->on_ground;
    ++pos.x
  ) {
    for (
      pos.y = b_i_min_y(e->box);
      pos.y <= b_i_max_y(e->box) && !e->on_ground;
      ++pos.y
    ) {
      if (is_solid(block_at(e->fr, pos))) {
        e->on_ground = 1;
      }
    }
  }
}

/*************
 * Functions *
 *************/

void adjust_resolution(void) {
  SUBSTEPS = (int) (DT / TARGET_RESOLUTION) & 1;
  SUB_DT = DT / SUBSTEPS;
}

void tick_physics(entity *e) {
  vector acceleration;
  // Integrate kinetics:
  acceleration.x = e->impulse.x / e->mass;
  acceleration.y = e->impulse.y / e->mass;
  acceleration.z = e->impulse.z / e->mass;
  acceleration.z -= GRAVITY;
  resolve_entity_collisions(e);
  vadd_scaled(&(e->vel), &acceleration, SUB_DT);
  e->vel.x *= (e->on_ground)*GROUND_DRAG + (1 - e->on_ground)*AIR_DRAG;
  e->vel.y *= (e->on_ground)*GROUND_DRAG + (1 - e->on_ground)*AIR_DRAG;
  check_on_ground(e);
  // No drag for z.
  integrate_control_inputs(e);
  update_position_collide_blocks(e);
  // TODO: collision detection & resolution
  // Recompute the entity's bounding box:
  compute_bb(e);
  // Reset impulse:
  vzero(&(e->impulse));
}
