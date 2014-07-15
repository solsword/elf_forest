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

#include "gen/worldgen.h"
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

ptrdiff_t SEED = 18234109;

/********
 * Main *
 ********/

int main(int argc, char** argv) {
  // Compute detailed starting location:
  region_pos origin;
  //rcpos__rpos(&CHUNK_ORIGIN, &origin);
  wmpos__rpos(&WORLD_ORIGIN, &origin);
  origin.x += 2;
  origin.y += 2;
  //origin.z = 450;
  origin.z = 2400;

  // Start the game:
  start_game(SEED, argc, argv, "tester", &origin);
  //start_game(seed, argc, argv, "dwarf", &origin);
  //start_game(seed, argc, argv, "elf", &origin);
  //start_game(seed, argc, argv, "sparrow", &origin);
  //start_game(seed, argc, argv, "tern", &origin);

  // This should never be reached.
  return 0;
}
