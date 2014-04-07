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
#include "prof/ptime.h"
#include "data/data.h"

/***********
 * Globals *
 ***********/

int TICK_COUNT = 0;

int TICK_AUTOLOAD = 1;

/*************
 * Functions *
 *************/

void init_tick(int autoload) {
  TICK_AUTOLOAD = autoload;
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
  size_t i, j;
  for (i = 0; i < steps; ++i) {
    TICK_COUNT = (TICK_COUNT + 1) % TICKS_PER_SECOND_I;
    tick_motion_controls();
    for (j = 0; j < PHYS_SUBSTEPS; ++j) {
      tick_active_entities();
    }
    warp_space(ACTIVE_AREA, PLAYER);
    // TODO: tick blocks
    //tick_blocks(ACTIVE_AREA);
    update_rate(&TICKRATE);
  }
  region_chunk_pos rcpos;
  rpos__rcpos(&(ACTIVE_AREA->origin), &rcpos);
  if (TICK_AUTOLOAD) {
    load_surroundings(&rcpos);
    tick_data();
  }
  clear_edge_triggers();
}
