#ifndef DISPLAY_H
#define DISPLAY_H

// display.h
// Functions for setting up display information.

#include "world/world.h"

/********************
 * Inline Functions *
 ********************/

static inline layer block_layer(block b) {
  if (is_transparent(b)) {
    return L_TRANSPARENT;
  } else if (is_translucent(b)) {
    return L_TRANSLUCENT;
  } else {
    return L_OPAQUE;
  }
}

/*************
 * Functions *
 *************/

// Allocate and fill in display lists for the given chunk/approximation.
void compile_chunk(chunk *c);
void compile_chunk_approx(chunk_approximation *ca);

#endif // ifndef DISPLAY_H
