#ifndef DISPLAY_H
#define DISPLAY_H

// display.h
// Functions for setting up display information.

#include "world/world.h"

/********************
 * Inline Functions *
 ********************/

static inline layer block_layer(block b) {
  if (b_transparent(b)) {
    return L_TRANSPARENT;
  } else if (b_translucent(b)) {
    return L_TRANSLUCENT;
  } else {
    return L_OPAQUE;
  }
}

/*************
 * Functions *
 *************/

// Allocate and fill in display lists for the given chunk/approximation.
void compile_chunk_or_approx(chunk_or_approx *coa);

#endif // ifndef DISPLAY_H
