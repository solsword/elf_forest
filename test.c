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
  setup_entities();
  setup_data();
  region_chunk_pos roff = {.x=0, .y=0, .z=0};
  setup_frame(&MAIN_FRAME, &roff);
  test_compile_frame(&MAIN_FRAME);
  printf("...done.\n");
  test_spawn_player(&MAIN_FRAME);
  // DEBUG: enable and set up some simple lighting:
  glEnable( GL_LIGHTING );
  glShadeModel( GL_FLAT );
  float pos[4] = { 0, 0, -50, 0 };
  float ambient[4] = { 0.3, 0.3, 0.3, 1.0 };
  float diffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
  float specular[4] = { 0.0, 0.0, 0.0, 1.0 };
  float grey[4] = { 0.5, 0.5, 0.5, 1.0 };
  float white[4] = { 1.0, 1.0, 1.0, 1.0 };
  glLightfv( GL_LIGHT0, GL_POSITION, pos);
  glLightfv( GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv( GL_LIGHT0, GL_SPECULAR, specular);
  glMaterialfv( GL_FRONT, GL_AMBIENT, grey );
  glMaterialfv( GL_FRONT, GL_DIFFUSE, white );
  glMaterialfv( GL_FRONT, GL_SPECULAR, white );
  glMaterialf( GL_FRONT, GL_SHININESS, 50.0 );
  glEnable( GL_LIGHT0 );
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
  printf("Computing exposure and compiling chunks...");
  for (idx.x = 0; idx.x < FRAME_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < FRAME_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < FRAME_SIZE; ++idx.z) {
        load_chunk(chunk_at(f, idx));
      }
    }
  }
  printf("\n  ...done.\n");
}

void test_spawn_player(frame *f) {
  vector pos = { .x=0.0, .y=0.0, .z=18.0 };
  PLAYER = spawn_entity("tester", &pos, &MAIN_FRAME);
}
