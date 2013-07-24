#ifndef RENDER_H
#define RENDER_H

// render.h
// Main render functions.

#include "world.h"
#include "display.h"
#include "physics.h"

/*********
 * Enums *
 *********/

typedef enum {
  VM_FIRST,
  VM_SECOND,
  VM_THIRD,
  NUM_VIEW_MODES
} view_mode;

/***********
 * Globals *
 ***********/

// The view mode (first, second, or third person).
extern view_mode VIEW_MODE;

// How far the camera will be from the model in third-person view.
extern float THIRD_PERSON_DISTANCE;

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

// Renders the given entity. Takes it as a void* instead of an entity* so that
// it can be passed to list's foreach().
void render_entity(void *e);

#endif // ifndef RENDER_H
