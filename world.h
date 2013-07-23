#ifndef WORLD_H
#define WORLD_H

// world.h
// Structures and functions for representing the world.

#include <stdint.h>
#include <math.h>

#include <GL/gl.h>

#include "blocks.h"
#include "list.h"
#include "vbo.h"
#include "vector.h"
#include "octree.h"

/**************
 * Structures *
 **************/

// An NxNxN chunk of the grid:
struct chunk_s;
typedef struct chunk_s chunk;

// Picks out a block within a chunk:
struct chunk_index_s;
typedef struct chunk_index_s chunk_index;

// An NxNxN-chunk frame:
struct frame_s;
typedef struct frame_s frame;

// Picks out a chunk within a frame:
struct frame_chunk_index_s;
typedef struct frame_chunk_index_s frame_chunk_index;

// Picks out a block within a frame:
struct frame_index_s;
typedef struct frame_index_s frame_index;

// Frame-coordinate integer position. 0, 0, 0 is at the center of the frame,
// with the edges at -HALF_FRAME and HALF_FRAME - 1.
struct frame_pos_s;
typedef struct frame_pos_s frame_pos;

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

// (16 * 16 * 16) * 16 = 65536 = 8 KB
// (16 * 16 * 16) * 32 = 131072 = 16 KB
struct chunk_s {
  block blocks[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE]; // Block data.
  vertex_buffer opaque_vertices; // The opaque vertices.
  vertex_buffer translucent_vertices; // The translucent vertices.
  uint16_t x, y; // Absolute location within the region.
  list *block_entities; // Tile entities.
};

struct chunk_index_s {
  uint16_t x, y, z;
};

// (16 * 16 * 16) * 65536 = 32 MB
// (16 * 16 * 16) * 131072 = 64 MB
// (32 * 32 * 32) * 65536 = 256 MB
struct frame_s {
  chunk chunks[FRAME_SIZE*FRAME_SIZE*FRAME_SIZE]; // Chunk data.
  uint16_t wx, wy; // World offsets
  uint8_t cx_o, cy_o, cz_o; // Data offsets
    // (to avoid having to shuffle data around within the array all the time)
  list *entities; // Active entities
  octree *oct; // An octree for the frame
};

struct frame_index_s {
  uint16_t x, y, z;
};

struct frame_chunk_index_s {
  uint16_t x, y, z;
};

struct frame_pos_s {
  int x, y, z;
};

/********************
 * Inline Functions *
 ********************/

// Coordinate conversions:
// Note that these are hand-inlined in various places for speed.

static inline void fpos__fidx(frame_pos *pos, frame_index *idx) {
  idx->x = (pos->x + HALF_FRAME) & FC_MASK;
  idx->y = (pos->y + HALF_FRAME) & FC_MASK;
  idx->z = (pos->z + HALF_FRAME) & FC_MASK;
}
static inline void fpos__cidx(frame_pos *pos, chunk_index *idx) {
  idx->x = (pos->x + HALF_FRAME) & CH_MASK;
  idx->y = (pos->y + HALF_FRAME) & CH_MASK;
  idx->z = (pos->z + HALF_FRAME) & CH_MASK;
}

static inline void fpos__fcidx(frame_pos *pos, frame_chunk_index *idx) {
  idx->x = ((pos->x + HALF_FRAME) & FC_MASK) >> CHUNK_BITS;
  idx->y = ((pos->y + HALF_FRAME) & FC_MASK) >> CHUNK_BITS;
  idx->z = ((pos->z + HALF_FRAME) & FC_MASK) >> CHUNK_BITS;
}

static inline void fidx__cidx(frame_index *fidx, chunk_index *cidx) {
  cidx->x = fidx->x & CH_MASK;
  cidx->y = fidx->y & CH_MASK;
  cidx->z = fidx->z & CH_MASK;
}

static inline void fidx__fcidx(frame_index *fidx, frame_chunk_index *fcidx) {
  fcidx->x = fidx->x >> CHUNK_BITS;
  fcidx->y = fidx->y >> CHUNK_BITS;
  fcidx->z = fidx->z >> CHUNK_BITS;
}

static inline void fidx__fpos(frame_index *idx, frame_pos *pos) {
  pos->x = (idx->x - HALF_FRAME);
  pos->y = (idx->y - HALF_FRAME);
  pos->z = (idx->z - HALF_FRAME);
}

static inline void fcidx__fpos(frame_chunk_index *idx, frame_pos *pos) {
  pos->x = (idx->x << CHUNK_BITS) - HALF_FRAME;
  pos->y = (idx->y << CHUNK_BITS) - HALF_FRAME;
  pos->z = (idx->z << CHUNK_BITS) - HALF_FRAME;
}

static inline void vec__fpos(vector *v, frame_pos *pos) {
  pos->x = fastfloor(v->x);
  pos->y = fastfloor(v->y);
  pos->z = fastfloor(v->z);
}

static inline void fpos__vec(frame_pos *pos, vector *v) {
  v->x = (float) pos->x;
  v->y = (float) pos->y;
  v->z = (float) pos->z;
}

// Indexing functions:
// These must be super-fast 'cause they crop up in all sorts of inner loops.

static inline chunk* chunk_at(frame *f, frame_chunk_index idx) {
  return &(
    (f->chunks)[
      ((idx.x + f->cx_o) & FR_MASK) +
      (((idx.y + f->cy_o) & FR_MASK) << FRAME_BITS) +
      (((idx.z + f->cz_o) & FR_MASK) << (FRAME_BITS*2))
    ]
  );
}

static inline block c_get_block(
  chunk *c,
  chunk_index idx
) {
  return (c->blocks)[
    (idx.x & CH_MASK) +
    ((idx.y & CH_MASK) << CHUNK_BITS) +
    ((idx.z & CH_MASK) << (CHUNK_BITS*2))
  ];
}

static inline void c_put_block(
  chunk *c,
  chunk_index idx,
  block b
) {
  (c->blocks)[
    (idx.x & CH_MASK) +
    ((idx.y & CH_MASK) << CHUNK_BITS) +
    ((idx.z & CH_MASK) << (CHUNK_BITS*2))
  ] = b;
}

static inline block block_at(frame *f, frame_pos pos) {
  chunk_index cidx;
  frame_chunk_index fcidx;
  // Note: these values will be out-of-range, but they'll be clamped by the
  // code in c_get_block and they won't overflow, so we're fine.
  cidx.x = (pos.x + HALF_FRAME) & FC_MASK;
  cidx.y = (pos.y + HALF_FRAME) & FC_MASK;
  cidx.z = (pos.z + HALF_FRAME) & FC_MASK;
  // These values will be correct:
  fcidx.x = cidx.x >> CHUNK_BITS;
  fcidx.y = cidx.y >> CHUNK_BITS;
  fcidx.z = cidx.z >> CHUNK_BITS;
  return c_get_block(
    chunk_at(
      f,
      fcidx
    ),
    cidx
  );
}

static inline block block_at_xyz(frame *f, int x, int y, int z) {
  chunk_index cidx;
  frame_chunk_index fcidx;
  // Note: these values will be out-of-range, but they'll be clamped by the
  // code in c_get_block and they won't overflow, so we're fine.
  cidx.x = (x + HALF_FRAME) & FC_MASK;
  cidx.y = (y + HALF_FRAME) & FC_MASK;
  cidx.z = (z + HALF_FRAME) & FC_MASK;
  // These values will be correct:
  fcidx.x = cidx.x >> CHUNK_BITS;
  fcidx.y = cidx.y >> CHUNK_BITS;
  fcidx.z = cidx.z >> CHUNK_BITS;
  return c_get_block(
    chunk_at(
      f,
      fcidx
    ),
    cidx
  );
}

static inline void set_block(frame *f, frame_pos pos, block b) {
  chunk_index cidx;
  frame_chunk_index fcidx;
  // Note: these values will be out-of-range, but they'll be clamped by the
  // code in c_get_block and they won't overflow, so we're fine.
  cidx.x = (pos.x + HALF_FRAME) & FC_MASK;
  cidx.y = (pos.y + HALF_FRAME) & FC_MASK;
  cidx.z = (pos.z + HALF_FRAME) & FC_MASK;
  // These values will be correct:
  fcidx.x = cidx.x >> CHUNK_BITS;
  fcidx.y = cidx.y >> CHUNK_BITS;
  fcidx.z = cidx.z >> CHUNK_BITS;
  c_put_block(
    chunk_at(
      f,
      fcidx
    ),
    cidx,
    b
  );
}

// These block neighbor functions will return 0x0000 when the pos argument is
// out-of-range. Given that block types *could* change, we'll define a constant
// here to represent that independent of blocks.h instead of just using B_VOID.
static const block OUT_OF_RANGE = 0;

static inline block block_above(frame *f, frame_pos pos) {
  return (pos.z < HALF_FRAME - 1) * block_at_xyz(f, pos.x, pos.y, pos.z+1);
}

static inline block block_below(frame *f, frame_pos pos) {
  return (pos.z > -HALF_FRAME) * block_at_xyz(f, pos.x, pos.y, pos.z-1);
}

static inline block block_north(frame *f, frame_pos pos) {
  return (pos.y < HALF_FRAME - 1) * block_at_xyz(f, pos.x, pos.y+1, pos.z);
}

static inline block block_south(frame *f, frame_pos pos) {
  return (pos.y > -HALF_FRAME) * block_at_xyz(f, pos.x, pos.y-1, pos.z);
}

static inline block block_east(frame *f, frame_pos pos) {
  return (pos.x < HALF_FRAME - 1) * block_at_xyz(f, pos.x+1, pos.y, pos.z);
}

static inline block block_west(frame *f, frame_pos pos) {
  return (pos.x > -HALF_FRAME) * block_at_xyz(f, pos.x-1, pos.y, pos.z);
}

/*************
 * Functions *
 *************/

// Computes block exposure for the given chunk (hence the need for the whole
// frame). All of the frame's chunk data should already be loaded.
void compute_exposure(frame *f, frame_chunk_index idx);

// Initializes the given frame:
void setup_frame(frame *f);

// Cleans up memory allocated by the given frame.
void cleanup_frame(frame *f);

// Initializes the given chunk:
void setup_chunk(chunk *c);

// Cleans up memory allocated by the given chunk.
void cleanup_chunk(chunk *c);

#endif // ifndef WORLD_H
