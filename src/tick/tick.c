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

count_data CHUNK_LAYERS_RENDERED;
count_data CHUNKS_LOADED;
count_data CHUNKS_COMPILED;

/*************
 * Functions *
 *************/

void init_tick(void) {
  setup_rate_data(&TICKRATE, DEFAULT_TRACKING_INTERVAL);
  setup_rate_data(&FRAMERATE, DEFAULT_TRACKING_INTERVAL);
  setup_count_data(&CHUNK_LAYERS_RENDERED, DEFAULT_TRACKING_INTERVAL);
  setup_count_data(&CHUNKS_LOADED, DEFAULT_TRACKING_INTERVAL);
  setup_count_data(&CHUNKS_COMPILED, DEFAULT_TRACKING_INTERVAL);
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
  rd->interval = interval;
  rd->rate = 0;
}

void setup_count_data(count_data *cd, double interval) {
  cd->count = 0;
  cd->next_idx = 0;
  cd->lasttime = glfwGetTime();
  cd->elapsed = 0;
  cd->interval = interval;
  cd->average = 0;
}

void update_rate(rate_data *rd) {
  double curtime;

  // Count this occurrence:
  rd->count += 1;

  // Compute elapsed time since the start of the interval:
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

void update_count(count_data *cd, int data_point) {
  double curtime;
  size_t i, limit;

  // Count this occurrence:
  cd->count += 1;
  cd->data[cd->next_idx] = data_point;
  cd->next_idx = (cd->next_idx + 1) % COUNT_DATA_BUFFER_SIZE;

  // Compute elapsed time since the start of the interval:
  curtime = glfwGetTime();
  cd->elapsed += curtime - cd->lasttime;
  cd->lasttime = curtime;

  // Update the average about every interval seconds:
  if (cd->elapsed >= cd->interval) {
    limit = cd->next_idx;
    if (cd->count > cd->next_idx) {
      limit = COUNT_DATA_BUFFER_SIZE;
    }
    cd->average = 0;
    for (i = 0; i < limit; ++i) {
      cd->average += cd->data[i];
    }
    cd->average /= limit;

    // Reset our counters:
    cd->count = 0;
    cd->next_idx = 0;
    cd->elapsed = 0;
  }
}
