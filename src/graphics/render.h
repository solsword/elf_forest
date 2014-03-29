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

// Fog densities for various materials (used for view fog when the player's
// head is in the given material):
extern float const AIR_FOG_DENSITY;
extern float const WATER_FOG_DENSITY;

// The table of maximum render distances for each level of detail. Note that
// this should have some relation to the LOAD_DISTANCES table in data.h
// (ideally the values in this table are slightly higher to allow
// higher-resolution data to continue to be rendered even as lower-resolution
// data is fetched for areas leaving the focus zone). If data of the desired
// quality is not loaded, lower-quality data may be used in its place, but
// higher-quality data won't be.
extern r_cpos_t const MAX_RENDER_DISTANCES[N_LODS];

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
void render_area(
  active_entity_area *area, // area to render
  vector *head_pos, // eye position
  float yaw, // [0,2PI]; 0 => north
  float pitch // [-PI,PI]; 0 => horizontal
  // roll is 0
);

void render_chunk_layer(
  chunk_or_approx *coa, // the target chunk/approximation
  region_pos *origin, // a reference point for the origin of the scene
  layer ly // which layer to render
);

// Renders the given entity. Takes it as a void* instead of an entity* so that
// this function can be passed to list's l_foreach().
void iter_render_entity(void *e);

#endif // ifndef RENDER_H
