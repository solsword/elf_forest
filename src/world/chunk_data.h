#ifndef CHUNK_DATA_H
#define CHUNK_DATA_H

// chunk_data.h
// Routines for computing various chunk data like exposure, lighting, etc.

#include <stdint.h>

#include "blocks.h"
#include "world.h"

#include "datatypes/list.h"

/*************
 * Functions *
 *************/

// Computes and returns the exposure of the given cell, assuming faces adjacent
// to unavailable neighbors are not exposed.
block compute_cell_exposure(
  chunk_or_approx *coa,
  chunk_index idx,
  chunk_or_approx *chunk_neighbors
);

#endif // ifndef CHUNK_DATA_H
