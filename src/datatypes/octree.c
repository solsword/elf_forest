// octree.c
// Octree implementation.

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <omp.h>

#include "vector.h"
#include "list.h"
#include "bbox.h"
#include "octree.h"

/*************
 * Constants *
 *************/

int const OCTREE_RESOLUTION = 8;
int const OCTREE_MAX_DEPTH = 6; // ~~ 5MB (vs. ~~ 37MB at depth 7)

/********************
 * Helper Functions *
 ********************/

octree * create_octree_recursive(size_t size, vector *origin, int depth) {
  int i;
  vector subori;
  octree *result = (octree *) malloc(sizeof(octree));
  if (result == NULL) {
    perror("Failed to create octant.");
    exit(errno);
  }
  vector vsize = { .x=size, .y=size, .z=size };
  compute_bbox(*origin, vsize, &(result->box));
  result->count = 0;
  result->contents = create_list();
  if (size > OCTREE_RESOLUTION && depth < OCTREE_MAX_DEPTH) {
    for (i = 0; i < 8; ++i) {
      subori.x = origin->x - (size >> 2);
      subori.x += ((size >> 1) * (i & 1));
      subori.y = origin->y - (size >> 2);
      subori.y += ((size >> 1) * ((i & 2) >> 1));
      subori.z = origin->z - (size >> 2);
      subori.z += ((size >> 1) * ((i & 4) >> 2));
      result->octants[i] = create_octree_recursive(size >> 1, &subori, depth+1);
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

void oct_insert_recursive(octree *ot, void *object, bbox *box) {
  int i, lastcount;
  if (oct_has_children(ot)) {
    for (i = 0; i < 8; ++i) {
      if (intersects(*box, ot->octants[i]->box)) {
        lastcount = ot->octants[i]->count;
        oct_insert_recursive(ot->octants[i], object, box);
        ot->count += ot->octants[i]->count - lastcount;
      }
    }
  }
  // Whether we added it to any children or not, we should store it here:
  l_append_element(ot->contents, object);
  ot->count += 1;
}

size_t oct_remove_recursive(octree *ot, void *object) {
  int i;
  int removed = l_remove_all_elements(ot->contents, object);
  if (removed > 0) {
    if (oct_has_children(ot)) {
      for (i = 0; i < 8; ++i) {
        if (!oct_is_empty(ot->octants[i])) {
          removed += oct_remove_recursive(ot->octants[i], object);
        }
      }
    }
  }
  ot->count -= removed;
  return removed;
}

/******************************
 * Constructors & Destructors *
 ******************************/

octree * create_octree(size_t span) {
  vector origin;
  octree *result;
  origin.x = 0;
  origin.y = 0;
  origin.z = 0;
  result = create_octree_recursive(span, &origin, 0);
  omp_init_lock(&(result->lock));
  return result;
}

void cleanup_octree(octree *ot) {
  omp_set_lock(&(ot->lock));
  omp_destroy_lock(&(ot->lock));
  cleanup_octree_recursive(ot);
}

/***********
 * Locking *
 ***********/

void oct_lock(octree *ot) { omp_set_lock(&(ot->lock)); }
void oct_unlock(octree *ot) { omp_unset_lock(&(ot->lock)); }

/*************
 * Functions *
 *************/

int oct_insert(octree *ot, void *object, bbox *box) {
  if (intersects(*box, ot->box)) {
    oct_insert_recursive(ot, object, box);
    return 1;
  } else {
    return 0;
  }
}

size_t oct_remove(octree *ot, void *object) {
  return oct_remove_recursive(ot, object);
}
