#ifndef RENDER_H
#define RENDER_H

// render.h
// Main render functions.

#include "display.h"

#include "physics/physics.h"
#include "world/world.h"
#include "gen/worldgen.h"

#include "tex/tex.h"
#include "tex/dta.h"

/*********
 * Enums *
 *********/

enum view_mode_e {
  VM_FIRST,
  VM_SECOND,
  VM_THIRD,
  NUM_VIEW_MODES
};
typedef enum view_mode_e view_mode;

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
extern gl_cpos_t const MAX_RENDER_DISTANCES[N_LODS];

// The distance at which frustum culling applies (Manhattan distance cells):
extern float const MIN_CULL_DIST;

// How many radians outside of the technical viewing area should be allowed
// before frustum culling kicks in.
extern float const RENDER_ANGLE_ALLOWANCE;

// How much of the surrounding world map to render as distant terrain. The
// total number of region anchors considered is 1 + 2*WORLD_NEIGHBORHOOD_SIZE.
#define WORLD_NEIGHBORHOOD_SIZE 1

// A number to add to the z position of the nearest vertex in the distant
// terrain so that surrounding positions look better.
extern gl_pos_t const THIS_REGION_DISTANT_TERRAIN_MESH_OFFSET;

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

// A callback to call after the model view matrix is set up but before anything
// is rendered in render_area:
typedef void (*area_render_callback)(active_entity_area *);
extern area_render_callback AREA_PRE_RENDER_CALLBACK;

// A vertex buffer for local world map geometry (distant terrain):
extern vertex_buffer* WM_VB;

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

// Sets up the rendering subsystem.
void setup_render(void);

// Cleans up the rendering subsystem.
void cleanup_render(void);

// Renders the world.
void render_area(
  active_entity_area *area, // area to render
  vector *head_pos, // eye position
  float yaw, // [0,2PI]; 0 => north
  float pitch, // [-PI,PI]; 0 => horizontal
  // roll is 0
  float fovx, // horizontal field of view in radians
  float fovy // vertical field of view in radians
);

// Renders the given layer of the given chunk.
int render_chunk_layer(
  dynamic_texture_atlas **atlases, // texture atlases for each layer
  chunk_or_approx *coa, // the target chunk/approximation
  global_pos *origin, // a reference point for the origin of the scene
  layer ly // which layer to render
);

// Renders the given entity. Takes it as a void* instead of an entity* so that
// this function can be passed to list's l_foreach().
void iter_render_entity(void *e);

// Compiles the area surrounding the given world region into the WM_VB vertex
// buffer.
void compile_distant_terrain(world_map_pos *origin);

// Renders the nearby portions of the world map. Recompiles the distant terrain
// buffer as necessary.
void render_world_neighborhood(world_map_pos *wmpos, global_pos *origin);

// Draws a highlight on the specified cell
void highlight_cell(global_pos* pos);

#endif // ifndef RENDER_H
