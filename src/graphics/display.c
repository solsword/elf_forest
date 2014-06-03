// display.c
// Functions for setting up display information.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include <assert.h>
#include <stdlib.h>

#include <GL/glew.h> // glBindBuffer etc.

#include "display.h"

#include "tex/tex.h"
#include "tex/dta.h"

#include "world/blocks.h"
#include "world/world.h"
#include "util.h"

/*************
 * Constants *
 *************/

float const Z_RECONCILIATION_OFFSET = 0.0005;

/*********************
 * Private Functions *
 *********************/

// The various push_* functions add vertex/index data for faces to the given
// vertex buffer. They use local xyz coordinates to specify the position of the
// triangles that they're adding. These faces are parameterized by offsets
// relative to their default positions, which are the faces of a unit cube.
// These offsets are all subtractive: "offset" moves the face towards the
// center of the cube (or past if if > 0.5), while the four side offsets move
// the edges inwards. A zf_off index can be given which adds a tiny offset to
// avoid z-fighting (positive indices push things outside the default cube).
// Note that because normals need to be specified using min and max short
// values, we temporarily define some macros to make this easier.
#define P1 smaxof(GLshort) // +1
#define N1 sminof(GLshort) // -1
static inline void push_top_face(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st,
  float offset,
  float north, float east, float south, float west,
  int zf_off
) {
  vertex v;
  // bottom left
  v.x = idx.x + scale * west;            v.nx =  0;    v.s = st.s + west;
  v.y = idx.y + scale * south;           v.ny =  0;    v.t = st.t + 1 - south;
  v.z = idx.z + scale * (1 - offset);    v.nz = P1;
  v.z += zf_off * Z_RECONCILIATION_OFFSET;
  add_vertex(&v, vb);

  // top left
  v.y = idx.y + scale * (1 - north);                   v.t = st.t + north;
  add_vertex(&v, vb);

  // top right
  v.x = idx.x + scale * (1 - east);                    v.s = st.s + 1 - east;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.y = idx.y + scale * south;                         v.t = st.t + 1 - south;
  add_vertex(&v, vb);
}

static inline void push_bottom_face(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st,
  float offset,
  float north, float east, float south, float west,
  int zf_off
) {
  vertex v;
  // bottom left
  v.x = idx.x + scale * (1 - east);    v.nx =  0;    v.s = st.s + east;
  v.y = idx.y + scale * south;         v.ny =  0;    v.t = st.t + 1 - south;
  v.z = idx.z + offset;                v.nz = N1;
  v.z -= zf_off * Z_RECONCILIATION_OFFSET;
  add_vertex(&v, vb);

  // top left
  v.y = idx.y + scale * (1 - north);                 v.t = st.t + north;
  add_vertex(&v, vb);

  // top right
  v.x = idx.x + scale * west;                        v.s = st.s + 1 - west;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.y = idx.y + scale * south;                       v.t = st.t + 1 - south;
  add_vertex(&v, vb);
}

static inline void push_north_face(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
  vertex v;
  // bottom left
  v.x = idx.x + scale * (1 - left)  ;    v.nx =  0;    v.s = st.s + left;
  v.y = idx.y + scale * (1 - offset);    v.ny = P1;    v.t = st.t + 1 - bot;
  v.z = idx.z + scale * bot;             v.nz =  0;
  v.y += zf_off * Z_RECONCILIATION_OFFSET;
  add_vertex(&v, vb);

  // top left
  v.z = idx.z + scale * (1 - top);                     v.t = st.t + top;
  add_vertex(&v, vb);

  // top right
  v.x = idx.x + scale * right;                         v.s = st.s + 1 - right;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z = idx.z + scale * bot;                           v.t = st.t + 1 - bot;
  add_vertex(&v, vb);
}

static inline void push_south_face(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
  vertex v;
  // bottom left
  v.x = idx.x + scale * left;      v.nx =  0;    v.s = st.s + left;
  v.y = idx.y + scale * offset;    v.ny = N1;    v.t = st.t + 1 - bot;
  v.z = idx.z + scale * bot;       v.nz =  0;
  v.y -= zf_off * Z_RECONCILIATION_OFFSET;
  add_vertex(&v, vb);

  // top left
  v.z = idx.z + scale * (1 - top);               v.t = st.t + top;
  add_vertex(&v, vb);

  // top right
  v.x = idx.x + scale * (1 - right);             v.s = st.s + 1 - right;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z = idx.z + scale * bot;                     v.t = st.t + 1 - bot;
  add_vertex(&v, vb);
}

static inline void push_east_face(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
  vertex v;
  // bottom left
  v.x = idx.x + scale * (1 - offset);    v.nx = P1;    v.s = st.s + left;
  v.y = idx.y + scale * left;            v.ny =  0;    v.t = st.t + 1 - bot;
  v.z = idx.z + scale * bot;             v.nz =  0;
  v.x += zf_off * Z_RECONCILIATION_OFFSET;
  add_vertex(&v, vb);

  // top left
  v.z = idx.z + scale * (1 - top);                     v.t = st.t + top;
  add_vertex(&v, vb);

  // top right
  v.y = idx.y + scale * (1 - right);                   v.s = st.s + 1 - right;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z = idx.z + scale * bot;                           v.t = st.t + 1 - bot;
  add_vertex(&v, vb);
}

static inline void push_west_face(
  vertex_buffer *vb,
  chunk_index idx,
  ch_idx_t scale,
  tcoords st,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
  vertex v;
  // bottom left
  v.x = idx.x + scale * offset;          v.nx = N1;    v.s = st.s + left;
  v.y = idx.y + scale * (1 - left);      v.ny =  0;    v.t = st.t + 1 - bot;
  v.z = idx.z + scale * bot;             v.nz =  0;
  v.x -= zf_off * Z_RECONCILIATION_OFFSET;
  add_vertex(&v, vb);

  // top left
  v.z = idx.z + scale * (1 - top);                     v.t = st.t + top;
  add_vertex(&v, vb);

  // top right
  v.y = idx.y + scale * right;                         v.s = st.s + 1 - right;
  add_vertex(&v, vb);

  reuse_vertex(-3, vb); // reuse bottom left

  reuse_vertex(-2, vb); // reuse top right

  // bottom right
  v.z = idx.z + scale * bot;                           v.t = st.t + 1 - bot;
  add_vertex(&v, vb);
}
// Clean up our short macro definitions:
#undef P1
#undef N1

// Block geometry construction functions:

static inline void add_solid_block(
  vertex_buffer *vb,
  block b,
  block exposure,
  chunk_index idx,
  ch_idx_t step,
  int zf_off
) {
  tcoords st = { .s=0, .t=0 };
  if (exposure & BF_EXPOSED_ABOVE) {
    compute_dynamic_face_tc(b, BD_ORI_UP, &st);
    push_top_face(vb, idx, step, st, 0, 0, 0, 0, 0, zf_off);
  }
  if (exposure & BF_EXPOSED_BELOW) {
    compute_dynamic_face_tc(b, BD_ORI_DOWN, &st);
    push_bottom_face(vb, idx, step, st, 0, 0, 0, 0, 0, zf_off);
  }
  if (exposure & BF_EXPOSED_NORTH) {
    compute_dynamic_face_tc(b, BD_ORI_NORTH, &st);
    push_north_face(vb, idx, step, st, 0, 0, 0, 0, 0, zf_off);
  }
  if (exposure & BF_EXPOSED_SOUTH) {
    compute_dynamic_face_tc(b, BD_ORI_SOUTH, &st);
    push_south_face(vb, idx, step, st, 0, 0, 0, 0, 0, zf_off);
  }
  if (exposure & BF_EXPOSED_EAST) {
    compute_dynamic_face_tc(b, BD_ORI_EAST, &st);
    push_east_face(vb, idx, step, st, 0, 0, 0, 0, 0, zf_off);
  }
  if (exposure & BF_EXPOSED_WEST) {
    compute_dynamic_face_tc(b, BD_ORI_WEST, &st);
    push_west_face(vb, idx, step, st, 0, 0, 0, 0, 0, zf_off);
  }
}

static inline void add_grass_block(
  vertex_buffer *vb,
  block b,
  block exposure,
  chunk_index idx,
  ch_idx_t step,
  int zf_off
) {
  tcoords st = { .s=0, .t=0 };
  if ((exposure & BF_EXPOSED_ABOVE) && !(exposure & BF_EXPOSED_BELOW)) {
    compute_dynamic_face_tc(b, BD_ORI_UP, &st);
    push_top_face(vb, idx, step, st, 1, 0, 0, 0, 0, zf_off);
  } else if ((exposure & BF_EXPOSED_BELOW) && !(exposure & BF_EXPOSED_ABOVE)) {
    compute_dynamic_face_tc(b, BD_ORI_DOWN, &st);
    push_bottom_face(vb, idx, step, st, 1, 0, 0, 0, 0, zf_off);
  } else {
    compute_dynamic_face_tc(b, BD_ORI_OUT, &st);
    push_top_face(vb, idx, step, st, 0.2, 0.2, 0.2, 0.2, 0.2, zf_off);
    push_bottom_face(vb, idx, step, st, 0.2, 0.2, 0.2, 0.2, 0.2,zf_off);
    push_north_face(vb, idx, step, st, 0.2, 0.2, 0.2, 0.2, 0.2, zf_off);
    push_south_face(vb, idx, step, st, 0.2, 0.2, 0.2, 0.2, 0.2, zf_off);
    push_east_face(vb, idx, step, st, 0.2, 0.2, 0.2, 0.2, 0.2, zf_off);
    push_west_face(vb, idx, step, st, 0.2, 0.2, 0.2, 0.2, 0.2, zf_off);
  }
}

/*************
 * Functions *
 *************/

void compile_chunk_or_approx(chunk_or_approx *coa) {
  // Count the number of "active" cells (those not surrounded by solid
  // blocks). We use the cached exposure data here (call compute_exposure
  // first!).
  chunk *c;
  chunk_approximation *ca;
  vertex_buffer *vb;
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
        exposure = cl_get_exposure(here);
        if (
          (exposure & BF_EXPOSED_ANY)
        &&
          !(
            b_is_invisible(here->primary)
          &&
            b_is_invisible(here->secondary)
          )
        ) {
          total += 1;
          counts[block_layer(here->primary)] += 1;
          counts[block_layer(here->secondary)] += 1;
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
        if (!b_is_invisible(here->primary)) {
          ensure_mapped(here->primary);
          geom = bi_geom(here->primary);
          vb = &((*layers)[block_layer(here->primary)]);
          if (geom == BI_GEOM_SOLID || geom == BI_GEOM_LIQUID) {
            add_solid_block(vb, here->primary, exposure, idx, step, 0);
          } else if (geom == BI_GEOM_GRASS) {
            add_grass_block(vb, here->primary, exposure, idx, step, 2);
          } else { // fall-back case is solid geometry:
            add_solid_block(vb, here->primary, exposure, idx, step, 0);
          }
        }
        if (!b_is_invisible(here->secondary)) {
          ensure_mapped(here->secondary);
          geom = bi_geom(here->secondary);
          vb = &((*layers)[block_layer(here->secondary)]);
          if (
            geom == BI_GEOM_SOLID
          || geom == BI_GEOM_VINE
          || geom == BI_GEOM_ROOT
          ) {
            add_solid_block(vb, here->secondary, exposure, idx, step, 1);
          } // else don't draw the secondary
#ifdef DEBUG
          else {
            printf(
              "Undrawn secondary: %s ~ %d\n",
              BLOCK_NAMES[b_id(here->secondary)],
              geom
            );
          }
#endif
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
