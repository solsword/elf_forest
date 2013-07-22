#ifndef WORLD_H
#define WORLD_H

// world.h
// Structures and functions for representing the world.

#include <stdint.h>
#include <math.h>

#include <GL/gl.h>

#include "blocks.h"

/**************
 * Structures *
 **************/

// An NxNxN chunk of the grid:
struct chunk_s;
typedef struct chunk_s chunk;

// An NxNxN-chunk frame:
struct frame_s;
typedef struct frame_s frame;

/***********
 * Globals *
 ***********/

extern frame MAIN_FRAME;

/*************
 * Constants *
 *************/

#define CHUNK_BITS 3
#define CHUNK_SIZE (1 << CHUNK_BITS)
#define FRAME_BITS 3
#define FRAME_SIZE (1 << FRAME_BITS)
#define FULL_FRAME (CHUNK_SIZE*FRAME_SIZE)
#define HALF_FRAME (FULL_FRAME >> 1)
#define CH_MASK (CHUNK_SIZE - 1) // Chunk mask
#define FR_MASK (FRAME_SIZE - 1) // Frame mask
#define FC_MASK (CHUNK_SIZE*FRAME_SIZE - 1) // Frame coordinate mask

#define TERRAIN_NOISE_SCALE 1.0/8.0
#define TERRAIN_SEA_LEVEL -5
#define TERRAIN_DIRT_DEPTH 10

/*************************
 * Structure Definitions *
 *************************/

struct chunk_s { // 4096*16 + 32*4 + 32 + 16*2 = 65664 =~ 8 KB
  block blocks[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE]; // Block data.
  GLuint opaque_vertex_bo; // The opaque vertex buffer object.
  GLuint opaque_index_bo; // The opaque index buffer object.
  GLuint translucent_vertex_bo; // The translucent vertex buffer object.
  GLuint translucent_index_bo; // The translucent index buffer object.
  uint32_t o_vcount; // The opaque vertex count for the buffer objects.
  uint32_t t_vcount; // The translucent vertex count for the buffer objects.
  uint16_t x, y; // Absolute location within the region.
};

struct frame_s { // (32 * 32 * 32) * 65664 + 16*2 + 8*3 = 2151678008 =~ 256.5 MB
  chunk chunks[FRAME_SIZE*FRAME_SIZE*FRAME_SIZE]; // Chunk data.
  uint16_t wx, wy; // World offsets
  uint8_t cx_o, cy_o, cz_o; // Data offsets
    // (to avoid having to shuffle data around within the array all the time)
};

/********************
 * Inline Functions *
 ********************/

static inline chunk* chunk_at(frame *f, uint16_t cx, uint16_t cy, uint16_t cz) {
  return &(
    (f->chunks)[
      ((cx + f->cx_o) & FR_MASK) +
      (((cy + f->cy_o) & FR_MASK) << FRAME_BITS) +
      (((cz + f->cz_o) & FR_MASK) << (FRAME_BITS*2))
    ]
  );
}

static inline block c_get_block(
  chunk *c,
  uint16_t lx,
  uint16_t ly,
  uint16_t lz
) {
  return (c->blocks)[
    (lx & CH_MASK) +
    ((ly & CH_MASK) << CHUNK_BITS) +
    ((lz & CH_MASK) << (CHUNK_BITS*2))
  ];
}

static inline void c_put_block(
  chunk *c,
  uint16_t lx,
  uint16_t ly,
  uint16_t lz,
  block b
) {
  (c->blocks)[
    (lx & CH_MASK) +
    ((ly & CH_MASK) << CHUNK_BITS) +
    ((lz & CH_MASK) << (CHUNK_BITS*2))
  ] = b;
}

static inline block block_at(frame *f, int x, int y, int z) {
  // -256 -- 255 (frame coords) => 0 -- 511 (array coords)
  uint16_t ax = (x + HALF_FRAME) & FC_MASK;
  uint16_t ay = (y + HALF_FRAME) & FC_MASK;
  uint16_t az = (z + HALF_FRAME) & FC_MASK;
  return c_get_block(
    chunk_at(
      f,
      ax >> CHUNK_BITS,
      ay >> CHUNK_BITS,
      az >> CHUNK_BITS
    ),
    ax, ay, az
  );
}

static inline void set_block(frame *f, int x, int y, int z, block b) {
  // -256 -- 255 (frame coords) => 0 -- 511 (array coords)
  uint16_t ax = (x + HALF_FRAME) & FC_MASK;
  uint16_t ay = (y + HALF_FRAME) & FC_MASK;
  uint16_t az = (z + HALF_FRAME) & FC_MASK;
  c_put_block(
    chunk_at(
      f,
      ax >> CHUNK_BITS,
      ay >> CHUNK_BITS,
      az >> CHUNK_BITS
    ),
    ax, ay, az,
    b
  );
}

static inline block block_above(frame *f, int x, int y, int z) {
  return (
    (z < HALF_FRAME - 1) && block_at(f, x, y, z+1)
  ) || B_VOID;
}

static inline block block_below(frame *f, int x, int y, int z) {
  return (
    (z > 0) && block_at(f, x, y, z-1)
  ) || B_VOID;
}

static inline block block_north(frame *f, int x, int y, int z) {
  return (
    (y < HALF_FRAME - 1) && block_at(f, x, y+1, z)
  ) || B_VOID;
}

static inline block block_south(frame *f, int x, int y, int z) {
  return (
    (y > 0) && block_at(f, x, y-1, z)
  ) || B_VOID;
}

static inline block block_east(frame *f, int x, int y, int z) {
  return (
    (x < HALF_FRAME - 1) && block_at(f, x+1, y, z)
  ) || B_VOID;
}

static inline block block_west(frame *f, int x, int y, int z) {
  return (
    (x > 0) && block_at(f, x-1, y, z)
  ) || B_VOID;
}

/*************
 * Functions *
 *************/

// Computes block exposure for the given chunk (hence the need for the whole
// frame). All of the frame's chunk data should already be loaded.
void compute_exposure(frame *f, uint16_t cx, uint16_t cy, uint16_t cz);

// Puts some sparse junk data into the main frame for testing.
void setup_test_world(frame *f);

// Uses noise to create a test frame.
void setup_test_world_terrain(frame *f);

// Computes exposure for and compiles every chunk in the given frame.
void test_compile_frame(frame *f);

#endif // ifndef WORLD_H
