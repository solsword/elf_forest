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

// Updates an entity's position while respecting solid blocks.
static inline void update_position_collide_blocks(entity *e, float dt) {
  int nx, ny, nz, xx, xy, xz; // current min/max integer coords
  int nb; // the last and next block coordinates
  frame_pos pos; // block position to inspect
  vector increment; // the increment vector
  vcopy(&increment, &(e->vel));
  vscale(&increment, dt);
  nx = b_i_min_x(e->box); xx = b_i_max_x(e->box);
  ny = b_i_min_y(e->box); xy = b_i_max_y(e->box);
  nz = b_i_min_z(e->box); xz = b_i_max_z(e->box);
  // first update z:
  if (increment.z > 0) {
    nb = fastfloor(e->box.max.z + increment.z);
    if (nb != xz) {
      pos.z = nb;
      for (pos.x = nx; pos.x <= xx; ++pos.x) {
        for (pos.y = ny; pos.y <= xy; ++pos.y) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.z = 0;
            increment.z = 0;
            e->pos.z = nb - (BOUNCE_DISTANCE + e->size.z / 2.0);
            goto done_z;
          }
        }
      }
    }
  } else {
    nb = fastfloor(e->box.min.z + increment.z);
    if (nb != nz) {
      pos.z = nb;
      for (pos.x = nx; pos.x <= xx; ++pos.x) {
        for (pos.y = ny; pos.y <= xy; ++pos.y) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.z = 0;
            increment.z = 0;
            e->pos.z = nz + BOUNCE_DISTANCE + e->size.z / 2.0;
            e->on_ground = 1;
            goto done_z;
          }
        }
      }
    }
  }
done_z:
  e->pos.z += increment.z;
  // next update x:
  if (increment.x > 0) {
    nb = fastfloor(e->box.max.x + increment.x);
    if (nb != xx) {
      pos.x = nb;
      for (pos.y = ny; pos.y <= xy; ++pos.y) {
        for (pos.z = nz; pos.z <= xz; ++pos.z) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.x = 0;
            increment.x = 0;
            e->pos.x = nb - (BOUNCE_DISTANCE + e->size.x / 2.0);
            goto done_x;
          }
        }
      }
    }
  } else {
    nb = fastfloor(e->box.min.x + increment.x);
    if (nb != nx) {
      pos.x = nb;
      for (pos.y = ny; pos.y <= xy; ++pos.y) {
        for (pos.z = nz; pos.z <= xz; ++pos.z) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.x = 0;
            increment.x = 0;
            e->pos.x = nb + 1 + BOUNCE_DISTANCE + e->size.x / 2.0;
            goto done_x;
          }
        }
      }
    }
  }
done_x:
  e->pos.x += increment.x;
  // and finally y:
  if (increment.y > 0) {
    nb = fastfloor(e->box.max.y + increment.y);
    if (nb != xy) {
      pos.y = nb;
      for (pos.x = nx; pos.x <= xx; ++pos.x) {
        for (pos.z = nz; pos.z <= xz; ++pos.z) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.y = 0;
            increment.y = 0;
            e->pos.y = nb - (BOUNCE_DISTANCE + e->size.y / 2.0);
            goto done_y;
          }
        }
      }
    }
  } else {
    nb = fastfloor(e->box.min.y + increment.y);
    if (nb != ny) {
      pos.y = nb;
      for (pos.x = nx; pos.x <= xx; ++pos.x) {
        for (pos.z = nz; pos.z <= xz; ++pos.z) {
          if (is_solid(block_at(e->fr, pos))) {
            e->vel.y = 0;
            increment.y = 0;
            e->pos.y = nb + 1 + BOUNCE_DISTANCE + e->size.y / 2.0;
            goto done_y;
          }
        }
      }
    }
  }
done_y:
  e->pos.y += increment.y;
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
  resolve_entity_collisions(e);
  // TODO: collision detection & resolution
  // Recompute the entity's bounding box:
  compute_bb(e);
  // Reset impulse:
  vzero(&(e->impulse));
}
