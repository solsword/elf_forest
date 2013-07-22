// world.c
// Structures and functions for representing the world.

#include <stdint.h>
#include <stdlib.h>
// DEBUG
#include <stdio.h>

#include "blocks.h"
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

void compute_exposure(frame *f, uint16_t cx, uint16_t cy, uint16_t cz) {
  // Compute exposure...
  // Get frame coords from chunk coords:
  int fx = (cx << CHUNK_BITS) - HALF_FRAME;
  int fy = (cy << CHUNK_BITS) - HALF_FRAME;
  int fz = (cz << CHUNK_BITS) - HALF_FRAME;
  int x, y, z;
  for (x = fx; x < fx + CHUNK_SIZE; ++x) {
    for (y = fy; y < fy + CHUNK_SIZE; ++y) {
      for (z = fz; z < fz + CHUNK_SIZE; ++z) {
        block b = block_at(f, x, y, z);
        if (is_invisible(b)) {
          //printf("invis: 0x%04x\n", b);
          set_block(f, x, y, z, set_exposed(b));
        } else {
          //printf("vis: 0x%04x\n", b);
          block ba = block_above(f, x, y, z);
          block bb = block_below(f, x, y, z);
          block bn = block_north(f, x, y, z);
          block bs = block_south(f, x, y, z);
          block be = block_east(f, x, y, z);
          block bw = block_west(f, x, y, z);
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
              set_block(f, x, y, z, set_exposed(b));
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
              set_block(f, x, y, z, set_exposed(b));
            }
          }
        }
      }
    }
  }
}

void setup_test_world(frame *f) {
  // Randomly place air, dirt and stone:
  int fx, fy, fz;
  int total = FULL_FRAME*FULL_FRAME*FULL_FRAME;
  int i = 0;
  printf("Placing blocks...");
  for (fx = -HALF_FRAME; fx < HALF_FRAME; ++fx) {
    for (fy = -HALF_FRAME; fy < HALF_FRAME; ++fy) {
      for (fz = -HALF_FRAME; fz < HALF_FRAME; ++fz) {
        //printf("xyz: %d, %d, %d\n", fx, fy, fz);
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
            set_block(f, fx, fy, fz, B_STONE);
          } else {
            set_block(f, fx, fy, fz, B_DIRT);
          }
        } else {
          set_block(f, fx, fy, fz, B_AIR);
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
  int fx, fy, fz;
  int terrain_height, soil_depth;
  float noise;
  int sea_level = TERRAIN_SEA_LEVEL;
  int total = FULL_FRAME*FULL_FRAME*FULL_FRAME;
  int i = 0;
  printf("Placing blocks...");
  for (fx = -HALF_FRAME; fx < HALF_FRAME; ++fx) {
    for (fy = -HALF_FRAME; fy < HALF_FRAME; ++fy) {
      noise = fractal_sxnoise_3d_table(
        (float) fx * TERRAIN_NOISE_SCALE, (float) fy * TERRAIN_NOISE_SCALE, 0.0,
        8, EX_TERRAIN, EX_TERRAIN_F
      );
      terrain_height = (int) ((noise + 1.0) * (HALF_FRAME - 4)) - HALF_FRAME;
      noise = fractal_sxnoise_3d_table(
        (float) fx * TERRAIN_NOISE_SCALE, (float) fy * TERRAIN_NOISE_SCALE, 1.1,
        3, BASE__NORM__NORM__NORM, NULL
      );
      soil_depth = (int) ((noise + 1.0) * (TERRAIN_DIRT_DEPTH/2)) + 1;
      for (fz = -HALF_FRAME; fz < HALF_FRAME; ++fz) {
        if (fz > terrain_height) {
          if (fz <= sea_level) {
            set_block(f, fx, fy, fz, B_WATER);
          } else {
            set_block(f, fx, fy, fz, B_AIR);
          }
        } else {
          if (fz == terrain_height && terrain_height >= sea_level) {
            set_block(f, fx, fy, fz, B_GRASS);
          } else if (terrain_height - fz <= soil_depth) {
            set_block(f, fx, fy, fz, B_DIRT);
          } else {
            set_block(f, fx, fy, fz, B_STONE);
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
  uint32_t cx, cy, cz;
  printf("Computing exposure and compiling chunks...");
  int total = FRAME_SIZE*FRAME_SIZE*FRAME_SIZE;
  int i = 0;
  for (cx = 0; cx < FRAME_SIZE; ++cx) {
    for (cy = 0; cy < FRAME_SIZE; ++cy) {
      for (cz = 0; cz < FRAME_SIZE; ++cz) {
        i += 1;
        printf("\rComputing exposure and compiling chunks... %d/%d", i, total);
        compute_exposure(f, cx, cy, cz);
        compile_chunk(f, cx, cy, cz);
      }
    }
  }
  printf("\n  ...done.\n");
}
