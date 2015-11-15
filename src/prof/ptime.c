// ptime.c
// Profiling utilities for tracking time and rates.

#include <GLFW/glfw3.h> // glfwGetTime

#include "ptime.h"

/*************
 * Constants *
 *************/

double const DEFAULT_TRACKING_INTERVAL = 0.25;

double const DEFAULT_AVERAGING_WEIGHT = 0.2;

/***********
 * Globals *
 ***********/

rate_data TICKRATE;
rate_data FRAMERATE;

duration_data RENDER_TIME;
duration_data RENDER_AREA_TIME;
duration_data RENDER_UI_TIME;
duration_data RENDER_CORE_TIME;
duration_data RENDER_INNER_TIME;
duration_data COMPILE_TIME;
duration_data PHYSICS_TIME;
duration_data DATA_TIME;
duration_data TGEN_TIME;
duration_data DISK_READ_TIME;
duration_data DISK_MISS_TIME;
duration_data DISK_WRITE_TIME;

count_data CHUNK_LAYERS_RENDERED;
count_data CHUNKS_LOADED;
count_data CHUNKS_COMPILED;
count_data CHUNKS_BIOGEND;
count_data CHUNKS_BIOSKIPPED;

/*************
 * Functions *
 *************/

void setup_duration_data(duration_data *dd, double weight) {
  dd->starttime = -1;
  dd->duration = -1;
  dd->weight = weight;
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

void init_ptime(void) {
  setup_rate_data(&TICKRATE, DEFAULT_TRACKING_INTERVAL);
  setup_rate_data(&FRAMERATE, DEFAULT_TRACKING_INTERVAL);

  setup_duration_data(&RENDER_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&RENDER_AREA_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&RENDER_UI_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&RENDER_CORE_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&RENDER_INNER_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&COMPILE_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&PHYSICS_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&DATA_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&TGEN_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&DISK_READ_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&DISK_MISS_TIME, DEFAULT_AVERAGING_WEIGHT);
  setup_duration_data(&DISK_WRITE_TIME, DEFAULT_AVERAGING_WEIGHT);

  setup_count_data(&CHUNK_LAYERS_RENDERED, DEFAULT_TRACKING_INTERVAL);
  setup_count_data(&CHUNKS_LOADED, DEFAULT_TRACKING_INTERVAL);
  setup_count_data(&CHUNKS_COMPILED, DEFAULT_TRACKING_INTERVAL);
  setup_count_data(&CHUNKS_BIOGEND, DEFAULT_TRACKING_INTERVAL);
  setup_count_data(&CHUNKS_BIOSKIPPED, DEFAULT_TRACKING_INTERVAL);
}

void start_duration(duration_data *dd) {
  dd->starttime = glfwGetTime();
}

void end_duration(duration_data *dd) {
  double curtime = glfwGetTime();
  double elapsed;
  if (dd->starttime > 0) {
    elapsed = curtime - dd->starttime;
    if (dd->duration >= 0) {
      dd->duration = (1 - dd->weight) * dd->duration + dd->weight * elapsed;
    } else {
      dd->duration = elapsed;
    }
  }
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
