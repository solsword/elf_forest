// chunk_data.c
// Routines for computing various chunk data like exposure, lighting, etc.

#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "blocks.h"
#include "world.h"
#include "chunk_data.h"

#include "datatypes/list.h"
#include "data/data.h"

/********************
 * Inline Functions *
 ********************/

static inline int occludes_face(block neighbor, block occluded) {
  return (
    b_is_void(neighbor)
  ||
    b_is_opaque(neighbor)
  ||
    (
      b_is_translucent(neighbor)
    &&
      b_is_translucent(occluded)
    &&
      // TODO: Fix this!
      b_same_liquid(neighbor, occluded)
    )
  );
}

// Macro-expanded face-checking functions:
#define CHECK_ANY_FACE \
  static inline int FN_NAME( \
    chunk_index idx, \
    ch_idx_t step, \
    chunk_or_approx *neighbor, \
    block here, block there \
  ) { \
    if ( b_is_void(there) && OOR_AXIS OOR_CMP OOR_LIMIT ) { \
      if (step > 1) { \
        return 0; \
      } else if ( (neighbor->type != CA_TYPE_NOT_LOADED) ) { \
        OOR_AXIS = OOR_REPLACE; \
        if (neighbor->type == CA_TYPE_CHUNK) { \
          there = c_cell((chunk *) (neighbor->ptr), idx)->primary; \
        } else { \
          there = ca_cell( \
            (chunk_approximation *) (neighbor->ptr), \
            idx \
          )->primary; \
        } \
      } \
    } \
    return occludes_face(there, here); \
  }

#define FN_NAME check_top_face
#define OOR_AXIS idx.z
#define OOR_CMP >=
#define OOR_LIMIT (CHUNK_SIZE - step)
#define OOR_REPLACE 0
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_CMP
#undef OOR_LIMIT
#undef OOR_REPLACE

#define FN_NAME check_bot_face
#define OOR_AXIS idx.z
#define OOR_CMP <=
#define OOR_LIMIT step
#define OOR_REPLACE (CHUNK_SIZE - 1)
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_CMP
#undef OOR_LIMIT
#undef OOR_REPLACE

#define FN_NAME check_north_face
#define OOR_AXIS idx.y
#define OOR_CMP >=
#define OOR_LIMIT (CHUNK_SIZE - step)
#define OOR_REPLACE 0
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_CMP
#undef OOR_LIMIT
#undef OOR_REPLACE

#define FN_NAME check_south_face
#define OOR_AXIS idx.y
#define OOR_CMP <=
#define OOR_LIMIT step
#define OOR_REPLACE (CHUNK_SIZE - 1)
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_CMP
#undef OOR_LIMIT
#undef OOR_REPLACE

#define FN_NAME check_east_face
#define OOR_AXIS idx.x
#define OOR_CMP >=
#define OOR_LIMIT (CHUNK_SIZE - step)
#define OOR_REPLACE 0
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_CMP
#undef OOR_LIMIT
#undef OOR_REPLACE

#define FN_NAME check_west_face
#define OOR_AXIS idx.x
#define OOR_CMP <=
#define OOR_LIMIT step
#define OOR_REPLACE (CHUNK_SIZE - 1)
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_CMP
#undef OOR_LIMIT
#undef OOR_REPLACE

/*************
 * Functions *
 *************/

void compute_exposure(chunk_or_approx *coa) {
  chunk_index idx;
  block b = 0;
  block flags_to_set = 0;
  block flags_to_clear = 0;
  block ba = 0, bb = 0, bn = 0, bs = 0, be = 0, bw = 0;
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
    c = NULL;
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
          b = c_cell(c, idx)->primary;
          c_get_primary_neighbors(c, idx, &ba, &bb, &bn, &bs, &be, &bw);
        } else {
          b = ca_cell(ca, idx)->primary;
          ca_get_primary_neighbors(ca, idx, &ba, &bb, &bn, &bs, &be, &bw);
        }
        // Check exposure:
        flags_to_set = 0;
        flags_to_clear = 0;
        if (!check_top_face(idx, step, &above, b, ba)) {
          flags_to_set |= BF_EXPOSED_ABOVE;
        } else {
          flags_to_clear |= BF_EXPOSED_ABOVE;
        }
        if (!check_bot_face(idx, step, &below, b, bb)) {
          flags_to_set |= BF_EXPOSED_BELOW;
        } else {
          flags_to_clear |= BF_EXPOSED_BELOW;
        }
        if (!check_north_face(idx, step, &north, b, bn)) {
          flags_to_set |= BF_EXPOSED_NORTH;
        } else {
          flags_to_clear |= BF_EXPOSED_NORTH;
        }
        if (!check_south_face(idx, step, &south, b, bs)) {
          flags_to_set |= BF_EXPOSED_SOUTH;
        } else {
          flags_to_clear |= BF_EXPOSED_SOUTH;
        }
        if (!check_east_face(idx, step, &east, b, be)) {
          flags_to_set |= BF_EXPOSED_EAST;
        } else {
          flags_to_clear |= BF_EXPOSED_EAST;
        }
        if (!check_west_face(idx, step, &west, b, bw)) {
          flags_to_set |= BF_EXPOSED_WEST;
        } else {
          flags_to_clear |= BF_EXPOSED_WEST;
        }
        // Set/clear the computed exposure flags:
        if (coa->type == CA_TYPE_CHUNK) {
          cl_set_exposure(c_cell(c, idx), flags_to_set);
          cl_clear_exposure(c_cell(c, idx), flags_to_clear);
        } else {
          cl_set_exposure(ca_cell(ca, idx), flags_to_set);
          cl_clear_exposure(ca_cell(ca, idx), flags_to_clear);
        }
      }
    }
  }
}
