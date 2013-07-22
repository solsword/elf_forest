#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "world.h"
#include "render.h"
#include "display.h"
#include "gfx.h"
#include "ctl.h"
#include "tex.h"

int main(int argc, char** argv) {
  srand(545438);
  // Prepare the GLUT context:
  prepare_default(&argc, argv);
  // Set up controls:
  setup_control();
  // Set up textures:
  setup_textures();
  // Set up the test world (must happen after glutInit!):
  printf("Loading test world...\n");
  setup_test_world_terrain(&MAIN_FRAME);
  printf("...done.\n");
  // Start the GLUT main loop:
  loop();
  cleanup_frame(&MAIN_FRAME);
  return 0;
}
