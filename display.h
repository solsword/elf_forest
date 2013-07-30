#ifndef DISPLAY_H
#define DISPLAY_H

// display.h
// Functions for setting up display information.

#include "world.h"

/****************
 * Enumerations *
 ****************/

// Layers of a chunk:
typedef enum {
  L_TRANSLUCENT,
  L_OPAQUE,
  N_LAYERS
} layer;

/*************
 * Functions *
 *************/

// Allocates and fills in display lists for the given chunk.
void compile_chunk(chunk *c);

#endif // ifndef DISPLAY_H
