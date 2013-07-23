// octree.c
// Octree implementation.

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "vector.h"
#include "list.h"
#include "bbox.h"
#include "octree.h"

/*************
 * Constants *
 *************/

const int OCTREE_RESOLUTION = 4;

/********************
 * Helper Functions *
 ********************/

octree * setup_octree_recursive(uint32_t size, vector *origin) {
  int i;
  vector subori;
  octree *result = (octree *) malloc(sizeof(octree));
  if (result == NULL) {
    perror("Failed to create octant.");
    exit(errno);
  }
  vector vsize = { .x=size, .y=size, .z=size };
  compute_bbox(*origin, vsize, &(result->box));
  result->contents = create_list();
  if (size > OCTREE_RESOLUTION) {
    for (i = 0; i < 8; ++i) {
      subori.x = origin->x - (size >> 2);
      subori.x += ((size >> 1) * (i & 1));
      subori.y = origin->y - (size >> 2);
      subori.y += ((size >> 1) * ((i & 2) >> 1));
      subori.z = origin->z - (size >> 2);
      subori.z += ((size >> 1) * ((i & 4) >> 2));
      result->octants[i] = setup_octree_recursive(size >> 1, &subori);
    }
  } else {
    for (i = 0; i < 8; ++i) {
      result->octants[i] = NULL;
    }
  }
  return result;
}

void cleanup_octree_recursive(octree *ot) {
  int i;
  for (i = 0; i < 8; ++i) {
    octree *child = ot->octants[i];
    if (child != NULL) {
      cleanup_octree_recursive(child);
    }
  }
  cleanup_list(ot->contents);
  free(ot);
}

void oct_insert_recursive(void *object, bbox *box, octree *ot) {
  int i;
  if (has_children(ot)) {
    for (i = 0; i < 8; ++i) {
      if (intersects(*box, ot->octants[i]->box)) {
        oct_insert_recursive(object, box, ot->octants[i]);
      }
    }
  } else { // Objects are only stored in leaf nodes.
    append_element(object, ot->contents);
  }
}

void oct_remove_recursive(void *object, octree *ot) {
  int i;
  if (has_children(ot)) {
    for (i = 0; i < 8; ++i) {
      oct_remove_recursive(object, ot->octants[i]);
    }
  }
  remove_elements(object, ot->contents);
}

/*************
 * Functions *
 *************/

octree * setup_octree(uint32_t span) {
  vector origin;
  origin.x = 0;
  origin.y = 0;
  origin.z = 0;
  return setup_octree_recursive(span, &origin);
}

void cleanup_octree(octree *ot) {
  cleanup_octree_recursive(ot);
}

void oct_insert(void *object, bbox *box, octree *ot) {
  oct_insert_recursive(object, box, ot);
}

void oct_remove(void *object, octree *ot) {
  oct_remove_recursive(object, ot);
}
