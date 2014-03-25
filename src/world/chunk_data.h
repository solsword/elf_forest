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

// Returns 1 if all of the chunks in the given chunk neighborhood are non-NULL
// and are loaded, or 0 if any of them are NULL or unloaded.
int is_fully_loaded(chunk_neighborhood *cnb);

// Computes block exposure for the given chunk, assuming that its entire
// neighborhood is loaded.
void compute_exposure(chunk_neighborhood *cnb);

#endif // ifndef CHUNK_DATA_H
