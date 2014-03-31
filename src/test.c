// test.c
// A testing program quite similar to the main program.

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "test.h"
#include "viewer.h"

#include "noise/noise.h"

#include "graphics/display.h"
#include "graphics/tex.h"
#include "graphics/gfx.h"

#include "world/world.h"
#include "world/entities.h"

#include "control/ctl.h"
#include "gen/terrain.h"
#include "data/data.h"
#include "tick/tick.h"
#include "ui/ui.h"

/*************
 * Constants *
 *************/

// TODO: test for oddness near the origin.
// 50% Hills:
//region_chunk_pos const CHUNK_ORIGIN = { .x = 8935, .y = -11980, .z = 7 };
// 70% Hills:
//region_chunk_pos const CHUNK_ORIGIN = { .x = 71240, .y = 54567, .z = 10 };
// Plains:
//region_chunk_pos const CHUNK_ORIGIN = { .x = 10235, .y = -10980, .z = 2 };
// Beach:
region_chunk_pos const CHUNK_ORIGIN = { .x = 0, .y = 4500, .z = 0 };

/********
 * Main *
 ********/

int main(int argc, char** argv) {
  // Compute detailed starting location:
  region_pos origin;
  rcpos__rpos(&CHUNK_ORIGIN, &origin);

  // Seed the random number generator:
  srand(545438);

  // Prepare the window context:
  prepare_default(&argc, argv);

  // Set up the test world:
  printf("Setting up test world...\n");

  // Initialize stateless subsystems:
  init_control();
  init_textures();
  init_tick(1);

  // Setup stateful subsystems:
  setup_ui();
  setup_data();
  setup_entities(&origin);

  printf("...done.\n");

  // Spawn the player:
  test_spawn_player(ACTIVE_AREA);

  // Start the main loop:
  loop();

  return 0;
}

/*************
 * Functions *
 *************/

/* TODO: Get rid of me
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
  for (idx.x = 1; idx.x < FRAME_SIZE - 1; ++idx.x) {
    for (idx.y = 1; idx.y < FRAME_SIZE - 1; ++idx.y) {
      for (idx.z = 1; idx.z < FRAME_SIZE - 1; ++idx.z) {
        cnb = get_neighborhood(f, idx);
        if (is_fully_loaded(cnb)) {
          compute_exposure(cnb);
        }
        compile_chunk(cnb->c);
        free(cnb);
      }
    }
  }
  printf("\n  ...done.\n");
}
*/

void test_spawn_player(active_entity_area *area) {
  vector pos = { .x=2.0, .y=2.0, .z=50.0 };
  //PLAYER = spawn_entity("tern", &pos, area);
  //PLAYER = spawn_entity("sparrow", &pos, area);
  PLAYER = spawn_entity("tester", &pos, area);
  //PLAYER = spawn_entity("elf", &pos, area);
  //PLAYER = spawn_entity("human", &pos, area);
  //PLAYER = spawn_entity("dwarf", &pos, area);
}
