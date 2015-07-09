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

// The global position of a cell:
typedef int64_t gl_pos_t;
// The global position of a chunk. Should be < the size of gl_pos_t:
typedef int32_t gl_cpos_t;

// Global cell position:
struct global_pos_s;
typedef struct global_pos_s global_pos;

// Global chunk position:
struct global_chunk_pos_s;
typedef struct global_chunk_pos_s global_chunk_pos;

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

// A 3x3x3 neighborhood of chunks (contains a pointer to each). By convention
// ordering is xyz, so an offset of +/- 1 moves you up/down, +/- 3 north/south,
// and +/- 9 east/west.
struct chunk_neighborhood_s;
typedef struct chunk_neighborhood_s chunk_neighborhood;

// A 3x3x3 neighborhood of chunk_or_approxs. Ordering is xyz.
struct approx_neighborhood_s;
typedef struct approx_neighborhood_s approx_neighborhood;

// Holds pointers to 27 cells in a 3x3x3 neighborhood. Ordering is xyz.
struct cell_neighborhood_s;
typedef struct cell_neighborhood_s cell_neighborhood;

// Macros and types for the size of a chunk:
// Note that CHUNK_BITS shouldn't be greater than 6, or extended chunk indices
// won't be packable into 32-bit ints.
#define CHUNK_BITS 5
#define CHUNK_SIZE (1 << CHUNK_BITS)
#define TOTAL_CHUNK_CELLS (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)
#define CH_MASK (CHUNK_SIZE - 1) // Chunk mask

// Picks out a cell within a chunk:
union chunk_index_u;
typedef union chunk_index_u chunk_index;

/*************
 * Constants *
 *************/

static chunk_flag const              CF_LOADED = 0x0001;
static chunk_flag const            CF_COMPILED = 0x0002;
static chunk_flag const         CF_HAS_BIOLOGY = 0x0004;
static chunk_flag const     CF_COMPILE_ON_LOAD = 0x0008;
static chunk_flag const      CF_QUEUED_TO_LOAD = 0x0010;
static chunk_flag const   CF_QUEUED_TO_COMPILE = 0x0020;
static chunk_flag const   CF_QUEUED_FOR_BIOGEN = 0x0040;

// The index of the central member in a 3x3x3 neighborhood:
static int const NBH_CENTER = 13;

/*************************
 * Structure Definitions *
 *************************/

struct global_pos_s {
  gl_pos_t x, y, z, w;
};

struct global_chunk_pos_s {
  gl_cpos_t x, y, z;
};


struct chunk_index_parts_s {
  int8_t x, y, z, w;
    // Note that the bit width here must be >= CHUNK_BITS + 2 so that we can
    // have a sign bit and enough room to index chunks within a 3x3x3
    // neighborhood.
} __attribute__((packed)); // Packed so that we can union with a uint32_t.

union chunk_index_u {
  uint32_t combined;
  struct chunk_index_parts_s xyz;
}; // defined this way, chunk indices can fit in a void*, so we can put them
// into lists, maps, etc.

// (16 * 16 * 16) * 16 = 65536 bits = 8 KB
// (16 * 16 * 16) * 32 = 131072 bits = 16 KB
// (16 * 16 * 16) * 64 = 262144 bits = 32 KB
// (32 * 32 * 32) * 64 = 2097152 bits = 256 KB <-
struct chunk_s {
  capprox_type type; // Always CA_TYPE_CHUNK
  global_chunk_pos glcpos; // Absolute location.
  vertex_buffer layers[N_LAYERS]; // The vertex buffers.
  chunk_flag chunk_flags; // Flags
  size_t growth_counter; // Cumulative growth cycles experienced by this chunk

  list *cell_entities; // Cell entities.
  // TODO: merge these?
  cell cells[TOTAL_CHUNK_CELLS]; // Cells.
};

struct chunk_approximation_s {
  capprox_type type; // Always CA_TYPE_APPROXIMATION
  global_chunk_pos glcpos; // Absolute location.
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

struct chunk_neighborhood_s {
  global_chunk_pos glcpos; // Absolute location of central chunk.
  chunk* members[27]; // Pointers to each member chunk in xyz order.
};

struct approx_neighborhood_s {
  // Note that unlike a chunk_neighborhood, this structure actually contains
  // the chunk_or_approx elements directly, since a chunk_or_approx is kinda
  // like a pointer itself.
  global_chunk_pos glcpos; // Absolute location of central chunk.
  chunk_or_approx members[27]; // chunk_or_approx members in xyz order.
};

struct cell_neighborhood_s {
  global_pos glpos; // Absolute location of the central cell.
  cell* members[27]; // Pointers to each member cell in xyz order.
};

/********************
 * Inline Functions *
 ********************/

// Hashing:

static inline ptrdiff_t chunk_hash(chunk *c) {
  return prng(
    575451 + c->glcpos.x * (
      454648 + c->glcpos.y * (
        171311 + c->glcpos.z
      )
    )
  );
}

static inline ptrdiff_t cell_hash(global_pos *pos) {
  return prng(
    81024 + pos->x * (
      640513 + pos->y * (
        2094841 + pos->z
      )
    )
  );
}

// Coordinate conversions:
// Note that these are hand-inlined in a few places for speed.

static inline void glcpos__glpos(
  global_chunk_pos const * const glcpos,
  global_pos *glpos
) {
  glpos->x = ((gl_pos_t) glcpos->x) << CHUNK_BITS;
  glpos->y = ((gl_pos_t) glcpos->y) << CHUNK_BITS;
  glpos->z = ((gl_pos_t) glcpos->z) << CHUNK_BITS;
  glpos->w = 0;
}

static inline void glpos__glcpos(
  global_pos const * const glpos,
  global_chunk_pos *glcpos
) {
  glcpos->x = glpos->x >> CHUNK_BITS;
  glcpos->y = glpos->y >> CHUNK_BITS;
  glcpos->z = glpos->z >> CHUNK_BITS;
}

static inline void cidx__glpos(
  chunk const * const c,
  chunk_index const * const idx,
  global_pos *pos
) {
  glcpos__glpos(&(c->glcpos), pos);
  pos->x += idx->xyz.x;
  pos->y += idx->xyz.y;
  pos->z += idx->xyz.z;
  pos->w = idx->xyz.w;
}

static inline void caidx__glpos(
  chunk_approximation const * const ca,
  chunk_index const * const idx,
  global_pos *pos
) {
  glcpos__glpos(&(ca->glcpos), pos);
  pos->x += idx->xyz.x;
  pos->y += idx->xyz.y;
  pos->z += idx->xyz.z;
  pos->w = idx->xyz.w;
}

static inline void glpos__cidx(
  global_pos const * const glpos,
  chunk_index *idx
) {
  idx->xyz.x = glpos->x & CH_MASK;
  idx->xyz.y = glpos->y & CH_MASK;
  idx->xyz.z = glpos->z & CH_MASK;
  idx->xyz.w = glpos->w;
}

static inline void glpos__vec(
  global_pos const * const origin,
  global_pos const * const glpos,
  vector *result
) {
  result->x = (glpos->x - origin->x);
  result->y = (glpos->y - origin->y);
  result->z = (glpos->z - origin->z);
}

static inline void vec__glpos(
  global_pos const * const origin,
  vector const * const v,
  global_pos *result
) {
  result->x = origin->x + fastfloor(v->x);
  result->y = origin->y + fastfloor(v->y);
  result->z = origin->z + fastfloor(v->z);
  result->w = 0;
}

// Takes a vector references from one global position and turns it into a
// vector referenced from the given global position.
static inline void reref_vec(
  global_pos const * const original_ref,
  vector* v,
  global_pos const * const new_ref
) {
  v->x -= (new_ref->x - original_ref->x);
  v->y -= (new_ref->y - original_ref->y);
  v->z -= (new_ref->z - original_ref->z);
}

static inline void ch__coa(chunk *c, chunk_or_approx *coa) {
  coa->type = CA_TYPE_CHUNK;
  coa->ptr = (void *) c;
}

static inline void aprx__coa(chunk_approximation *ca, chunk_or_approx *coa) {
  coa->type = CA_TYPE_APPROXIMATION;
  coa->ptr = (void *) ca;
}

// Returns the level of detail of a chunk or approximation. Returns N_LODS if
// the given chunk or approximation has type CA_TYPE_NOT_LOADED.
static inline lod coa_detail_level(chunk_or_approx *coa) {
  if (coa->type == CA_TYPE_CHUNK) {
    return LOD_BASE;
  } else if (coa->type == CA_TYPE_APPROXIMATION) {
    return ((chunk_approximation*)coa->ptr)->detail;
  } else {
    return N_LODS;
  }
}

// Copying, adding, and other pseudo-conversion functions:

static inline void copy_glpos(
  global_pos const * const source,
  global_pos *destination
) {
  destination->x = source->x;
  destination->y = source->y;
  destination->z = source->z;
  destination->w = source->w;
}

static inline void copy_glcpos(
  global_chunk_pos const * const source,
  global_chunk_pos *destination
) {
  destination->x = source->x;
  destination->y = source->y;
  destination->z = source->z;
}

static inline chunk_index cidx_add(
  chunk_index first,
  chunk_index second
) {
  chunk_index result;
  result.combined = first.combined;
  result.xyz.x += second.xyz.x;
  result.xyz.y += second.xyz.y;
  result.xyz.z += second.xyz.z;
  result.xyz.w += second.xyz.w;
  result.xyz.w = posmod(result.xyz.w, 2);
  return result;
}

// Indexing functions:
// These must be super-fast 'cause they crop up in all sorts of inner loops.

static inline cell* c_cell(
  chunk *c,
  chunk_index idx
) {
  return &(c->cells[
    (((int) idx.xyz.x) & CH_MASK) +
    ((((int) idx.xyz.y) & CH_MASK) << CHUNK_BITS) +
    ((((int) idx.xyz.z) & CH_MASK) << (CHUNK_BITS*2))
  ]);
}

static inline block* c_block(
  chunk *c,
  chunk_index idx
) {
  return &(c_cell(c, idx)->blocks[idx.xyz.w]);
}

static inline void c_paste_cell(
  chunk *c,
  chunk_index idx,
  cell *cl
) {
  cell *dst = c_cell(c, idx);
  copy_cell(cl, dst);
}

// Picks out a cell from a chunk neighborhood based on an extended chunk index.
static inline cell* nb_cell(
  chunk_neighborhood *nbh,
  chunk_index idx
) {
  size_t nb_i = NBH_CENTER; // default is central chunk of the neighborhood
  if (idx.xyz.x < 0) {
    idx.xyz.x += CHUNK_SIZE;
    nb_i -= 9;
  } else if (idx.xyz.x >= CHUNK_SIZE) {
    idx.xyz.x -= CHUNK_SIZE;
    nb_i += 9;
  }
  if (idx.xyz.y < 0) {
    idx.xyz.y += CHUNK_SIZE;
    nb_i -= 3;
  } else if (idx.xyz.y >= CHUNK_SIZE) {
    idx.xyz.y -= CHUNK_SIZE;
    nb_i += 3;
  }
  if (idx.xyz.z < 0) {
    idx.xyz.z += CHUNK_SIZE;
    nb_i -= 1;
  } else if (idx.xyz.z >= CHUNK_SIZE) {
    idx.xyz.z -= CHUNK_SIZE;
    nb_i += 1;
  }
  return c_cell(nbh->members[nb_i], idx);
}

// General utility functions:

static inline void c_erase_cell_data(chunk *c) {
  memset(
    c->cells,
    0,
    TOTAL_CHUNK_CELLS * sizeof(cell)
  );
}

static inline void c_fill_with_block(chunk *c, block b) {
  chunk_index idx = { .xyz = { .x = 0, .y = 0, .z = 0, .w=0 } };
  c_erase_cell_data(c);
  for (idx.xyz.x = 0; idx.xyz.x < CHUNK_SIZE; ++idx.xyz.x) {
    for (idx.xyz.y = 0; idx.xyz.y < CHUNK_SIZE; ++idx.xyz.y) {
      for (idx.xyz.z = 0; idx.xyz.z < CHUNK_SIZE; ++idx.xyz.z) {
        *c_block(c, idx) = b;
      }
    }
  }
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and initializes a new chunk at the given position. Allocates but
// does not initialize the chunk's cell data.
chunk * create_chunk(global_chunk_pos const * const glcpos);

// Cleans up memory allocated for the given chunk.
void cleanup_chunk(chunk *c);

// Allocates and initializes a new chunk approximation at the given position
// with the given level of detail.
chunk_approximation * create_chunk_approximation(
  global_chunk_pos *glcpos,
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

// Picks out a cell from an approximate neighborhood based on an extended chunk
// index. Returns NULL if it is within an unloaded chunk.
static inline cell* nb_approx_cell(
  approx_neighborhood *nbh,
  chunk_index idx
) {
  size_t nb_i = NBH_CENTER; // default is central chunk of the neighborhood
  chunk_or_approx *coa;
  if (idx.xyz.x < 0) {
    idx.xyz.x += CHUNK_SIZE;
    nb_i -= 9;
  } else if (idx.xyz.x >= CHUNK_SIZE) {
    idx.xyz.x -= CHUNK_SIZE;
    nb_i += 9;
  }
  if (idx.xyz.y < 0) {
    idx.xyz.y += CHUNK_SIZE;
    nb_i -= 3;
  } else if (idx.xyz.y >= CHUNK_SIZE) {
    idx.xyz.y -= CHUNK_SIZE;
    nb_i += 3;
  }
  if (idx.xyz.z < 0) {
    idx.xyz.z += CHUNK_SIZE;
    nb_i -= 1;
  } else if (idx.xyz.z >= CHUNK_SIZE) {
    idx.xyz.z -= CHUNK_SIZE;
    nb_i += 1;
  }
  coa = &(nbh->members[nb_i]);
  if (coa->type == CA_TYPE_CHUNK) {
    return c_cell((chunk*) (coa->ptr), idx);
  } else if (coa->type == CA_TYPE_APPROXIMATION) {
    return ca_cell((chunk_approximation*) (coa->ptr), idx);
  } else {
    return NULL;
  }
}

// cell_at returns the cell at the given global position according to the best
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
cell* cell_at(global_pos const * const glpos);

// These inline functions call cell_at for neighboring cells:
static inline cell* cell_above(global_pos const * const glpos) {
  global_pos above;
  copy_glpos(glpos, &above);
  above.z += 1;
  return cell_at(&above);
}

static inline cell* cell_below(global_pos const * const glpos) {
  global_pos below;
  copy_glpos(glpos, &below);
  below.z -= 1;
  return cell_at(&below);
}

static inline cell* cell_north(global_pos const * const glpos) {
  global_pos north;
  copy_glpos(glpos, &north);
  north.y += 1;
  return cell_at(&north);
}

static inline cell* cell_south(global_pos const * const glpos) {
  global_pos south;
  copy_glpos(glpos, &south);
  south.y -= 1;
  return cell_at(&south);
}

static inline cell* cell_east(global_pos const * const glpos) {
  global_pos east;
  copy_glpos(glpos, &east);
  east.x += 1;
  return cell_at(&east);
}

static inline cell* cell_west(global_pos const * const glpos) {
  global_pos west;
  copy_glpos(glpos, &west);
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

// Iterates over the cells in the path of the given ray, starting with the cell
// containing its origin (the origin vector should be relative to the given
// reference cell position). Calls the given iteration function passing in the
// extra data pointer, the current cell position, the origin point relative to
// the current cell (could be epsilon outside due to rounding), the heading
// vector, and the current ray length. Iteration stops when the ray length
// exceeds the limit returned by the iteration function (which can of course be
// dynamic; returning zero halts iteration immediately). If iter_ray is given a
// zero-length heading vector, it will return without calling the iteration
// function. When iteration ends, the final iteration position is returned.
void iter_ray(
  global_pos const * const reference,
  vector origin,
  vector heading,
  void* data,
  float (*f)(void*, global_pos*, vector, vector, float)
);

// TODO: How to tick cells?

#endif // ifndef WORLD_H
