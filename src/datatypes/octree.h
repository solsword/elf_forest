#ifndef OCTREE_H
#define OCTREE_H

// octree.h
// Octree implementation.

#include <omp.h>

#include "boilerplate.h"

#include "vector.h"
#include "list.h"
#include "bbox.h"

/*********
 * Enums *
 *********/

enum octant_e {
  OCT_BOT_SW = 0x00,
  OCT_BOT_SE = 0x01,
  OCT_BOT_NW = 0x02,
  OCT_BOT_NE = 0x03,
  OCT_TOP_SW = 0x04,
  OCT_TOP_SE = 0x05,
  OCT_TOP_NW = 0x06,
  OCT_TOP_NE = 0x07,
};
typedef enum octant_e octant;

/**************
 * Structures *
 **************/

// An octree:
struct octree_s;
typedef struct octree_s octree;

/*************
 * Constants *
 *************/

extern int const OCTREE_RESOLUTION;
extern int const OCTREE_MAX_DEPTH;

/*************************
 * Structure Definitions *
 *************************/

struct octree_s {
  bbox box;
  size_t count;
  octree *octants[8];
  list *contents;
  omp_lock_t lock;
};

/********************
 * Inline Functions *
 ********************/

// Returns whether the given octree has children or not.
static inline int oct_has_children(octree *ot) {
  return (ot->octants[0] != NULL);
}

// Returns whether the given octree is empty or not.
static inline int oct_is_empty(octree *ot) {
  return (ot->count == 0);
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and initializes an octree which subdivides the given span. The
// octree will have OCTREE_RESOLUTION dimension at its lowest point, unless
// that would require it to be deeper than OCTREE_MAX_DEPTH, in which case
// it'll be OCTREE_MAX_DEPTH deep.
octree * create_octree(size_t span);

// Frees the memory associated with an octree. Note that this also frees the
// contents list of each octant in the tree (but not, of course, the elements
// of that contents list, since elements are duplicated throughout the tree).
CLEANUP_DECL(octree);

/***********
 * Locking *
 ***********/

void oct_lock(octree *ot);
void oct_unlock(octree *ot);

/*************
 * Functions *
 *************/

// Inserts the given object into the given octree using the given bounding box.
// If the given bounding box doesn't overlap with the area covered by the
// octree at all, it returns 0, otherwise it returns 1.
int oct_insert(octree *ot, void *object, bbox *box);

// Removes all copies of the given object from the given octant and its
// children. Returns the number of copies removed.
size_t oct_remove(octree *ot, void *object);

#endif //ifndef OCTREE_H
