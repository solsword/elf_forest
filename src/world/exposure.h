#ifndef EXPOSURE_H
#define EXPOSURE_H

// exposure.h
// Computing the exposure of the blocks in a chunk neighborhood.

#include <stdint.h>

#include "blocks.h"
#include "world.h"

#include "datatypes/list.h"

/**************
 * Structures *
 **************/

// Holds 7 chunk pointers: a main chunk and its neighbors.
struct chunk_neighborhood_s;
typedef struct chunk_neighborhood_s chunk_neighborhood;

/*************************
 * Structure Definitions *
 *************************/

struct chunk_neighborhood_s {
  chunk *c, *above, *below, *north, *south, *east, *west;
};

/*************
 * Functions *
 *************/

// Allocates and returns a chunk neighborhood object centered at the given
// position.
chunk_neighborhood * get_neighborhood(frame *f, frame_chunk_index fcidx);

// Computes block exposure for the given chunk, assuming that its entire
// neighborhood is loaded.
void compute_exposure(chunk_neighborhood *cnb);

#endif // ifndef EXPOSURE_H
