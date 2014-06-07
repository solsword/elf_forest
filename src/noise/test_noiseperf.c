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
  speed_data sd;
  setup_speed_data(&sd);
  for (x = 0; x < SIZE; ++x) {
    for (y = 0; y < SIZE; ++y) {
      f[x+y*SIZE] = sxnoise_2d(x*SCALE, y*SCALE);
      update_speed(&sd);
      //printf("time: %.5f\n", glfwGetTime());
    }
  }
  x = (int) f[0];
  printf(
    "Average time per sxnoise_2d call (ns): %0.8f\n",
    sd.duration*1000*1000
  );
  return 0;
}
