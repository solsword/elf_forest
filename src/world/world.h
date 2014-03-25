#ifndef WORLD_H
#define WORLD_H

// world.h
// Structures and functions for representing the world.

#include <stdint.h>
#include <math.h>

#include <GL/gl.h>

#include "blocks.h"

#include "datatypes/vector.h"
#include "datatypes/list.h"
#include "datatypes/octree.h"
#include "graphics/vbo.h"

/****************
 * Enumerations *
 ****************/

// Layers of a chunk:
typedef enum {
  L_TRANSLUCENT,
  L_TRANSPARENT,
  L_OPAQUE,
  N_LAYERS
} layer;

/**************
 * Structures *
 **************/

// Block position within a region:
struct region_pos_s;
typedef struct region_pos_s region_pos;

// Chunk position within a region:
struct region_chunk_pos_s;
typedef struct region_chunk_pos_s region_chunk_pos;

// Flags for a chunk:
typedef uint16_t chunk_flag;

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

// Frame-coordinate integer block position. 0, 0, 0 is at the center of the
// frame, with the edges at -HALF_FRAME and HALF_FRAME - 1.
struct frame_pos_s;
typedef struct frame_pos_s frame_pos;

/***********
 * Globals *
 ***********/

extern frame MAIN_FRAME;

/*************
 * Constants *
 *************/

#define CHUNK_BITS 4
#define CHUNK_SIZE (1 << CHUNK_BITS)
#define FRAME_BITS 4
#define FRAME_SIZE (1 << FRAME_BITS)
#define FULL_FRAME (CHUNK_SIZE*FRAME_SIZE)
#define HALF_FRAME (FULL_FRAME >> 1)
#define CH_MASK (CHUNK_SIZE - 1) // Chunk mask
#define FR_MASK (FRAME_SIZE - 1) // Frame mask
#define FC_MASK (CHUNK_SIZE*FRAME_SIZE - 1) // Frame coordinate mask

static chunk_flag const           CF_LOADED = 0x0001;
static chunk_flag const     CF_NEEDS_RELOAD = 0x0002;
static chunk_flag const  CF_NEEDS_RECOMIPLE = 0x0004;

/*************************
 * Structure Definitions *
 *************************/

struct region_pos_s {
  long int x, y, z;
};

struct region_chunk_pos_s {
  int x, y, z;
};

struct chunk_index_s {
  unsigned int x, y, z;
};

// (16 * 16 * 16) * 16 = 65536 = 8 KB
// (16 * 16 * 16) * 32 = 131072 = 16 KB
struct chunk_s {
  block blocks[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE]; // Block data.
  block_flag block_flags[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE]; // Block flags.
  // TODO: merge those
  vertex_buffer layers[N_LAYERS]; // The vertex buffers.
  region_chunk_pos rpos; // Absolute location within the region.
  list *block_entities; // Tile entities.
  chunk_flag chunk_flags; // Flags
};

struct frame_index_s {
  unsigned int x, y, z;
};

struct frame_chunk_index_s {
  unsigned int x, y, z;
};

struct frame_pos_s {
  int x, y, z;
};

// (16 * 16 * 16) * 65536 = 32 MB
// (16 * 16 * 16) * 131072 = 64 MB
// (32 * 32 * 32) * 65536 = 256 MB
struct frame_s {
  chunk chunks[FRAME_SIZE*FRAME_SIZE*FRAME_SIZE]; // chunk data
  region_chunk_pos region_offset; // location of frame origin within the region
  frame_chunk_index chunk_offset; // data offset
    // (to avoid having to shuffle data around within the array all the time)
  list *entities; // active entities
  octree *oct; // An octree for the frame
};

/********************
 * Inline Functions *
 ********************/

// Coordinate conversions:
// Note that these are hand-inlined in a few places for speed.

static inline void rcpos__rpos(
  region_chunk_pos *rcpos,
  region_pos *rpos
) {
  rpos->x = ((long int) rcpos->x) << CHUNK_BITS;
  rpos->y = ((long int) rcpos->y) << CHUNK_BITS;
  rpos->z = ((long int) rcpos->z) << CHUNK_BITS;
}

static inline void rpos__rcpos(
  region_pos *rpos,
  region_chunk_pos *rcpos
) {
  rcpos->x = rpos->x >> CHUNK_BITS;
  rcpos->y = rpos->y >> CHUNK_BITS;
  rcpos->z = rpos->z >> CHUNK_BITS;
}

static inline void cidx__rpos(chunk *c, chunk_index *idx, region_pos *pos) {
  rcpos__rpos(&(c->rpos), pos);
  pos->x += idx->x;
  pos->y += idx->y;
  pos->z += idx->z;
}

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

static inline void fidx__cidx(frame_index *fidx, chunk_index *cidx) {
  cidx->x = fidx->x & CH_MASK;
  cidx->y = fidx->y & CH_MASK;
  cidx->z = fidx->z & CH_MASK;
}

static inline void fidx__fpos(frame_index *idx, frame_pos *pos) {
  pos->x = (idx->x - HALF_FRAME);
  pos->y = (idx->y - HALF_FRAME);
  pos->z = (idx->z - HALF_FRAME);
}

static inline void fcidx__rcpos(
  frame_chunk_index *idx,
  frame *f,
  region_chunk_pos *pos
) {
  pos->x = f->region_offset.x + idx->x - (FRAME_SIZE / 2);
  pos->y = f->region_offset.y + idx->y - (FRAME_SIZE / 2);
  pos->z = f->region_offset.z + idx->z - (FRAME_SIZE / 2);
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
      ((idx.x + f->chunk_offset.x) & FR_MASK) +
      (((idx.y + f->chunk_offset.y) & FR_MASK) << FRAME_BITS) +
      (((idx.z + f->chunk_offset.z) & FR_MASK) << (FRAME_BITS*2))
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

static inline block_flag c_get_flags(
  chunk *c,
  chunk_index idx
) {
  return (c->block_flags)[
    (idx.x & CH_MASK) +
    ((idx.y & CH_MASK) << CHUNK_BITS) +
    ((idx.z & CH_MASK) << (CHUNK_BITS*2))
  ];
}

static inline void c_put_flags(
  chunk *c,
  chunk_index idx,
  block_flag flags
) {
  (c->block_flags)[
    (idx.x & CH_MASK) +
    ((idx.y & CH_MASK) << CHUNK_BITS) +
    ((idx.z & CH_MASK) << (CHUNK_BITS*2))
  ] = flags;
}

static inline void c_set_flags(
  chunk *c,
  chunk_index idx,
  block_flag flags
) {
  (c->block_flags)[
    (idx.x & CH_MASK) +
    ((idx.y & CH_MASK) << CHUNK_BITS) +
    ((idx.z & CH_MASK) << (CHUNK_BITS*2))
  ] |= flags;
}

static inline void c_clear_flags(
  chunk *c,
  chunk_index idx,
  block_flag flags
) {
  (c->block_flags)[
    (idx.x & CH_MASK) +
    ((idx.y & CH_MASK) << CHUNK_BITS) +
    ((idx.z & CH_MASK) << (CHUNK_BITS*2))
  ] &= ~flags;
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
static block const OUT_OF_RANGE = 0;

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

static inline void c_get_neighbors(
  chunk *c,
  chunk_index idx,
  block *ba, block *bb,
  block *bn, block *bs,
  block *be, block *bw
) {
  // note that even underflow should wrap correctly here
  chunk_index nbr;
  nbr.x = idx.x;
  nbr.y = idx.y;
  nbr.z = idx.z + 1;
  *ba = (idx.z < CHUNK_SIZE - 1) * c_get_block(c, nbr);
  nbr.z = idx.z - 1;
  *bb = (idx.z > 0) *c_get_block(c, nbr);
  nbr.z = idx.z;
  nbr.y = idx.y + 1;
  *bn = (idx.y < CHUNK_SIZE - 1) * c_get_block(c, nbr);
  nbr.y = idx.y - 1;
  *bs = (idx.y > 0) * c_get_block(c, nbr);
  nbr.y = idx.y;
  nbr.x = idx.x + 1;
  *be = (idx.x < CHUNK_SIZE - 1) * c_get_block(c, nbr);
  nbr.x = idx.x - 1;
  *bw = (idx.x > 0) * c_get_block(c, nbr);
}

/*************
 * Functions *
 *************/

// Initializes the given frame:
void setup_frame(frame *f, region_chunk_pos *roff);

// Cleans up memory allocated by the given frame.
void cleanup_frame(frame *f);

// Initializes the given chunk:
void setup_chunk(chunk *c, region_chunk_pos *rpos);

// Cleans up memory allocated by the given chunk.
void cleanup_chunk(chunk *c);

// Ticks all blocks attached to the given frame:
void tick_blocks(frame *f);

#endif // ifndef WORLD_H
