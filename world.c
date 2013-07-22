// world.c
// Structures and functions for representing the world.

#include <stdint.h>
#include <stdlib.h>
// DEBUG
#include <stdio.h>

#include "blocks.h"
#include "vbo.h"
#include "world.h"
#include "display.h"
#include "noise.h"

/***********
 * Globals *
 ***********/

frame MAIN_FRAME;

/*************
 * Functions *
 *************/

void compute_exposure(frame *f, frame_chunk_index idx) {
  frame_pos base, pos;
  // Get frame coords from chunk coords:
  fcidx__fpos(&idx, &base);
  for (pos.x = base.x; pos.x < base.x + CHUNK_SIZE; ++pos.x) {
    for (pos.y = base.y; pos.y < base.y + CHUNK_SIZE; ++pos.y) {
      for (pos.z = base.z; pos.z < base.z + CHUNK_SIZE; ++pos.z) {
        block b = block_at(f, pos);
        if (is_invisible(b)) {
          //printf("invis: 0x%04x\n", b);
          set_block(f, pos, set_exposed(b));
        } else {
          //printf("vis: 0x%04x\n", b);
          block ba = block_above(f, pos);
          block bb = block_below(f, pos);
          block bn = block_north(f, pos);
          block bs = block_south(f, pos);
          block be = block_east(f, pos);
          block bw = block_west(f, pos);
          if (is_translucent(b)) {
            //printf("TL!\n");
            if (
              !(is_opaque(ba) || shares_translucency(b, ba))
              || !(is_opaque(bb) || shares_translucency(b, bb))
              || !(is_opaque(bn) || shares_translucency(b, bn))
              || !(is_opaque(bs) || shares_translucency(b, bs))
              || !(is_opaque(be) || shares_translucency(b, be))
              || !(is_opaque(bw) || shares_translucency(b, bw))
            ) {
              //printf("exposed\n");
              set_block(f, pos, set_exposed(b));
            }
          } else {
            if (
              !is_opaque(ba)
              || !is_opaque(bb)
              || !is_opaque(bn)
              || !is_opaque(bs)
              || !is_opaque(be)
              || !is_opaque(bw)
            ) {
              //printf("exposed\n");
              set_block(f, pos, set_exposed(b));
            }
          }
        }
      }
    }
  }
}

void cleanup_frame(frame *f) {
  frame_chunk_index idx;
  for (idx.x = 0; idx.x < FRAME_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < FRAME_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < FRAME_SIZE; ++idx.z) {
        cleanup_chunk(chunk_at(f, idx));
      }
    }
  }
}

void cleanup_chunk(chunk *c) {
  cleanup_vertex_buffer(&(c->opaque_vertices));
  cleanup_vertex_buffer(&(c->translucent_vertices));
}

void setup_test_world(frame *f) {
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

void setup_test_world_terrain(frame *f) {
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
