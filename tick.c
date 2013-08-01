// tick.c
// Rate control and updates.

#include <GLFW/glfw3.h> // glfwGetTime

#include <stdlib.h>
#include <math.h>

#include "tick.h"
#include "world.h"
#include "ctl.h"
#include "entities.h"
#include "physics.h"
#include "data.h"

/***********
 * Globals *
 ***********/

int TICK_COUNT = 0;

/*************
 * Functions *
 *************/

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
    return;
  }
  adjust_physics_resolution();
  int i;
  for (i = 0; i < steps; ++i) {
    TICK_COUNT = (TICK_COUNT + 1) % TICKS_PER_SECOND_I;
    tick_motion_controls();
    tick_entities(&MAIN_FRAME);
    warp_space(&MAIN_FRAME, PLAYER);
    tick_blocks(&MAIN_FRAME);
  }
  tick_data();
  clear_edge_triggers();
}
