// test.c
// Testing functions.

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "test.h"
#include "noise.h"
#include "display.h"

#include "world.h"
#include "tex.h"
#include "ctl.h"
#include "gfx.h"
#include "terrain.h"
#include "entities.h"
#include "data.h"
#include "ui.h"

/********
 * Main *
 ********/

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
  setup_ui();
  setup_entities();
  setup_data();
  region_chunk_pos roff = {.x=0, .y=0, .z=0};
  setup_frame(&MAIN_FRAME, &roff);
  test_compile_frame(&MAIN_FRAME);
  printf("...done.\n");
  test_spawn_player(&MAIN_FRAME);
  // Start the main loop:
  loop();
  cleanup_frame(&MAIN_FRAME);
  cleanup_entities();
  cleanup_data();
  return 0;
}

/*************
 * Functions *
 *************/

void test_compile_frame(frame *f) {
  frame_chunk_index idx;
  chunk_neighborhood *cnb;
  printf("Computing exposure and compiling chunks...");
  for (idx.x = 0; idx.x < FRAME_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < FRAME_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < FRAME_SIZE; ++idx.z) {
        cnb = get_neighborhood(f, idx);
        load_chunk(cnb);
        free(cnb);
      }
    }
  }
  printf("\n  ...done.\n");
}

void test_spawn_player(frame *f) {
  vector pos = { .x=2.0, .y=2.0, .z=70.0 };
  PLAYER = spawn_entity("tester", &pos, &MAIN_FRAME);
  //PLAYER = spawn_entity("elf", &pos, &MAIN_FRAME);
}
