// tick.c
// Rate control and updates.

#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#include "tick.h"
#include "ctl.h"
#include "entities.h"

/*************
 * Constants *
 *************/

const float TICKS_PER_SECOND = 60.0;
const int TICKS_PER_SECOND_I = 60;

int TICK_COUNT = 0;

/*************
 * Functions *
 *************/

int ticks_expected(void) {
  static uint8_t first = 1;
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
  int i;
  for (i = 0; i < steps; ++i) {
    TICK_COUNT = (TICK_COUNT + 1) % TICKS_PER_SECOND_I;
    tick_entities();
  }
}
