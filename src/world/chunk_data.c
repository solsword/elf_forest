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

void compute_exposure(chunk_or_approx *coa) {
  static cell dummy = {
    .primary = 0,
    .secondary = 0,
    .p_data = 0,
    .s_data = 0
  };
  chunk_index idx;
  block flags_to_set = 0;
  block flags_to_clear = 0;
  region_chunk_pos rcpos;
  chunk *c;
  chunk_approximation *ca;
  chunk_or_approx chunk_neighbors[27]; // zxy order
  cell* neighborhood[27]; // also zxy order
  int step = 1;

  if (coa->type == CA_TYPE_CHUNK) {
    c = (chunk*) (coa->ptr);
    ca = NULL;
    step = 1;
    rcpos.x = c->rcpos.x;
    rcpos.y = c->rcpos.y;
    rcpos.z = c->rcpos.z;
  } else if (coa->type == CA_TYPE_APPROXIMATION) {
    c = NULL;
    ca = (chunk_approximation*) (coa->ptr);
    step = 1 << (ca->detail);
    rcpos.x = ca->rcpos.x;
    rcpos.y = ca->rcpos.y;
    rcpos.z = ca->rcpos.z;
  } else {
    fprintf(stderr, "Attempted to compute exposure of unloaded chunk.\n");
    exit(1);
  }

  // Get the chunk neighborhood:
  get_chunk_neighborhood(&rcpos, chunk_neighbors);

  for (idx.x = 0; idx.x < CHUNK_SIZE; idx.x += step) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; idx.y += step) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; idx.z += step) {
        // Get main block and neighbors:
        get_cell_neighborhood(idx, chunk_neighbors, neighborhood, step, &dummy);
        // Check exposure:
        flags_to_set = 0;
        flags_to_clear = 0;
        if (
          !occludes_face(
            neighborhood[13+9]->primary,
            neighborhood[13]->primary
          )
        ) {
          flags_to_set |= BF_EXPOSED_ABOVE;
        } else {
          flags_to_clear |= BF_EXPOSED_ABOVE;
        }
        if (
          !occludes_face(
            neighborhood[13-9]->primary,
            neighborhood[13]->primary
          )
        ) {
          flags_to_set |= BF_EXPOSED_BELOW;
        } else {
          flags_to_clear |= BF_EXPOSED_BELOW;
        }
        if (
          !occludes_face(
            neighborhood[13+1]->primary,
            neighborhood[13]->primary
          )
        ) {
          flags_to_set |= BF_EXPOSED_NORTH;
        } else {
          flags_to_clear |= BF_EXPOSED_NORTH;
        }
        if (
          !occludes_face(
            neighborhood[13-1]->primary,
            neighborhood[13]->primary
          )
        ) {
          flags_to_set |= BF_EXPOSED_SOUTH;
        } else {
          flags_to_clear |= BF_EXPOSED_SOUTH;
        }
        if (
          !occludes_face(
            neighborhood[13+3]->primary,
            neighborhood[13]->primary
          )
        ) {
          flags_to_set |= BF_EXPOSED_EAST;
        } else {
          flags_to_clear |= BF_EXPOSED_EAST;
        }
        if (
          !occludes_face(
            neighborhood[13-3]->primary,
            neighborhood[13]->primary
          )
        ) {
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


block compute_cell_exposure(
  chunk_or_approx *coa,
  chunk_index idx,
  chunk_or_approx *chunk_neighbors
) {
  static cell dummy = {
    .primary = 0,
    .secondary = 0,
    .p_data = 0,
    .s_data = 0
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
