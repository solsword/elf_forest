#ifndef TICK_H
#define TICK_H

// tick.h
// Thread management, rate control, and updates.

#include "world/world.h"

/*************
 * Constants *
 *************/

// The desired number of ticks per second. The number of ticks per frame may
// vary, as may the number of frames per second, but the number of ticks per
// second shouldn't rise above this value (and will only fall below it if the
// machine can't keep up).
#define TICKS_PER_SECOND 60.0
#define TICKS_PER_SECOND_I 60

// Inverse of TICKS_PER_SECOND:
#define SECONDS_PER_TICK (1.0 / TICKS_PER_SECOND)

/***********
 * Globals *
 ***********/

// The number of ticks since the start of this second:
extern int TICK_COUNT;

// Whether or not automatic data loading should be performed every tick:
extern int TICK_AUTOLOAD;

// Locks for the current position and for the block data.
extern omp_lock_t POSITION_LOCK;
extern omp_lock_t DATA_LOCK;

/*************
 * Functions *
 *************/

// Starts the game, spawning the core threads and setting up the various
// modules. Needs the arguments to main() (to pass to GLFW) as well as a string
// specifying the type of entity that the player should be, and a region
// position where the player will be spawned.
void start_game(
  int argc,
  char **argv,
  char *player_entity_type,
  region_pos *spawn_point
);

// Shuts down the system, cleaning things up and stopping all of the core
// threads.
void shutdown(int returnval);

// Sets up the tick system, in particular initializing the tick rate tracker
// and setting the TICK_AUTOLOAD variable.
void init_tick(int autoload);

// Computes how many ticks should happen based on how much time has elapsed
// since the last call to ticks_expected().
int ticks_expected(void);

// Ticks forward the given number of steps.
void tick(int steps);

#endif //ifndef TICK_H
