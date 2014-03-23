// display.c
// Functions for setting up display information.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include <assert.h>
#include <stdlib.h>

#include <GLee.h> // glBindBuffer etc.

#include "tex.h"
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
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x;        v.nx =  0;    v.s = st.s;
  v.y = idx.y;        v.ny =  0;    v.t = st.t + 1;
  v.z = idx.z + 1;    v.nz = P1;
  add_vertex(&v, vb);

  // top left
  v.y += 1;                        v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.x += 1;                        v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.y -= 1;                        v.t += 1;
  add_vertex(&v, vb);
}

static inline void push_bottom(
  vertex_buffer *vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x + 1;    v.nx =  0;    v.s = st.s;
  v.y = idx.y;        v.ny =  0;    v.t = st.t + 1;
  v.z = idx.z;        v.nz = N1;
  add_vertex(&v, vb);

  // top left
  v.y += 1;                        v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.x -= 1;                        v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.y -= 1;                        v.t += 1;
  add_vertex(&v, vb);
}

static inline void push_north(
  vertex_buffer *vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x + 1;    v.nx =  0;    v.s = st.s;
  v.y = idx.y + 1;    v.ny = P1;    v.t = st.t + 1;
  v.z = idx.z;        v.nz =  0;
  add_vertex(&v, vb);

  // top left
  v.z += 1;                        v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.x -= 1;                        v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z -= 1;                        v.t += 1;
  add_vertex(&v, vb);
}

static inline void push_south(
  vertex_buffer *vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x;    v.nx =  0;    v.s = st.s;
  v.y = idx.y;    v.ny = N1;    v.t = st.t + 1;
  v.z = idx.z;    v.nz =  0;
  add_vertex(&v, vb);

  // top left
  v.z += 1;                        v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.x += 1;                        v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z -= 1;                        v.t += 1;
  add_vertex(&v, vb);
}

static inline void push_east(
  vertex_buffer *vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x + 1;    v.nx = P1;    v.s = st.s;
  v.y = idx.y;        v.ny =  0;    v.t = st.t + 1;
  v.z = idx.z;        v.nz =  0;
  add_vertex(&v, vb);

  // top left
  v.z += 1;                        v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.y += 1;                        v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z -= 1;                        v.t += 1;
  add_vertex(&v, vb);
}

static inline void push_west(
  vertex_buffer *vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x;        v.nx = N1;    v.s = st.s;
  v.y = idx.y + 1;    v.ny =  0;    v.t = st.t + 1;
  v.z = idx.z;        v.nz =  0;
  add_vertex(&v, vb);

  // top left
  v.z += 1;                        v.t -= 1;
  add_vertex(&v, vb);

  // top right
  v.y -= 1;                        v.s += 1;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z -= 1;                        v.t += 1;
  add_vertex(&v, vb);
}
// Clean up our short macro definitions:
#undef P1
#undef N1

/*************
 * Functions *
 *************/

void compile_chunk(chunk *c) {
  // Count the number of "active" blocks (those not surrounded by solid
  // blocks). We use the cached exposure data here (call compute_exposure
  // first!).
  uint16_t counts[N_LAYERS];
  uint16_t total = 0;
  layer i;
  for (i = 0; i < N_LAYERS; ++i) {
    counts[i] = 0;
  }
  chunk_index idx;
  block here = 0;
  block_flag flags = 0;
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        here = c_get_block(c, idx);
        flags = c_get_flags(c, idx);
        if ((flags & BF_EXPOSED_ANY) && !is_invisible(here)) {
          total += 1;
          counts[block_layer(here)] += 1;
        }
      }
    }
  }
  if (total == 0) {
    for (i = 0; i < N_LAYERS; ++i) {
      cleanup_vertex_buffer(&(c->layers[i]));
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
  // For a frame where an entire plane is visible, there might be total of
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
      &(c->layers[i])
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
      printf( \
        "Vertex count exceeded for layer %d: %d > %d\n", \
        i, \
        vcounts[i], \
        counts[i] * 24 \
      ); \
      exit(-1); \
    } \
    if (icounts[i] > counts[i]*36) { \
      printf( \
        "Index count exceeded for layer %d: %d > %d\n", \
        i, \
        icounts[i], \
        counts[i] * 36 \
      ); \
      exit(-1); \
    }
#endif

  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        // get local block and neighbors:
        here = c_get_block(c, idx);
        if (is_invisible(here)) {
          continue;
        }
        flags = c_get_flags(c, idx);
        ly = block_layer(here);
        if (flags & BF_EXPOSED_ABOVE) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_face_tc(here, BD_ORI_UP, &st);
          push_top(&(c->layers[ly]), idx, st);
        }
        if (flags & BF_EXPOSED_BELOW) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_face_tc(here, BD_ORI_DOWN, &st);
          push_bottom(&(c->layers[ly]), idx, st);
        }
        if (flags & BF_EXPOSED_NORTH) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_face_tc(here, BD_ORI_NORTH, &st);
          push_north(&(c->layers[ly]), idx, st);
        }
        if (flags & BF_EXPOSED_SOUTH) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_face_tc(here, BD_ORI_SOUTH, &st);
          push_south(&(c->layers[ly]), idx, st);
        }
        if (flags & BF_EXPOSED_EAST) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_face_tc(here, BD_ORI_EAST, &st);
          push_east(&(c->layers[ly]), idx, st);
        }
        if (flags & BF_EXPOSED_WEST) {
#ifdef DEBUG
          CHECK_LAYER(ly)
#endif
          compute_face_tc(here, BD_ORI_WEST, &st);
          push_west(&(c->layers[ly]), idx, st);
        }
      }
    }
  }
  // Compile or cleanup each buffer:
  for (i = 0; i < N_LAYERS; ++i) {
    if (counts[i] > 0) {
      compile_buffers(&(c->layers[i]));
    } else {
      cleanup_vertex_buffer(&(c->layers[i]));
    }
  }
}
