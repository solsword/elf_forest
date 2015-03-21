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

/*************
 * Functions *
 *************/

block compute_cell_exposure(
  chunk_or_approx *coa,
  chunk_index idx,
  chunk_or_approx *chunk_neighbors
) {
  static cell dummy = {
    .primary = 0,
    .secondary = 0,
  };
  static cell* neighborhood[27]; // also zxy order
  block result = 0;
  int step = 1;

  if (coa->type == CA_TYPE_CHUNK) {
    step = 1;
  } else if (coa->type == CA_TYPE_APPROXIMATION) {
    step = 1 << (((chunk_approximation*) (coa->ptr))->detail);
  } else {
    fprintf(stderr, "Attempted to compute exposure of unloaded cell.\n");
    exit(1);
  }

  // Get main block and neighbors:
  get_cell_neighborhood(idx, chunk_neighbors, neighborhood, step, &dummy);

  // Check exposure:
  if (
    !occludes_face(
      neighborhood[13+9]->primary,
      neighborhood[13]->primary
    )
  ) {
    result |= BF_EXPOSED_ABOVE;
  } else {
    result &= ~BF_EXPOSED_ABOVE;
  }
  if (
    !occludes_face(
      neighborhood[13-9]->primary,
      neighborhood[13]->primary
    )
  ) {
    result |= BF_EXPOSED_BELOW;
  } else {
    result &= ~BF_EXPOSED_BELOW;
  }
  if (
    !occludes_face(
      neighborhood[13+1]->primary,
      neighborhood[13]->primary
    )
  ) {
    result |= BF_EXPOSED_NORTH;
  } else {
    result &= ~BF_EXPOSED_NORTH;
  }
  if (
    !occludes_face(
      neighborhood[13-1]->primary,
      neighborhood[13]->primary
    )
  ) {
    result |= BF_EXPOSED_SOUTH;
  } else {
    result &= ~BF_EXPOSED_SOUTH;
  }
  if (
    !occludes_face(
      neighborhood[13+3]->primary,
      neighborhood[13]->primary
    )
  ) {
    result |= BF_EXPOSED_EAST;
  } else {
    result &= ~BF_EXPOSED_EAST;
  }
  if (
    !occludes_face(
      neighborhood[13-3]->primary,
      neighborhood[13]->primary
    )
  ) {
    result |= BF_EXPOSED_WEST;
  } else {
    result &= ~BF_EXPOSED_WEST;
  }

  return result;
}
