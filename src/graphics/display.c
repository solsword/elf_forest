// display.c
// Functions for setting up display information.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include <assert.h>
#include <stdlib.h>

#include <GLee.h> // glBindBuffer etc.

#include "tex.h"
#include "dta.h"
#include "display.h"

#include "world/blocks.h"
#include "world/world.h"
#include "util.h"

/*********************
 * Private Functions *
 *********************/

// The various push_* functions add vertex/index data for faces of a cube to
// the given vertex buffer. They use local xyz coordinates to specify the
// position of the triangles that they're adding. Note that because normals
// need to be specified using min and max short values, we temporarily define
// some macros to make this easier.
#define P1 smaxof(GLshort) // +1
#define N1 sminof(GLshort) // -1
static inline void push_top(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x;            v.nx =  0;    v.s = st.s;
  v.y = idx.y;            v.ny =  0;    v.t = st.t + 1;
  v.z = idx.z + scale;    v.nz = P1;
  add_vertex(&v, vb);

  // top left
  v.y += scale;                         v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.x += scale;                         v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.y -= scale;                         v.t += 1;
  add_vertex(&v, vb);
}

static inline void push_bottom(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x + scale;    v.nx =  0;    v.s = st.s;
  v.y = idx.y;            v.ny =  0;    v.t = st.t + 1;
  v.z = idx.z;            v.nz = N1;
  add_vertex(&v, vb);

  // top left
  v.y += scale;                         v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.x -= scale;                         v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.y -= scale;                         v.t += 1;
  add_vertex(&v, vb);
}

static inline void push_north(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x + scale;    v.nx =  0;    v.s = st.s;
  v.y = idx.y + scale;    v.ny = P1;    v.t = st.t + 1;
  v.z = idx.z;            v.nz =  0;
  add_vertex(&v, vb);

  // top left
  v.z += scale;                         v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.x -= scale;                         v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z -= scale;                         v.t += 1;
  add_vertex(&v, vb);
}

static inline void push_south(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x;    v.nx =  0;    v.s = st.s;
  v.y = idx.y;    v.ny = N1;    v.t = st.t + 1;
  v.z = idx.z;    v.nz =  0;
  add_vertex(&v, vb);

  // top left
  v.z += scale;                 v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.x += scale;                 v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z -= scale;                 v.t += 1;
  add_vertex(&v, vb);
}

static inline void push_east(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x + scale;    v.nx = P1;    v.s = st.s;
  v.y = idx.y;            v.ny =  0;    v.t = st.t + 1;
  v.z = idx.z;            v.nz =  0;
  add_vertex(&v, vb);

  // top left
  v.z += scale;                         v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.y += scale;                         v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z -= scale;                         v.t += 1;
  add_vertex(&v, vb);
}

static inline void push_west(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x;            v.nx = N1;    v.s = st.s;
  v.y = idx.y + scale;    v.ny =  0;    v.t = st.t + 1;
  v.z = idx.z;            v.nz =  0;
  add_vertex(&v, vb);

  // top left
  v.z += scale;                         v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.y -= scale;                         v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z -= scale;                         v.t += 1;
  add_vertex(&v, vb);
}
// Clean up our short macro definitions:
#undef P1
#undef N1

/*************
 * Functions *
 *************/

void compile_chunk_or_approx(chunk_or_approx *coa) {
  // Count the number of "active" cells (those not surrounded by solid
  // blocks). We use the cached exposure data here (call compute_exposure
  // first!).
  chunk *c;
  chunk_approximation *ca;
  block_info geom;
  ch_idx_t step = 1;
 // A pointer to an array of vertex buffers:
  vertex_buffer (*layers)[] = NULL;
  if (coa->type == CA_TYPE_CHUNK) {
    c = (chunk *) (coa->ptr);
    ca = NULL;
    step = 1;
    layers = &(c->layers);
  } else if (coa->type == CA_TYPE_APPROXIMATION) {
    c = NULL;
    ca = (chunk_approximation *) (coa->ptr);
    step = 1 << (ca->detail);
    layers = &(ca->layers);
  } else {
    // We can't deal with unloaded chunks
    return;
  }
  uint16_t counts[N_LAYERS];
  uint16_t total = 0;
  layer i;
  for (i = 0; i < N_LAYERS; ++i) {
    counts[i] = 0;
  }
  chunk_index idx;
  cell *here = NULL;
  block exposure = 0;
  for (idx.x = 0; idx.x < CHUNK_SIZE; idx.x += step) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; idx.y += step) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; idx.z += step) {
        if (coa->type == CA_TYPE_CHUNK) {
          here = c_cell(c, idx);
        } else {
          here = ca_cell(ca, idx);
        }
        ensure_mapped(here->primary);
        ensure_mapped(here->secondary);
        exposure = cl_get_exposure(here);
        geom = bi_geom(here->primary);
        // TODO: HERE!
        if ((exposure & BF_EXPOSED_ANY) && !b_is_invisible(here->primary)) {
          total += 1;
          counts[block_layer(here->primary)] += 1;
        }
      }
    }
  }
  if (total == 0) {
    for (i = 0; i < N_LAYERS; ++i) {
      cleanup_vertex_buffer(&((*layers)[i]));
    }
    if (coa->type == CA_TYPE_CHUNK) {
      c->chunk_flags |= CF_COMPILED;
    } else {
      ca->chunk_flags |= CF_COMPILED;
    }
    return;
  }

  // Some quick space calculations:
  // 4 vertices/face * 6 faces/cube = 24 vertices/cube
  // 3 indices/triangle * 2 triangles/face * 6 faces/cube = 36 indices/cube
  //
  // Each vertex is 16 bytes, and each index is 4 bytes.
  // 
  // 16 bytes/vertex * 24 vertices/cube = 384 bytes/cube
  // 4 bytes/index * 36 indices/cube = 144 bytes/cube
  //   384+144 = 528 bytes/cube
  // For a place where an entire 32 * 32 plane of chunks is visible, there
  // might be total of
  //   32 * 32 * 16 * 16 = 262144 cubes
  // This makes a total of 528*262144 = 138412032 bytes, or 144 MB across all
  // active vertex arrays. Of course, hills and valleys might increase this,
  // but culling should usually decrease it, and it's the right order of
  // magnitude.
  
  // (Re)allocate caches for the vertex buffers. Note that we might cull some
  // faces later, but we're going to ignore that for now, since these arrays
  // are temporary.
  for (i = 0; i < N_LAYERS; ++i) {
    setup_cache(
      24*counts[i],
      36*counts[i],
      &((*layers)[i])
    );
  }

  layer ly; // which layer a block falls into
  tcoords st = { .s=0, .t=0 }; // texture coordinates

#ifdef DEBUG
  int vcounts[N_LAYERS];
  int icounts[N_LAYERS];
  for (i = 0; i < N_LAYERS; ++i) {
    vcounts[i] = 0;
    icounts[i] = 0;
  }
  #define CHECK_LAYER(i) \
    vcounts[i] += 4; \
    icounts[i] += 6; \
    if (vcounts[i] > counts[i]*24) { \
      fprintf( \
        stderr, \
        "Vertex count exceeded for layer %d: %d > %d\n", \
        i, \
        vcounts[i], \
        counts[i] * 24 \
      ); \
      exit(-1); \
    } \
    if (icounts[i] > counts[i]*36) { \
      fprintf( \
        stderr, \
        "Index count exceeded for layer %d: %d > %d\n", \
        i, \
        icounts[i], \
        counts[i] * 36 \
      ); \
      exit(-1); \
    }
#endif

  for (idx.x = 0; idx.x < CHUNK_SIZE; idx.x += step) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; idx.y += step) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; idx.z += step) {
        // get local cell and neighbors:
        if (coa->type == CA_TYPE_CHUNK) {
          here = c_cell(c, idx);
        } else {
          here = ca_cell(ca, idx);
        }
        exposure = cl_get_exposure(here);
        if (b_is_invisible(here->primary)) {
          continue;
        }
        ly = block_layer(here->primary);
        if (exposure & BF_EXPOSED_ABOVE) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_dynamic_face_tc(
            here->primary,
            BD_ORI_UP,
            &st
          );
          push_top(&((*layers)[ly]), idx, step, st);
        }
        if (exposure & BF_EXPOSED_BELOW) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_dynamic_face_tc(
            here->primary,
            BD_ORI_DOWN,
            &st
          );
          push_bottom(&((*layers)[ly]), idx, step, st);
        }
        if (exposure & BF_EXPOSED_NORTH) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_dynamic_face_tc(
            here->primary,
            BD_ORI_NORTH,
            &st
          );
          push_north(&((*layers)[ly]), idx, step, st);
        }
        if (exposure & BF_EXPOSED_SOUTH) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_dynamic_face_tc(
            here->primary,
            BD_ORI_SOUTH,
            &st
          );
          push_south(&((*layers)[ly]), idx, step, st);
        }
        if (exposure & BF_EXPOSED_EAST) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_dynamic_face_tc(
            here->primary,
            BD_ORI_EAST,
            &st
          );
          push_east(&((*layers)[ly]), idx, step, st);
        }
        if (exposure & BF_EXPOSED_WEST) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_dynamic_face_tc(
            here->primary,
            BD_ORI_WEST,
            &st
          );
          push_west(&((*layers)[ly]), idx, step, st);
        }
      }
    }
  }
  // Compile or cleanup each buffer:
  for (i = 0; i < N_LAYERS; ++i) {
    if (counts[i] > 0) {
      compile_buffers(&((*layers)[i]));
    } else {
      cleanup_vertex_buffer(&((*layers)[i]));
    }
  }
  // Mark the chunk as compiled:
  if (coa->type == CA_TYPE_CHUNK) {
    c->chunk_flags |= CF_COMPILED;
  } else {
    ca->chunk_flags |= CF_COMPILED;
  }
}
