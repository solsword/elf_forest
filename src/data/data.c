// data.c
// Data management.

#include <stdint.h>
#include <stdio.h>

#include "data.h"

#include "persist.h"

#include "datatypes/queue.h"
#include "datatypes/map.h"

#include "graphics/display.h"
#include "gen/worldgen.h"
#include "gen/biology.h"
#include "prof/ptime.h"
#include "world/blocks.h"
#include "world/world.h"
#include "world/entities.h"
#include "world/chunk_data.h"
#include "control/ctl.h"

/*************
 * Constants *
 *************/

size_t const CHUNK_QUEUE_SET_MAP_SIZE = 2048;
size_t const CHUNK_CACHE_MAP_SIZE = 16384;

// TODO: dynamic capping?
//int const LOAD_CAP = 128;
//int const COMPILE_CAP = 64;
//int const BIOGEN_CAP = 64;
int const LOAD_CAP = 64;
int const COMPILE_CAP = 8;
int const BIOGEN_CAP = 16;
//int const LOAD_CAP = 1;
//int const COMPILE_CAP = 2;
//int const BIOGEN_CAP = 2;

// TODO: Good values here
//gl_cpos_t const LOAD_DISTANCES[N_LODS] = { 6, 16, 50, 150, 500 };
//gl_cpos_t const LOAD_DISTANCES[N_LODS] = { 8, 16, 32, 64, 128 };
//gl_cpos_t const LOAD_DISTANCES[N_LODS] = { 4, 8, 12, 16, 20 };
gl_cpos_t const LOAD_DISTANCES[N_LODS] = { 4, 7, 12, 12, 12 };
//gl_cpos_t const LOAD_DISTANCES[N_LODS] = { 4, 6, 7, 7, 7 };
//gl_cpos_t const LOAD_DISTANCES[N_LODS] = { 3, 4, 4, 4, 4 };
//gl_cpos_t const LOAD_DISTANCES[N_LODS] = { 2, 3, 3, 3, 3 };
//gl_cpos_t const LOAD_DISTANCES[N_LODS] = { 1, 2, 2, 2, 2 };

int const VERTICAL_LOAD_BIAS = 2;

int const LOAD_AREA_TRIM_FRACTION = 10;

/***********
 * Globals *
 ***********/

chunk_queue_set *LOAD_QUEUES = NULL;
chunk_queue_set *COMPILE_QUEUES = NULL;
chunk_queue_set *BIOGEN_QUEUES = NULL;

chunk_cache *CHUNK_CACHE = NULL;

/********************
 * Search Functions *
 ********************/

int find_chunk_at_position(void *element, void *reference) {
  chunk *c = (chunk *) element;
  global_chunk_pos *glcpos = (global_chunk_pos *) reference;
  return (
    c->glcpos.x == glcpos->x
  &&
    c->glcpos.y == glcpos->y
  &&
    c->glcpos.z == glcpos->z
  );
}
int find_chunk_approx_at_position(void *element, void *reference) {
  chunk_approximation *ca = (chunk_approximation *) element;
  global_chunk_pos *glcpos = (global_chunk_pos *) reference;
  return (
    ca->glcpos.x == glcpos->x
  &&
    ca->glcpos.y == glcpos->y
  &&
    ca->glcpos.z == glcpos->z
  );
}

/****************************
 * Private Inline Functions *
 ****************************/

static inline int is_loaded(global_chunk_pos *glcpos, lod detail) {
  m_lock(CHUNK_CACHE->levels[detail]);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  int result = m3_contains_key(
    CHUNK_CACHE->levels[detail],
    (map_key_t) glcpos->x,
    (map_key_t) glcpos->y,
    (map_key_t) glcpos->z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
  m_unlock(CHUNK_CACHE->levels[detail]);
  return result;
}

static inline int is_loading(global_chunk_pos *glcpos, lod detail) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  return m3_contains_key(
    LOAD_QUEUES->maps[detail],
    (map_key_t) glcpos->x,
    (map_key_t) glcpos->y,
    (map_key_t) glcpos->z
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
  BIOGEN_QUEUES = create_chunk_queue_set();
  CHUNK_CACHE = create_chunk_cache();
}

void cleanup_data(void) {
  destroy_chunk_queue_set(LOAD_QUEUES);
  cleanup_chunk_queue_set(COMPILE_QUEUES);
  cleanup_chunk_queue_set(BIOGEN_QUEUES);
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
  q_lock(cqs->levels[LOD_BASE]);
  q_push_element(cqs->levels[LOD_BASE], (void *) c);
  q_unlock(cqs->levels[LOD_BASE]);
  m_lock(cqs->maps[LOD_BASE]);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  m3_put_value(
    cqs->maps[LOD_BASE],
    (void *) c,
    (map_key_t) c->glcpos.x,
    (map_key_t) c->glcpos.y,
    (map_key_t) c->glcpos.z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
  m_unlock(cqs->maps[LOD_BASE]);
}

void enqueue_chunk_approximation(chunk_queue_set *cqs, chunk_approximation *ca){
  q_lock(cqs->levels[ca->detail]);
  q_push_element(cqs->levels[ca->detail], (void *) ca);
  q_unlock(cqs->levels[ca->detail]);
  m_lock(cqs->maps[ca->detail]);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
  m3_put_value(
    cqs->maps[ca->detail],
    (void *) ca,
    (map_key_t) ca->glcpos.x,
    (map_key_t) ca->glcpos.y,
    (map_key_t) ca->glcpos.z
  );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
  m_unlock(cqs->maps[ca->detail]);
}

void mark_for_loading(global_chunk_pos *glcpos, lod detail) {
  if (is_loaded(glcpos, detail) || is_loading(glcpos, detail)) {
    return;
  }
  if (detail == LOD_BASE) {
    chunk *c = create_chunk(glcpos);
    c->chunk_flags |= CF_QUEUED_TO_LOAD;
    c->chunk_flags |= CF_COMPILE_ON_LOAD;
    enqueue_chunk(LOAD_QUEUES, c);
  } else {
    chunk_approximation *ca = create_chunk_approximation(glcpos, detail);
    ca->chunk_flags |= CF_QUEUED_TO_LOAD;
    ca->chunk_flags |= CF_COMPILE_ON_LOAD;
    enqueue_chunk_approximation(LOAD_QUEUES, ca);
  }
}

void mark_for_compilation(chunk_or_approx *coa) {
  if (coa->type == CA_TYPE_CHUNK) {
    chunk *c = (chunk *) coa->ptr;
    if (c->chunk_flags & CF_QUEUED_TO_COMPILE) { return; }
    c->chunk_flags |= CF_QUEUED_TO_COMPILE;
    enqueue_chunk(COMPILE_QUEUES, c);
  } else if (coa->type == CA_TYPE_APPROXIMATION) {
    chunk_approximation *ca = (chunk_approximation *) coa->ptr;
    if (ca->chunk_flags & CF_QUEUED_TO_COMPILE) { return; }
    ca->chunk_flags |= CF_QUEUED_TO_COMPILE;
    enqueue_chunk_approximation(COMPILE_QUEUES, ca);
  } else {
    fprintf(stderr, "Can't mark an unloaded chunk for compilation.\n");
    exit(EXIT_FAILURE);
  }
}

void mark_neighbors_for_compilation(global_chunk_pos *center) {
  chunk_or_approx coa;
  global_chunk_pos glcpos;
  glcpos.x = center->x;
  glcpos.y = center->y;
  glcpos.z = center->z;

  glcpos.x += 1;
  get_best_data(&glcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }

  glcpos.x -= 2;
  get_best_data(&glcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }

  glcpos.x += 1;
  glcpos.y += 1;
  get_best_data(&glcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }

  glcpos.y -= 2;
  get_best_data(&glcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }

  glcpos.y += 1;
  glcpos.z += 1;
  get_best_data(&glcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }

  glcpos.z -= 2;
  get_best_data(&glcpos, &coa);
  if (coa.ptr != NULL) { mark_for_compilation(&coa); }
}

void mark_for_biogen(chunk *c) {
  if (
     (c->chunk_flags & CF_QUEUED_FOR_BIOGEN)
  || (c->chunk_flags & CF_HAS_BIOLOGY)
  ) { return; }
  enqueue_chunk(BIOGEN_QUEUES, c);
}

void mark_neighbors_for_biogen(global_chunk_pos *center) {
  chunk_or_approx coa;
  global_chunk_pos glcpos;
  glcpos.x = center->x;
  glcpos.y = center->y;
  glcpos.z = center->z;

  for (glcpos.x = center->x - 1; glcpos.x < center->x + 2; ++glcpos.x) {
    for (glcpos.y = center->y - 1; glcpos.y < center->y + 2; ++glcpos.y) {
      for (glcpos.z = center->z - 1; glcpos.z < center->z + 2; ++glcpos.z) {
        get_best_data(&glcpos, &coa);
        if (coa.ptr != NULL && coa.type == CA_TYPE_CHUNK) {
          mark_for_biogen(coa.ptr);
        }
      }
    }
  }
}

lod get_best_loaded_level(global_chunk_pos *glcpos) {
  lod detail = N_LODS; // level of detail being considered
  for (detail = LOD_BASE; detail < N_LODS; ++detail) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    m_lock(CHUNK_CACHE->levels[detail]);
    if (
      m3_contains_key(
        CHUNK_CACHE->levels[detail],
        (map_key_t) glcpos->x,
        (map_key_t) glcpos->y,
        (map_key_t) glcpos->z
      )
    ) {
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
      m_unlock(CHUNK_CACHE->levels[detail]);
      return detail;
    }
    m_unlock(CHUNK_CACHE->levels[detail]);
  }
  return N_LODS;
}

void get_best_data(global_chunk_pos *glcpos, chunk_or_approx *coa) {
  get_best_data_limited(glcpos, LOD_BASE, 0, coa);
}

void get_best_data_limited(
  global_chunk_pos *glcpos,
  lod limit,
  uint8_t compiled,
  chunk_or_approx *coa
) {
  lod detail = LOD_BASE; // level of detail being considered
  if (limit <= LOD_BASE) {
    coa->type = CA_TYPE_CHUNK;
    m_lock(CHUNK_CACHE->levels[LOD_BASE]);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    coa->ptr = m3_get_value(
      CHUNK_CACHE->levels[LOD_BASE],
      (map_key_t) glcpos->x,
      (map_key_t) glcpos->y,
      (map_key_t) glcpos->z
    );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
    m_unlock(CHUNK_CACHE->levels[LOD_BASE]);
    if (
      coa->ptr != NULL
    &&
      (!compiled || (((chunk*) (coa->ptr))->chunk_flags) & CF_COMPILED)
    ) {
      return;
    }
    limit = LOD_BASE + 1; // Increase our limit so that the for loop works.
  }
  coa->type = CA_TYPE_APPROXIMATION;
  for (detail = limit; detail < N_LODS; ++detail) {
    m_lock(CHUNK_CACHE->levels[detail]);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    coa->ptr = m3_get_value(
      CHUNK_CACHE->levels[detail],
      (map_key_t) glcpos->x,
      (map_key_t) glcpos->y,
      (map_key_t) glcpos->z
    );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
    m_unlock(CHUNK_CACHE->levels[detail]);
    if (
      coa->ptr != NULL
    &&
      (
        !compiled
      ||
        (((chunk_approximation*) (coa->ptr))->chunk_flags) & CF_COMPILED
      )
    ) {
      return;
    }
  }
  coa->type = CA_TYPE_NOT_LOADED;
  coa->ptr = NULL;
  return;
}

void load_surroundings(global_chunk_pos *center) {
  lod detail = LOD_BASE; // level of detail being considered
  global_chunk_pos glcpos = { .x=0, .y=0, .z=0 }; // current chunk
  // Max distance at which to load anything, trimmed a bit:
  gl_cpos_t max_distance = LOAD_DISTANCES[N_LODS - 1];
  max_distance -= max_distance / LOAD_AREA_TRIM_FRACTION;
  // TODO: spherical iteration here?
  for (
    glcpos.x = center->x - max_distance;
    glcpos.x < center->x + max_distance;
    ++glcpos.x
  ) {
    for (
      glcpos.y = center->y - max_distance;
      glcpos.y < center->y + max_distance;
      ++glcpos.y
    ) {
      for (
        glcpos.z = center->z - (max_distance / VERTICAL_LOAD_BIAS);
        glcpos.z < center->z + (max_distance / VERTICAL_LOAD_BIAS);
        ++glcpos.z
      ) {
        detail = desired_detail_at(center, &glcpos);
        if (detail < N_LODS) {
          mark_for_loading(&glcpos, detail);
        }
      }
    }
  }
}

void tick_load_chunks(global_chunk_pos *load_center) {
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
      (map_key_t) c->glcpos.x,
      (map_key_t) c->glcpos.y,
      (map_key_t) c->glcpos.z
    );
    if (desired_detail_at(load_center, &(c->glcpos)) > LOD_BASE) {
      // discard this chunk and don't load it.
      cleanup_chunk(c);
      continue;
    }
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
    load_chunk(c);
    c->chunk_flags &= ~CF_QUEUED_TO_LOAD;
    m_lock(CHUNK_CACHE->levels[LOD_BASE]);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    old_chunk = (chunk *) m3_put_value(
      CHUNK_CACHE->levels[LOD_BASE],
      (void *) c,
      (map_key_t) c->glcpos.x,
      (map_key_t) c->glcpos.y,
      (map_key_t) c->glcpos.z
    );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
    m_unlock(CHUNK_CACHE->levels[LOD_BASE]);
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
        (map_key_t) ca->glcpos.x,
        (map_key_t) ca->glcpos.y,
        (map_key_t) ca->glcpos.z
      );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
      if (desired_detail_at(load_center, &(ca->glcpos)) > detail) {
        // discard this chunk and don't load it.
        cleanup_chunk_approximation(ca);
        continue;
      }
      load_chunk_approx(ca);
      ca->chunk_flags &= ~CF_QUEUED_TO_LOAD;
      m_lock(CHUNK_CACHE->levels[ca->detail]);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
      old_approx = (chunk_approximation *) m3_put_value(
        CHUNK_CACHE->levels[ca->detail],
        (void *) ca,
        (map_key_t) ca->glcpos.x,
        (map_key_t) ca->glcpos.y,
        (map_key_t) ca->glcpos.z
      );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
      m_unlock(CHUNK_CACHE->levels[ca->detail]);
      if (old_approx != NULL) {
        cleanup_chunk_approximation(old_approx);
      }
      n += 1;
    }
  }
  update_count(&CHUNKS_LOADED, n);
}

void tick_compile_chunks(void) {
  int n = 0;
  chunk *c = NULL;
  chunk_approximation *ca = NULL;
  lod detail = LOD_BASE;
  queue *q = COMPILE_QUEUES->levels[LOD_BASE];
  map *m = COMPILE_QUEUES->maps[LOD_BASE];
  chunk_or_approx coa;
  while (n < COMPILE_CAP && q_get_length(q) > 0) {
    c = (chunk *) q_pop_element(q);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    m3_pop_value(
      m,
      (map_key_t) c->glcpos.x,
      (map_key_t) c->glcpos.y,
      (map_key_t) c->glcpos.z
    );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
    ch__coa(c, &coa);
    compile_chunk_or_approx(&coa);
    c->chunk_flags &= ~CF_QUEUED_TO_COMPILE;
    n += 1;
  }
  for (detail = LOD_BASE + 1; detail < N_LODS; ++detail) {
    q = COMPILE_QUEUES->levels[detail];
    m = COMPILE_QUEUES->maps[detail];
    while (n < COMPILE_CAP && q_get_length(q) > 0) {
      ca = (chunk_approximation *) q_pop_element(q);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
      m3_pop_value(
        m,
        (map_key_t) ca->glcpos.x,
        (map_key_t) ca->glcpos.y,
        (map_key_t) ca->glcpos.z
      );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
      aprx__coa(ca, &coa);
      compile_chunk_or_approx(&coa);
      ca->chunk_flags &= ~CF_QUEUED_TO_COMPILE;
      n += 1;
    }
  }
  update_count(&CHUNKS_COMPILED, n);
}

void tick_biogen(void) {
  int n = 0, ns = 0, in_queue = 0;
  chunk *c = NULL;
  chunk_or_approx coa;
  queue *q = BIOGEN_QUEUES->levels[LOD_BASE];
  map *m = BIOGEN_QUEUES->maps[LOD_BASE];
  in_queue = q_get_length(q);
  while (n < BIOGEN_CAP && (n + ns) < in_queue) {
    c = (chunk *) q_pop_element(q);
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    m3_pop_value(
      m,
      (map_key_t) c->glcpos.x,
      (map_key_t) c->glcpos.y,
      (map_key_t) c->glcpos.z
    );
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
    add_biology(c);
    if (c->chunk_flags & CF_HAS_BIOLOGY) {
      n += 1;
      persist_chunk(c); // when the chunk was loaded we saved a bare-terrain
      // version, now that we've added biology let's save that too.
      // Mark this chunk for re-compilation now that it's been changed.
      coa.type = CA_TYPE_CHUNK;
      coa.ptr = c;
      mark_for_compilation(&coa);
    } else {
      ns += 1;
      // Put it back in the queue:
      mark_for_biogen(c); // infinite loop avoided in_queue isn't changed
    }
    c->chunk_flags &= ~CF_QUEUED_FOR_BIOGEN;
  }
  // TODO: Is it fine to ignore other LODs? Maybe we should be adding some fake
  // plants to them?
  update_count(&CHUNKS_BIOGEND, n);
  update_count(&CHUNKS_BIOSKIPPED, ns);
}

void load_chunk(chunk *c) {
  // TODO: Cell entities!
  chunk_or_approx coa;
#ifdef PROFILE_TIME
  start_duration(&DISK_READ_TIME);
  start_duration(&DISK_MISS_TIME);
#endif
  if (load_chunk_data(c)) {
#ifdef PROFILE_TIME
    end_duration(&DISK_READ_TIME);
    // in this case we don't record DISK_MISS_TIME
#endif
    // TODO: Anything here?
  } else {
#ifdef PROFILE_TIME
    end_duration(&DISK_MISS_TIME);
    // in this case we don't record DISK_READ_TIME
    start_duration(&TGEN_TIME);
#endif
    generate_chunk(c);
#ifdef PROFILE_TIME
    end_duration(&TGEN_TIME);
    start_duration(&DISK_WRITE_TIME);
#endif
    // TODO: Move/duplicate this somewhere else?
    persist_chunk(c);
#ifdef PROFILE_TIME
    end_duration(&DISK_WRITE_TIME);
#endif
  }
  c->chunk_flags |= CF_LOADED;
  if (c->chunk_flags & CF_COMPILE_ON_LOAD) {
    c->chunk_flags &= ~CF_COMPILE_ON_LOAD;
    coa.type = CA_TYPE_CHUNK;
    coa.ptr = c;
    mark_for_compilation(&coa);
    mark_neighbors_for_compilation(&(c->glcpos));
  }
  // TODO: This could be more efficient at the cost of a bit more memory
  // probably.
  if (!(c->chunk_flags & CF_HAS_BIOLOGY)) {
    mark_for_biogen(c);
  }
  mark_neighbors_for_biogen(&(c->glcpos));
}

void load_chunk_approx(chunk_approximation *ca) {
  // TODO: Data from disk!
  // TODO: Cell entities!
  int step = (1 << (ca->detail));
  block_index idx;
  global_pos glpos;
  chunk_or_approx coa;
  lod previous_detail;
  // TODO: Better approximation?
  idx.xyz.w = 0;
  for (idx.xyz.x = 0; idx.xyz.x < CHUNK_SIZE; idx.xyz.x += step) {
    for (idx.xyz.y = 0; idx.xyz.y < CHUNK_SIZE; idx.xyz.y += step) {
      for (idx.xyz.z = 0; idx.xyz.z < CHUNK_SIZE; idx.xyz.z += step) {
        caidx__glpos(ca, &idx, &glpos);
        world_cell(THE_WORLD, &glpos, ca_cell(ca, idx));
      }
    }
  }
  ca->chunk_flags |= CF_LOADED;
  if (ca->chunk_flags & CF_COMPILE_ON_LOAD) {
    ca->chunk_flags &= ~CF_COMPILE_ON_LOAD;
    coa.type = CA_TYPE_APPROXIMATION;
    coa.ptr = ca;
    mark_for_compilation(&coa);
    previous_detail = get_best_loaded_level(&(ca->glcpos));
    // Only recompile neighbors if the newly-loaded approximation is a step up
    // (or at least sideways) from the previous approximation:
    if (previous_detail >= ca->detail) {
      mark_neighbors_for_compilation(&(ca->glcpos));
    }
  }
  // TODO: Mark for biogen as well?
}
