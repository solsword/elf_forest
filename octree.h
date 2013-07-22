#ifndef OCTREE_H
#define OCTREE_H

// octree.h
// Octree implementation.

#include "vector.h"
#include "list.h"
#include "bbox.h"

/*********
 * Enums *
 *********/

typedef enum {
  OCT_BOT_SW = 0x00,
  OCT_BOT_SE = 0x01,
  OCT_BOT_NW = 0x02,
  OCT_BOT_NE = 0x03,
  OCT_TOP_SW = 0x04,
  OCT_TOP_SE = 0x05,
  OCT_TOP_NW = 0x06,
  OCT_TOP_NE = 0x07,
} octant;

/**************
 * Structures *
 **************/

// An octree:
struct octree_s;
typedef struct octree_s octree;

/*************
 * Constants *
 *************/

extern const int OCTREE_RESOLUTION;

/*************************
 * Structure Definitions *
 *************************/

struct octree_s {
  bbox box;
  octree *octants[8];
  list *contents;
};

/********************
 * Inline Functions *
 ********************/

// Returns whether the given octree has children or not.
static inline uint8_t has_children(octree *ot) {
  return (ot->octants[0] != NULL);
}

/*************
 * Functions *
 *************/

// Allocates and initializes an octree that spans a frame down to the defined
// minimum dimension (OCTREE_RESOLUTION).
octree * setup_octree(void);

// Frees the memory associated with an octree. Note that this also frees the
// contents list of each octant in the tree.
void cleanup_octree(octree *ot);

// Inserts the given object into the given octree using the given bounding box.
void oct_insert(void *object, bbox *box, octree *ot);

// Removes all copies of the given object from the given octree.
void oct_remove(void *object, octree *ot);

#endif //ifndef OCTREE_H
