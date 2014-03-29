#ifndef DATA_H
#define DATA_H

// data.h
// Loading from and saving to disk.

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

// Max chunks to load or compile per tick:
extern int const LOAD_CAP;
extern int const COMPILE_CAP;

// Distances at which to load chunks at different levels of detail, expressed
// in chunks.
extern int const LOAD_DISTANCES[N_LODS];

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
extern chunk_queue_set LOAD_QUEUES;
extern chunk_queue_set COMPILE_QUEUES;

// The global chunk cache:
extern chunk_cache CHUNK_CACHE;

/*************************
 * Structure Definitions *
 *************************/

struct chunk_queue_set_s {
  queue *levels[N_LODS];
};

struct chunk_cache_s {
  map3 *levels[N_LODS];
};

/********************
 * Inline Functions *
 ********************/


// These functions return data for the chunk at the given position if it is
// loaded, and return NULL otherwise.
static inline chunk * get_chunk(region_chunk_pos *rcpos) {
  return (chunk *) m3_get_value(
    CHUNK_CACHE->levels[LOD_BASE],
    rcpos->x,
    rcpos->y,
    rcpos->z
  );
}

static inline chunk_approximation * get_chunk_approx(
  region_chunk_pos *rcpos,
  lod detail
) {
  return (chunk_approximation *) m3_get_value(
    CHUNK_CACHE->levels[detail],
    rcpos->x,
    rcpos->y,
    rcpos->z
  );
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
// type CA_TYPE_NOT_LOADED.
void get_best_data(region_chunk_pos *rcpos, chunk_or_approx *coa);

// Marks for loading all chunks near the given chunk, as defined by the
// LOAD_DISTANCES array.
void load_surroundings(region_chunk_pos *rcpos);

// Ticks the chunk data system, loading/unloading/compiling as many chunks as
// allowed and appropriate. Prioritizes more-detailed areas when loading data.
void tick_data(void);

// Loads data from disk for the given chunk. Uses the chunk's x/y/z coordinates
// to determine what contents it should have.
void load_chunk(chunk_neighborhood *c);

#endif // ifndef DATA_H
