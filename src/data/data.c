// data.c
// Loading from and saving to disk.

#include <stdint.h>
#include <stdio.h>

#include "data.h"

#include "datatypes/queue.h"
#include "datatypes/map.h"

#include "graphics/display.h"
#ifdef TERRAIN_MODE_BASIC
  #include "gen/terrain.h"
#else
  #include "gen/worldgen.h"
#endif
#include "prof/ptime.h"
#include "world/blocks.h"
#include "world/world.h"
#include "world/chunk_data.h"

/*************
 * Constants *
 *************/

size_t const CHUNK_QUEUE_SET_MAP_SIZE = 2048;
size_t const CHUNK_CACHE_MAP_SIZE = 16384;

// TODO: dynamic capping?
//int const LOAD_CAP = 128;
//int const COMPILE_CAP = 2048;
int const LOAD_CAP = 16;
int const COMPILE_CAP = 1024;
//int const LOAD_CAP = 1;
//int const COMPILE_CAP = 16;

// TODO: Good values here
//r_cpos_t const LOAD_DISTANCES[N_LODS] = { 6, 16, 50, 150, 500 };
//r_cpos_t const LOAD_DISTANCES[N_LODS] = { 8, 16, 32, 64, 128 };
//r_cpos_t const LOAD_DISTANCES[N_LODS] = { 4, 8, 12, 16, 20 };
//r_cpos_t const LOAD_DISTANCES[N_LODS] = { 5, 6, 7, 7, 7 };
//r_cpos_t const LOAD_DISTANCES[N_LODS] = { 3, 4, 5, 5, 5 };
r_cpos_t const LOAD_DISTANCES[N_LODS] = { 1, 2, 2, 2, 2 };

int const VERTICAL_LOAD_BIAS = 2;

int const LOAD_AREA_TRIM_FRACTION = 10;

/***********
 * Globals *
 ***********/

chunk_queue_set *LOAD_QUEUES = NULL;
chunk_queue_set *COMPILE_QUEUES = NULL;

chunk_cache *CHUNK_CACHE = NULL;

/********************
 * Search Functions *
 ********************/

int find_chunk_at_position(void *element, void *reference) {
  chunk *c = (chunk *) element;
  region_chunk_pos *rcpos = (region_chunk_pos *) reference;
  return (
    c->rcpos.x == rcpos->x
  &&
    c->rcpos.y == rcpos->y
  &&
    c->rcpos.z == rcpos->z
  );
}
int find_chunk_approx_at_position(void *element, void *reference) {
  chunk_approximation *ca = (chunk_approximation *) element;
  region_chunk_pos *rcpos = (region_chunk_pos *) reference;
  return (
    ca->rcpos.x == rcpos->x
  &&
    ca->rcpos.y == rcpos->y
  &&
    ca->rcpos.z == rcpos->z
  );
}

/****************************
 * Private Inline Functions *
 ****************************/

static inline int is_loaded(region_chunk_pos *rcpos, lod detail) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  return m3_contains_key(
    CHUNK_CACHE->levels[detail],
    (map_key_t) rcpos->x,
    (map_key_t) rcpos->y,
    (map_key_t) rcpos->z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
}

static inline int is_loading(region_chunk_pos *rcpos, lod detail) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  return m3_contains_key(
    LOAD_QUEUES->maps[detail],
    (map_key_t) rcpos->x,
    (map_key_t) rcpos->y,
    (map_key_t) rcpos->z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
}

void iter_cleanup_chunk(void * ptr) {
  cleanup_chunk((chunk *) ptr);
}

void iter_cleanup_chunk_approx(void * ptr) {
  cleanup_chunk_approximation((chunk_approximation *) ptr);
}

/******************************
 * Constructors & Destructors *
 ******************************/

void setup_data(void) {
  LOAD_QUEUES = create_chunk_queue_set();
  COMPILE_QUEUES = create_chunk_queue_set();
  CHUNK_CACHE = create_chunk_cache();
}

void cleanup_data(void) {
  destroy_chunk_queue_set(LOAD_QUEUES);
  cleanup_chunk_queue_set(COMPILE_QUEUES);
  cleanup_chunk_cache(CHUNK_CACHE);
}

chunk_queue_set *create_chunk_queue_set(void) {
  chunk_queue_set *cqs = (chunk_queue_set *) malloc(sizeof(chunk_queue_set));
  size_t i;
  for (i = LOD_BASE; i < N_LODS; ++i) {
    cqs->levels[i] = create_queue();
    cqs->maps[i] = create_map(3, CHUNK_QUEUE_SET_MAP_SIZE);
  }
  return cqs;
}

void cleanup_chunk_queue_set(chunk_queue_set *cqs) {
  size_t i;
  for (i = LOD_BASE; i < N_LODS; ++i) {
    cleanup_queue(cqs->levels[i]);
    cleanup_map(cqs->maps[i]);
  }
  free(cqs);
}

void destroy_chunk_queue_set(chunk_queue_set *cqs) {
  size_t i;
  q_foreach(cqs->levels[LOD_BASE], &iter_cleanup_chunk);
  cleanup_queue(cqs->levels[LOD_BASE]);
  cleanup_map(cqs->maps[LOD_BASE]);
  for (i = LOD_BASE + 1; i < N_LODS; ++i) {
    q_foreach(cqs->levels[i], &iter_cleanup_chunk_approx);
    cleanup_queue(cqs->levels[i]);
    cleanup_map(cqs->maps[i]);
  }
  free(cqs);
}

chunk_cache *create_chunk_cache(void) {
  chunk_cache *cc = (chunk_cache *) malloc(sizeof(chunk_cache));
  size_t i;
  for (i = LOD_BASE; i < N_LODS; ++i) {
    cc->levels[i] = create_map(3, CHUNK_CACHE_MAP_SIZE);
  }
  return cc;
}

void cleanup_chunk_cache(chunk_cache *cc) {
  size_t i;
  m_foreach(cc->levels[LOD_BASE], &iter_cleanup_chunk);
  cleanup_map(cc->levels[LOD_BASE]);
  for (i = LOD_BASE + 1; i < N_LODS; ++i) {
    m_foreach(cc->levels[i], &iter_cleanup_chunk_approx);
    cleanup_map(cc->levels[i]);
  }
  free(cc);
}

/*************
 * Functions *
 *************/

void enqueue_chunk(chunk_queue_set *cqs, chunk *c) {
  q_push_element(cqs->levels[LOD_BASE], (void *) c);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  m3_put_value(
    cqs->maps[LOD_BASE],
    (void *) c,
    (map_key_t) c->rcpos.x,
    (map_key_t) c->rcpos.y,
    (map_key_t) c->rcpos.z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
}

void enqueue_chunk_approximation(chunk_queue_set *cqs, chunk_approximation *ca){
  q_push_element(cqs->levels[ca->detail], (void *) ca);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  m3_put_value(
    cqs->maps[ca->detail],
    (void *) ca,
    (map_key_t) ca->rcpos.x,
    (map_key_t) ca->rcpos.y,
    (map_key_t) ca->rcpos.z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
}

void mark_for_loading(region_chunk_pos *rcpos, lod detail) {
  if (is_loaded(rcpos, detail) || is_loading(rcpos, detail)) {
    return;
  }
  if (detail == LOD_BASE) {
    chunk *c = create_chunk(rcpos);
    c->chunk_flags &= ~CF_LOADED;
    c->chunk_flags &= ~CF_COMPILED;
    c->chunk_flags |= CF_QUEUED_TO_LOAD;
    c->chunk_flags |= CF_COMPILE_ON_LOAD;
    enqueue_chunk(LOAD_QUEUES, c);
  } else {
    chunk_approximation *ca = create_chunk_approximation(rcpos, detail);
    ca->chunk_flags &= ~CF_LOADED;
    ca->chunk_flags &= ~CF_COMPILED;
    ca->chunk_flags |= CF_QUEUED_TO_LOAD;
    ca->chunk_flags |= CF_COMPILE_ON_LOAD;
    enqueue_chunk_approximation(LOAD_QUEUES, ca);
  }
}

void mark_for_compilation(chunk_or_approx *coa) {
  if (coa->type == CA_TYPE_CHUNK) {
    chunk *c = (chunk *) coa->ptr;
    if (c->chunk_flags & CF_QUEUED_TO_COMPILE) { return; }
    c->chunk_flags &= ~CF_COMPILED;
    c->chunk_flags |= CF_QUEUED_TO_COMPILE;
    enqueue_chunk(COMPILE_QUEUES, c);
  } else if (coa->type == CA_TYPE_APPROXIMATION) {
    chunk_approximation *ca = (chunk_approximation *) coa->ptr;
    if (ca->chunk_flags & CF_QUEUED_TO_COMPILE) { return; }
    ca->chunk_flags &= ~CF_COMPILED;
    ca->chunk_flags |= CF_QUEUED_TO_COMPILE;
    enqueue_chunk_approximation(COMPILE_QUEUES, ca);
  } else {
    fprintf(stderr, "Can't mark an unloaded chunk for compilation.\n");
    exit(1);
  }
}

void mark_neighbors_for_compilation(region_chunk_pos *center) {
  chunk_or_approx coa;
  region_chunk_pos rcpos;
  rcpos.x = center->x;
  rcpos.y = center->y;
  rcpos.z = center->z;

  rcpos.x += 1;
  get_best_data(&rcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }

  rcpos.x -= 2;
  get_best_data(&rcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }

  rcpos.x += 1;
  rcpos.y += 1;
  get_best_data(&rcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }

  rcpos.y -= 2;
  get_best_data(&rcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }

  rcpos.y += 1;
  rcpos.z += 1;
  get_best_data(&rcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }

  rcpos.z -= 2;
  get_best_data(&rcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }
}

lod get_best_loaded_level(region_chunk_pos *rcpos) {
  lod detail = N_LODS; // level of detail being considered
  for (detail = LOD_BASE; detail < N_LODS; ++detail) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    if (
      m3_contains_key(
        CHUNK_CACHE->levels[detail],
        (map_key_t) rcpos->x,
        (map_key_t) rcpos->y,
        (map_key_t) rcpos->z
      )
    ) {
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
      return detail;
    }
  }
  return N_LODS;
}

void get_best_data(region_chunk_pos *rcpos, chunk_or_approx *coa) {
  get_best_data_limited(rcpos, LOD_BASE, coa);
}

void get_best_data_limited(
  region_chunk_pos *rcpos,
  lod limit,
  chunk_or_approx *coa
) {
  lod detail = LOD_BASE; // level of detail being considered
  if (limit <= LOD_BASE) {
    coa->type = CA_TYPE_CHUNK;
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    coa->ptr = m3_get_value(
      CHUNK_CACHE->levels[LOD_BASE],
      (map_key_t) rcpos->x,
      (map_key_t) rcpos->y,
      (map_key_t) rcpos->z
    );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
    if (coa->ptr != NULL) {
      return;
    }
    limit = LOD_BASE + 1; // Increase our limit so that the for loop works.
  }
  coa->type = CA_TYPE_APPROXIMATION;
  for (detail = limit; detail < N_LODS; ++detail) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    coa->ptr = m3_get_value(
      CHUNK_CACHE->levels[detail],
      (map_key_t) rcpos->x,
      (map_key_t) rcpos->y,
      (map_key_t) rcpos->z
    );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
    if (coa->ptr != NULL) {
      return;
    }
  }
  coa->type = CA_TYPE_NOT_LOADED;
  return;
}

void load_surroundings(region_chunk_pos *center) {
  lod detail = LOD_BASE; // level of detail being considered
  region_chunk_pos rcpos = { .x=0, .y=0, .z=0 }; // current chunk
  r_cpos_t d2 = 0; // squared Euclidean distance to current chunk
  r_cpos_t edge = 0; // edge of the current LOD
  // Max distance at which to load anything, trimmed a bit:
  r_cpos_t max_distance = LOAD_DISTANCES[N_LODS - 1];
  max_distance -= max_distance / LOAD_AREA_TRIM_FRACTION;
  // TODO: spherical iteration here?
  for (
    rcpos.x = center->x - max_distance;
    rcpos.x < center->x + max_distance;
    ++rcpos.x
  ) {
    for (
      rcpos.y = center->y - max_distance;
      rcpos.y < center->y + max_distance;
      ++rcpos.y
    ) {
      for (
        rcpos.z = center->z - (max_distance / VERTICAL_LOAD_BIAS);
        rcpos.z < center->z + (max_distance / VERTICAL_LOAD_BIAS);
        ++rcpos.z
      ) {
        d2 = (
          (rcpos.x - center->x) * (rcpos.x - center->x)
        +
          (rcpos.y - center->y) * (rcpos.y - center->y)
        +
          (
            (rcpos.z - center->z) * (rcpos.z - center->z)
          *
            VERTICAL_LOAD_BIAS * VERTICAL_LOAD_BIAS
          )
        );
        for (detail = LOD_BASE; detail < N_LODS; ++detail) {
          edge = LOAD_DISTANCES[detail];
          if (d2 <= edge * edge) {
            mark_for_loading(&rcpos, detail);
            break;
          }
        }
      }
    }
  }
}

void tick_data(void) {
  int n = 0;
  chunk *c = NULL;
  chunk *old_chunk = NULL;
  chunk_approximation *ca = NULL;
  chunk_approximation *old_approx = NULL;
  lod detail = LOD_BASE;
  queue *q = LOAD_QUEUES->levels[LOD_BASE];
  map *m = LOAD_QUEUES->maps[LOD_BASE];
  while (n < LOAD_CAP && q_get_length(q) > 0) {
    c = (chunk *) q_pop_element(q);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    m3_pop_value(
      m,
      (map_key_t) c->rcpos.x,
      (map_key_t) c->rcpos.y,
      (map_key_t) c->rcpos.z
    );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
    load_chunk(c);
    c->chunk_flags &= ~CF_QUEUED_TO_LOAD;
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    old_chunk = (chunk *) m3_put_value(
      CHUNK_CACHE->levels[LOD_BASE],
      (void *) c,
      (map_key_t) c->rcpos.x,
      (map_key_t) c->rcpos.y,
      (map_key_t) c->rcpos.z
    );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
    if (old_chunk != NULL) {
      cleanup_chunk(old_chunk);
    }
    n += 1;
  }
  for (detail = LOD_BASE + 1; detail < N_LODS; ++detail) {
    q = LOAD_QUEUES->levels[detail];
    m = LOAD_QUEUES->maps[detail];
    while (n < LOAD_CAP && q_get_length(q) > 0) {
      ca = (chunk_approximation *) q_pop_element(q);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
      m3_pop_value(
        m,
        (map_key_t) ca->rcpos.x,
        (map_key_t) ca->rcpos.y,
        (map_key_t) ca->rcpos.z
      );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
      load_chunk_approx(ca);
      ca->chunk_flags &= ~CF_QUEUED_TO_LOAD;
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
      old_approx = (chunk_approximation *) m3_put_value(
        CHUNK_CACHE->levels[ca->detail],
        (void *) ca,
        (map_key_t) ca->rcpos.x,
        (map_key_t) ca->rcpos.y,
        (map_key_t) ca->rcpos.z
      );
      if (old_approx != NULL) {
        cleanup_chunk_approximation(old_approx);
      }
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
      n += 1;
    }
  }
  update_count(&CHUNKS_LOADED, n);
  n = 0;
  q = COMPILE_QUEUES->levels[LOD_BASE];
  chunk_or_approx coa;
  while (n < COMPILE_CAP && q_get_length(q) > 0) {
    c = (chunk *) q_pop_element(q);
    ch__coa(c, &coa);
    compute_exposure(&coa);
    compile_chunk_or_approx(&coa);
    c->chunk_flags &= ~CF_QUEUED_TO_COMPILE;
    n += 1;
  }
  for (detail = LOD_BASE + 1; detail < N_LODS; ++detail) {
    q = COMPILE_QUEUES->levels[detail];
    while (n < COMPILE_CAP && q_get_length(q) > 0) {
      ca = (chunk_approximation *) q_pop_element(q);
      aprx__coa(ca, &coa);
      compute_exposure(&coa);
      compile_chunk_or_approx(&coa);
      ca->chunk_flags &= ~CF_QUEUED_TO_COMPILE;
      n += 1;
    }
  }
  update_count(&CHUNKS_COMPILED, n);
}

void load_chunk(chunk *c) {
  // TODO: Data from disk!
  // TODO: Cell entities!
  chunk_index idx;
  region_pos rpos;
  chunk_or_approx coa;
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        cidx__rpos(c, &idx, &rpos);
#ifdef TERRAIN_MODE_BASIC
        terrain_cell(&rpos, c_cell(c, idx));
#else
        world_cell(THE_WORLD, &rpos, c_cell(c, idx));
#endif
      }
    }
  }
  c->chunk_flags |= CF_LOADED;
  if (c->chunk_flags & CF_COMPILE_ON_LOAD) {
    c->chunk_flags &= ~CF_COMPILE_ON_LOAD;
    coa.type = CA_TYPE_CHUNK;
    coa.ptr = c;
    mark_for_compilation(&coa);
    mark_neighbors_for_compilation(&(c->rcpos));
  }
}

void load_chunk_approx(chunk_approximation *ca) {
  // TODO: Data from disk!
  // TODO: Cell entities!
  ch_idx_t step = (1 << (ca->detail));
  chunk_index idx;
  region_pos rpos;
  chunk_or_approx coa;
  lod previous_detail;
  // TODO: Better approximation?
  for (idx.x = 0; idx.x < CHUNK_SIZE; idx.x += step) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; idx.y += step) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; idx.z += step) {
        caidx__rpos(ca, &idx, &rpos);
#ifdef TERRAIN_MODE_BASIC
        terrain_cell(&rpos, ca_cell(ca, idx));
#else
        world_cell(THE_WORLD, &rpos, ca_cell(ca, idx));
#endif
      }
    }
  }
  ca->chunk_flags |= CF_LOADED;
  if (ca->chunk_flags & CF_COMPILE_ON_LOAD) {
    ca->chunk_flags &= ~CF_COMPILE_ON_LOAD;
    coa.type = CA_TYPE_APPROXIMATION;
    coa.ptr = ca;
    mark_for_compilation(&coa);
    previous_detail = get_best_loaded_level(&(ca->rcpos));
    // Only recompile neighbors if the newly-loaded approximation is a step up
    // (or at least sideways) from the previous approximation:
    if (previous_detail >= ca->detail) {
      mark_neighbors_for_compilation(&(ca->rcpos));
    }
  }
}
