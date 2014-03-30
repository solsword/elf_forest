#ifndef CHUNK_DATA_H
#define CHUNK_DATA_H

// chunk_data.h
// Routines for computing various chunk data like exposure, lighting, etc.

#include <stdint.h>

#include "blocks.h"
#include "world.h"

#include "datatypes/list.h"

/**************
 * Structures *
 **************/

// TODO: Get rid of this
// Holds 7 chunk pointers: a main chunk and its neighbors.
struct chunk_neighborhood_s;
typedef struct chunk_neighborhood_s chunk_neighborhood;

/*************************
 * Structure Definitions *
 *************************/

// TODO: Get rid of this
struct chunk_neighborhood_s {
  chunk *c, *above, *below, *north, *south, *east, *west;
};

/*************
 * Functions *
 *************/

/* TODO: Get rid of these:
// Fills in the pointers in the given chunk neighborhood by reading from the
// global chunk cache.
void get_neighborhood(chunk_neighborhood *cnb);

// Returns 1 if all of the chunks in the given chunk neighborhood are non-NULL
// and are loaded, or 0 if any of them are NULL or unloaded.
int is_fully_loaded(chunk_neighborhood *cnb);
*/

// Computes block exposure for the given chunk/approximation, assuming faces
// adjacent to unavailable neighbors are not exposed.
void compute_exposure(chunk_or_approx *coa);

/* TODO: Get rid of me
static inline void compute_chunk_exposure(chunk *c) {
  chunk_or_approx coa;
  ch__coa(c, &coa)
  compute_exposure(&coa);
}

static inline void compute_approx_exposure(chunk_approximation *ca) {
  chunk_or_approx coa;
  aprx__coa(ca, &coa)
  compute_exposure(&coa);
}
*/

#endif // ifndef CHUNK_DATA_H
