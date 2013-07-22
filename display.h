#ifndef DISPLAY_H
#define RENDER_H

// display.h
// Functions for setting up display information.

#include "world.h"

/*************
 * Constants *
 *************/

static const uint16_t VERTEX_STRIDE = 8;

/*************
 * Functions *
 *************/

void compile_chunk(frame *f, uint16_t cx, uint16_t cy, uint16_t cz);

// Removes all of the display lists associated with the given frame.
void cleanup_frame(frame *f);

#endif // ifndef RENDER_H
