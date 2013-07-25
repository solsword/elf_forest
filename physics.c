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

const float TARGET_RESOLUTION = 1.0/120.0;

const float TREAD_DEPTH = 0.01; // TODO: Test vs bevel.

const float BOUNCE_DISTANCE = 0.005;

const float CORNER_BEVEL = 0.025;

// The axis-aligned directions as bit flags
#define F_ABOVE 0x01
#define F_BELOW 0x02
#define F_NORTH 0x04
#define F_SOUTH 0x08
#define  F_EAST 0x10
#define  F_WEST 0x20

/***********
 * Globals *
 ***********/

float GRAVITY = 9.81;

float DRAG = 0.975;

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

// Returns 1 if the given penetration dimensions indicate a real overlap or 0
// if the overlap falls within the corner bevel.
static inline int exclude_corners(
  const float pd_x,
  const float pd_y,
  const float pd_z
) {
  return (
    (pd_x < CORNER_BEVEL)
  +
    (pd_z < CORNER_BEVEL)
  +
    (pd_y < CORNER_BEVEL)
  ) < 2;
}

// Returns bits indicating which direction the position of the given vector is
// in relative to the given block.
static inline int get_face(const frame_pos *pos, const vector *v) {
  vector d;
  d.x = v->x - pos->x;
  d.y = v->y - pos->y;
  d.z = v->z - pos->z;
  int result = 0xffffff00;
  int t;
  // Build an opposite-map:
  t = (d.x > d.y);
  result |= t * (F_NORTH | F_WEST) + (1 - t) * (F_SOUTH | F_EAST);
  t = (d.x > -d.y);
  result |= t * (F_SOUTH | F_WEST) + (1 - t) * (F_NORTH | F_EAST);
  t = (d.x > d.z);
  result |= t * (F_ABOVE | F_WEST) + (1 - t) * (F_BELOW | F_EAST);
  t = (d.x > -d.z);
  result |= t * (F_BELOW | F_WEST) + (1 - t) * (F_ABOVE | F_EAST);
  t = (d.y > d.z);
  result |= t * (F_ABOVE | F_SOUTH) + (1 - t) * (F_BELOW | F_NORTH);
  t = (d.y > -d.z);
  result |= t * (F_BELOW | F_SOUTH) + (1 - t) * (F_ABOVE | F_NORTH);
  // Negate it:
  return ~result;
}

// Resolves block collisions via strict repositioning and velocity rewriting.
static inline void resolve_block_collisions(entity *e) {
  int nx, ny, nz; // integer min coords
  int xx, xy, xz; // integer max coords
  float pd_x = 0, pd_y = 0, pd_z = 0; // penetration depths
  frame_pos pos; // position of block to test
  frame *f = e->fr; // grab a reference to the frame
  // Look up min/max coords:
  nx = b_i_min_x(e->box); xx = b_i_max_x(e->box);
  ny = b_i_min_y(e->box); xy = b_i_max_y(e->box);
  nz = b_i_min_z(e->box); xz = b_i_max_z(e->box);
  e->on_ground = 0; // Clear our on_ground state
  printf("RBC\n");
  // north/south
  for (pos.x = nx; pos.x < xx; ++pos.x) {
    pd_x = (
      (pos.x == nx) * ((nx + 1) - e->box.min.x)
    +
      (pos.x == xx) * (e->box.max.x - (xx - 1)) 
    +
      (pos.x != nx && pos.x != xx)
    );
          for (pos.z = nz; pos.z < xz; ++pos.z) {
            pd_z = (
              (pos.z == nz) * ((nz + 1) - e->box.min.z)
            +
              (pos.z == xz) * (e->box.max.z - (xz - 1)) 
            +
              (pos.z != nz && pos.z != xz)
            );
              pos.y = ny; // south
              pd_y = ((ny + 1) - e->box.min.y);
              if (
                pd_y > 0
              &&
                exclude_corners(pd_x, pd_y, pd_z)
              &&
                is_solid(block_at(f, pos))
              &&
                get_face(&pos, &(e->pos)) | F_NORTH
              ) {
                printf("rbc: south\n");
                e->box.min.y += pd_y + BOUNCE_DISTANCE;
                e->pos.y += pd_y + BOUNCE_DISTANCE;
                e->vel.y *= (e->vel.y > 0);
              }
              pos.y = xy - 1; // north
              pd_y = (e->box.max.y - (xy - 1));
              if (
                pd_y > 0
              &&
                exclude_corners(pd_x, pd_y, pd_z)
              &&
                is_solid(block_at(f, pos))
              &&
                get_face(&pos, &(e->pos)) | F_SOUTH
              ) {
                printf("rbc: north\n");
                e->box.max.y -= pd_y + BOUNCE_DISTANCE;
                e->pos.y -= pd_y + BOUNCE_DISTANCE;
                e->vel.y *= (e->vel.y < 0);
              }
          }
  }
  // east/west
      for (pos.y = ny; pos.y < xy; ++pos.y) {
          for (pos.z = nz; pos.z < xz; ++pos.z) {
              pos.x = nx; // west
              pd_x = ((nx + 1) - e->box.min.x);
              if (
                pd_x > 0
              &&
                exclude_corners(pd_x, pd_y, pd_z)
              &&
                is_solid(block_at(f, pos))
              &&
                get_face(&pos, &(e->pos)) | F_EAST
              ) {
                printf("rbc: west\n");
                e->box.min.x += pd_x + BOUNCE_DISTANCE;
                e->pos.x += pd_x + BOUNCE_DISTANCE;
                e->vel.x *= (e->vel.x > 0);
              }
              pos.x = xx - 1; // east
              pd_x = (e->box.max.x - (xx - 1));
              if (
                pd_x > 0
              &&
                exclude_corners(pd_x, pd_y, pd_z)
              &&
                is_solid(block_at(f, pos))
              &&
                get_face(&pos, &(e->pos)) | F_WEST
              ) {
                printf("rbc: east\n");
                e->box.max.x -= pd_x + BOUNCE_DISTANCE;
                e->pos.x -= pd_x + BOUNCE_DISTANCE;
                e->vel.x *= (e->vel.x < 0);
              }
          }
      }
  // top/bottom
  for (pos.x = nx; pos.x < xx; ++pos.x) {
      for (pos.y = ny; pos.y < xy; ++pos.y) {
              pos.z = nz; // bot
              pd_z = ((nz + 1) - e->box.min.z);
              pd_z -= TREAD_DEPTH;
              pd_z *= (pd_z > 0);
              if (
                pd_z > TREAD_DEPTH
              &&
                exclude_corners(pd_x, pd_y, pd_z)
              &&
                is_solid(block_at(f, pos))
              &&
                get_face(&pos, &(e->pos)) | F_ABOVE
              ) {
                printf("rbc: bot\n");
                e->on_ground = 1;
                e->box.min.z += pd_z + BOUNCE_DISTANCE;
                e->pos.z += pd_z + BOUNCE_DISTANCE;
                e->vel.z *= (e->vel.z > 0);
              }
              pos.z = xz - 1; // top
              pd_z = (e->box.max.z - (xz - 1));
              if (
                pd_z > 0
              &&
                exclude_corners(pd_x, pd_y, pd_z)
              &&
                is_solid(block_at(f, pos))
              &&
                get_face(&pos, &(e->pos)) | F_BELOW
              ) {
                printf("rbc: top\n");
                e->box.max.z -= pd_z + BOUNCE_DISTANCE;
                e->pos.z -= pd_z + BOUNCE_DISTANCE;
                e->vel.z *= (e->vel.z < 0);
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
  //acceleration.z -= GRAVITY;
  vadd_scaled(&(e->vel), &acceleration, dt);
  vadd_scaled(&(e->pos), &(e->vel), dt);
  vscale(&(e->vel), DRAG);
  resolve_entity_collisions(e);
  resolve_block_collisions(e);
  // TODO: collision detection & resolution
  // Recompute the entity's bounding box:
  compute_bb(e);
  // Reset impulse:
  vzero(&(e->impulse));
}
