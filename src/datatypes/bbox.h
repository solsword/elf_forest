#ifndef BBOX_H
#define BBOX_H

// bbox.h
// Bounding box implementation.

#include "util.h"
#include "vector.h"

/**************
 * Structures *
 **************/

// A bounding box:
struct bbox_s;
typedef struct bbox_s bbox;

/*************************
 * Structure Definitions *
 *************************/

struct bbox_s {
  vector min;
  vector max;
};

/********************
 * Inline Functions *
 ********************/

// Computes a bounding box centered at the given origin with x/y/z sizes given
// as the size vector. Stores the results in the given bounding box.
static inline void compute_bbox(vector origin, vector size, bbox *box) {
  box->min.x = origin.x - (size.x/2.0);
  box->min.y = origin.y - (size.y/2.0);
  box->min.z = origin.z - (size.z/2.0);
  box->max.x = origin.x + (size.x/2.0);
  box->max.y = origin.y + (size.y/2.0);
  box->max.z = origin.z + (size.z/2.0);
}

// Returns 1 if boxes b1 and b2 intersect, and 0 otherwise:
static inline int intersects(bbox b1, bbox b2) {
  return (
    (b1.min.x <= b2.max.x) && (b2.min.x <= b1.max.x)
  &&
    (b1.min.y <= b2.max.y) && (b2.min.y <= b1.max.y)
  &&
    (b1.min.z <= b2.max.z) && (b2.min.z <= b1.max.z)
  );
}

// Integer min/max coordinates of the box:
static inline int b_i_min_x(bbox b) { return fastfloor(b.min.x); }
static inline int b_i_max_x(bbox b) { return fastfloor(b.max.x); }
static inline int b_i_min_y(bbox b) { return fastfloor(b.min.y); }
static inline int b_i_max_y(bbox b) { return fastfloor(b.max.y); }
static inline int b_i_min_z(bbox b) { return fastfloor(b.min.z); }
static inline int b_i_max_z(bbox b) { return fastfloor(b.max.z); }

#endif //ifndef BBOX_H
