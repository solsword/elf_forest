// display.c
// Functions for setting up display information.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h> // glBindBuffer etc.

#include "display.h"

#include "tex/tex.h"
#include "tex/dta.h"

#include "data/data.h"
#include "world/blocks.h"
#include "world/world.h"
#include "util.h"

/*************
 * Constants *
 *************/

float const Z_RECONCILIATION_OFFSET = 0.0005;

float const DF_LOWER = 0.5 - M_SQRT2/4.0;
float const DF_UPPER = 0.5 + M_SQRT2/4.0;

uint8_t const BASE_LIGHT_LEVEL = 80;
uint8_t const AMBIENT_LIGHT_STRENGTH = 30;

/***********
 * Globals *
 ***********/

#ifdef DEBUG
size_t VERTEX_COUNT = 0;
size_t INDEX_COUNT = 0;
#endif

/*********************
 * Private Functions *
 *********************/

// The various push_* functions add vertex/index data for faces to the given
// vertex buffer. They use local xyz coordinates to specify the position of the
// triangles that they're adding. These faces are parameterized by offsets
// relative to their default positions, which are mostly the faces of a unit
// cube. These offsets are all subtractive: "offset" moves the face towards the
// center of the cube (or past if if > 0.5), while the four side offsets move
// the edges inwards. A zf_off index can be given which adds a tiny offset to
// avoid z-fighting (positive indices push things outside the default cube).
// Note that because normals need to be specified using min and max short
// values, we temporarily define some macros to make this easier.
#define P1 smaxof(GLshort) // +1
#define N1 sminof(GLshort) // -1
#define PRT ((GLshort) M_SQRT1_2 * smaxof(GLshort)) // + sqrt(1/2)
#define NRT ((GLshort) M_SQRT1_2 * sminof(GLshort)) // - sqrt(1/2)
static inline void push_top_face(
  vertex_buffer *vb,
  block_index idx,
  int scale,
  tcoords st,
  face_illumination light,
  float offset,
  float north, float east, float south, float west,
  int zf_off
) {
#ifdef DEBUG
  VERTEX_COUNT += 4;
  INDEX_COUNT += 6;
#endif
  vertex v = { .g = 0, .b = 0 };
  // bottom left
  v.x = idx.xyz.x + scale * west;           v.nx =  0;   v.s = st.s + west;
  v.y = idx.xyz.y + scale * south;          v.ny =  0;   v.t = st.t + 1 - south;
  v.z = idx.xyz.z + scale * (1 - offset);   v.nz = P1;
  v.r = vertex_light(light, 0);
  v.z += zf_off * Z_RECONCILIATION_OFFSET;
  vb_add_vertex(&v, vb);

  // top left
  v.y = idx.xyz.y + scale * (1 - north);                 v.t = st.t + north;
  v.r = vertex_light(light, 1);
  vb_add_vertex(&v, vb);

  // top right
  v.x = idx.xyz.x + scale * (1 - east);                  v.s = st.s + 1 - east;
  v.r = vertex_light(light, 2);
  vb_add_vertex(&v, vb);

  vb_reuse_vertex(2, vb); // reuse bottom left

  vb_reuse_vertex(1, vb); // reuse top right

  // bottom right
  v.y = idx.xyz.y + scale * south;                       v.t = st.t + 1 - south;
  v.r = vertex_light(light, 3);
  vb_add_vertex(&v, vb);
}

static inline void push_bottom_face(
  vertex_buffer *vb,
  block_index idx,
  int scale,
  tcoords st,
  face_illumination light,
  float offset,
  float north, float east, float south, float west,
  int zf_off
) {
#ifdef DEBUG
  VERTEX_COUNT += 4;
  INDEX_COUNT += 6;
#endif
  vertex v = { .g = 0, .b = 0 };
  // bottom left
  v.x = idx.xyz.x + scale * (1 - east);   v.nx =  0;   v.s = st.s + east;
  v.y = idx.xyz.y + scale * south;        v.ny =  0;   v.t = st.t + 1 - south;
  v.z = idx.xyz.z + offset;               v.nz = N1;
  v.r = vertex_light(light, 0);
  v.z -= zf_off * Z_RECONCILIATION_OFFSET;
  vb_add_vertex(&v, vb);

  // top left
  v.y = idx.xyz.y + scale * (1 - north);               v.t = st.t + north;
  v.r = vertex_light(light, 1);
  vb_add_vertex(&v, vb);

  // top right
  v.x = idx.xyz.x + scale * west;                      v.s = st.s + 1 - west;
  v.r = vertex_light(light, 2);
  vb_add_vertex(&v, vb);

  vb_reuse_vertex(2, vb); // reuse bottom left

  vb_reuse_vertex(1, vb); // reuse top right

  // bottom right
  v.y = idx.xyz.y + scale * south;                     v.t = st.t + 1 - south;
  v.r = vertex_light(light, 3);
  vb_add_vertex(&v, vb);
}

static inline void push_north_face(
  vertex_buffer *vb,
  block_index idx,
  int scale,
  tcoords st,
  face_illumination light,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
#ifdef DEBUG
  VERTEX_COUNT += 4;
  INDEX_COUNT += 6;
#endif
  vertex v = { .g = 0, .b = 0 };
  // bottom left
  v.x = idx.xyz.x + scale * (1 - left)  ;   v.nx =  0;   v.s = st.s + left;
  v.y = idx.xyz.y + scale * (1 - offset);   v.ny = P1;   v.t = st.t + 1 - bot;
  v.z = idx.xyz.z + scale * bot;            v.nz =  0;
  v.r = vertex_light(light, 0);
  v.y += zf_off * Z_RECONCILIATION_OFFSET;
  vb_add_vertex(&v, vb);

  // top left
  v.z = idx.xyz.z + scale * (1 - top);                   v.t = st.t + top;
  v.r = vertex_light(light, 1);
  vb_add_vertex(&v, vb);

  // top right
  v.x = idx.xyz.x + scale * right;                       v.s = st.s + 1 - right;
  v.r = vertex_light(light, 2);
  vb_add_vertex(&v, vb);

  vb_reuse_vertex(2, vb); // reuse bottom left

  vb_reuse_vertex(1, vb); // reuse top right

  // bottom right
  v.z = idx.xyz.z + scale * bot;                         v.t = st.t + 1 - bot;
  v.r = vertex_light(light, 3);
  vb_add_vertex(&v, vb);
}

static inline void push_south_face(
  vertex_buffer *vb,
  block_index idx,
  int scale,
  tcoords st,
  face_illumination light,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
#ifdef DEBUG
  VERTEX_COUNT += 4;
  INDEX_COUNT += 6;
#endif
  vertex v = { .g = 0, .b = 0 };
  // bottom left
  v.x = idx.xyz.x + scale * left;      v.nx =  0;    v.s = st.s + left;
  v.y = idx.xyz.y + scale * offset;    v.ny = N1;    v.t = st.t + 1 - bot;
  v.z = idx.xyz.z + scale * bot;       v.nz =  0;
  v.r = vertex_light(light, 0);
  v.y -= zf_off * Z_RECONCILIATION_OFFSET;
  vb_add_vertex(&v, vb);

  // top left
  v.z = idx.xyz.z + scale * (1 - top);               v.t = st.t + top;
  v.r = vertex_light(light, 1);
  vb_add_vertex(&v, vb);

  // top right
  v.x = idx.xyz.x + scale * (1 - right);             v.s = st.s + 1 - right;
  v.r = vertex_light(light, 2);
  vb_add_vertex(&v, vb);

  vb_reuse_vertex(2, vb); // reuse bottom left

  vb_reuse_vertex(1, vb); // reuse top right

  // bottom right
  v.z = idx.xyz.z + scale * bot;                     v.t = st.t + 1 - bot;
  v.r = vertex_light(light, 3);
  vb_add_vertex(&v, vb);
}

static inline void push_east_face(
  vertex_buffer *vb,
  block_index idx,
  int scale,
  tcoords st,
  face_illumination light,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
#ifdef DEBUG
  VERTEX_COUNT += 4;
  INDEX_COUNT += 6;
#endif
  vertex v = { .g = 0, .b = 0 };
  // bottom left
  v.x = idx.xyz.x + scale * (1 - offset);   v.nx = P1;   v.s = st.s + left;
  v.y = idx.xyz.y + scale * left;           v.ny =  0;   v.t = st.t + 1 - bot;
  v.z = idx.xyz.z + scale * bot;            v.nz =  0;
  v.r = vertex_light(light, 0);
  v.x += zf_off * Z_RECONCILIATION_OFFSET;
  vb_add_vertex(&v, vb);

  // top left
  v.z = idx.xyz.z + scale * (1 - top);                   v.t = st.t + top;
  v.r = vertex_light(light, 1);
  vb_add_vertex(&v, vb);

  // top right
  v.y = idx.xyz.y + scale * (1 - right);                 v.s = st.s + 1 - right;
  v.r = vertex_light(light, 2);
  vb_add_vertex(&v, vb);

  vb_reuse_vertex(2, vb); // reuse bottom left

  vb_reuse_vertex(1, vb); // reuse top right

  // bottom right
  v.z = idx.xyz.z + scale * bot;                         v.t = st.t + 1 - bot;
  v.r = vertex_light(light, 3);
  vb_add_vertex(&v, vb);
}

static inline void push_west_face(
  vertex_buffer *vb,
  block_index idx,
  int scale,
  tcoords st,
  face_illumination light,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
#ifdef DEBUG
  VERTEX_COUNT += 4;
  INDEX_COUNT += 6;
#endif
  vertex v = { .g = 0, .b = 0 };
  // bottom left
  v.x = idx.xyz.x + scale * offset;         v.nx = N1;   v.s = st.s + left;
  v.y = idx.xyz.y + scale * (1 - left);     v.ny =  0;   v.t = st.t + 1 - bot;
  v.z = idx.xyz.z + scale * bot;            v.nz =  0;
  v.r = vertex_light(light, 0);
  v.x -= zf_off * Z_RECONCILIATION_OFFSET;
  vb_add_vertex(&v, vb);

  // top left
  v.z = idx.xyz.z + scale * (1 - top);                   v.t = st.t + top;
  v.r = vertex_light(light, 1);
  vb_add_vertex(&v, vb);

  // top right
  v.y = idx.xyz.y + scale * right;                       v.s = st.s + 1 - right;
  v.r = vertex_light(light, 2);
  vb_add_vertex(&v, vb);

  vb_reuse_vertex(2, vb); // reuse bottom left

  vb_reuse_vertex(1, vb); // reuse top right

  // bottom right
  v.z = idx.xyz.z + scale * bot;                         v.t = st.t + 1 - bot;
  v.r = vertex_light(light, 3);
  vb_add_vertex(&v, vb);
}

static inline void push_ne_sw_face(
  vertex_buffer *vb,
  block_index idx,
  int scale,
  tcoords st,
  face_illumination light,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
#ifdef DEBUG
  VERTEX_COUNT += 4;
  INDEX_COUNT += 6;
#endif
  vertex v = { .g = 0, .b = 0 };
  // bottom left
  v.x = idx.xyz.x + scale * (DF_UPPER - left * M_SQRT1_2 + offset * M_SQRT1_2);
  v.y = idx.xyz.y + scale * (DF_UPPER - left * M_SQRT1_2 - offset * M_SQRT1_2);
  v.z = idx.xyz.z + scale * bot;
  v.x -= zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.y += zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.nx = NRT;
  v.ny = PRT;
  v.nz =  0;
  v.s = st.s + left;
  v.t = st.t + 1 - bot;

  v.r = vertex_light(light, 0);
  vb_add_vertex(&v, vb);

  // top left
  v.z = idx.xyz.z + scale * (1 - top);
  v.t = st.t + top;
  v.r = vertex_light(light, 1);
  vb_add_vertex(&v, vb);

  // top right
  v.x = idx.xyz.x + scale * (DF_LOWER + right * M_SQRT1_2 + offset * M_SQRT1_2);
  v.y = idx.xyz.y + scale * (DF_LOWER + right * M_SQRT1_2 - offset * M_SQRT1_2);
  v.x -= zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.y += zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.s = st.s + 1 - right;
  v.r = vertex_light(light, 2);
  vb_add_vertex(&v, vb);

  vb_reuse_vertex(2, vb); // reuse bottom left

  vb_reuse_vertex(1, vb); // reuse top right

  // bottom right
  v.z = idx.xyz.z + scale * bot;
  v.t = st.t + 1 - bot;
  v.r = vertex_light(light, 3);
  vb_add_vertex(&v, vb);
}

static inline void push_sw_ne_face(
  vertex_buffer *vb,
  block_index idx,
  int scale,
  tcoords st,
  face_illumination light,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
#ifdef DEBUG
  VERTEX_COUNT += 4;
  INDEX_COUNT += 6;
#endif
  vertex v = { .g = 0, .b = 0 };
  // bottom left
  v.x = idx.xyz.x + scale * (DF_LOWER + left * M_SQRT1_2 - offset * M_SQRT1_2);
  v.y = idx.xyz.y + scale * (DF_LOWER + left * M_SQRT1_2 + offset * M_SQRT1_2);
  v.z = idx.xyz.z + scale * bot;
  v.x += zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.y -= zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.nx = PRT;
  v.ny = NRT;
  v.nz =  0;
  v.s = st.s + left;
  v.t = st.t + 1 - bot;

  v.r = vertex_light(light, 0);
  vb_add_vertex(&v, vb);

  // top left
  v.z = idx.xyz.z + scale * (1 - top);
  v.t = st.t + top;
  v.r = vertex_light(light, 1);
  vb_add_vertex(&v, vb);

  // top right
  v.x = idx.xyz.x + scale * (DF_UPPER - right * M_SQRT1_2 - offset * M_SQRT1_2);
  v.y = idx.xyz.y + scale * (DF_UPPER - right * M_SQRT1_2 + offset * M_SQRT1_2);
  v.x += zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.y -= zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.s = st.s + 1 - right;
  v.r = vertex_light(light, 2);
  vb_add_vertex(&v, vb);

  vb_reuse_vertex(2, vb); // reuse bottom left

  vb_reuse_vertex(1, vb); // reuse top right

  // bottom right
  v.z = idx.xyz.z + scale * bot;
  v.t = st.t + 1 - bot;
  v.r = vertex_light(light, 3);
  vb_add_vertex(&v, vb);
}

static inline void push_nw_se_face(
  vertex_buffer *vb,
  block_index idx,
  int scale,
  tcoords st,
  face_illumination light,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
#ifdef DEBUG
  VERTEX_COUNT += 4;
  INDEX_COUNT += 6;
#endif
  vertex v = { .g = 0, .b = 0 };
  // bottom left
  v.x = idx.xyz.x + scale * (DF_LOWER + left * M_SQRT1_2 + offset * M_SQRT1_2);
  v.y = idx.xyz.y + scale * (DF_UPPER - left * M_SQRT1_2 + offset * M_SQRT1_2);
  v.z = idx.xyz.z + scale * bot;
  v.x -= zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.y -= zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.nx = NRT;
  v.ny = NRT;
  v.nz =  0;
  v.s = st.s + left;
  v.t = st.t + 1 - bot;

  v.r = vertex_light(light, 0);
  vb_add_vertex(&v, vb);

  // top left
  v.z = idx.xyz.z + scale * (1 - top);
  v.t = st.t + top;
  v.r = vertex_light(light, 1);
  vb_add_vertex(&v, vb);

  // top right
  v.x = idx.xyz.x + scale * (DF_UPPER - right * M_SQRT1_2 + offset * M_SQRT1_2);
  v.y = idx.xyz.y + scale * (DF_LOWER + right * M_SQRT1_2 + offset * M_SQRT1_2);
  v.x -= zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.y -= zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.s = st.s + 1 - right;
  v.r = vertex_light(light, 2);
  vb_add_vertex(&v, vb);

  vb_reuse_vertex(2, vb); // reuse bottom left

  vb_reuse_vertex(1, vb); // reuse top right

  // bottom right
  v.z = idx.xyz.z + scale * bot;
  v.t = st.t + 1 - bot;
  v.r = vertex_light(light, 3);
  vb_add_vertex(&v, vb);
}

static inline void push_se_nw_face(
  vertex_buffer *vb,
  block_index idx,
  int scale,
  tcoords st,
  face_illumination light,
  float offset,
  float top, float right, float bot, float left,
  int zf_off
) {
#ifdef DEBUG
  VERTEX_COUNT += 4;
  INDEX_COUNT += 6;
#endif
  vertex v = { .g = 0, .b = 0 };
  // bottom left
  v.x = idx.xyz.x + scale * (DF_UPPER - left * M_SQRT1_2 - offset * M_SQRT1_2);
  v.y = idx.xyz.y + scale * (DF_LOWER + left * M_SQRT1_2 - offset * M_SQRT1_2);
  v.z = idx.xyz.z + scale * bot;
  v.x += zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.y += zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.nx = PRT;
  v.ny = PRT;
  v.nz =  0;
  v.s = st.s + left;
  v.t = st.t + 1 - bot;

  v.r = vertex_light(light, 0);
  vb_add_vertex(&v, vb);

  // top left
  v.z = idx.xyz.z + scale * (1 - top);
  v.t = st.t + top;
  v.r = vertex_light(light, 1);
  vb_add_vertex(&v, vb);

  // top right
  v.x = idx.xyz.x + scale * (DF_LOWER + right * M_SQRT1_2 - offset * M_SQRT1_2);
  v.y = idx.xyz.y + scale * (DF_UPPER - right * M_SQRT1_2 - offset * M_SQRT1_2);
  v.x += zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.y += zf_off * Z_RECONCILIATION_OFFSET * M_SQRT1_2;
  v.s = st.s + 1 - right;
  v.r = vertex_light(light, 2);
  vb_add_vertex(&v, vb);

  vb_reuse_vertex(2, vb); // reuse bottom left

  vb_reuse_vertex(1, vb); // reuse top right

  // bottom right
  v.z = idx.xyz.z + scale * bot;
  v.t = st.t + 1 - bot;
  v.r = vertex_light(light, 3);
  vb_add_vertex(&v, vb);
}

// TODO: More face types here?

// Clean up our short macro definitions:
#undef P1
#undef N1
#undef PRT
#undef NRT

// Block geometry construction functions:

static inline void add_solid_block(
  vertex_buffer *vb,
  block b,
  block exposure,
  cube_illumination* lighting,
  block_index idx,
  int step,
  int zf_off
) {
  tcoords st = { .s=0, .t=0 };
  face_illumination light = 0xff;
  if (exposure & BF_EXPOSED_ABOVE) {
    compute_dynamic_face_tc(b, BD_ORI_UP, &st);
    light = face_light(lighting, BD_FACE_TOP);
    push_top_face(vb, idx, step, st, light, 0, 0, 0, 0, 0, zf_off);
  }
  if (exposure & BF_EXPOSED_BELOW) {
    compute_dynamic_face_tc(b, BD_ORI_DOWN, &st);
    light = face_light(lighting, BD_FACE_BOT);
    push_bottom_face(vb, idx, step, st, light, 0, 0, 0, 0, 0, zf_off);
  }
  if (exposure & BF_EXPOSED_NORTH) {
    compute_dynamic_face_tc(b, BD_ORI_NORTH, &st);
    light = face_light(lighting, BD_FACE_FRONT);
    push_north_face(vb, idx, step, st, light, 0, 0, 0, 0, 0, zf_off);
  }
  if (exposure & BF_EXPOSED_SOUTH) {
    compute_dynamic_face_tc(b, BD_ORI_SOUTH, &st);
    light = face_light(lighting, BD_FACE_BACK);
    push_south_face(vb, idx, step, st, light, 0, 0, 0, 0, 0, zf_off);
  }
  if (exposure & BF_EXPOSED_EAST) {
    compute_dynamic_face_tc(b, BD_ORI_EAST, &st);
    light = face_light(lighting, BD_FACE_LEFT);
    push_east_face(vb, idx, step, st, light, 0, 0, 0, 0, 0, zf_off);
  }
  if (exposure & BF_EXPOSED_WEST) {
    compute_dynamic_face_tc(b, BD_ORI_WEST, &st);
    light = face_light(lighting, BD_FACE_RIGHT);
    push_west_face(vb, idx, step, st, light, 0, 0, 0, 0, 0, zf_off);
  }
}

static inline void add_grass_block(
  vertex_buffer *vb,
  block b,
  block exposure,
  cube_illumination* ext_lighting,
  cube_illumination* int_lighting,
  block_index idx,
  int step,
  int zf_off
) {
  tcoords st = { .s=0, .t=0 };
  face_illumination light = 0xff;
  if ((exposure & BF_EXPOSED_ABOVE) && !(exposure & BF_EXPOSED_BELOW)) {
    compute_dynamic_face_tc(b, BD_ORI_UP, &st);
    light = face_light(int_lighting, BD_FACE_TOP);
    push_top_face(vb, idx, step, st, light, 1, 0, 0, 0, 0, zf_off);
  } else if ((exposure & BF_EXPOSED_BELOW) && !(exposure & BF_EXPOSED_ABOVE)) {
    compute_dynamic_face_tc(b, BD_ORI_DOWN, &st);
    light = face_light(int_lighting, BD_FACE_BOT);
    push_bottom_face(vb, idx, step, st, light, 1, 0, 0, 0, 0, zf_off);
  } else {
    // TODO: Something else here...
    compute_dynamic_face_tc(b, BD_ORI_OUT, &st);
    light = face_light(ext_lighting, BD_FACE_TOP);
    push_top_face(vb, idx, step, st, light, 0.2, 0.2, 0.2, 0.2, 0.2, zf_off);
    light = face_light(ext_lighting, BD_FACE_BOT);
    push_bottom_face(vb, idx, step, st, light, 0.2, 0.2, 0.2, 0.2, 0.2,zf_off);
    light = face_light(ext_lighting, BD_FACE_FRONT);
    push_north_face(vb, idx, step, st, light, 0.2, 0.2, 0.2, 0.2, 0.2, zf_off);
    light = face_light(ext_lighting, BD_FACE_BACK);
    push_south_face(vb, idx, step, st, light, 0.2, 0.2, 0.2, 0.2, 0.2, zf_off);
    light = face_light(ext_lighting, BD_FACE_LEFT);
    push_east_face(vb, idx, step, st, light, 0.2, 0.2, 0.2, 0.2, 0.2, zf_off);
    light = face_light(ext_lighting, BD_FACE_RIGHT);
    push_west_face(vb, idx, step, st, light, 0.2, 0.2, 0.2, 0.2, 0.2, zf_off);
  }
}

static inline void add_herb_block(
  vertex_buffer *vb,
  block b,
  block exposure,
  cube_illumination* ext_lighting,
  cube_illumination* int_lighting,
  block_index idx,
  int step,
  int zf_off
) {
  tcoords st = { .s=0, .t=0 };
  face_illumination light = 0xff;
  // TODO: Care about exposure?
  compute_dynamic_face_tc(b, BD_ORI_OUT, &st);
  // TODO: Better than this!
  light = face_light(ext_lighting, BD_FACE_TOP);
  push_ne_sw_face(vb, idx, step, st, light, 0, 0, 0, 0, 0, zf_off);
  push_sw_ne_face(vb, idx, step, st, light, 0, 0, 0, 0, 0, zf_off);
  push_nw_se_face(vb, idx, step, st, light, 0, 0, 0, 0, 0, zf_off);
  push_se_nw_face(vb, idx, step, st, light, 0, 0, 0, 0, 0, zf_off);
}

/*************
 * Functions *
 *************/

void compile_chunk_or_approx(chunk_or_approx *coa) {
  static cell dummy = {
    .blocks = { 0, 0 }
  };
  chunk *c;
  chunk_approximation *ca;
  global_chunk_pos* glcpos;
  approx_neighborhood apx_nbh;
  cell_neighborhood cl_nbh;
  vertex_buffer *vb;
  block_info geom;
  int step = 1;
  cube_illumination ext_lighting;
  cube_illumination int_lighting;

  // We start by counting the number of "active" cells (those not surrounded by
  // solid blocks). We use the cached exposure data here (call compute_exposure
  // first!).

  // A pointer to an array of vertex buffers:
  vertex_buffer (*layers)[] = NULL;
  if (coa->type == CA_TYPE_CHUNK) {
    c = (chunk *) (coa->ptr);
    ca = NULL;
    step = 1;
    layers = &(c->layers);
    glcpos = &(c->glcpos);
  } else if (coa->type == CA_TYPE_APPROXIMATION) {
    c = NULL;
    ca = (chunk_approximation *) (coa->ptr);
    step = 1 << (ca->detail);
    layers = &(ca->layers);
    glcpos = &(ca->glcpos);
  } else {
    // We can't deal with unloaded chunks
    return;
  }

  // Get the chunk neighborhood:
  fill_approx_neighborhood(glcpos, &apx_nbh);

  uint16_t counts[N_LAYERS];
  uint16_t total = 0;
  layer i;
  // Initialize counts and clear out any old data:
  for (i = 0; i < N_LAYERS; ++i) {
    counts[i] = 0;
    reset_vertex_buffer(&((*layers)[i]));
  }
  block_index idx;
  cell *here = NULL;
  block exposure = 0;
  for (idx.xyz.x = 0; idx.xyz.x < CHUNK_SIZE; idx.xyz.x += step) {
    for (idx.xyz.y = 0; idx.xyz.y < CHUNK_SIZE; idx.xyz.y += step) {
      for (idx.xyz.z = 0; idx.xyz.z < CHUNK_SIZE; idx.xyz.z += step) {
        if (coa->type == CA_TYPE_CHUNK) {
          here = c_cell(c, idx);
        } else {
          here = ca_cell(ca, idx);
        }
        exposure = compute_cell_exposure(coa, idx, &apx_nbh);
        if (
          (exposure & BF_EXPOSED_ANY)
        &&
          !(
            b_is_invisible(here->blocks[0])
          &&
            b_is_invisible(here->blocks[1])
          )
        ) {
          total += 1;
          counts[block_layer(here->blocks[0])] += 1;
          counts[block_layer(here->blocks[1])] += 1;
        }
      }
    }
  }
  if (total == 0) {
    for (i = 0; i < N_LAYERS; ++i) {
      reset_vertex_buffer(&((*layers)[i]));
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
    vb_setup_cache(&((*layers)[i]));
#ifdef DEBUG
    VERTEX_COUNT = 0;
    INDEX_COUNT = 0;
#endif
  }

  for (idx.xyz.x = 0; idx.xyz.x < CHUNK_SIZE; idx.xyz.x += step) {
    for (idx.xyz.y = 0; idx.xyz.y < CHUNK_SIZE; idx.xyz.y += step) {
      for (idx.xyz.z = 0; idx.xyz.z < CHUNK_SIZE; idx.xyz.z += step) {
        // get local cell and neighbors:
        fill_cell_neighborhood(idx, &apx_nbh, &cl_nbh, step, &dummy);
        here = cl_nbh.members[NBH_CENTER];
        exposure = compute_cell_exposure(coa, idx, &apx_nbh);
        if (!b_is_invisible(here->blocks[0])) {
          ensure_mapped(here->blocks[0]);
          geom = bi_geom(here->blocks[0]);
          vb = &((*layers)[block_layer(here->blocks[0])]);
          if (geom == BI_GEOM_SOLID || geom == BI_GEOM_LIQUID) {
            compute_lighting(&cl_nbh, 0, &ext_lighting);
            add_solid_block(
              vb,
              here->blocks[0],
              exposure,
              &ext_lighting,
              idx,
              step,
              0
            );
          } else if (geom == BI_GEOM_GRASS || geom == BI_GEOM_FILM) {
            compute_lighting(&cl_nbh, 0, &ext_lighting);
            compute_lighting(&cl_nbh, 1, &int_lighting);
            add_grass_block(
              vb,
              here->blocks[0],
              exposure,
              &ext_lighting,
              &int_lighting,
              idx,
              step,
              2
            );
          } else if (geom == BI_GEOM_HERB) {
            compute_lighting(&cl_nbh, 0, &ext_lighting);
            compute_lighting(&cl_nbh, 1, &int_lighting);
            add_herb_block(
              vb,
              here->blocks[0],
              exposure,
              &ext_lighting,
              &int_lighting,
              idx,
              step,
              0
            );
          } else { // fall-back case is solid geometry:
            compute_lighting(&cl_nbh, 0, &ext_lighting);
            add_solid_block(
              vb,
              here->blocks[0],
              exposure,
              &ext_lighting,
              idx,
              step,
              0
            );
          }
        }
        if (!b_is_invisible(here->blocks[1])) {
          ensure_mapped(here->blocks[1]);
          geom = bi_geom(here->blocks[1]);
          vb = &((*layers)[block_layer(here->blocks[1])]);
          if (
            geom == BI_GEOM_SOLID
          || geom == BI_GEOM_VINE
          || geom == BI_GEOM_ROOT
          ) {
            // TODO: this probably isn't necessary a second time
            compute_lighting(&cl_nbh, 0, &ext_lighting);
            add_solid_block(
              vb,
              here->blocks[1],
              exposure,
              &ext_lighting,
              idx,
              step,
              1
            );
          } else if (geom == BI_GEOM_HERB) {
            compute_lighting(&cl_nbh, 0, &ext_lighting);
            compute_lighting(&cl_nbh, 1, &int_lighting);
            add_herb_block(
              vb,
              here->blocks[1],
              exposure,
              &ext_lighting,
              &int_lighting,
              idx,
              step,
              1
            );
          } // else don't draw the secondary
#ifdef DEBUG
          /* TODO: DEBUG
          else {
            printf(
              "Undrawn secondary: %s ~ %d\n",
              BLOCK_NAMES[b_id(here->blocks[1])],
              geom
            );
          }
          */
#endif
        }
      }
    }
  }
  // Compile or reset each buffer:
  for (i = 0; i < N_LAYERS; ++i) {
    if (counts[i] > 0) {
      vb_compile_buffers(&((*layers)[i]));
      vb_free_cache(&((*layers)[i]));
    } else {
      reset_vertex_buffer(&((*layers)[i]));
    }
  }
  // Mark the chunk as compiled:
  if (coa->type == CA_TYPE_CHUNK) {
    c->chunk_flags |= CF_COMPILED;
  } else {
    ca->chunk_flags |= CF_COMPILED;
  }
}
