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
#include "math/manifold.h"
#include "control/ctl.h"
#include "prof/ptime.h"
#include "gen/terrain.h"
#include "data/data.h"
#include "tick/tick.h"
#include "ui/ui.h"

#include "util.h"

/*************
 * Constants *
 *************/

// Center of the world:
world_map_pos WORLD_ORIGIN = { .x = WORLD_WIDTH/2, .y=WORLD_HEIGHT/2 };

ptrdiff_t SEED = 18224442;

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
  // note origin.z is ignored by start_game

  SEED = prng(prng(prng(prng(SEED))));

  // Start the game:
  //start_game(SEED, argc, argv, "tester", &origin);
  start_game(SEED, argc, argv, "viewer", &origin);
  //start_game(SEED, argc, argv, "dwarf", &origin);
  //start_game(SEED, argc, argv, "elf", &origin);
  //start_game(SEED, argc, argv, "sparrow", &origin);
  //start_game(SEED, argc, argv, "tern", &origin);

  // This should never be reached.
  return 0;
}
