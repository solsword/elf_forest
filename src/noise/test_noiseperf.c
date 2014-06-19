// test_noiseperf.c
// noise performance testing

#include <stdlib.h>
#include <stdio.h>

#include <GLFW/glfw3.h>

#include "prof/ptime.h"

#include "noise.h"

#define SIZE 10000

static float const SCALE = 1/32.0;

int main(int argc, char** argv) {
  glfwInit();
  int x, y;
  float f[SIZE*SIZE];
  duration_data dd;
  // sxnoise_2d:
  setup_duration_data(&dd, 0.2);
  for (x = 0; x < SIZE; ++x) {
    for (y = 0; y < SIZE; ++y) {
      f[x+y*SIZE] = sxnoise_2d(x*SCALE, y*SCALE);
      update_duration(&dd);
    }
  }
  x = (int) f[0];
  printf(
    "Average time per sxnoise_2d call (us): %0.8f\n",
    dd.duration*1000*1000
  );
  // DEBUG:
  exit(0);

  // managed_sxnoise_2d:
  setup_duration_data(&dd, 0.2);
  for (x = 0; x < SIZE; ++x) {
    for (y = 0; y < SIZE; ++y) {
      f[x+y*SIZE] = managed_sxnoise_2d(x, y, 1/SCALE, 1/SCALE, 5.7);
      update_duration(&dd);
    }
  }
  x = (int) f[0];
  printf(
    "Average time per managed_sxnoise_2d call (us): %0.8f\n",
    dd.duration*1000*1000
  );

  // wrnoise_2d:
  setup_duration_data(&dd, 0.2);
  for (x = 0; x < SIZE; ++x) {
    for (y = 0; y < SIZE; ++y) {
      f[x+y*SIZE] = wrnoise_2d(x*SCALE, y*SCALE);
      update_duration(&dd);
    }
  }
  x = (int) f[0];
  printf(
    "Average time per wrnoise_2d call (us): %0.8f\n",
    dd.duration*1000*1000
  );

  // managed_wrnoise_2d:
  setup_duration_data(&dd, 0.2);
  for (x = 0; x < SIZE; ++x) {
    for (y = 0; y < SIZE; ++y) {
      f[x+y*SIZE] = managed_wrnoise_2d(x, y, 1/SCALE, 1/SCALE, 8.4);
      update_duration(&dd);
    }
  }
  x = (int) f[0];
  printf(
    "Average time per managed_wrnoise_2d call (us): %0.8f\n",
    dd.duration*1000*1000
  );
  return 0;
}
