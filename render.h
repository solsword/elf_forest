#ifndef RENDER_H
#define RENDER_H

// render.h
// Main render functions.

#include "world.h"

/*************
 * Functions *
 *************/

// Renders the world.
void render_frame(
  frame *f, // frame to render
  float ex, float ey, float ez, // eye position
  float yaw, // [0,2PI]; 0 => north
  float pitch // [-PI,PI]; 0 => horizontal
  // roll is 0
);

void render_chunk_opaque(
  frame *f, // The frame that the target chunk is in.
  uint16_t cx, uint16_t cy, uint16_t cz // Chunk coordinates within the frame.
);

void render_chunk_translucent(
  frame *f, // The frame that the target chunk is in.
  uint16_t cx, uint16_t cy, uint16_t cz // Chunk coordinates within the frame.
);

#endif // ifndef RENDER_H
