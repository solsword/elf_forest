// display.c
// Functions for setting up display information.

#include <assert.h>
#include <stdlib.h>
// DEBUG
#include <stdio.h>

#include <GLee.h> // glBindBuffer etc.

#include "blocks.h"
#include "tex.h"
#include "display.h"

/*********************
 * Private Functions *
 *********************/

static inline block cull_face(block here, block neighbor, block at_edge) {
  return (
    at_edge
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
// the given data/indices arrays. They manipulate the di, ei, and ii indices
// to keep track of where in the array they're writing (and what index values
// to write, in the case of ei). They use local xyz coordinates to specify the
// position of the triangles that they're adding.
static inline void push_top(
  GLshort *data, GLuint *indices,
  int *di, int *ei, int *ii,
  GLshort lx, GLshort ly, GLshort lz,
  tcoords *st
) {
  // P1
  data[(*di) + 0] = lx; // x
  data[(*di) + 1] = ly; // y
  data[(*di) + 2] = lz + 1; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 0; // ny
  data[(*di) + 5] = 1; // nz

  data[(*di) + 6] = st->s; // s
  data[(*di) + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;

  // P2
  data[(*di) + 0] = lx; // x
  data[(*di) + 1] = ly + 1; // y
  data[(*di) + 2] = lz + 1; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 0; // ny
  data[(*di) + 5] = 1; // nz

  data[(*di) + 6] = st->s; // s
  data[(*di) + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;

  // P3
  data[(*di) + 0] = lx + 1; // x
  data[(*di) + 1] = ly + 1; // y
  data[(*di) + 2] = lz + 1; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 0; // ny
  data[(*di) + 5] = 1; // nz

  data[(*di) + 6] = st->s + 1; // s
  data[(*di) + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;

  // Reuse point P1
  indices[*ii] = (*ei) - 3;

  *ii = *ii + 1;

  // Reuse point P3
  indices[*ii] = (*ei) - 1;

  *ii = *ii + 1;

  // P4
  data[(*di) + 0] = lx + 1; // x
  data[(*di) + 1] = ly; // y
  data[(*di) + 2] = lz + 1; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 0; // ny
  data[(*di) + 5] = 1; // nz

  data[(*di) + 6] = st->s + 1; // s
  data[(*di) + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;
}

static inline void push_bottom(
  GLshort *data, GLuint *indices,
  int *di, int *ei, int *ii,
  uint16_t lx, uint16_t ly, uint16_t lz,
  tcoords *st
) {
  // P1
  data[(*di) + 0] = lx; // x
  data[(*di) + 1] = ly; // y
  data[(*di) + 2] = lz; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 0; // ny
  data[(*di) + 5] = -1; // nz

  data[(*di) + 6] = st->s; // s
  data[(*di) + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;

  // P2
  data[(*di) + 0] = lx; // x
  data[(*di) + 1] = ly + 1; // y
  data[(*di) + 2] = lz; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 0; // ny
  data[(*di) + 5] = -1; // nz

  data[(*di) + 6] = st->s; // s
  data[(*di) + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;

  // P3
  data[(*di) + 0] = lx + 1; // x
  data[(*di) + 1] = ly + 1; // y
  data[(*di) + 2] = lz; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 0; // ny
  data[(*di) + 5] = -1; // nz

  data[(*di) + 6] = st->s + 1; // s
  data[(*di) + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;

  // Reuse point P1
  indices[*ii] = (*ei) - 3;

  *ii = *ii + 1;

  // Reuse point P3
  indices[*ii] = (*ei) - 1;

  *ii = *ii + 1;

  // P4
  data[(*di) + 0] = lx + 1; // x
  data[(*di) + 1] = ly; // y
  data[(*di) + 2] = lz; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 0; // ny
  data[(*di) + 5] = -1; // nz

  data[(*di) + 6] = st->s + 1; // s
  data[(*di) + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;
}

static inline void push_north(
  GLshort *data, GLuint *indices,
  int *di, int *ei, int *ii,
  uint16_t lx, uint16_t ly, uint16_t lz,
  tcoords *st
) {
  // P1
  data[(*di) + 0] = lx; // x
  data[(*di) + 1] = ly + 1; // y
  data[(*di) + 2] = lz; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 1; // ny
  data[(*di) + 5] = 0; // nz

  data[(*di) + 6] = st->s + 1; // s
  data[(*di) + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;

  // P2
  data[(*di) + 0] = lx; // x
  data[(*di) + 1] = ly + 1; // y
  data[(*di) + 2] = lz + 1; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 1; // ny
  data[(*di) + 5] = 0; // nz

  data[(*di) + 6] = st->s + 1; // s
  data[(*di) + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;

  // P3
  data[(*di) + 0] = lx + 1; // x
  data[(*di) + 1] = ly + 1; // y
  data[(*di) + 2] = lz + 1; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 1; // ny
  data[(*di) + 5] = 0; // nz

  data[(*di) + 6] = st->s; // s
  data[(*di) + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;

  // Reuse point P1
  indices[*ii] = (*ei) - 3;

  *ii = *ii + 1;

  // Reuse point P3
  indices[*ii] = (*ei) - 1;

  *ii = *ii + 1;

  // P4
  data[(*di) + 0] = lx + 1; // x
  data[(*di) + 1] = ly + 1; // y
  data[(*di) + 2] = lz; // z

  data[(*di) + 3] = 0; // nx
  data[(*di) + 4] = 1; // ny
  data[(*di) + 5] = 0; // nz

  data[(*di) + 6] = st->s; // s
  data[(*di) + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = (*di) + VERTEX_STRIDE;
  *ei = (*ei) + 1;
  *ii = (*ii) + 1;
}

static inline void push_south(
  GLshort *data, GLuint *indices,
  int *di, int *ei, int *ii,
  uint16_t lx, uint16_t ly, uint16_t lz,
  tcoords *st
) {
  // P1
  data[*di + 0] = lx; // x
  data[*di + 1] = ly; // y
  data[*di + 2] = lz; // z

  data[*di + 3] = 0; // nx
  data[*di + 4] = -1; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s; // s
  data[*di + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;

  // P2
  data[*di + 0] = lx; // x
  data[*di + 1] = ly; // y
  data[*di + 2] = lz + 1; // z

  data[*di + 3] = 0; // nx
  data[*di + 4] = -1; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s; // s
  data[*di + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;

  // P3
  data[*di + 0] = lx + 1; // x
  data[*di + 1] = ly; // y
  data[*di + 2] = lz + 1; // z

  data[*di + 3] = 0; // nx
  data[*di + 4] = -1; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s + 1; // s
  data[*di + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;

  // Reuse point P1
  indices[*ii] = (*ei) - 3;

  *ii = *ii + 1;

  // Reuse point P3
  indices[*ii] = (*ei) - 1;

  *ii = *ii + 1;

  // P4
  data[*di + 0] = lx + 1; // x
  data[*di + 1] = ly; // y
  data[*di + 2] = lz; // z

  data[*di + 3] = 0; // nx
  data[*di + 4] = -1; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s + 1; // s
  data[*di + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;
}

static inline void push_east(
  GLshort *data, GLuint *indices,
  int *di, int *ei, int *ii,
  uint16_t lx, uint16_t ly, uint16_t lz,
  tcoords *st
) {
  // P1
  data[*di + 0] = lx + 1; // x
  data[*di + 1] = ly; // y
  data[*di + 2] = lz; // z

  data[*di + 3] = 1; // nx
  data[*di + 4] = 0; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s; // s
  data[*di + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;

  // P2
  data[*di + 0] = lx + 1; // x
  data[*di + 1] = ly; // y
  data[*di + 2] = lz + 1; // z

  data[*di + 3] = 1; // nx
  data[*di + 4] = 0; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s; // s
  data[*di + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;

  // P3
  data[*di + 0] = lx + 1; // x
  data[*di + 1] = ly + 1; // y
  data[*di + 2] = lz + 1; // z

  data[*di + 3] = 1; // nx
  data[*di + 4] = 0; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s + 1; // s
  data[*di + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;

  // Reuse point P1
  indices[*ii] = (*ei) - 3;

  *ii = *ii + 1;

  // Reuse point P3
  indices[*ii] = (*ei) - 1;

  *ii = *ii + 1;

  // P4
  data[*di + 0] = lx + 1; // x
  data[*di + 1] = ly + 1; // y
  data[*di + 2] = lz; // z

  data[*di + 3] = 1; // nx
  data[*di + 4] = 0; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s + 1; // s
  data[*di + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;
}

static inline void push_west(
  GLshort *data, GLuint *indices,
  int *di, int *ei, int *ii,
  uint16_t lx, uint16_t ly, uint16_t lz,
  tcoords *st
) {
  // P1
  data[*di + 0] = lx; // x
  data[*di + 1] = ly + 1; // y
  data[*di + 2] = lz; // z

  data[*di + 3] = -1; // nx
  data[*di + 4] = 0; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s; // s
  data[*di + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;

  // P2
  data[*di + 0] = lx; // x
  data[*di + 1] = ly + 1; // y
  data[*di + 2] = lz + 1; // z

  data[*di + 3] = -1; // nx
  data[*di + 4] = 0; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s; // s
  data[*di + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;

  // P3
  data[*di + 0] = lx; // x
  data[*di + 1] = ly; // y
  data[*di + 2] = lz + 1; // z

  data[*di + 3] = -1; // nx
  data[*di + 4] = 0; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s + 1; // s
  data[*di + 7] = st->t; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;

  // Reuse point P1
  indices[*ii] = (*ei) - 3;

  *ii = *ii + 1;

  // Reuse point P3
  indices[*ii] = (*ei) - 1;

  *ii = *ii + 1;

  // P4
  data[*di + 0] = lx; // x
  data[*di + 1] = ly; // y
  data[*di + 2] = lz; // z

  data[*di + 3] = -1; // nx
  data[*di + 4] = 0; // ny
  data[*di + 5] = 0; // nz

  data[*di + 6] = st->s + 1; // s
  data[*di + 7] = st->t + 1; // t

  indices[*ii] = *ei;

  *di = *di + VERTEX_STRIDE;
  *ei = *ei + 1;
  *ii = *ii + 1;
}

/*************
 * Functions *
 *************/

void compile_chunk(frame *f, uint16_t cx, uint16_t cy, uint16_t cz) {
  chunk *c = chunk_at(f, cx, cy, cz);
  // (Re)generate buffer objects for the vertices and indices:
  if (c->opaque_vertex_bo != 0) {
    glDeleteBuffers(1, &(c->opaque_vertex_bo));
    c->opaque_vertex_bo = 0;
  }
  if (c->opaque_index_bo != 0) {
    glDeleteBuffers(1, &(c->opaque_index_bo));
    c->opaque_index_bo = 0;
  }
  if (c->translucent_vertex_bo != 0) {
    glDeleteBuffers(1, &(c->translucent_vertex_bo));
    c->translucent_vertex_bo = 0;
  }
  if (c->translucent_index_bo != 0) {
    glDeleteBuffers(1, &(c->translucent_index_bo));
    c->translucent_index_bo = 0;
  }

  // Count the number of "active" blocks (those not surrounded by solid blocks):
  // We use the cached exposure data here (call compute_exposure first!).
  uint16_t opaque_count = 0;
  uint16_t translucent_count = 0;
  GLshort lx, ly, lz;
  for (lx = 0; lx < CHUNK_SIZE; ++lx) {
    for (ly = 0; ly < CHUNK_SIZE; ++ly) {
      for (lz = 0; lz < CHUNK_SIZE; ++lz) {
        block here = c_get_block(c, lx, ly, lz);
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
  if (opaque_count > 0) {
    glGenBuffers(1, &(c->opaque_vertex_bo));
    glGenBuffers(1, &(c->opaque_index_bo));
  }
  if (translucent_count > 0) {
    glGenBuffers(1, &(c->translucent_vertex_bo));
    glGenBuffers(1, &(c->translucent_index_bo));
  }
  if (opaque_count == 0 && translucent_count == 0) {
    return;
  }
  // Coordinates, normals and texture coordinates make 8 entries/vertex.
  // 8 entries/vertex * 4 vertices/face * 6 triangles/cube =
  //   192 entries/cube
  // 2 bytes/entry = 384 bytes/cube
  // There are also indices, which are 12 * 3 = 36 indices per cube.
  // 4 bytes/index = 144 bytes/cube
  //   384+144 = 528 bytes/cube
  // For a frame where an entire plane is visible, there are a total of
  //   32 * 32 * 16 * 16 = 262144 cubes
  // This makes a total of 528*262144 = 138412032 bytes, or 144 MB across all
  // active vertex arrays. Of course, hills and valleys might increase this,
  // but culling should usually decrease it, and it's the right order of
  // magnitude.
  
  // TODO: Use a single chunk staging area instead of re-mallocing each time?
  int opaque_data_size = opaque_count*192*sizeof(GLshort);
  int opaque_indices_size = opaque_count*36*sizeof(GLuint);
  int translucent_data_size = translucent_count*192*sizeof(GLshort);
  int translucent_indices_size = translucent_count*36*sizeof(GLuint);
  GLshort *opaque_data = (GLshort*) malloc(opaque_data_size);
  GLuint *opaque_indices = (GLuint*) malloc(opaque_indices_size);
  GLshort *translucent_data = (GLshort*) malloc(translucent_data_size);
  GLuint *translucent_indices = (GLuint*) malloc(translucent_indices_size);
  // Push cube data and indices into our arrays:
  int o_di = 0; // index within the opaque data array
  int o_ei = 0; // opaque element index
  int o_ii = 0; // index within the opaque index array
  int t_di = 0; // translucent versions
  int t_ei = 0;
  int t_ii = 0;
  tcoords st; // texture coordinates
  st.s = 0;
  st.t = 0;
  block here = 0;
  block neighbor = 0;
  block at_edge = 0;
  int fx = (cx << CHUNK_BITS) - HALF_FRAME;
  int fy = (cy << CHUNK_BITS) - HALF_FRAME;
  int fz = (cz << CHUNK_BITS) - HALF_FRAME;
  int x, y, z;
  for (x = fx; x < fx + CHUNK_SIZE; ++x) {
    for (y = fy; y < fy + CHUNK_SIZE; ++y) {
      for (z = fz; z < fz + CHUNK_SIZE; ++z) {
        lx = (x + HALF_FRAME) % CHUNK_SIZE;
        ly = (y + HALF_FRAME) % CHUNK_SIZE;
        lz = (z + HALF_FRAME) % CHUNK_SIZE;
        here = block_at(f, x, y, z);
        neighbor = here;
        if (is_exposed(here) && !is_invisible(here)) {
          neighbor = block_at(f, x, y, z + 1);
          at_edge = z + 1 >= HALF_FRAME;
          if (!cull_face(here, neighbor, at_edge)) {
            compute_face_tc(here, BD_ORI_UP, &st);
            if (is_translucent(here)) {
              push_top(
                translucent_data, translucent_indices,
                &t_di, &t_ei, &t_ii,
                lx, ly, lz,
                &st
              );
            } else {
              push_top(
                opaque_data, opaque_indices,
                &o_di, &o_ei, &o_ii,
                lx, ly, lz,
                &st
              );
            }
          }
          neighbor = block_at(f, x, y + 1, z);
          at_edge = y + 1 >= HALF_FRAME;
          if (!cull_face(here, neighbor, at_edge)) {
            compute_face_tc(here, BD_ORI_NORTH, &st);
            if (is_translucent(here)) {
              push_north(
                translucent_data, translucent_indices,
                &t_di, &t_ei, &t_ii,
                lx, ly, lz,
                &st
              );
            } else {
              push_north(
                opaque_data, opaque_indices,
                &o_di, &o_ei, &o_ii,
                lx, ly, lz,
                &st
              );
            }
          }
          neighbor = block_at(f, x, y - 1, z);
          at_edge = y - 1 < -HALF_FRAME;
          if (!cull_face(here, neighbor, at_edge)) {
            compute_face_tc(here, BD_ORI_SOUTH, &st);
            if (is_translucent(here)) {
              push_south(
                translucent_data, translucent_indices,
                &t_di, &t_ei, &t_ii,
                lx, ly, lz,
                &st
              );
            } else {
              push_south(
                opaque_data, opaque_indices,
                &o_di, &o_ei, &o_ii,
                lx, ly, lz,
                &st
              );
            }
          }
          neighbor = block_at(f, x + 1, y, z);
          at_edge = x + 1 >= HALF_FRAME;
          if (!cull_face(here, neighbor, at_edge)) {
            compute_face_tc(here, BD_ORI_EAST, &st);
            if (is_translucent(here)) {
              push_east(
                translucent_data, translucent_indices,
                &t_di, &t_ei, &t_ii,
                lx, ly, lz,
                &st
              );
            } else {
              push_east(
                opaque_data, opaque_indices,
                &o_di, &o_ei, &o_ii,
                lx, ly, lz,
                &st
              );
            }
          }
          neighbor = block_at(f, x - 1, y, z);
          at_edge = x - 1 < -HALF_FRAME;
          if (!cull_face(here, neighbor, at_edge)) {
            compute_face_tc(here, BD_ORI_WEST, &st);
            if (is_translucent(here)) {
              push_west(
                translucent_data, translucent_indices,
                &t_di, &t_ei, &t_ii,
                lx, ly, lz,
                &st
              );
            } else {
              push_west(
                opaque_data, opaque_indices,
                &o_di, &o_ei, &o_ii,
                lx, ly, lz,
                &st
              );
            }
          }
          neighbor = block_at(f, x, y, z - 1);
          at_edge = z - 1 < -HALF_FRAME;
          if (!cull_face(here, neighbor, at_edge)) {
            compute_face_tc(here, BD_ORI_DOWN, &st);
            if (is_translucent(here)) {
              push_bottom(
                translucent_data, translucent_indices,
                &t_di, &t_ei, &t_ii,
                lx, ly, lz,
                &st
              );
            } else {
              push_bottom(
                opaque_data, opaque_indices,
                &o_di, &o_ei, &o_ii,
                lx, ly, lz,
                &st
              );
            }
          }
        }
      }
    }
  }
  // Bind our buffers, copy in the data, and unbind the buffers:
  glBindBuffer( GL_ARRAY_BUFFER, c->opaque_vertex_bo );
  glBufferData(
    GL_ARRAY_BUFFER,
    opaque_data_size,
    opaque_data,
    GL_STATIC_DRAW
  );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, c->opaque_index_bo );
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    opaque_indices_size,
    opaque_indices,
    GL_STATIC_DRAW
  );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ARRAY_BUFFER, c->translucent_vertex_bo );
  glBufferData(
    GL_ARRAY_BUFFER,
    translucent_data_size,
    translucent_data,
    GL_STATIC_DRAW
  );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, c->translucent_index_bo );
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER,
    translucent_indices_size,
    translucent_indices,
    GL_STATIC_DRAW
  );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  // We've copied the data over to OpenGL, so we're done with it here:
  free(opaque_data);
  free(opaque_indices);
  free(translucent_data);
  free(translucent_indices);

  // Record our vertex count:
  c->o_vcount = o_ii;
  c->t_vcount = t_ii;
}

void cleanup_frame(frame *f) {
  uint16_t cx, cy, cz;
  for (cx = 0; cx < FRAME_SIZE; ++cx) {
    for (cy = 0; cy < FRAME_SIZE; ++cy) {
      for (cz = 0; cz < FRAME_SIZE; ++cz) {
        glDeleteBuffers(1, &(chunk_at(f, cx, cy, cz)->opaque_vertex_bo));
        glDeleteBuffers(1, &(chunk_at(f, cx, cy, cz)->opaque_index_bo));
        glDeleteBuffers(1, &(chunk_at(f, cx, cy, cz)->translucent_vertex_bo));
        glDeleteBuffers(1, &(chunk_at(f, cx, cy, cz)->translucent_index_bo));
      }
    }
  }
}
