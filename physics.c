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
  int nx, ny, nz; // integer bbox min coords
  int xx, xy, xz; // integer bbox max coords
  int cx, cy, cz; // integer center coords
  int snx, sny, snz; // integer 'side' min coords
  int sxx, sxy, sxz; // integer 'side' max coords
  frame_pos pos; // the voxel we're considering
  nx = b_i_min_x(e->box); xx = b_i_max_x(e->box);
  ny = b_i_min_y(e->box); xy = b_i_max_y(e->box);
  nz = b_i_min_z(e->box); xz = b_i_max_z(e->box);
  cx = fastfloor(e->pos.x);
  cy = fastfloor(e->pos.y);
  cz = fastfloor(e->pos.z);
  snx = (cx < nx + 1) * cx + (1 - (cx < nx + 1)) * (nx + 1);
  sxx = (cx > xx - 1) * cx + (1 - (cx > xx - 1)) * (xx - 1);
  sny = (cy < ny + 1) * cy + (1 - (cy < ny + 1)) * (ny + 1);
  sxy = (cy > xy - 1) * cy + (1 - (cy > xy - 1)) * (xy - 1);
  snz = (cz < nz + 1) * cz + (1 - (cz < nz + 1)) * (nz + 1);
  sxz = (cz > cz - 1) * cz + (1 - (cz > cz - 1)) * (xz - 1);
  /*
  printf(
    "rbf/ppos: %.3f %.3f %.3f\n",
    e->pos.x, e->pos.y, e->pos.z
  );
  printf(
    "rbf/bb: %d %d : %d %d : %d %d\n",
    nx, xx,
    ny, xy,
    nz, xz
  );
  printf(
    "rbf/si: %d %d : %d %d : %d %d\n",
    snx, sxx,
    sny, sxy,
    snz, sxz
  );
  // */
  pos.x = nx;
  // Slide based on side blocks only:
  int constrained = 0; // collision flags for each direction
  float adj = 0, adj_alt = 0; // how far to adjust (main/alt)
  // north/south
  for (pos.x = snx; pos.x <= sxx; ++pos.x) {
    //for (pos.y = sny; pos.y <= sxy; ++pos.y) {
      for (pos.z = snz; pos.z <= sxz; ++pos.z) {
        if ((constrained & F_NORTH) && (constrained & F_SOUTH)) {
          // If we've already adjusted both top and bot, we can break:
          goto eastwest;
        }
        if (!(constrained & F_SOUTH)) {
          pos.y = ny; // south
          if (is_solid(block_at(e->fr, pos))) {
            // Adjust position, velocity, and bounding box.
            adj = ((ny + 1) - e->box.min.y) + BOUNCE_DISTANCE;
            e->pos.y += adj;
            e->box.min.y += adj;
            e->vel.y *= (e->vel.y > 0);
            constrained |= F_SOUTH;
          }
        }
        if (!(constrained & F_NORTH)) {
          pos.y = xy; // north
          if (is_solid(block_at(e->fr, pos))) {
            // Adjust position, velocity, and bounding box.
            adj = (xy - e->box.max.y) - BOUNCE_DISTANCE;
            e->pos.y += adj;
            e->box.max.y += adj;
            e->vel.y *= (e->vel.y < 0);
            constrained |= F_NORTH;
          }
        }
      }
    //}
  }
  eastwest:
  // east/west
  //for (pos.x = snx; pos.x <= sxx; ++pos.x) {
    for (pos.y = sny; pos.y <= sxy; ++pos.y) {
      for (pos.z = snz; pos.z <= sxz; ++pos.z) {
        if ((constrained & F_EAST) && (constrained & F_WEST)) {
          // If we've already adjusted both top and bot, we can break:
          goto topbot;
        }
        if (!(constrained & F_WEST)) {
          pos.x = nx; // west
          if (is_solid(block_at(e->fr, pos))) {
            // Adjust position, velocity, and bounding box.
            adj = (nx + 1) - e->box.min.x + BOUNCE_DISTANCE;
            e->pos.x += adj;
            e->box.min.x += adj;
            e->vel.x *= (e->vel.x > 0);
            constrained |= F_WEST;
          }
        }
        if (!(constrained & F_EAST)) {
          pos.x = xx; // east
          if (is_solid(block_at(e->fr, pos))) {
            // Adjust position, velocity, and bounding box.
            adj = (xx - e->box.max.x) - BOUNCE_DISTANCE;
            e->pos.x += adj;
            e->box.max.x += adj;
            e->vel.x *= (e->vel.x < 0);
            constrained |= F_EAST;
          }
        }
      }
    }
  //}
  topbot:
  // top/bot
  for (pos.x = snx; pos.x <= sxx; ++pos.x) {
    for (pos.y = sny; pos.y <= sxy; ++pos.y) {
      //for (pos.z = snz; pos.z <= sxz; ++pos.z) {
        if ((constrained & F_ABOVE) && (constrained & F_BELOW)) {
          // If we've already adjusted both top and bot, we can break:
          goto diagonals;
        }
        if (!(constrained & F_BELOW)) {
          pos.z = nz; // bot
          if (is_solid(block_at(e->fr, pos))) {
            // Adjust position, velocity, and bounding box.
            adj = ((nz + 1) - e->box.min.z) - TREAD_DEPTH;
            e->pos.z += adj;
            e->box.min.z += adj;
            e->vel.z *= (e->vel.z > 0);
            constrained |= F_BELOW;
          }
        }
        if (!(constrained & F_ABOVE)) {
          pos.z = xz; // top
          if (is_solid(block_at(e->fr, pos))) {
            // Adjust position, velocity, and bounding box.
            adj = (xz - e->box.max.z) - BOUNCE_DISTANCE;
            e->pos.z += adj;
            e->box.max.z += adj;
            e->vel.z *= (e->vel.z < 0);
            constrained |= F_ABOVE;
          }
        }
      //}
    }
  }
  // Check diagonals if we need to:
  diagonals:
  adj = 0;
  /*
  // tn/ts/bn/bs
  for (pos.x = nx; pos.x <= xx; ++pos.x) {
    if (!(constrained & F_SOUTH)) { // south
      pos.y = ny;
      adj = (ny + 1) - e->box.min.y + BOUNCE_DISTANCE;
      pos.z = nz;
      // bot-south
      if ( !(constrained & F_BELOW) && is_solid(block_at(e->fr, pos))) {
        adj_alt = ((nz + 1) - e->box.min.z) - TREAD_DEPTH;
        if (adj > adj_alt) {
          e->pos.y += adj;
          e->box.min.y += adj;
          e->vel.y *= (e->vel.y > 0);
          constrained |= F_SOUTH;
        } else {
          e->pos.z += adj_alt;
          e->box.min.z += adj_alt;
          e->vel.z *= (e->vel.z > 0);
          constrained |= F_BELOW;
        }
      }
      pos.z = xz;
      // top-south
      if ( !(constrained & F_ABOVE) && is_solid(block_at(e->fr, pos))) {
        adj_alt = (xz - e->box.max.z) - BOUNCE_DISTANCE;
        if (adj > adj_alt) {
          e->pos.y += adj;
          e->box.min.y += adj;
          e->vel.y *= (e->vel.y > 0);
          constrained |= F_SOUTH;
        } else {
          e->pos.z += adj_alt;
          e->box.max.z += adj_alt;
          e->vel.z *= (e->vel.z < 0);
          constrained |= F_ABOVE;
        }
      }
    }
    if (!(constrained & F_NORTH)) { // north
      pos.y = xy;
      adj = (xy - e->box.max.y) - BOUNCE_DISTANCE;
      pos.z = nz;
      // bot-north
      if ( !(constrained & F_BELOW) && is_solid(block_at(e->fr, pos))) {
        adj_alt = ((nz + 1) - e->box.min.z) - TREAD_DEPTH;
        if (adj > adj_alt) {
          e->pos.y += adj;
          e->box.max.y += adj;
          e->vel.y *= (e->vel.y < 0);
          constrained |= F_NORTH;
        } else {
          e->pos.z += adj_alt;
          e->box.min.z += adj_alt;
          e->vel.z *= (e->vel.z > 0);
          constrained |= F_BELOW;
        }
      }
      pos.z = xz;
      // top-north
      if ( !(constrained & F_ABOVE) && is_solid(block_at(e->fr, pos))) {
        adj_alt = (xz - e->box.max.z) - BOUNCE_DISTANCE;
        if (adj > adj_alt) {
          e->pos.y += adj;
          e->box.max.y += adj;
          e->vel.y *= (e->vel.y < 0);
          constrained |= F_NORTH;
        } else {
          e->pos.z += adj_alt;
          e->box.max.z += adj_alt;
          e->vel.z *= (e->vel.z < 0);
          constrained |= F_ABOVE;
        }
      }
    }
  }
  // te/tw/be/bw
  for (pos.y = ny; pos.y <= xy; ++pos.y) {
    if (!(constrained & F_WEST)) { // west
      pos.x = nx;
      adj = (nx + 1) - e->box.min.x + BOUNCE_DISTANCE;
      pos.z = nz;
      // bot-west
      if ( !(constrained & F_BELOW) && is_solid(block_at(e->fr, pos))) {
        adj_alt = ((nz + 1) - e->box.min.z) - TREAD_DEPTH;
        if (adj > adj_alt) {
          e->pos.x += adj;
          e->box.min.x += adj;
          e->vel.x *= (e->vel.x > 0);
          constrained |= F_WEST;
        } else {
          e->pos.z += adj_alt;
          e->box.min.z += adj_alt;
          e->vel.z *= (e->vel.z > 0);
          constrained |= F_BELOW;
        }
      }
      pos.z = xz;
      // top-west
      if ( !(constrained & F_ABOVE) && is_solid(block_at(e->fr, pos))) {
        adj_alt = (xz - e->box.max.z) - BOUNCE_DISTANCE;
        if (adj > adj_alt) {
          e->pos.x += adj;
          e->box.min.x += adj;
          e->vel.x *= (e->vel.x > 0);
          constrained |= F_WEST;
        } else {
          e->pos.z += adj_alt;
          e->box.max.z += adj_alt;
          e->vel.z *= (e->vel.z < 0);
          constrained |= F_ABOVE;
        }
      }
    }
    if (!(constrained & F_EAST)) { // east
      pos.x = xx;
      adj = (xx - e->box.max.x) - BOUNCE_DISTANCE;
      pos.z = nz;
      // bot-east
      if ( !(constrained & F_BELOW) && is_solid(block_at(e->fr, pos))) {
        adj_alt = ((nz + 1) - e->box.min.z) - TREAD_DEPTH;
        if (adj > adj_alt) {
          e->pos.x += adj;
          e->box.max.x += adj;
          e->vel.x *= (e->vel.x < 0);
          constrained |= F_EAST;
        } else {
          e->pos.z += adj_alt;
          e->box.min.z += adj_alt;
          e->vel.z *= (e->vel.z > 0);
          constrained |= F_BELOW;
        }
      }
      pos.z = xz;
      // top-east
      if ( !(constrained & F_ABOVE) && is_solid(block_at(e->fr, pos))) {
        adj_alt = (xz - e->box.max.z) - BOUNCE_DISTANCE;
        if (adj > adj_alt) {
          e->pos.x += adj;
          e->box.max.x += adj;
          e->vel.x *= (e->vel.x < 0);
          constrained |= F_EAST;
        } else {
          e->pos.z += adj_alt;
          e->box.max.z += adj_alt;
          e->vel.z *= (e->vel.z < 0);
          constrained |= F_ABOVE;
        }
      }
    }
  }
  // ne/nw/se/sw
  for (pos.z = nz; pos.z <= xz; ++pos.z) {
    if (!(constrained & F_WEST)) { // west
      pos.x = nx;
      adj = (nx + 1) - e->box.min.x + BOUNCE_DISTANCE;
      pos.y = ny;
      // south-west
      if ( !(constrained & F_SOUTH) && is_solid(block_at(e->fr, pos))) {
        adj_alt = ((ny + 1) - e->box.min.y) + BOUNCE_DISTANCE;
        if (adj > adj_alt) {
          e->pos.x += adj;
          e->box.min.x += adj;
          e->vel.x *= (e->vel.x > 0);
          constrained |= F_WEST;
        } else {
          e->pos.y += adj_alt;
          e->box.min.y += adj_alt;
          e->vel.y *= (e->vel.y > 0);
          constrained |= F_SOUTH;
        }
      }
      pos.y = xy;
      // north-west
      if ( !(constrained & F_NORTH) && is_solid(block_at(e->fr, pos))) {
        adj_alt = (xy - e->box.max.y) - BOUNCE_DISTANCE;
        if (adj > adj_alt) {
          e->pos.x += adj;
          e->box.min.x += adj;
          e->vel.x *= (e->vel.x > 0);
          constrained |= F_WEST;
        } else {
          e->pos.y += adj_alt;
          e->box.max.y += adj_alt;
          e->vel.y *= (e->vel.y < 0);
          constrained |= F_NORTH;
        }
      }
    }
    if (!(constrained & F_EAST)) { // east
      pos.x = xx;
      adj = (xx - e->box.max.x) - BOUNCE_DISTANCE;
      pos.y = ny;
      // south-east
      if ( !(constrained & F_SOUTH) && is_solid(block_at(e->fr, pos))) {
        adj_alt = ((ny + 1) - e->box.min.y) + BOUNCE_DISTANCE;
        if (adj > adj_alt) {
          e->pos.x += adj;
          e->box.max.x += adj;
          e->vel.x *= (e->vel.x < 0);
          constrained |= F_EAST;
        } else {
          e->pos.y += adj_alt;
          e->box.min.y += adj_alt;
          e->vel.y *= (e->vel.y > 0);
          constrained |= F_SOUTH;
        }
      }
      pos.y = xy;
      // north-east
      if ( !(constrained & F_NORTH) && is_solid(block_at(e->fr, pos))) {
        adj_alt = (xy - e->box.max.y) - BOUNCE_DISTANCE;
        if (adj > adj_alt) {
          e->pos.x += adj;
          e->box.max.x += adj;
          e->vel.x *= (e->vel.x < 0);
          constrained |= F_EAST;
        } else {
          e->pos.y += adj_alt;
          e->box.max.y += adj_alt;
          e->vel.y *= (e->vel.y < 0);
          constrained |= F_NORTH;
        }
      }
    }
  }
  // */
  if (constrained & F_BELOW) {
    e->on_ground = 1;
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
  vadd_scaled(&(e->pos), &(e->vel), dt);
  e->vel.x *= DRAG;
  e->vel.y *= DRAG;
  resolve_entity_collisions(e);
  resolve_block_collisions(e);
  // TODO: collision detection & resolution
  // Recompute the entity's bounding box:
  compute_bb(e);
  // Reset impulse:
  vzero(&(e->impulse));
}
