// display.c
// Functions for setting up display information.

#include <assert.h>
#include <stdlib.h>
// DEBUG
#include <stdio.h>

#include <GLee.h> // glBindBuffer etc.

#include "blocks.h"
#include "world.h"
#include "tex.h"
#include "display.h"

/*********************
 * Private Functions *
 *********************/

static inline block cull_face(block here, block neighbor) {
  return (
    block_is(neighbor, B_VOID)
  ||
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
// position of the triangles that they're adding.
static inline void push_top(
  vertex_buffer vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x;        v.nx = 0;    v.s = st.s;
  v.y = idx.y;        v.ny = 0;    v.t = st.t + 1;
  v.z = idx.z + 1;    v.nz = 1;
  add_vertex(&v, &vb);

  // top left
  v.y += 1;                        v.t -= 1;
  add_vertex(&v, &vb);

  // top right
  v.x += 1;                        v.s += 1;
  add_vertex(&v, &vb);

  reuse_vertex(-3, &vb); // reuse bottom left

  reuse_vertex(-2, &vb); // reuse top right

  // bottom right
  v.y -= 1;                        v.t += 1;
  add_vertex(&v, &vb);
}

static inline void push_bottom(
  vertex_buffer vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x + 1;    v.nx =  0;    v.s = st.s;
  v.y = idx.y;        v.ny =  0;    v.t = st.t + 1;
  v.z = idx.z;        v.nz = -1;
  add_vertex(&v, &vb);

  // top left
  v.y += 1;                        v.t -= 1;
  add_vertex(&v, &vb);

  // top right
  v.x -= 1;                        v.s += 1;
  add_vertex(&v, &vb);

  reuse_vertex(-3, &vb); // reuse bottom left

  reuse_vertex(-2, &vb); // reuse top right

  // bottom right
  v.y -= 1;                        v.t += 1;
  add_vertex(&v, &vb);
}

static inline void push_north(
  vertex_buffer vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x + 1;    v.nx = 0;    v.s = st.s;
  v.y = idx.y + 1;    v.ny = 1;    v.t = st.t + 1;
  v.z = idx.z;        v.nz = 0;
  add_vertex(&v, &vb);

  // top left
  v.z += 1;                        v.t -= 1;
  add_vertex(&v, &vb);

  // top right
  v.x -= 1;                        v.s += 1;
  add_vertex(&v, &vb);

  reuse_vertex(-3, &vb); // reuse bottom left

  reuse_vertex(-2, &vb); // reuse top right

  // bottom right
  v.z -= 1;                        v.t += 1;
  add_vertex(&v, &vb);
}

static inline void push_south(
  vertex_buffer vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x;    v.nx =  0;    v.s = st.s;
  v.y = idx.y;    v.ny = -1;    v.t = st.t + 1;
  v.z = idx.z;    v.nz =  0;
  add_vertex(&v, &vb);

  // top left
  v.z += 1;                        v.t -= 1;
  add_vertex(&v, &vb);

  // top right
  v.x += 1;                        v.s += 1;
  add_vertex(&v, &vb);

  reuse_vertex(-3, &vb); // reuse bottom left

  reuse_vertex(-2, &vb); // reuse top right

  // bottom right
  v.z -= 1;                        v.t += 1;
  add_vertex(&v, &vb);
}

static inline void push_east(
  vertex_buffer vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x + 1;    v.nx = 1;    v.s = st.s;
  v.y = idx.y;        v.ny = 0;    v.t = st.t + 1;
  v.z = idx.z;        v.nz = 0;
  add_vertex(&v, &vb);

  // top left
  v.z += 1;                        v.t -= 1;
  add_vertex(&v, &vb);

  // top right
  v.y += 1;                        v.s += 1;
  add_vertex(&v, &vb);

  reuse_vertex(-3, &vb); // reuse bottom left

  reuse_vertex(-2, &vb); // reuse top right

  // bottom right
  v.z -= 1;                        v.t += 1;
  add_vertex(&v, &vb);
}

static inline void push_west(
  vertex_buffer vb,
  chunk_index idx,
  tcoords st
) {
  vertex v;
  // bottom left
  v.x = idx.x;        v.nx = -1;    v.s = st.s;
  v.y = idx.y + 1;    v.ny =  0;    v.t = st.t + 1;
  v.z = idx.z;        v.nz =  0;
  add_vertex(&v, &vb);

  // top left
  v.z += 1;                        v.t -= 1;
  add_vertex(&v, &vb);

  // top right
  v.y -= 1;                        v.s += 1;
  add_vertex(&v, &vb);

  reuse_vertex(-3, &vb); // reuse bottom left

  reuse_vertex(-2, &vb); // reuse top right

  // bottom right
  v.z -= 1;                        v.t += 1;
  add_vertex(&v, &vb);
}

/*************
 * Functions *
 *************/

void compile_chunk(frame *f, frame_chunk_index idx) {
  chunk *c = chunk_at(f, idx);

  // Count the number of "active" blocks (those not surrounded by solid
  // blocks). We use the cached exposure data here (call compute_exposure
  // first!).
  uint16_t opaque_count = 0;
  uint16_t translucent_count = 0;
  chunk_index cidx;
  for (cidx.x = 0; cidx.x < CHUNK_SIZE; ++cidx.x) {
    for (cidx.y = 0; cidx.y < CHUNK_SIZE; ++cidx.y) {
      for (cidx.z = 0; cidx.z < CHUNK_SIZE; ++cidx.z) {
        block here = c_get_block(c, cidx);
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
  printf("checking counts... %d  %d\n", opaque_count, translucent_count);
  if (opaque_count == 0 && translucent_count == 0) {
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
  block here = 0; // local blocks:
  block neighbor = 0;

  frame_pos base, pos;
  // chunk_index cidx; from above
  fcidx__fpos(&idx, &base);
  for (pos.x = base.x; pos.x < base.x + CHUNK_SIZE; ++pos.x) {
    for (pos.y = base.y; pos.y < base.y + CHUNK_SIZE; ++pos.y) {
      for (pos.z = base.z; pos.z < base.z + CHUNK_SIZE; ++pos.z) {
        fpos__cidx(&pos, &cidx);
        here = block_at(f, pos);
        neighbor = here;
        if (is_exposed(here) && !is_invisible(here)) {
          neighbor = block_above(f, pos);
          if (!cull_face(here, neighbor)) {
            compute_face_tc(here, BD_ORI_UP, &st);
            if (is_translucent(here)) {
              push_top(c->translucent_vertices, cidx, st);
            } else {
              push_top(c->opaque_vertices, cidx, st);
            }
          }
          neighbor = block_north(f, pos);
          if (!cull_face(here, neighbor)) {
            compute_face_tc(here, BD_ORI_NORTH, &st);
            if (is_translucent(here)) {
              push_north(c->translucent_vertices, cidx, st);
            } else {
              push_north(c->opaque_vertices, cidx, st);
            }
          }
          neighbor = block_south(f, pos);
          if (!cull_face(here, neighbor)) {
            compute_face_tc(here, BD_ORI_SOUTH, &st);
            if (is_translucent(here)) {
              push_south(c->translucent_vertices, cidx, st);
            } else {
              push_south(c->opaque_vertices, cidx, st);
            }
          }
          neighbor = block_east(f, pos);
          if (!cull_face(here, neighbor)) {
            compute_face_tc(here, BD_ORI_EAST, &st);
            if (is_translucent(here)) {
              push_east(c->translucent_vertices, cidx, st);
            } else {
              push_east(c->opaque_vertices, cidx, st);
            }
          }
          neighbor = block_west(f, pos);
          if (!cull_face(here, neighbor)) {
            compute_face_tc(here, BD_ORI_WEST, &st);
            if (is_translucent(here)) {
              push_west(c->translucent_vertices, cidx, st);
            } else {
              push_west(c->opaque_vertices, cidx, st);
            }
          }
          neighbor = block_below(f, pos);
          if (!cull_face(here, neighbor)) {
            compute_face_tc(here, BD_ORI_DOWN, &st);
            if (is_translucent(here)) {
              push_bottom(c->translucent_vertices, cidx, st);
            } else {
              push_bottom(c->opaque_vertices, cidx, st);
            }
          }
        }
      }
    }
  }
  // Compile the buffers:
  printf("compiling...\n");
  compile_buffers(&(c->opaque_vertices));
  compile_buffers(&(c->translucent_vertices));
  /*
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
  */
}
