#ifndef RNGTABLE_H
#define RNGTABLE_H

// rngtable.h
// A fixed-size table of values with weights for random selection.

#include <stddef.h>

#include "boilerplate.h"

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
CLEANUP_DECL(rngtable);

/*************
 * Functions *
 *************/

// Picks a result from the table corresponding to the given seed. Returns NULL
// if the table is empty. Note that the resolution of the selection algorithm
// is a bit limited, so some distortion is expected when very small weights (in
// relation to the total weight) are used.
void* rt_pick_result(rngtable const * const t, ptrdiff_t seed);

// Works just like rt_pick_result, but only uses elements from the table that
// pass the given filter function (given a table element as the first argument
// and the fixed argument as the second argument).
void* rt_pick_filtered_result(
  rngtable const * const t,
  void *arg,
  int (*filter)(void*, void*),
  ptrdiff_t seed
);

#endif // #ifndef RNGTABLE_H
