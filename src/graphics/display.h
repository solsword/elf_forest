#ifndef DISPLAY_H
#define DISPLAY_H

// display.h
// Functions for setting up display information.

#include "world/world.h"
#include "world/blocks.h"

/*********
 * Types *
 *********/

// Contains illumination data for 6 faces.
struct cube_illumination_s;
typedef struct cube_illumination_s cube_illumination;

// 2 bits for each of four vertices:
typedef uint8_t face_illumination;

// A full 8 bits of vertex illumination is interpolated across faces on the
// graphics card:
typedef GLubyte vertex_illumination;

// (really 2 bits): vertex on a face:
//   0 = southwest
//   1 = northwest
//   2 = northeast
//   3 = northwest
typedef uint8_t face_vertex;

/*************
 * Constants *
 *************/

// How far to push things to prevent z-fighting:
extern float const Z_RECONCILIATION_OFFSET;

// Where to place the corners of diagonal faces to avoid distortion:
extern float const DF_LOWER;
extern float const DF_UPPER;

// Lighting parameters for ambient occlusion lighting:
extern uint8_t const BASE_LIGHT_LEVEL;
extern uint8_t const AMBIENT_LIGHT_STRENGTH;

/*************************
 * Structure Definitions *
 *************************/

struct cube_illumination_s {
  face_illumination faces[6];
};

/********************
 * Inline Functions *
 ********************/

static inline layer block_layer(block b) {
  if (b_is_transparent(b)) {
    return L_TRANSPARENT;
  } else if (b_is_translucent(b)) {
    return L_TRANSLUCENT;
  } else {
    return L_OPAQUE;
  }
}

// Looks up a vertex illumination (2 bits) in a face illumination value and
// adjusts to the full 1-byte range. The vertex argument should be either 0, 1,
// 2, or 3.
static inline vertex_illumination vertex_light(
  face_illumination light,
  face_vertex vertex
) {
  return (
    BASE_LIGHT_LEVEL +
    AMBIENT_LIGHT_STRENGTH * ((light >> (2*vertex)) & 0x3)
  );
}

// Looks up a face illumination (8 bits) in a cube_illumination value. The
// vertex argument should be either 0, 1, 2, or 3.
static inline face_illumination face_light(
  cube_illumination* i,
  block_data face
) {
  return i->faces[face];
}

// Computes lighting for the vertices of a cube. Normally computes external
// illumination, unless internal is 1, in which case it computes internal
// illumination.
static inline void compute_lighting(
  cell_neighborhood* nbh,
  int internal,
  cube_illumination* result
) {
  block_data face = BD_FACE_FRONT;
  face_vertex vertex;
  // Which face determines what north/south/east/west mean and the base offset:
  int o_base = 0;
  int o_n = 0, o_e = 0, o_s = 0, o_w = 0;
  int i0, i1, ic;
  int side0, side1, corner;
  for (face = BD_FACE_FRONT; face < BD_FACE_INSIDE; ++face) {
    // clear the lighting for this face:
    result->faces[face] = 0x00;
    switch (face) {
      case M_BD_FACE_FRONT:
      default:
        o_base = 3;
        o_n = 1;
        o_e = -9;
        o_s = -1;
        o_w = 9;
        break;
      case M_BD_FACE_BACK:
        o_base = -3;
        o_n = 1;
        o_e = 9;
        o_s = -1;
        o_w = -9;
        break;
      case M_BD_FACE_RIGHT:
        o_base = -9;
        o_n = 1;
        o_e = -3;
        o_s = -1;
        o_w = 3;
        break;
      case M_BD_FACE_LEFT:
        o_base = 9;
        o_n = 1;
        o_e = 3;
        o_s = -1;
        o_w = -3;
        break;
      case M_BD_FACE_TOP:
        o_base = 1;
        o_n = 3;
        o_e = 9;
        o_s = -3;
        o_w = -9;
        break;
      case M_BD_FACE_BOT:
        o_base = -1;
        o_n = 3;
        o_e = -9;
        o_s = -3;
        o_w = 9;
        break;
    }
    if (internal) {
      o_base = 0;
    }
    for (vertex = 0; vertex < 4; ++vertex) {
      i0 = NBH_CENTER + o_base;
      i1 = NBH_CENTER + o_base;
      ic = NBH_CENTER + o_base;
      switch (vertex) {
        case 0:
        default:
          i0 += o_s;
          i1 += o_w;
          ic += o_s + o_w;
          break;
        case 1:
          i0 += o_n;
          i1 += o_w;
          ic += o_n + o_w;
          break;
        case 2:
          i0 += o_n;
          i1 += o_e;
          ic += o_n + o_e;
          break;
        case 3:
          i0 += o_s;
          i1 += o_e;
          ic += o_s + o_e;
          break;
      }
      // TODO: non-primary blocks!
      side0 = b_is_opaque(nbh->members[i0]->blocks[0]);
      side1 = b_is_opaque(nbh->members[i1]->blocks[0]);
      corner = b_is_opaque(nbh->members[ic]->blocks[0]);
      // DEBUG:
      /*
      if (face == BD_FACE_FRONT) {
        result->faces[face] = 0x00;
      } else if (face == BD_FACE_BACK) {
        result->faces[face] = 0x00;
      } else if (face == BD_FACE_RIGHT) {
        result->faces[face] = 0x55;
      } else if (face == BD_FACE_LEFT) {
        result->faces[face] = 0x55;
      } else if (face == BD_FACE_TOP) {
        result->faces[face] = 0xaa;
      } else if (face == BD_FACE_BOT) {
        result->faces[face] = 0xaa;
      }
      // */
      //*
      if (!(side0 && side1)) {
        result->faces[face] |= (3 - (side0 + side1 + corner)) << (2*vertex);
      } // else lighting is 0 so we do nothing since that's the default
      // */
    }
  }
}

/*************
 * Functions *
 *************/

// Allocate and fill in display lists for the given chunk/approximation.
void compile_chunk_or_approx(chunk_or_approx *coa);

#endif // ifndef DISPLAY_H
