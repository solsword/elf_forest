// tick.c
// Rate control and updates.

#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

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
  static float ticks_per_us = 0.0;
  static float stored = 0.0;
  static struct timeval lasttime;
  struct timeval curtime;
  if (first) {
    gettimeofday(&lasttime, NULL);
    stored = 0;
    ticks_per_us = TICKS_PER_SECOND / 1000000.0;
    first = 0;
  }
  gettimeofday(&curtime, NULL);
  int last_us = lasttime.tv_usec;
  int now_us = curtime.tv_usec;
  int elapsed = now_us - last_us;
  if (lasttime.tv_sec != curtime.tv_sec) {
    elapsed += (curtime.tv_sec - lasttime.tv_sec) * 1000000;
  }
  lasttime.tv_sec = curtime.tv_sec;
  lasttime.tv_usec = curtime.tv_usec;
  float ticks_due = ((float) elapsed) * ticks_per_us + stored;
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
  tick_load();
  clear_edge_triggers();
}
