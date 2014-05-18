#ifndef WORLD_H
#define WORLD_H

// world.h
// Structures and functions for representing the world.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include <stdint.h>
#include <string.h>
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
    cell cells[ \
      (1 << (CHUNK_BITS - N)) * \
      (1 << (CHUNK_BITS - N)) * \
      (1 << (CHUNK_BITS - N)) \
    ]; \
  }

// The function names for approximate cell getting/putting routines (function
// body macros are defined in world.c).
#define CA_CELL_FN(SCALE) ca_cell_ ## SCALE
#define CA_CELL_SIG(NAME) \
  cell * NAME( \
    chunk_approximation const * const ca, \
    chunk_index const * const idx \
  )

#define CA_PASTE_CELL_FN(SCALE) ca_paste_cell_ ## SCALE
#define CA_PASTE_CELL_SIG(NAME) \
  void NAME( \
    chunk_approximation const * const ca, \
    chunk_index const * const idx, \
    cell *cl \
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
enum layer_e {
  L_OPAQUE = 0,
  L_TRANSPARENT = 1,
  L_TRANSLUCENT = 2,
  N_LAYERS = 3
};
typedef enum layer_e layer;

// The level-of-detail for a chunk:
enum lod_e {
  LOD_BASE = 0,
  LOD_HALF = 1,
  LOD_QUARTER = 2,
  LOD_EIGHTH = 3,
  LOD_SIXTEENTH = 4,
  N_LODS = 5
};
typedef enum lod_e lod;

// Whether something is a chunk or an approximation thereof:
enum capprox_type_e {
  CA_TYPE_NOT_LOADED,
  CA_TYPE_CHUNK,
  CA_TYPE_APPROXIMATION
};
typedef enum capprox_type_e capprox_type;

/**************
 * Structures *
 **************/

// Defines the size of a region in cells:
typedef int64_t r_pos_t;
// Defines the size of a region in chunks. Must be < the size of r_pos_t:
typedef int32_t r_cpos_t;

// Cell position within a region:
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

// Holds approximate cell data at one of several scales:
union approx_data_u;
typedef union approx_data_u approx_data;

// Cell data structures for approximate chunks at various scales (1/2^N):
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

// Picks out a cell within a chunk:
struct chunk_index_s;
typedef struct chunk_index_s chunk_index;

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

  list *cell_entities; // Cell entities.
  // TODO: merge these?
  cell cells[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE]; // Cells.
};

struct chunk_approximation_s {
  capprox_type type; // Always CA_TYPE_APPROXIMATION
  region_chunk_pos rcpos; // Absolute location within the region.
  vertex_buffer layers[N_LAYERS]; // Vertex buffers.
  chunk_flag chunk_flags; // Flags

  lod detail; // The highest level of approximation contained here.
  approx_data *data; // Approximate cell data.
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

static inline cell *c_cell(
  chunk *c,
  chunk_index idx
) {
  return &(c->cells[
    (idx.x & CH_MASK) +
    ((idx.y & CH_MASK) << CHUNK_BITS) +
    ((idx.z & CH_MASK) << (CHUNK_BITS*2))
  ]);
}

static inline void c_paste_cell(
  chunk *c,
  chunk_index idx,
  cell *cl
) {
  cell *dst = c_cell(c, idx);
  copy_cell(cl, dst);
}

static inline block cl_get_exposure(cell *cl) { return b_exp(cl->primary); }

static inline void cl_set_exposure(cell *cl, block exposure) {
#ifdef DEBUG
  if (exposure & (~BM_EXPOSURE)) {
    fprintf(
      stderr,
      "Warning: cl_set_exposure will corrupt block: 0x%x.\n",
      exposure
    );
  }
#endif
  cl->primary |= (exposure << BS_EXP);
}

static inline void cl_clear_exposure(cell *cl, block exposure) {
#ifdef DEBUG
  if (exposure & (~BM_EXPOSURE)) {
    fprintf(
      stderr,
      "Warning: cl_clear_exposure will corrupt block: 0x%x.\n",
      exposure
    );
  }
#endif
  cl->primary &= ~(exposure << BS_EXP);
}

// General utility functions:

static inline void c_erase_cell_data(chunk *c) {
  memset(
    c->cells,
    0,
    CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * sizeof(cell)
  );
}

static inline void c_fill_with_block(chunk *c, block b) {
  chunk_index idx = { .x = 0, .y = 0, .z = 0 };
  c_erase_cell_data(c);
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        c_cell(c, idx)->primary = b;
      }
    }
  }
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and initializes a new chunk at the given position. Allocates but
// does not initialize the chunk's cell data.
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

// Getting/putting approximate cells within a chunk approximation:
DECLARE_APPROX_FN_VARIANTS(CA_CELL_SIG, CA_CELL_FN)
DECLARE_APPROX_FN_VARIANTS(CA_PASTE_CELL_SIG, CA_PASTE_CELL_FN)

// Inline functions to shorten function table access to the function tables
// defined by the macros above:
static inline cell* ca_cell(
  chunk_approximation const * const ca,
  chunk_index idx
) {
  return ca_cell_table[ca->detail](ca, &idx);
}

static inline void c_get_primary_neighbors(
  chunk *c,
  chunk_index idx,
  block *ba, block *bb,
  block *bn, block *cs,
  block *be, block *bw
) {
  // note that even underflow should wrap correctly here
  chunk_index nbr;
  nbr.x = idx.x;
  nbr.y = idx.y;
  nbr.z = idx.z + 1;
  *ba = (idx.z < CHUNK_SIZE - 1) * c_cell(c, nbr)->primary;
  nbr.z = idx.z - 1;
  *bb = (idx.z > 0) *c_cell(c, nbr)->primary;
  nbr.z = idx.z;
  nbr.y = idx.y + 1;
  *bn = (idx.y < CHUNK_SIZE - 1) * c_cell(c, nbr)->primary;
  nbr.y = idx.y - 1;
  *cs = (idx.y > 0) * c_cell(c, nbr)->primary;
  nbr.y = idx.y;
  nbr.x = idx.x + 1;
  *be = (idx.x < CHUNK_SIZE - 1) * c_cell(c, nbr)->primary;
  nbr.x = idx.x - 1;
  *bw = (idx.x > 0) * c_cell(c, nbr)->primary;
}

// This has to be declared after ca_cell.
static inline void ca_get_primary_neighbors(
  chunk_approximation *ca,
  chunk_index idx,
  block *ba, block *bb,
  block *bn, block *cs,
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
  *ba = (idx.z < CHUNK_SIZE - step) * ca_cell(ca, nbr)->primary;
  nbr.z = (idx.z & mask) - step;
  *bb = (idx.z > step - 1) * ca_cell(ca, nbr)->primary;
  nbr.z = (idx.z & mask);
  nbr.y = (idx.y & mask) + step;
  *bn = (idx.y < CHUNK_SIZE - step) * ca_cell(ca, nbr)->primary;
  nbr.y = (idx.y & mask) - step;
  *cs = (idx.y > step - 1) * ca_cell(ca, nbr)->primary;
  nbr.y = (idx.y & mask);
  nbr.x = (idx.x & mask) + step;
  *be = (idx.x < CHUNK_SIZE - step) * ca_cell(ca, nbr)->primary;
  nbr.x = (idx.x & mask) - step;
  *bw = (idx.x > step - 1) * ca_cell(ca, nbr)->primary;
}

// cell_at returns the cell at the given region position according to the best
// available currently-loaded data, returning NULL if there is no data loaded
// for that position. Note that this will cache the chunk used and re-use it
// when possible, subject to changes in the value of CELL_AT_SALT, so
// refresh_cell_at_cache should be called before any set of calls to cell_at to
// ensure that you don't use stale cell data or worse, an invalid chunk
// pointer.
extern uint8_t CELL_AT_SALT;
static inline void refresh_cell_at_cache(void) {
  CELL_AT_SALT += 1; // overflow is fine
}
cell* cell_at(region_pos const * const rpos);

// These inline functions call cell_at for neighboring cells:
static inline cell* cell_above(region_pos const * const rpos) {
  region_pos above;
  copy_rpos(rpos, &above);
  above.z += 1;
  return cell_at(&above);
}

static inline cell* cell_below(region_pos const * const rpos) {
  region_pos below;
  copy_rpos(rpos, &below);
  below.z -= 1;
  return cell_at(&below);
}

static inline cell* cell_north(region_pos const * const rpos) {
  region_pos north;
  copy_rpos(rpos, &north);
  north.y += 1;
  return cell_at(&north);
}

static inline cell* cell_south(region_pos const * const rpos) {
  region_pos south;
  copy_rpos(rpos, &south);
  south.y -= 1;
  return cell_at(&south);
}

static inline cell* cell_east(region_pos const * const rpos) {
  region_pos east;
  copy_rpos(rpos, &east);
  east.x += 1;
  return cell_at(&east);
}

static inline cell* cell_west(region_pos const * const rpos) {
  region_pos west;
  copy_rpos(rpos, &west);
  west.x -= 1;
  return cell_at(&west);
}

// These functions compute and return the number of bytes used by a
// chunk/approximation for direct data storage (cell data only), overhead (all
// other data in RAM) and rendering (data stored on the GPU).
size_t chunk_data_size(chunk *c);
size_t chunk_overhead_size(chunk *c);
size_t chunk_gpu_size(chunk *c);

size_t chunk_approx_data_size(chunk_approximation *ca);
size_t chunk_approx_overhead_size(chunk_approximation *ca);
size_t chunk_approx_gpu_size(chunk_approximation *ca);

// TODO: How to tick cells?

#endif // ifndef WORLD_H
