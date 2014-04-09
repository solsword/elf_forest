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
#include "prof/ptime.h"
#include "gen/terrain.h"
#include "data/data.h"
#include "tick/tick.h"
#include "ui/ui.h"

/*************
 * Constants *
 *************/

// TODO: test for oddness near the origin.
// 50% Hills:
//region_chunk_pos const CHUNK_ORIGIN = { .x = 70240, .y = 54567, .z = 10 };
// 70% Hills:
//region_chunk_pos const CHUNK_ORIGIN = { .x = 71240, .y = 54567, .z = 10 };
// Plains:
//region_chunk_pos const CHUNK_ORIGIN = { .x = 69240, .y = 54067, .z = 10 };
// Beach:
region_chunk_pos const CHUNK_ORIGIN = { .x = 10229, .y = -10966, .z = 0 };

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
  init_ptime();

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

void test_spawn_player(active_entity_area *area) {
  vector pos = { .x=2.0, .y=2.0, .z=50.0 };
  //PLAYER = spawn_entity("tern", &pos, area);
  //PLAYER = spawn_entity("sparrow", &pos, area);
  PLAYER = spawn_entity("tester", &pos, area);
  //PLAYER = spawn_entity("elf", &pos, area);
  //PLAYER = spawn_entity("human", &pos, area);
  //PLAYER = spawn_entity("dwarf", &pos, area);
}
