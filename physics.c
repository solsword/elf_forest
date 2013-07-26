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

/*************
 * Constants *
 *************/

const float TARGET_RESOLUTION = 1.0/180.0;

const float TREAD_DEPTH = 0.001;

const float BOUNCE_DISTANCE = 0.0005;

/***********
 * Globals *
 ***********/

float GRAVITY = 20.0;

float DRAG = 0.985;

// Initially we're at 1:1 time.
float DT = SECONDS_PER_TICK;

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

// Updates an entity's position while respecting solid blocks.
static inline void update_position_collide_blocks(entity *e, float dt) {
  frame_pos min, max; // min/max block positions
  vector increment; // the increment vector
  // fill in min/max coords
  min.x = b_i_min_x(e->box); max.x = b_i_max_x(e->box);
  min.y = b_i_min_y(e->box); max.y = b_i_max_y(e->box);
  min.z = b_i_min_z(e->box); max.z = b_i_max_z(e->box);
  // compute increment
  vcopy(&increment, &(e->vel));
  vscale(&increment, dt);
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

static inline void adjust_resolution(void) {
  SUBSTEPS = (int) (DT / TARGET_RESOLUTION) & 1;
}

/*************
 * Functions *
 *************/

void tick_physics(entity *e) {
  adjust_resolution();
  float dt = DT/SUBSTEPS;
  vector acceleration;
  // Integrate kinetics:
  acceleration.x = e->impulse.x / e->mass;
  acceleration.y = e->impulse.y / e->mass;
  acceleration.z = e->impulse.z / e->mass;
  acceleration.z -= GRAVITY;
  vadd_scaled(&(e->vel), &acceleration, dt);
  e->vel.x *= DRAG;
  e->vel.y *= DRAG;
  update_position_collide_blocks(e, dt);
  check_on_ground(e);
  resolve_entity_collisions(e);
  // TODO: collision detection & resolution
  // Recompute the entity's bounding box:
  compute_bb(e);
  // Reset impulse:
  vzero(&(e->impulse));
}
