// test.c
// A testing program quite similar to the main program.

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "test.h"
#include "viewer.h"

#include "noise/noise.h"

#include "graphics/display.h"
#include "graphics/gfx.h"

#include "tex/tex.h"

#include "shaders/pipeline.h"

#include "world/world.h"
#include "world/entities.h"

#ifndef TERRAIN_MODE_BASIC
  #include "gen/worldgen.h"
#endif

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
//region_chunk_pos const CHUNK_ORIGIN = { .x = 11929, .y = -199356, .z = 2 };

// Center of the world:
world_map_pos WORLD_ORIGIN = { .x = WORLD_WIDTH/2, .y=WORLD_HEIGHT/2 };

/********
 * Main *
 ********/

int main(int argc, char** argv) {
  // Compute detailed starting location:
  region_pos origin;
  //rcpos__rpos(&CHUNK_ORIGIN, &origin);
  wmpos__rpos(&WORLD_ORIGIN, &origin);
  origin.z = 1620;
  //origin.z = 20000;

  // Seed the random number generator:
  srand(545438);

  // Prepare the window context:
  prepare_default(&argc, argv);

  // Set up the test world:
  printf("Setting up test world...\n");

  // Initialize stateless subsystems:
  printf("  ...control...\n");
  init_control();
  printf("  ...tick...\n");
  init_tick(1);
  printf("  ...ptime...\n");
  init_ptime();

  // Setup stateful subsystems:
  printf("  ...jobs...\n");
  setup_jobs();
  printf("  ...shaders...\n");
  setup_shaders();
  printf("  ...textures...\n");
  setup_textures();
  printf("  ...ui...\n");
  setup_ui();
  printf("  ...data...\n");
  setup_data();
  printf("  ...entities...\n");
  setup_entities(&origin);
  printf("  ...world_map...\n");
  setup_worldgen();

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
  vector pos = { .x=2.0, .y=2.0, .z=2.0 };
  //PLAYER = spawn_entity("tern", &pos, area);
  //PLAYER = spawn_entity("sparrow", &pos, area);
  PLAYER = spawn_entity("tester", &pos, area);
  //PLAYER = spawn_entity("elf", &pos, area);
  //PLAYER = spawn_entity("human", &pos, area);
  //PLAYER = spawn_entity("dwarf", &pos, area);
}
