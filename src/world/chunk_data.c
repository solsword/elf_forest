// chunk_data.c
// Routines for computing various chunk data like exposure, lighting, etc.

#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "blocks.h"
#include "world.h"
#include "chunk_data.h"

#include "datatypes/list.h"

/********************
 * Inline Functions *
 ********************/

static inline int occludes_face(block neighbor, block occluded) {
  return (
    block_is(neighbor, OUT_OF_RANGE)
  ||
    is_opaque(neighbor)
  ||
    (
      is_translucent(neighbor)
    &&
      is_translucent(occluded)
    &&
      shares_translucency(neighbor, occluded)
    )
  );
}

// Macro-expanded face-checking functions:
#define CHECK_ANY_FACE \
  static inline int FN_NAME( \
    chunk_index idx, \
    chunk *neighbor, \
    block here, block there \
  ) { \
    if (block_is(there, OUT_OF_RANGE) && neighbor) { \
      if (OOR_AXIS != OOR_RESET) { \
        fprintf(stderr, "Error: OUT_OF_RANGE at non-edge!\n");\
        fprintf(stderr, "  OOR_AXIS = %d, edge = %d\n", OOR_AXIS, OOR_RESET);\
        exit(-1); \
      } \
      OOR_AXIS = OOR_REPLACE; \
      there = c_get_block(neighbor, idx); \
    } \
    return occludes_face(there, here); \
  } \

#define FN_NAME check_top_face
#define OOR_AXIS idx.z
#define OOR_REPLACE 0
#define OOR_RESET (CHUNK_SIZE - 1)
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET

#define FN_NAME check_bot_face
#define OOR_AXIS idx.z
#define OOR_REPLACE (CHUNK_SIZE - 1)
#define OOR_RESET 0
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET

#define FN_NAME check_north_face
#define OOR_AXIS idx.y
#define OOR_REPLACE 0
#define OOR_RESET (CHUNK_SIZE - 1)
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET

#define FN_NAME check_south_face
#define OOR_AXIS idx.y
#define OOR_REPLACE (CHUNK_SIZE - 1)
#define OOR_RESET 0
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET

#define FN_NAME check_east_face
#define OOR_AXIS idx.x
#define OOR_REPLACE 0
#define OOR_RESET (CHUNK_SIZE - 1)
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET

#define FN_NAME check_west_face
#define OOR_AXIS idx.x
#define OOR_REPLACE (CHUNK_SIZE - 1)
#define OOR_RESET 0
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET

/*************
 * Functions *
 *************/

chunk_neighborhood * get_neighborhood(frame *f, frame_chunk_index fcidx) {
  chunk_neighborhood *result =
    (chunk_neighborhood *) malloc(sizeof(chunk_neighborhood));
  if (result == NULL) {
    fprintf(stderr, "Failed to allocate a chunk neighborhood.\n");
    exit(errno);
  }
  result->c = chunk_at(f, fcidx);
  if (fcidx.z < FRAME_SIZE - 1) {
    fcidx.z += 1;
    result->above = chunk_at(f, fcidx);
    fcidx.z -= 1;
  } else {
    result->above = NULL;
  }
  if (fcidx.z > 0) {
    fcidx.z -= 1;
    result->below = chunk_at(f, fcidx);
    fcidx.z += 1;
  } else {
    result->below = NULL;
  }
  if (fcidx.y < FRAME_SIZE - 1) {
    fcidx.y += 1;
    result->north = chunk_at(f, fcidx);
    fcidx.y -= 1;
  } else {
    result->north = NULL;
  }
  if (fcidx.y > 0) {
    fcidx.y -= 1;
    result->south = chunk_at(f, fcidx);
    fcidx.y += 1;
  } else {
    result->south = NULL;
  }
  if (fcidx.x < FRAME_SIZE - 1) {
    fcidx.x += 1;
    result->east = chunk_at(f, fcidx);
    fcidx.x -= 1;
  } else {
    result->east = NULL;
  }
  if (fcidx.x > 0) {
    fcidx.x -= 1;
    result->west = chunk_at(f, fcidx);
    fcidx.x += 1;
  } else {
    result->west = NULL;
  }
  return result;
}

int is_fully_loaded(chunk_neighborhood *cnb) {
  return (
    (cnb->c != NULL && !(cnb->c->chunk_flags & CF_NEEDS_RELOAD))
  &&
    (cnb->above != NULL && !(cnb->above->chunk_flags & CF_NEEDS_RELOAD))
  &&
    (cnb->below != NULL && !(cnb->below->chunk_flags & CF_NEEDS_RELOAD))
  &&
    (cnb->north != NULL && !(cnb->north->chunk_flags & CF_NEEDS_RELOAD))
  &&
    (cnb->south != NULL && !(cnb->south->chunk_flags & CF_NEEDS_RELOAD))
  &&
    (cnb->east != NULL && !(cnb->east->chunk_flags & CF_NEEDS_RELOAD))
  &&
    (cnb->west != NULL && !(cnb->west->chunk_flags & CF_NEEDS_RELOAD))
  );
}

void compute_exposure(chunk_neighborhood *cnb) {
  chunk_index idx;
  block b = 0;
  block_flag flags = 0;
  block ba = 0, bb = 0, bn = 0, bs = 0, be = 0, bw = 0;
  if (!is_fully_loaded(cnb)) {
    fprintf(stderr, "Error: compute_exposure on non-fully-loaded chunk!\n");
    fprintf(
      stderr,
      "  rpos = (%d, %d, %d)\n",
      cnb->c->rpos.x,
      cnb->c->rpos.y,
      cnb->c->rpos.z
    );
    exit(-1);
  }
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        // get main block and neighbors:
        b = c_get_block(cnb->c, idx);
        c_get_neighbors(cnb->c, idx, &ba, &bb, &bn, &bs, &be, &bw);
        flags = 0;
        if (!check_top_face(idx, cnb->above, b, ba)) {
          flags |= BF_EXPOSED_ABOVE;
        }
        if (!check_bot_face(idx, cnb->below, b, bb)) {
          flags |= BF_EXPOSED_BELOW;
        }
        if (!check_north_face(idx, cnb->north, b, bn)) {
          flags |= BF_EXPOSED_NORTH;
        }
        if (!check_south_face(idx, cnb->south, b, bs)) {
          flags |= BF_EXPOSED_SOUTH;
        }
        if (!check_east_face(idx, cnb->east, b, be)) {
          flags |= BF_EXPOSED_EAST;
        }
        if (!check_west_face(idx, cnb->west, b, bw)) {
          flags |= BF_EXPOSED_WEST;
        }
        // Set the computed exposure flags:
        c_put_flags(cnb->c, idx, flags);
      }
    }
  }
}
