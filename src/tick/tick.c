// tick.c
// Thread management, rate control, and updates.

#include <GLFW/glfw3.h> // glfwGetTime

#include <stdlib.h>
#include <math.h>

#include <omp.h>

#include "tick.h"

#include "snek/summon.h"
#include "world/world.h"
#include "world/entities.h"
#include "world/species.h"
#include "control/ctl.h"
#include "control/interact.h"
#include "physics/physics.h"
#include "prof/ptime.h"
#include "data/data.h"
#include "data/persist.h"
#include "graphics/gfx.h"
#include "graphics/render.h"
#include "shaders/pipeline.h"
#include "jobs/jobs.h"
#include "tex/tex.h"
#include "gen/worldgen.h"
#include "ui/ui.h"

#include "util.h"

/***********
 * Globals *
 ***********/

int TICK_COUNT = 0;

int TICK_AUTOLOAD = 1;

omp_lock_t POSITION_LOCK;
omp_lock_t DATA_LOCK;

/*******************
 * Private Globals *
 *******************/

int SHUTDOWN = 0;
int RENDERING_DONE = 0;
int DATA_DONE = 0;

/*********************
 * Private Functions *
 *********************/

void _get_everything_set_up(ptrdiff_t seed, int argc, char** argv) {
  // Seed the random number generator:
  srand(seed);

  // Prepare the window context:
  prepare_default(&argc, argv);

  // Set up the test world:
  printf("Setting up subsystems...\n");

  // The ELFSCRIPT data format setup goes first:
  printf("  ...Elf Forest data...\n");
  //setup_elf_forest_data(1); // track error contexts
  setup_elf_forest_data(0); // don't track error contexts
  printf("  ...loading common data...\n");
  summon_snek();

  // Initialize stateless subsystems:
  printf("  ...control...\n");
  init_control();
  printf("  ...tick...\n");
  init_tick(1);
  printf("  ...ptime...\n");
  init_ptime();
  printf("  ...blocks...\n");
  init_blocks();

  // Setup stateful subsystems:
  // TODO: Do we need/want this subsystem?
  //printf("  ...jobs...\n");
  //setup_jobs();
  printf("  ...rendering...\n");
  setup_render();
  printf("  ...shaders...\n");
  setup_shaders();
  printf("  ...textures...\n");
  setup_textures();
  printf("  ...ui...\n");
  setup_ui();
  printf("  ...data...\n");
  setup_data();
  printf("  ...persist...\n");
  setup_persist(PS_DEFAULT_WORLD_DIR);
  printf("  ...entities...\n");
  setup_entities();
  printf("  ...species...\n");
  setup_species();
  printf("  ...worldgen...\n");
  setup_worldgen(seed);

  printf("...done.\n");
}

void _spawn_the_player(
  char *type,
  active_entity_area *area,
  global_pos *origin
) {
  manifold_point dontcare, th;
  compute_terrain_height(THE_WORLD, origin, &dontcare, &dontcare, &th);
  origin->z = (gl_pos_t) fastfloor(th.z);
  origin->z += 4;
  vector pos;
  glpos__vec(&(area->origin), origin, &pos);
  warp_space(ACTIVE_AREA, &pos);
  pos.x -= CHUNK_SIZE * fastfloor(pos.x / CHUNK_SIZE);
  pos.y -= CHUNK_SIZE * fastfloor(pos.y / CHUNK_SIZE);
  pos.z -= CHUNK_SIZE * fastfloor(pos.z / CHUNK_SIZE);
  PLAYER = spawn_entity(type, &pos, area);
}

/*************
 * Functions *
 *************/

void start_game(
  ptrdiff_t seed,
  int argc,
  char **argv,
  char *player_entity_type,
  global_pos *spawn_point
) {
  int thread_id = 0;
  global_chunk_pos area_origin, last_origin;

  // Start the main threads:
#pragma omp parallel num_threads(2) firstprivate(thread_id)
  {
    thread_id = omp_get_thread_num();
    // Everyone waits while the graphics thread performs setup:
    if (thread_id == 0) {
      _get_everything_set_up(seed, argc, argv);
      _spawn_the_player(player_entity_type, ACTIVE_AREA, spawn_point);
      // A couple of sequential cycles to start things off smoothly:
      tick(2);
      glpos__glcpos(&(ACTIVE_AREA->origin), &area_origin);
      tick_load_chunks(&area_origin);
      tick_compile_chunks();
      tick_biogen();
      render(WINDOW);
      glfwPollEvents();
    }
  #pragma omp barrier
    // And then each thread starts its own loop:
    if (thread_id == 0) {
      // The main thread (which gets to have the graphics context):
      // Ensure control of the GLFW window:
      glfwMakeContextCurrent(WINDOW);
      while (!SHUTDOWN) {
        if (glfwWindowShouldClose(WINDOW)) {
          break;
        }
        // Compile chunks
#ifdef PROFILE_TIME
        start_duration(&COMPILE_TIME);
#endif
        tick_compile_chunks();
#ifdef PROFILE_TIME
        end_duration(&COMPILE_TIME);
        start_duration(&PHYSICS_TIME);
#endif
        tick(ticks_expected());
#ifdef PROFILE_TIME
        end_duration(&PHYSICS_TIME);
#endif
        omp_set_lock(&POSITION_LOCK);
        glpos__glcpos(&(ACTIVE_AREA->origin), &area_origin);
        omp_unset_lock(&POSITION_LOCK);
        if (RENDER) {
#ifdef PROFILE_TIME
          start_duration(&RENDER_TIME);
#endif
          render(WINDOW);
#ifdef PROFILE_TIME
          end_duration(&RENDER_TIME);
#endif
          glfwPollEvents();
        } else {
          glfwPollEvents();
          nap(50);
        }
      }
      RENDERING_DONE = 1;
      // This thread will shut down the whole system when it exits (if the
      // system isn't already being shutdown).
      if (!SHUTDOWN) {
        core_shutdown(0);
      }
    } else if (thread_id == 1) {
      // The data management thread:
      while (!SHUTDOWN) {
        if (TICK_AUTOLOAD) {
          omp_set_lock(&POSITION_LOCK);
          last_origin.x = area_origin.x;
          last_origin.y = area_origin.y;
          last_origin.z = area_origin.z;
          omp_unset_lock(&POSITION_LOCK);
#ifdef PROFILE_TIME
          start_duration(&DATA_TIME);
#endif
          load_surroundings(&last_origin);
          tick_load_chunks(&last_origin);
          tick_biogen();
#ifdef PROFILE_TIME
          end_duration(&DATA_TIME);
#endif
        }
        nap(10);
      }
      DATA_DONE = 1;
    } else {
      fprintf(stderr, "Error: unexpected thread ID %d. Aborting.\n", thread_id);
      core_shutdown(-1);
    }
  } // end parallel block

}

void cleanup(void) {
  cleanup_worldgen();
  cleanup_entities();
  cleanup_data();
  cleanup_ui();
  cleanup_textures();
  cleanup_shaders();
  cleanup_render();
  cleanup_elf_forest_data();
  //cleanup_jobs();
}

void core_shutdown(int returnval) {
  if (returnval == 0) {
    printf("Shutting down normally.\n");
  } else {
    printf("Shutting down due to error (code %d).\n", returnval);
  }
  SHUTDOWN = 1;
  int patience = 100;
  // Wait for all threads to finish:
  while (patience > 0 && (!RENDERING_DONE || !DATA_DONE)) {
    nap(5);
    patience -= 1;
  }
  cleanup();
  glfwTerminate();
  exit(returnval);
}

void init_tick(int autoload) {
  TICK_AUTOLOAD = autoload;
  omp_init_lock(&POSITION_LOCK);
  omp_init_lock(&DATA_LOCK);
}

int ticks_expected(void) {
  static int first = 1;
  static double stored = 0.0;
  static double lasttime;
  double curtime, elapsed;
  if (first) {
    lasttime = glfwGetTime();
    stored = 0;
    first = 0;
  }
  curtime = glfwGetTime();
  elapsed = curtime - lasttime;
  lasttime = curtime;
  float ticks_due = ((float) elapsed) * TICKS_PER_SECOND + stored;
  stored = ticks_due - floor(ticks_due);
  return (int) floor(ticks_due);
}

void tick(int steps) {
  tick_general_controls();
  if (steps == 0 || PAUSED) {
    clear_edge_triggers();
    nap(50);
    return;
  }
  adjust_physics_resolution();
  size_t i, j;
  for (i = 0; i < steps; ++i) {
    TICK_COUNT = (TICK_COUNT + 1) % TICKS_PER_SECOND_I;
    tick_motion_controls();
    tick_interaction();
    for (j = 0; j < PHYS_SUBSTEPS; ++j) {
      tick_active_entities();
    }
    warp_space(ACTIVE_AREA, &(PLAYER->pos));
    // TODO: tick cells
    //tick_cells(ACTIVE_AREA);
    update_rate(&TICKRATE);
  }
  clear_edge_triggers();
}
