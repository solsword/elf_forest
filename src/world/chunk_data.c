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
    block_is(neighbor, B_VOID)
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
    chunk_or_approx *neighbor, \
    block here, block there \
  ) { \
    if ( \
      block_is(there, B_VOID) \
    && \
      (neighbor->type != CA_TYPE_NOT_LOADED) \
    ) { \
#ifdef DEBUG \
      if (OOR_AXIS != OOR_RESET) { \
        fprintf(stderr, "Error: void block at non-edge!\n");\
        fprintf(stderr, "  OOR_AXIS = %d, edge = %d\n", OOR_AXIS, OOR_RESET);\
        exit(-1); \
      } \
#endif \
      OOR_AXIS = OOR_REPLACE; \
      if (neighbor->type == CA_TYPE_CHUNK) { \
        there = c_get_block((chunk *) (neighbor->ptr), idx); \
      } else { \
        chunk_approximation *ca = (chunk_approximation *) (neighbor->ptr); \
        there = ca_get_block(ca, idx); \
      } \
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

/* TODO: Get rid of these
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
*/

void compute_exposure(chunk_or_approx *coa) {
  chunk_index idx;
  block b = 0;
  block_flag flags_to_set = 0;
  block_flag flags_to_clear = 0;
  block ba = 0, bb = 0, bn = 0, bs = 0, be = 0, bw = 0;
  region_chunk_pos base;
  region_chunk_pos rcpos;
  chunk *c;
  chunk_approximation *ca;
  chunk_or_approx above, below, north, south, east, west;
  ch_idx_t step = 1;

  if (coa->type == CA_TYPE_CHUNK) {
    c = (chunk*) (coa->ptr);
    ca = NULL;
    rcpos.x = c->rcpos.x;
    rcpos.y = c->rcpos.y;
    rcpos.z = c->rcpos.z;
  } else if (coa->type == CA_TYPE_APPROXIMATION) {
    c = NULL:
    ca = (chunk_approximation*) (coa->ptr);
    rcpos.x = ca->rcpos.x;
    rcpos.y = ca->rcpos.y;
    rcpos.z = ca->rcpos.z;
  } else {
    fprintf(stderr, "Attempted to compute exposure of unloaded chunk.\n");
    exit(1);
  }

  // Get neighbor data, returning 0 if it's not available:
  rcpos.x += 1;
  get_best_data(&rcpos, &east);
  rcpos.x -= 2;
  get_best_data(&rcpos, &west);
  rcpos.x += 1;

  rcpos.y += 1;
  get_best_data(&rcpos, &north);
  rcpos.y -= 2;
  get_best_data(&rcpos, &south);
  rcpos.y += 1;

  rcpos.z += 1;
  get_best_data(&rcpos, &above);
  rcpos.z -= 2;
  get_best_data(&rcpos, &below);

  if (coa->type == CA_TYPE_CHUNK) {
    step = 1;
  } else {
    step = 1 << (ca->detail);
  }
  for (idx.x = 0; idx.x < CHUNK_SIZE; idx.x += step) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; idx.y += step) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; idx.z += step) {
        // get main block and neighbors:
        if (coa->type == CA_TYPE_CHUNK) {
          b = c_get_block(c, idx);
          c_get_neighbors(c, idx, &ba, &bb, &bn, &bs, &be, &bw);
        } else {
          b = ca_get_block(ca, idx);
          ca_get_neighbors(ca, idx, &ba, &bb, &bn, &bs, &be, &bw);
        }
        // Check exposure:
        flags_to_set = 0;
        flags_to_clear = 0;
        if (!check_top_face(idx, &above, b, ba)) {
          flags_to_set |= BF_EXPOSED_ABOVE;
        } else {
          flags_to_clear |= BF_EXPOSED_ABOVE;
        }
        if (!check_bot_face(idx, &below, b, bb)) {
          flags_to_set |= BF_EXPOSED_BELOW;
        } else {
          flags_to_clear |= BF_EXPOSED_BELOW;
        }
        if (!check_north_face(idx, &north, b, bn)) {
          flags_to_set |= BF_EXPOSED_NORTH;
        } else {
          flags_to_clear |= BF_EXPOSED_NORTH;
        }
        if (!check_south_face(idx, &south, b, bs)) {
          flags_to_set |= BF_EXPOSED_SOUTH;
        } else {
          flags_to_clear |= BF_EXPOSED_SOUTH;
        }
        if (!check_east_face(idx, &east, b, be)) {
          flags_to_set |= BF_EXPOSED_EAST;
        } else {
          flags_to_clear |= BF_EXPOSED_EAST;
        }
        if (!check_west_face(idx, &west, b, bw)) {
          flags_to_set |= BF_EXPOSED_WEST;
        } else {
          flags_to_clear |= BF_EXPOSED_WEST;
        }
        // Set/clear the computed exposure flags:
        if (coa->type == CA_TYPE_CHUNK) {
          c_set_flags(c, idx, flags_to_set);
          c_clear_flags(c, idx, flags_to_clear);
        } else {
          ca_set_flags(ca, idx, flags_to_set);
          ca_clear_flags(ca, idx, flags_to_clear);
        }
      }
    }
  }
}
