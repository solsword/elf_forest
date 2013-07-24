// physics.c
// Physical simulation.

#include <math.h>

#include "octree.h"
#include "vector.h"
#include "tick.h"
#include "world.h"
#include "entities.h"
#include "physics.h"

/*************
 * Constants *
 *************/

float GRAVITY = 9.81;

/*************
 * Functions *
 *************/

void tick_physics(entity *e) {
  float dt = SECONDS_PER_TICK;
  vector acceleration;
  // Integrate kinetics:
  acceleration.x = e->impulse.x / e->mass;
  acceleration.y = e->impulse.y / e->mass;
  acceleration.z = e->impulse.z / e->mass;
  //acceleration.z -= GRAVITY;
  vadd_scaled(&(e->vel), &acceleration, dt);
  vadd_scaled(&(e->pos), &(e->vel), dt);
  // TODO: collision detection & resolution
  // Reset impulse:
  vzero(&(e->impulse));
}
