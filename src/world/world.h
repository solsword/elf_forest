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
#include "datatypes/map.h"
#include "datatypes/octree.h"
#include "graphics/vbo.h"

/**********
 * Macros *
 **********/

// Structures for storing approximate data. N defines the scale (a structure
// for N=X stores 1/2^X as much info as the base structure).
#define APPROX_DATA_TN(N) approx_data_ ## N
#define APPROX_DATA_SN(N) approx_data_ ## N ## _s
#define APPROX_DATA_STRUCT(N) \
  struct APPROX_DATA_SN(N) { \
    block blocks[ \
      (1 << (CHUNK_BITS - N)) * \
      (1 << (CHUNK_BITS - N)) * \
      (1 << (CHUNK_BITS - N)) \
    ]; \
    block_flag block_flags[ \
      (1 << (CHUNK_BITS - N)) * \
      (1 << (CHUNK_BITS - N)) * \
      (1 << (CHUNK_BITS - N)) \
    ]; \
  }

// The function names for approximate block getting/putting routines (function
// body macros are defined in world.c).
#define CA_GET_BLOCK_FN(SCALE) ca_get_block_ ## SCALE
#define CA_GET_BLOCK_SIG(NAME) \
  block NAME( \
    chunk_approximation const * const ca, \
    chunk_index const * const idx \
  )

#define CA_PUT_BLOCK_FN(SCALE) ca_put_block_ ## SCALE
#define CA_PUT_BLOCK_SIG(NAME) \
 void NAME(chunk_approximation *ca, chunk_index const * const idx, block b)

#define CA_GET_FLAGS_FN(SCALE) ca_get_flags_ ## SCALE
#define CA_GET_FLAGS_SIG(NAME) \
  block_flag NAME( \
    chunk_approximation const * const ca, \
    chunk_index const * const idx \
  )

#define CA_PUT_FLAGS_FN(SCALE) ca_put_flags_ ## SCALE
#define CA_PUT_FLAGS_SIG(NAME) \
  void NAME( \
    chunk_approximation *ca, \
    chunk_index const * const idx, \
    block_flag flags \
  )

#define CA_SET_FLAGS_FN(SCALE) ca_set_flags_ ## SCALE
#define CA_SET_FLAGS_SIG(NAME) \
  void NAME( \
    chunk_approximation *ca, \
    chunk_index const * const idx, \
    block_flag flags \
  )

#define CA_CLEAR_FLAGS_FN(SCALE) ca_clear_flags_ ## SCALE
#define CA_CLEAR_FLAGS_SIG(NAME) \
  void NAME( \
    chunk_approximation *ca, \
    chunk_index const * const idx, \
    block_flag flags \
  )

#define DECLARE_APPROX_FN_VARIANTS(SIG_MACRO,FN_MACRO) \
  typedef SIG_MACRO((*FN_MACRO(ptr))); \
  SIG_MACRO(FN_MACRO(1)); \
  SIG_MACRO(FN_MACRO(2)); \
  SIG_MACRO(FN_MACRO(3)); \
  SIG_MACRO(FN_MACRO(4)); \
  extern FN_MACRO(ptr) FN_MACRO(table)[N_LODS];

#define DECLARE_APPROX_FN_VARIANTS_TABLE(SIG_MACRO,FN_MACRO) \
  /* Function pointer table (index by LOD): */ \
  FN_MACRO(ptr) FN_MACRO(table)[N_LODS] = { \
    NULL, \
    &FN_MACRO(1), \
    &FN_MACRO(2), \
    &FN_MACRO(3), \
    &FN_MACRO(4) \
  };

/****************
 * Enumerations *
 ****************/

// Layers of a chunk:
typedef enum {
  L_OPAQUE = 0,
  L_TRANSPARENT = 1,
  L_TRANSLUCENT = 2,
  N_LAYERS = 3
} layer;

// The level-of-detail for a chunk:
typedef enum {
  LOD_BASE = 0,
  LOD_HALF = 1,
  LOD_QUARTER = 2,
  LOD_EIGHTH = 3,
  LOD_SIXTEENTH = 4,
  N_LODS = 5
} lod;

// Whether something is a chunk or an approximation thereof:
typedef enum {
  CA_TYPE_NOT_LOADED,
  CA_TYPE_CHUNK,
  CA_TYPE_APPROXIMATION
} capprox_type;

/**************
 * Structures *
 **************/

// Defines the size of a region in blocks:
typedef int64_t r_pos_t;
// Defines the size of a region in chunks. Must be < the size of r_pos_t:
typedef int32_t r_cpos_t;

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

// Holds (a) lower-resolution representation(s) of a chunk:
struct chunk_approximation_s;
typedef struct chunk_approximation_s chunk_approximation;

// Holds a pointer to either a chunk or a chunk approximation, along with a
// flag indicating which.
struct chunk_or_approx_s;
typedef struct chunk_or_approx_s chunk_or_approx;

// Holds approximate block data at one of several scales:
union approx_data_u;
typedef union approx_data_u approx_data;

// Block data structures for approximate chunks at various scales (1/2^N):
// (see the macros above)
struct APPROX_DATA_SN(1); typedef struct APPROX_DATA_SN(1) APPROX_DATA_TN(1);
struct APPROX_DATA_SN(2); typedef struct APPROX_DATA_SN(2) APPROX_DATA_TN(2);
struct APPROX_DATA_SN(3); typedef struct APPROX_DATA_SN(3) APPROX_DATA_TN(3);
struct APPROX_DATA_SN(4); typedef struct APPROX_DATA_SN(4) APPROX_DATA_TN(4);

// Macros and types for the size of a chunk:
#define CHUNK_BITS 4
#define CHUNK_SIZE (1 << CHUNK_BITS)
#define CH_MASK (CHUNK_SIZE - 1) // Chunk mask
typedef uint8_t ch_idx_t; // Needs to be big enough to hold CHUNK_BITS bits.

// Picks out a block within a chunk:
struct chunk_index_s;
typedef struct chunk_index_s chunk_index;

/* TODO: get rid of these
// An NxNxN-chunk frame:
// TODO: get rid of this!
struct frame_s;
typedef struct frame_s frame;

// Macros and types for the size of a frame:
#define FRAME_BITS 4
#define FRAME_SIZE (1 << FRAME_BITS)
#define FULL_FRAME (CHUNK_SIZE*FRAME_SIZE)
#define HALF_FRAME (FULL_FRAME >> 1)
#define FR_MASK (FRAME_SIZE - 1) // Frame mask
#define FC_MASK (CHUNK_SIZE*FRAME_SIZE - 1) // Frame block position mask
typedef uint32_t fr_idx_t; // Needs to hold FRAME_BITS * CHUNK_BITS bits.
typedef uint8_t fr_cidx_t; // Needs to hold FRAME_BITS bits.
typedef uint8_t fr_pos_t; // Needs to hold FRAME_BITS bits.

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
*/

/*************
 * Constants *
 *************/

static chunk_flag const              CF_LOADED = 0x0001;
static chunk_flag const            CF_COMPILED = 0x0002;
static chunk_flag const     CF_COMPILE_ON_LOAD = 0x0004;
static chunk_flag const      CF_QUEUED_TO_LOAD = 0x0008;
static chunk_flag const   CF_QUEUED_TO_COMPILE = 0x0010;

/*************************
 * Structure Definitions *
 *************************/

struct region_pos_s {
  r_pos_t x, y, z;
};

struct region_chunk_pos_s {
  r_cpos_t x, y, z;
};

struct chunk_index_s {
  ch_idx_t x, y, z;
};

// (16 * 16 * 16) * 16 = 65536 bits = 8 KB
// (16 * 16 * 16) * 32 = 131072 bits = 16 KB
struct chunk_s {
  capprox_type type; // Always CA_TYPE_CHUNK
  region_chunk_pos rcpos; // Absolute location within the region.
  vertex_buffer layers[N_LAYERS]; // The vertex buffers.
  chunk_flag chunk_flags; // Flags

  list *block_entities; // Block entities.
  // TODO: merge these?
  block blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE]; // Blocks.
  block_flag block_flags[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE]; // Block flags.
};

struct chunk_approximation_s {
  capprox_type type; // Always CA_TYPE_APPROXIMATION
  region_chunk_pos rcpos; // Absolute location within the region.
  vertex_buffer layers[N_LAYERS]; // Vertex buffers.
  chunk_flag chunk_flags; // Flags

  lod detail; // The highest level of approximation contained here.
  approx_data *data; // Approximate block data.
};

struct chunk_or_approx_s {
  capprox_type type;
  void *ptr;
};

APPROX_DATA_STRUCT(1);
APPROX_DATA_STRUCT(2);
APPROX_DATA_STRUCT(3);
APPROX_DATA_STRUCT(4);

union approx_data_u {
  APPROX_DATA_TN(1) d1;
  APPROX_DATA_TN(2) d2;
  APPROX_DATA_TN(3) d3;
  APPROX_DATA_TN(4) d4;
};

/********************
 * Inline Functions *
 ********************/

// Coordinate conversions:
// Note that these are hand-inlined in a few places for speed.

static inline void rcpos__rpos(
  region_chunk_pos const * const rcpos,
  region_pos *rpos
) {
  rpos->x = ((r_pos_t) rcpos->x) << CHUNK_BITS;
  rpos->y = ((r_pos_t) rcpos->y) << CHUNK_BITS;
  rpos->z = ((r_pos_t) rcpos->z) << CHUNK_BITS;
}

static inline void rpos__rcpos(
  region_pos const * const rpos,
  region_chunk_pos *rcpos
) {
  rcpos->x = rpos->x >> CHUNK_BITS;
  rcpos->y = rpos->y >> CHUNK_BITS;
  rcpos->z = rpos->z >> CHUNK_BITS;
}

static inline void cidx__rpos(
  chunk const * const c,
  chunk_index const * const idx,
  region_pos *pos
) {
  rcpos__rpos(&(c->rcpos), pos);
  pos->x += idx->x;
  pos->y += idx->y;
  pos->z += idx->z;
}

static inline void caidx__rpos(
  chunk_approximation const * const ca,
  chunk_index const * const idx,
  region_pos *pos
) {
  rcpos__rpos(&(ca->rcpos), pos);
  pos->x += idx->x;
  pos->y += idx->y;
  pos->z += idx->z;
}

static inline void rpos__cidx(
  region_pos const * const rpos,
  chunk_index *idx
) {
  idx->x = rpos->x & CH_MASK;
  idx->y = rpos->y & CH_MASK;
  idx->z = rpos->z & CH_MASK;
}

static inline void rpos__vec(
  region_pos const * const origin,
  region_pos const * const rpos,
  vector *result
) {
  result->x = (rpos->x - origin->x);
  result->y = (rpos->y - origin->y);
  result->z = (rpos->z - origin->z);
}

static inline void vec__rpos(
  region_pos const * const origin,
  vector const * const v,
  region_pos *result
) {
  result->x = origin->x + fastfloor(v->x);
  result->y = origin->y + fastfloor(v->y);
  result->z = origin->z + fastfloor(v->z);
}

/* TODO: Remove this
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
*/

/* TODO: remove these
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
*/

static inline void ch__coa(chunk *c, chunk_or_approx *coa) {
  coa->type = CA_TYPE_CHUNK;
  coa->ptr = (void *) c;
}

static inline void aprx__coa(chunk_approximation *ca, chunk_or_approx *coa) {
  coa->type = CA_TYPE_APPROXIMATION;
  coa->ptr = (void *) ca;
}

// Copying, adding, and other pseudo-conversion functions:

static inline void copy_rpos(
  region_pos const * const source,
  region_pos *destination
) {
  destination->x = source->x;
  destination->y = source->y;
  destination->z = source->z;
}

static inline void copy_rcpos(
  region_chunk_pos const * const source,
  region_chunk_pos *destination
) {
  destination->x = source->x;
  destination->y = source->y;
  destination->z = source->z;
}

// Indexing functions:
// These must be super-fast 'cause they crop up in all sorts of inner loops.

static inline block c_get_block(
  chunk const * const c,
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
  chunk const * const c,
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

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and initializes a new chunk at the given position. Does not
// initialize the chunk's block data or flags.
chunk * create_chunk(region_chunk_pos const * const rcpos);

// Cleans up memory allocated for the given chunk.
void cleanup_chunk(chunk *c);

// Allocates and initializes a new chunk approximation at the given position
// with the given level of detail.
chunk_approximation * create_chunk_approximation(
  region_chunk_pos *rcpos,
  lod detail
);

// Cleans up memory allocated for the given chunk approximation.
void cleanup_chunk_approximation(chunk_approximation *ca);

/*************
 * Functions *
 *************/

// Getting/putting approximate blocks within a chunk approximation:
DECLARE_APPROX_FN_VARIANTS(CA_GET_BLOCK_SIG, CA_GET_BLOCK_FN)
DECLARE_APPROX_FN_VARIANTS(CA_PUT_BLOCK_SIG, CA_PUT_BLOCK_FN)

// Getting/putting approximate flags within a chunk approximation:
DECLARE_APPROX_FN_VARIANTS(CA_GET_FLAGS_SIG, CA_GET_FLAGS_FN)
DECLARE_APPROX_FN_VARIANTS(CA_PUT_FLAGS_SIG, CA_PUT_FLAGS_FN)
DECLARE_APPROX_FN_VARIANTS(CA_SET_FLAGS_SIG, CA_SET_FLAGS_FN)
DECLARE_APPROX_FN_VARIANTS(CA_CLEAR_FLAGS_SIG, CA_CLEAR_FLAGS_FN)

// Inline functions to shorten function table access to the function tables
// defined by the macros above:
static inline block ca_get_block(
  chunk_approximation const * const ca,
  chunk_index idx
) {
  return ca_get_block_table[ca->detail](ca, &idx);
}
static inline void ca_put_block(
  chunk_approximation *ca,
  chunk_index idx,
  block b
) {
  return ca_put_block_table[ca->detail](ca, &idx, b);
}

static inline block_flag ca_get_flags(
  chunk_approximation const * const ca,
  chunk_index idx
) {
  return ca_get_flags_table[ca->detail](ca, &idx);
}
static inline void ca_put_flags(
  chunk_approximation *ca,
  chunk_index idx,
  block_flag f
) {
  return ca_put_flags_table[ca->detail](ca, &idx, f);
}
static inline void ca_set_flags(
  chunk_approximation *ca,
  chunk_index idx,
  block_flag f
) {
  return ca_set_flags_table[ca->detail](ca, &idx, f);
}
static inline void ca_clear_flags(
  chunk_approximation *ca,
  chunk_index idx,
  block_flag f
) {
  return ca_clear_flags_table[ca->detail](ca, &idx, f);
}

static inline void c_get_neighbors(
  chunk const * const c,
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

// This has to be declared after ca_get_block.
static inline void ca_get_neighbors(
  chunk_approximation const * const ca,
  chunk_index idx,
  block *ba, block *bb,
  block *bn, block *bs,
  block *be, block *bw
) {
  // note that even underflow should wrap correctly here
  ch_idx_t step = 1 << (ca->detail);
  ch_idx_t mask = umaxof(ch_idx_t) << (ca->detail);
  chunk_index nbr;
  nbr.x = idx.x;
  nbr.y = idx.y;
  nbr.z = idx.z;

  nbr.z = (idx.z & mask) + step;
  *ba = (idx.z < CHUNK_SIZE - step) * ca_get_block(ca, nbr);
  nbr.z = (idx.z & mask) - step;
  *bb = (idx.z > step - 1) * ca_get_block(ca, nbr);
  nbr.z = (idx.z & mask);
  nbr.y = (idx.y & mask) + step;
  *bn = (idx.y < CHUNK_SIZE - step) * ca_get_block(ca, nbr);
  nbr.y = (idx.y & mask) - step;
  *bs = (idx.y > step - 1) * ca_get_block(ca, nbr);
  nbr.y = (idx.y & mask);
  nbr.x = (idx.x & mask) + step;
  *be = (idx.x < CHUNK_SIZE - step) * ca_get_block(ca, nbr);
  nbr.x = (idx.x & mask) - step;
  *bw = (idx.x > step - 1) * ca_get_block(ca, nbr);
}

// block_at returns the block at the given region position according to the
// best available currently-loaded data. Note that this will cache the chunk
// used and re-use it when possible, subject to changes in the value of
// BLOCK_AT_SALT, so refresh_block_at_cache should be called before any set of
// calls to block_at to ensure that you don't use stale block data or worse, an
// invalid chunk pointer.
extern uint8_t BLOCK_AT_SALT;
static inline void refresh_block_at_cache(void) {
  BLOCK_AT_SALT += 1; // overflow is fine
}
block block_at(region_pos const * const rpos);

// These inline functions call block_at for neighboring blocks:
static inline block block_above(region_pos const * const rpos) {
  region_pos above;
  copy_rpos(rpos, &above);
  above.z += 1;
  return block_at(&above);
}

static inline block block_below(region_pos const * const rpos) {
  region_pos below;
  copy_rpos(rpos, &below);
  below.z -= 1;
  return block_at(&below);
}

static inline block block_north(region_pos const * const rpos) {
  region_pos north;
  copy_rpos(rpos, &north);
  north.y += 1;
  return block_at(&north);
}

static inline block block_south(region_pos const * const rpos) {
  region_pos south;
  copy_rpos(rpos, &south);
  south.y -= 1;
  return block_at(&south);
}

static inline block block_east(region_pos const * const rpos) {
  region_pos east;
  copy_rpos(rpos, &east);
  east.x += 1;
  return block_at(&east);
}

static inline block block_west(region_pos const * const rpos) {
  region_pos west;
  copy_rpos(rpos, &west);
  west.x -= 1;
  return block_at(&west);
}

// TODO: How to tick blocks?

#endif // ifndef WORLD_H
