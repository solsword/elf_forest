#ifndef RENDER_H
#define RENDER_H

// render.h
// Main render functions.

#include "world.h"
#include "display.h"
#include "physics.h"

/*************
 * Functions *
 *************/

// Renders the world.
void render_frame(
  frame *f, // frame to render
  vector *eye_pos, // eye position
  float yaw, // [0,2PI]; 0 => north
  float pitch // [-PI,PI]; 0 => horizontal
  // roll is 0
);

void render_chunk_layer(
  frame *f, // The frame that the target chunk is in.
  frame_chunk_index idx, // Chunk coordinates within the frame.
  layer l // Which layer to render.
);

#endif // ifndef RENDER_H
