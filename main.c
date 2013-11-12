#include <stdlib.h>
#include <stdio.h>

#include "world.h"
#include "tex.h"
#include "ctl.h"
#include "gfx.h"

int main(int argc, char** argv) {
  srand(545438);
  // Prepare the window context:
  prepare_default(&argc, argv);
  // Set up controls:
  setup_control();
  // Set up textures:
  setup_textures();
  // Set up the test world:
  printf("Loading test world...\n");
  //setup_test_world_terrain(&MAIN_FRAME);
  printf("...done.\n");
  // Start the main loop:
  loop();
  cleanup_frame(&MAIN_FRAME);
  return 0;
}
