#ifndef DATA_H
#define DATA_H

// data.h
// Data management.

#include <stdint.h>

#include "datatypes/queue.h"
#include "datatypes/map.h"

#include "world/blocks.h"
#include "world/world.h"
#include "world/chunk_data.h"

/**************
 * Structures *
 **************/

// A chunk_queue_set is an array of queues for each level of detail.
struct chunk_queue_set_s;
typedef struct chunk_queue_set_s chunk_queue_set;

// A chunk cache holds loaded chunks at various levels of detail:
struct chunk_cache_s;
typedef struct chunk_cache_s chunk_cache;

/*************
 * Constants *
 *************/

// The map table sizes for the chunk queue set maps and for the chunk cache map.
extern size_t const CHUNK_QUEUE_SET_MAP_SIZE;
extern size_t const CHUNK_CACHE_MAP_SIZE;

// Max chunks to load or compile per tick:
extern int const LOAD_CAP;
extern int const COMPILE_CAP;

// Distances at which to load chunks at different levels of detail, expressed
// in chunks.
extern gl_cpos_t const LOAD_DISTANCES[N_LODS];

// Vertical bias for load distances: in loading calculations the z-axis
// distance is multiplied by this amount.
extern int const VERTICAL_LOAD_BIAS;

// The fraction of the max load area to ignore: this will cut off the
// extremities of the min LOD sphere, possibly also affecting other levels of
// detail depending on the values in LOAD_DISTANCES.
extern int const LOAD_AREA_TRIM_FRACTION;

/***********
 * Globals *
 ***********/

// The global loading and compiling queues:
extern chunk_queue_set *LOAD_QUEUES;
extern chunk_queue_set *COMPILE_QUEUES;
extern chunk_queue_set *BIOGEN_QUEUES;

// The global chunk cache:
extern chunk_cache *CHUNK_CACHE;

/*************************
 * Structure Definitions *
 *************************/

struct chunk_queue_set_s {
  queue *levels[N_LODS];
  map *maps[N_LODS];
};

struct chunk_cache_s {
  map *levels[N_LODS];
};

/********************
 * Inline Functions *
 ********************/


// These functions return data for the chunk at the given position if it is
// loaded, and return NULL otherwise.
static inline chunk * get_chunk(global_chunk_pos *glcpos) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  return (chunk *) m3_get_value(
    CHUNK_CACHE->levels[LOD_BASE],
    (map_key_t) glcpos->x,
    (map_key_t) glcpos->y,
    (map_key_t) glcpos->z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
}

static inline chunk_approximation * get_chunk_approx(
  global_chunk_pos *glcpos,
  lod detail
) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  return (chunk_approximation *) m3_get_value(
    CHUNK_CACHE->levels[detail],
    (map_key_t) glcpos->x,
    (map_key_t) glcpos->y,
    (map_key_t) glcpos->z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
}

// Computes the desired detail level at the given position (assuming the player
// is at the given center position). Returns N_LODS if the given chunk is
// outside the max loading distance.
static inline lod desired_detail_at(
  global_chunk_pos* center,
  global_chunk_pos* pos
) {
  lod result = N_LODS;
  lod detail;
  gl_cpos_t edge;
  float d2 = (
    (pos->x - center->x) * (pos->x - center->x) +
    (pos->y - center->y) * (pos->y - center->y) +
    (
      (pos->z - center->z) * (pos->z - center->z)
      *
      VERTICAL_LOAD_BIAS
    )
  );
  for (detail = LOD_BASE; detail < N_LODS; ++detail) {
    edge = LOAD_DISTANCES[detail];
    if (d2 <= edge * edge) {
      result = detail;
      break;
    }
  }
  return result;
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Sets up the data subsytem.
void setup_data(void);

// Cleans up the data subsytem.
void cleanup_data(void);

// Allocates and returns a new chunk queue set.
chunk_queue_set *create_chunk_queue_set(void);

// Cleans up a chunk queue set.
void cleanup_chunk_queue_set(chunk_queue_set *cqs);

// Cleans up a chunk queue set, also freeing the contents of each queue.
void destroy_chunk_queue_set(chunk_queue_set *cqs);

// Allocates and returns a new chunk cache:
chunk_cache *create_chunk_cache(void);

// Cleans up a chunk cache.
void cleanup_chunk_cache(chunk_cache *cc);

/*************
 * Functions *
 *************/

// Adds the given chunk/approx to the queue set at the base level of detail.
void enqueue_chunk(chunk_queue_set *cqs, chunk *c);
void enqueue_chunk_approximation(chunk_queue_set *cqs, chunk_approximation *ca);

// Creates a new chunk at the given position and level of detail and marks it
// for loading. Does nothing if a chunk with the same coordinates and level of
// detail is either already loaded or already queued for loading.
void mark_for_loading(global_chunk_pos *glcpos, lod detail);

// Marks the given chunk or approximation for (re)compilation.
void mark_for_compilation(chunk_or_approx *coa);

// Marks the six best-quality loaded neighbors of the given position for
// (re)compilation.
void mark_neighbors_for_compilation(global_chunk_pos *glcpos);

// Marks the given chunk for biogeneration. Does nothing if the chunk is
// already in the biogen queue or if it already has biology.
void mark_for_biogen(chunk *c);

// Works like mark_neighbors_for_compilation but marks the neighbors for
// biogeneration instead. Marks all 27 adjacent chunks rather than just the 6
// face-adjacent neighbors.
void mark_neighbors_for_biogen(global_chunk_pos *glcpos);

// Returns the best level-of-detail at which the given chunk is loaded, or
// N_LODS if the given chunk isn't loaded.
lod get_best_loaded_level(global_chunk_pos *glcpos);

// Fills in the given chunk_or_approx struct with a pointer to the best
// available data for the given chunk, or NULL if there is no loaded data for
// that chunk. If there isn't any data, it will return a chunk_or_approx with
// type CA_TYPE_NOT_LOADED. The limited version accepts an upper bound on the
// resolution of data to return, and a flag for requesting only compiled chunk
// data.
void get_best_data(global_chunk_pos *glcpos, chunk_or_approx *coa);
void get_best_data_limited(
  global_chunk_pos *glcpos,
  lod limit,
  uint8_t compiled,
  chunk_or_approx *coa
);

// Marks for loading all chunks near the given chunk, as defined by the
// LOAD_DISTANCES array.
void load_surroundings(global_chunk_pos *glcpos);

// Ticks the chunk loading system, loading/unloading as many chunks as allowed
// and appropriate. Prioritizes more-detailed areas when loading data.
void tick_load_chunks(global_chunk_pos *load_center);

// Ticks the chunk compilation system, compiling as many chunks as allowed and
// appropriate. Prioritizes more-detailed areas when loading data. This should
// be called from the graphics thread as it needs an OpenGL context.
void tick_compile_chunks(void);

// Ticks the biogeneration system which adds biology to terrain-generated
// chunks.
void tick_biogen(void);

// Loads data from disk for the given chunk/approximation. Uses the chunk's
// x/y/z coordinates to determine what contents it should have.
void load_chunk(chunk *c);
void load_chunk_approx(chunk_approximation *ca);

/**************************
 * Extra Inline Functions *
 **************************/

// Fills in an approx_neighborhood struct using the best available data.
static inline void fill_approx_neighborhood(
  global_chunk_pos* glcpos,
  approx_neighborhood* r_neighborhood
) {
  copy_glcpos(glcpos, &(r_neighborhood->glcpos));
  global_chunk_pos nbpos;
  size_t i = 0;
  for (nbpos.x = glcpos->x - 1; nbpos.x <= glcpos->x + 1; nbpos.x += 1) {
    for (nbpos.y = glcpos->y - 1; nbpos.y <= glcpos->y + 1; nbpos.y += 1) {
      for (nbpos.z = glcpos->z - 1; nbpos.z <= glcpos->z + 1; nbpos.z += 1) {
        get_best_data(&nbpos, &(r_neighborhood->members[i]));
        i += 1;
      }
    }
  }
}

// Fills in a chunk_neighborhood struct. If the entire neighborhood isn't
// available, it will set the first entry in the result to NULL (the others may
// or may not be changed as well, but their values shouldn't be depended upon).
static inline void fill_chunk_neighborhood(
  global_chunk_pos *glcpos,
  chunk_neighborhood* r_neighborhood
) {
  copy_glcpos(glcpos, &(r_neighborhood->glcpos));
  chunk_or_approx* coa = NULL;
  global_chunk_pos nbpos;
  size_t i = 0;
  for (nbpos.z = glcpos->z - 1; nbpos.z <= glcpos->z + 1; nbpos.z += 1) {
    for (nbpos.x = glcpos->x - 1; nbpos.x <= glcpos->x + 1; nbpos.x += 1) {
      for (nbpos.y = glcpos->y - 1; nbpos.y <= glcpos->y + 1; nbpos.y += 1) {
        get_best_data(&nbpos, coa);
        if (coa->type != CA_TYPE_CHUNK) {
          r_neighborhood->members[0] = NULL;
          return;
        }
        r_neighborhood->members[i] = (chunk*) coa->ptr;
        i += 1;
      }
    }
  }
}

// The step value indicates how far to go to reach a "neighbor." The dummy
// value is substituted for missing cells. Note that the base index should be
// within the central chunk, to guarantee that its neighbors don't overflow the
// bounds of the entire chunk neighborhood.
static inline void fill_cell_neighborhood(
  block_index idx,
  approx_neighborhood* nbh,
  cell_neighborhood* result,
  int step,
  cell* dummy
) {
  block_index nbr;
  chunk_or_approx *coa = &(nbh->members[NBH_CENTER]);
  lod center_detail = coa_detail_level(coa);
  size_t i = 0, j = 0;

  // Set the global position of the neighborhood:
  if (center_detail == LOD_BASE) {
    cidx__glpos((chunk*) coa->ptr, &idx, &(result->glpos));
  } else {
    caidx__glpos((chunk_approximation*) coa->ptr, &idx, &(result->glpos));
  }

  for (
    nbr.xyz.x = idx.xyz.x - step;
    nbr.xyz.x <= idx.xyz.x + step;
    nbr.xyz.x += step
  ) {
    for (
      nbr.xyz.y = idx.xyz.y - step;
      nbr.xyz.y <= idx.xyz.y + step;
      nbr.xyz.y += step
    ) {
      for (
        nbr.xyz.z = idx.xyz.z - step;
        nbr.xyz.z <= idx.xyz.z + step;
        nbr.xyz.z += step
      ) {
        j = NBH_CENTER; // the center of the chunk neighborhood
        if (nbr.xyz.x < 0) {
          j -= 9;
        } else if (nbr.xyz.x >= CHUNK_SIZE) {
          j += 9;
        }

        if (nbr.xyz.y < 0) {
          j -= 3;
        } else if (nbr.xyz.y >= CHUNK_SIZE) {
          j += 3;
        }

        if (nbr.xyz.z < 0) {
          j -= 1;
        } else if (nbr.xyz.z >= CHUNK_SIZE) {
          j += 1;
        }
        coa = &(nbh->members[j]);
        if (center_detail > coa_detail_level(coa)) {
          // If the central chunk is less-detailed than its neighbor, we use
          // dummy data instead of the neighbor's data. This helps the
          // rendering algorithm assume that faces of approximations are
          // exposed (their neighbors get the dummy value which is empty) when
          // an approximation borders a more-detailed approximation or a real
          // chunk.
          result->members[i] = dummy;
        } else {
          result->members[i] = nb_approx_cell(nbh, nbr);
        }

        if (result->members[i] == NULL) {
          result->members[i] = dummy;
        }
        i += 1;
      }
    }
  }
}

// Works like fill_cell_neighborhood but requires a chunk neighborhood and
// produces an exact cell neighborhood.
static inline void fill_cell_neighborhood_exact(
  block_index idx,
  chunk_neighborhood* nbh,
  cell_neighborhood* result
) {
  // Note that the underflow should wrap correctly here, but we're fixing up
  // the values anyways.
  block_index n_idx;
  size_t i = 0;
  for (
    n_idx.xyz.x = idx.xyz.x - 1;
    n_idx.xyz.x <= idx.xyz.x + 1;
    n_idx.xyz.x += 1
  ) {
    for (
      n_idx.xyz.y = idx.xyz.y - 1;
      n_idx.xyz.y <= idx.xyz.y + 1;
      n_idx.xyz.y += 1
    ) {
      for (
        n_idx.xyz.z = idx.xyz.z - 1;
        n_idx.xyz.z <= idx.xyz.z + 1;
        n_idx.xyz.z += 1
      ) {
        result->members[i] = nb_cell(nbh, n_idx);
        i += 1;
      }
    }
  }
}

#endif // ifndef DATA_H
