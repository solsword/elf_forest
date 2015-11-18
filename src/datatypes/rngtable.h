#ifndef RNGTABLE_H
#define RNGTABLE_H

// rngtable.h
// A fixed-size table of values with weights for random selection.

#include <stddef.h>

/**************
 * Structures *
 **************/

struct rngtable_s;
typedef struct rngtable_s rngtable;

/*************************
 * Structure Definitions *
 *************************/

struct rngtable_s {
  size_t size;
  void* *values;
  float *weights;
};

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and sets up a new empty rngtable of the given size. Note that
// constant rngtables should just be declared statically rather than using this
// function and then setting them up.
rngtable *create_rngtable(size_t size);

// Frees the memory associated with an rngtable.
void cleanup_rngtable(rngtable *t);

/*************
 * Functions *
 *************/

// Picks a result from the table corresponding to the given seed. Returns NULL
// if the table is empty. Note that the resolution of the selection algorithm
// is a bit limited, so some distortion is expected when very small weights (in
// relation to the total weight) are used.
void* rt_pick_result(rngtable const * const t, ptrdiff_t seed);

#endif // #ifndef RNGTABLE_H
