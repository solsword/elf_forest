// display.c
// Functions for setting up display information.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include <assert.h>
#include <stdlib.h>

#include <GLee.h> // glBindBuffer etc.

#include "blocks.h"
#include "world.h"
#include "tex.h"
#include "display.h"
#include "util.h"

/*********************
 * Private Functions *
 *********************/

static inline block cull_face(block here, block neighbor) {
  return (
    !block_is(neighbor, OUT_OF_RANGE)
  &&
    (
      is_opaque(neighbor)
    ||
      (
        is_translucent(here)
      &&
        shares_translucency(here, neighbor)
      )
    )
  );
}

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
  uint16_t opaque_count = 0;
  uint16_t translucent_count = 0;
  chunk_index idx;
  block here = 0;
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        here = c_get_block(c, idx);
        if (is_exposed(here) && !is_invisible(here)) {
          if (is_translucent(here)) {
            translucent_count += 1;
          } else {
            opaque_count += 1;
          }
        }
      }
    }
  }
  if (opaque_count == 0 && translucent_count == 0) {
    cleanup_vertex_buffer(&(c->opaque_vertices));
    cleanup_vertex_buffer(&(c->translucent_vertices));
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
  setup_cache(
    24*opaque_count,
    36*opaque_count,
    &(c->opaque_vertices)
  );
  setup_cache(
    24*translucent_count,
    36*translucent_count,
    &(c->translucent_vertices)
  );

  tcoords st; // texture coordinates
  st.s = 0;
  st.t = 0;
  block ba = 0, bb = 0, bn = 0, bs = 0, be = 0, bw = 0;

#ifdef DEBUG
  int opaque_vcount = 0;
  int opaque_icount = 0;
  int translucent_vcount = 0;
  int translucent_icount = 0;
  #define CHECK_OPAQUE \
    opaque_vcount += 4; \
    opaque_icount += 6; \
    if (opaque_vcount > opaque_count*24) { \
      printf( \
        "Opaque vertex count exceeded: %d > %d\n", \
        opaque_vcount, \
        opaque_count * 24 \
      ); \
      exit(-1); \
    } \
    if (opaque_icount > opaque_count*36) { \
      printf( \
        "Opaque index count exceeded: %d > %d\n", \
        opaque_icount, \
        opaque_count * 36 \
      ); \
      exit(-1); \
    }
  #define CHECK_TRANSLUCENT \
    if (translucent_vcount > translucent_count*24) { \
      printf( \
        "Translucent vertex count exceeded: %d > %d\n", \
        translucent_vcount, \
        translucent_count * 24 \
      ); \
      exit(-1); \
    } \
    if (translucent_icount > translucent_count*36) { \
      printf( \
        "Translucent index count exceeded: %d > %d\n", \
        translucent_icount, \
        translucent_count * 24 \
      ); \
      exit(-1); \
    }
#endif

  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        // get local block and neighbors:
        here = c_get_block(c, idx);
        c_get_neighbors(c, idx, &ba, &bb, &bn, &bs, &be, &bw);
        if (is_exposed(here) && !is_invisible(here)) {
          if (!cull_face(here, ba)) {
            compute_face_tc(here, BD_ORI_UP, &st);
            if (is_translucent(here)) {
#ifdef DEBUG
              CHECK_TRANSLUCENT
#endif
              push_top(&(c->translucent_vertices), idx, st);
            } else {
#ifdef DEBUG
              CHECK_OPAQUE
#endif
              push_top(&(c->opaque_vertices), idx, st);
            }
          }
          if (!cull_face(here, bn)) {
            compute_face_tc(here, BD_ORI_NORTH, &st);
            if (is_translucent(here)) {
#ifdef DEBUG
              CHECK_TRANSLUCENT
#endif
              push_north(&(c->translucent_vertices), idx, st);
            } else {
#ifdef DEBUG
              CHECK_OPAQUE
#endif
              push_north(&(c->opaque_vertices), idx, st);
            }
          }
          if (!cull_face(here, bs)) {
            compute_face_tc(here, BD_ORI_SOUTH, &st);
            if (is_translucent(here)) {
#ifdef DEBUG
              CHECK_TRANSLUCENT
#endif
              push_south(&(c->translucent_vertices), idx, st);
            } else {
#ifdef DEBUG
              CHECK_OPAQUE
#endif
              push_south(&(c->opaque_vertices), idx, st);
            }
          }
          if (!cull_face(here, be)) {
            compute_face_tc(here, BD_ORI_EAST, &st);
            if (is_translucent(here)) {
#ifdef DEBUG
              CHECK_TRANSLUCENT
#endif
              push_east(&(c->translucent_vertices), idx, st);
            } else {
#ifdef DEBUG
              CHECK_OPAQUE
#endif
              push_east(&(c->opaque_vertices), idx, st);
            }
          }
          if (!cull_face(here, bw)) {
            compute_face_tc(here, BD_ORI_WEST, &st);
            if (is_translucent(here)) {
#ifdef DEBUG
              CHECK_TRANSLUCENT
#endif
              push_west(&(c->translucent_vertices), idx, st);
            } else {
#ifdef DEBUG
              CHECK_OPAQUE
#endif
              push_west(&(c->opaque_vertices), idx, st);
            }
          }
          if (!cull_face(here, bb)) {
            compute_face_tc(here, BD_ORI_DOWN, &st);
            if (is_translucent(here)) {
#ifdef DEBUG
              CHECK_TRANSLUCENT
#endif
              push_bottom(&(c->translucent_vertices), idx, st);
            } else {
#ifdef DEBUG
              CHECK_OPAQUE
#endif
              push_bottom(&(c->opaque_vertices), idx, st);
            }
          }
        }
      }
    }
  }
  // Compile the buffers:
  if (opaque_count > 0) {
    compile_buffers(&(c->opaque_vertices));
  } else {
    cleanup_vertex_buffer(&(c->opaque_vertices));
  }
  if (translucent_count > 0) {
    compile_buffers(&(c->translucent_vertices));
  } else {
    cleanup_vertex_buffer(&(c->translucent_vertices));
  }
  // Mark the chunk as compiled:
  c->flags &= ~CF_NEEDS_RECOMIPLE;
}
