#ifndef RENDER_H
#define RENDER_H

// render.h
// Main render functions.

#include "display.h"

#include "physics/physics.h"
#include "world/world.h"

/*********
 * Enums *
 *********/

typedef enum {
  VM_FIRST,
  VM_SECOND,
  VM_THIRD,
  NUM_VIEW_MODES
} view_mode;

/*************
 * Constants *
 *************/

extern float const AIR_FOG_DENSITY;
extern float const WATER_FOG_DENSITY;

/***********
 * Globals *
 ***********/

// The view mode (first, second, or third person).
extern view_mode VIEW_MODE;

// Base camera<->model distances in second/third person views:
extern float SECOND_PERSON_DISTANCE;
extern float THIRD_PERSON_DISTANCE;

// The fog distance:
extern float FOG_DENSITY;

/********************
 * Inline Functions *
 ********************/

static inline void set_tint(float r, float g, float b, float a) {
  glBlendColor(r, g, b, a);
}

static inline void no_tint(void) {
  glBlendColor(1.0, 1.0, 1.0, 1.0);
}

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
// this function can be passed to list's l_foreach().
void render_entity(void *e);

#endif // ifndef RENDER_H
