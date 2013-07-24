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
#include "entities.h"

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
  setup_frame(&MAIN_FRAME);
  test_setup_world_terrain(&MAIN_FRAME);
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
  return 0;
}

/*************
 * Functions *
 *************/

void test_setup_world_junk(frame *f) {
  // Randomly place air, dirt and stone:
  frame_pos pos;
  int total = FULL_FRAME*FULL_FRAME*FULL_FRAME;
  int i = 0;
  printf("Placing blocks...");
  for (pos.x = -HALF_FRAME; pos.x < HALF_FRAME; ++pos.x) {
    for (pos.y = -HALF_FRAME; pos.y < HALF_FRAME; ++pos.y) {
      for (pos.z = -HALF_FRAME; pos.z < HALF_FRAME; ++pos.z) {
        //printf("xyz: %d, %d, %d\n", pos.x, pos.y, pos.z);
        i += 1;
        if (i % 100 == 0) {
          printf("\rPlacing blocks... %d/%d", i, total);
        }
        if (i % 200 != 0) {
          continue;
        }
        float threshold = 0.8;
        if (rand() > threshold*RAND_MAX) {
          if (rand() > 0.4*RAND_MAX) {
            set_block(f, pos, B_STONE);
          } else {
            set_block(f, pos, B_DIRT);
          }
        } else {
          set_block(f, pos, B_AIR);
        }
      }
    }
  }
  printf("\n  ...done.\n");
  // Compute the exposure of each chunk and compile it.
  test_compile_frame(f);
}

void test_setup_world_terrain(frame *f) {
  // Place air, dirt, water, and stone using noise:
  frame_pos pos;
  int terrain_height, soil_depth;
  float noise;
  int sea_level = TERRAIN_SEA_LEVEL;
  int total = FULL_FRAME*FULL_FRAME*FULL_FRAME;
  int i = 0;
  printf("Placing blocks...");
  for (pos.x = -HALF_FRAME; pos.x < HALF_FRAME; ++pos.x) {
    for (pos.y = -HALF_FRAME; pos.y < HALF_FRAME; ++pos.y) {
      noise = fractal_sxnoise_3d_table(
        (float) pos.x * TERRAIN_NOISE_SCALE,
        (float) pos.y * TERRAIN_NOISE_SCALE,
        0.0,
        8, EX_TERRAIN, EX_TERRAIN_F
      );
      terrain_height = (int) ((noise + 1.0) * (HALF_FRAME - 4)) - HALF_FRAME;
      noise = fractal_sxnoise_3d_table(
        (float) pos.x * TERRAIN_NOISE_SCALE,
        (float) pos.y * TERRAIN_NOISE_SCALE,
        1.1,
        3, BASE__NORM__NORM__NORM, NULL
      );
      soil_depth = (int) ((noise + 1.0) * (TERRAIN_DIRT_DEPTH/2)) + 1;
      for (pos.z = -HALF_FRAME; pos.z < HALF_FRAME; ++pos.z) {
        if (pos.z > terrain_height) {
          if (pos.z <= sea_level) {
            set_block(f, pos, B_WATER);
          } else {
            set_block(f, pos, B_AIR);
          }
        } else {
          if (pos.z == terrain_height && terrain_height >= sea_level) {
            set_block(f, pos, B_GRASS);
          } else if (terrain_height - pos.z <= soil_depth) {
            set_block(f, pos, B_DIRT);
          } else {
            set_block(f, pos, B_STONE);
          }
        }
        i += 1;
        if (i % 100 == 0) {
          printf("\rPlacing blocks... %d/%d", i, total);
        }
      }
    }
  }
  printf("\n  ...done.\n");
  // Compute the exposure of each chunk and compile it.
  test_compile_frame(f);
}

void test_compile_frame(frame *f) {
  frame_chunk_index idx;
  printf("Computing exposure and compiling chunks...");
  int total = FRAME_SIZE*FRAME_SIZE*FRAME_SIZE;
  int i = 0;
  for (idx.x = 0; idx.x < FRAME_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < FRAME_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < FRAME_SIZE; ++idx.z) {
        i += 1;
        printf("\rComputing exposure and compiling chunks... %d/%d", i, total);
        compute_exposure(f, idx);
        compile_chunk(f, idx);
      }
    }
  }
  printf("\n  ...done.\n");
}

void test_spawn_player(frame *f) {
  vector pos = { .x=0.0, .y=0.0, .z=1.0 };
  PLAYER = spawn_entity("elf", &pos, &MAIN_FRAME);
}
