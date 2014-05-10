#include <stdlib.h>
#include <stdio.h>

#include "tex/tex.h"
#include "graphics/gfx.h"
#include "control/ctl.h"
#include "world/world.h"

int main(int argc, char** argv) {
  // TODO: copy functionality from test's main.
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
  return 0;
}
