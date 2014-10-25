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
extern r_cpos_t const LOAD_DISTANCES[N_LODS];

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
static inline chunk * get_chunk(region_chunk_pos *rcpos) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  return (chunk *) m3_get_value(
    CHUNK_CACHE->levels[LOD_BASE],
    (map_key_t) rcpos->x,
    (map_key_t) rcpos->y,
    (map_key_t) rcpos->z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
}

static inline chunk_approximation * get_chunk_approx(
  region_chunk_pos *rcpos,
  lod detail
) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  return (chunk_approximation *) m3_get_value(
    CHUNK_CACHE->levels[detail],
    (map_key_t) rcpos->x,
    (map_key_t) rcpos->y,
    (map_key_t) rcpos->z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
}

// Computes the desired detail level at the given position (assuming the player
// is at the given center position). Returns N_LODS if the given chunk is
// outside the max loading distance.
static inline lod desired_detail_at(
  region_chunk_pos* center,
  region_chunk_pos* pos
) {
  lod result = N_LODS;
  lod detail;
  r_cpos_t edge;
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
void mark_for_loading(region_chunk_pos *rcpos, lod detail);

// Marks the given chunk or approximation for (re)compilation.
void mark_for_compilation(chunk_or_approx *coa);

// Marks the six best-quality loaded neighbors of the given position for
// (re)compilation.
void mark_neighbors_for_compilation(region_chunk_pos *rcpos);

// Returns the best level-of-detail at which the given chunk is loaded, or
// N_LODS if the given chunk isn't loaded.
lod get_best_loaded_level(region_chunk_pos *rcpos);

// Fills in the given chunk_or_approx struct with a pointer to the best
// available data for the given chunk, or NULL if there is no loaded data for
// that chunk. If there isn't any data, it will return a chunk_or_approx with
// type CA_TYPE_NOT_LOADED. The limited version accepts an upper bound on the
// resolution of data to return, and a flag for requesting only compiled chunk
// data.
void get_best_data(region_chunk_pos *rcpos, chunk_or_approx *coa);
void get_best_data_limited(
  region_chunk_pos *rcpos,
  lod limit,
  uint8_t compiled,
  chunk_or_approx *coa
);

// Marks for loading all chunks near the given chunk, as defined by the
// LOAD_DISTANCES array.
void load_surroundings(region_chunk_pos *rcpos);

// Ticks the chunk loading system, loading/unloading as many chunks as allowed
// and appropriate. Prioritizes more-detailed areas when loading data.
void tick_load_chunks(region_chunk_pos *load_center);

// Ticks the chunk compilation system, compiling as many chunks as allowed and
// appropriate. Prioritizes more-detailed areas when loading data. This should
// be called from the graphics thread as it needs an OpenGL context.
void tick_compile_chunks(void);

// Loads data from disk for the given chunk/approximation. Uses the chunk's
// x/y/z coordinates to determine what contents it should have.
void load_chunk(chunk *c);
void load_chunk_approx(chunk_approximation *ca);

/**************************
 * Extra Inline Functions *
 **************************/

// neighborhood should be a 27-dimensional array of chunk_or_approxes, which
// will be filled in in zxy order.
static inline void get_chunk_neighborhood(
  region_chunk_pos* rcpos,
  chunk_or_approx* neighborhood
) {
  region_chunk_pos nbpos;
  size_t i = 0;
  for (nbpos.z = rcpos->z - 1; nbpos.z <= rcpos->z + 1; nbpos.z += 1) {
    for (nbpos.x = rcpos->x - 1; nbpos.x <= rcpos->x + 1; nbpos.x += 1) {
      for (nbpos.y = rcpos->y - 1; nbpos.y <= rcpos->y + 1; nbpos.y += 1) {
        get_best_data(&nbpos, &(neighborhood[i]));
        i += 1;
      }
    }
  }
}

// chunk_neighbors should be a 27-dimensional array of chunk_or_approxes in zxy
// order, while neighborhood should be a pointer to a 27-dimensional array of
// cell pointers in zxy order. The step value indicates how far to go to reach
// a "neighbor." The dummy value is substituted for missing cells.
static inline void get_cell_neighborhood(
  chunk_index idx,
  chunk_or_approx const * const chunk_neighbors,
  cell* neighborhood[],
  int step,
  cell* dummy
) {
  // Note that the underflow should wrap correctly here, but we're fixing up
  // the values anyways.
  chunk_index nbr;
  int dx, dy, dz;
  chunk_or_approx const * coa;
  capprox_type center_type = chunk_neighbors[13].type;
  size_t i = 0, j = 0;
  for (dz = -step; dz <= step; dz += step) {
    for (dx = -step; dx <= step; dx += step) {
      for (dy = -step; dy <= step; dy += step) {
        nbr.x = idx.x + dx;
        nbr.y = idx.y + dy;
        nbr.z = idx.z + dz;
        j = 13; // the center of the chunk neighborhood
        if (idx.z < step && dz == -step) { j -= 9; nbr.z = CHUNK_SIZE - 1; }
        if (idx.z >= CHUNK_SIZE - step && dz == step) { j += 9; nbr.z = 0; }
        if (idx.x < step && dx == -step) { j -= 3; nbr.x = CHUNK_SIZE - 1; }
        if (idx.x >= CHUNK_SIZE - step && dx == step) { j += 3; nbr.x = 0; }
        if (idx.y < step && dy == -step) { j -= 1; nbr.y = CHUNK_SIZE - 1; }
        if (idx.y >= CHUNK_SIZE - step && dy == step) { j += 1; nbr.y = 0; }
        coa = &(chunk_neighbors[j]);
        if (center_type == CA_TYPE_APPROXIMATION && coa->type == CA_TYPE_CHUNK){
          // Expose all faces of approximations that border actual chunks
          neighborhood[i] = dummy;
        } else if (coa->type == CA_TYPE_CHUNK) {
          neighborhood[i] = c_cell((chunk*) (coa->ptr), nbr);
        } else if (coa->type == CA_TYPE_APPROXIMATION) {
          neighborhood[i] = ca_cell((chunk_approximation*) (coa->ptr), nbr);
        } else {
          neighborhood[i] = dummy;
        }
        i += 1;
      }
    }
  }
}

#endif // ifndef DATA_H
