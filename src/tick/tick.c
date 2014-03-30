// tick.c
// Rate control and updates.

#include <GLFW/glfw3.h> // glfwGetTime

#include <stdlib.h>
#include <math.h>

#include "tick.h"

#include "world/world.h"
#include "world/entities.h"
#include "control/ctl.h"
#include "physics/physics.h"
#include "data/data.h"

/*************
 * Constants *
 *************/

double const DEFAULT_TRACKING_INTERVAL = 0.25;

/***********
 * Globals *
 ***********/

int TICK_COUNT = 0;

rate_data TICKRATE;
rate_data FRAMERATE;

/*************
 * Functions *
 *************/

void init_tick(void) {
  setup_rate_data(&TICKRATE, DEFAULT_TRACKING_INTERVAL);
  setup_rate_data(&FRAMERATE, DEFAULT_TRACKING_INTERVAL);
}

int ticks_expected(void) {
  static int first = 1;
  static double stored = 0.0;
  static double lasttime;
  double curtime, elapsed;
  if (first) {
    lasttime = glfwGetTime();
    stored = 0;
    first = 0;
  }
  curtime = glfwGetTime();
  elapsed = curtime - lasttime;
  lasttime = curtime;
  float ticks_due = ((float) elapsed) * TICKS_PER_SECOND + stored;
  stored = ticks_due - floor(ticks_due);
  return (int) floor(ticks_due);
}

void tick(int steps) {
  tick_general_controls();
  if (steps == 0 || PAUSED) {
    clear_edge_triggers();
    return;
  }
  adjust_physics_resolution();
  int i;
  for (i = 0; i < steps; ++i) {
    TICK_COUNT = (TICK_COUNT + 1) % TICKS_PER_SECOND_I;
    tick_motion_controls();
    tick_active_entities();
    warp_space(ACTIVE_AREA, PLAYER);
    // TODO: tick blocks
    //tick_blocks(ACTIVE_AREA);
    update_rate(&TICKRATE);
  }
  region_chunk_pos rcpos;
  rpos__rcpos(&(ACTIVE_AREA->origin), &rcpos);
  load_surroundings(&rcpos);
  tick_data();
  clear_edge_triggers();
}

void setup_rate_data(rate_data *rd, double interval) {
  rd->count = 0;
  rd->lasttime = glfwGetTime();
  rd->elapsed = 0;
  rd->rate = 0;
  rd->interval = interval;
}

void update_rate(rate_data *rd) {
  double curtime;

  // Count this occurrence:
  rd->count += 1;

  // Compute elapsed time since the last occurrence:
  curtime = glfwGetTime();
  rd->elapsed += curtime - rd->lasttime;
  rd->lasttime = curtime;

  // Update the rate about every interval seconds:
  if (rd->elapsed >= rd->interval) {
    rd->rate = rd->count / rd->elapsed;
    rd->count = 0;
    rd->elapsed = 0;
  }
}
