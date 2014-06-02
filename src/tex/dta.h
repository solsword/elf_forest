#ifndef DTA_H
#define DTA_H

// dta.h
// Dynamic texture atlas functionality.

#include "tex.h"

#include "datatypes/map.h"
#include "datatypes/bitmap.h"

#include "graphics/display.h"

#include "world/blocks.h"
#include "world/world.h"

/**************
 * Structures *
 **************/

// A texture atlas that dynamically stores 32x32 textures. It contains a map
// that maps block variants to texture coordinates, as well as a texture that
// holds an atlas of all stored sub-textures and a handle for an OpenGL texture
// that is a copy of the CPU-side texture.
struct dynamic_texture_atlas_s;
typedef struct dynamic_texture_atlas_s dynamic_texture_atlas;

/*************
 * Constants *
 *************/

// The size (side, not total) of the dynamic texture atlases. This gives a
// total texture capacity of 1024 32x32 textures, while itself being a
// 1024x1024 texel texture.
static size_t const DYNAMIC_ATLAS_SIZE = 32;

// Stores the additional offset for each specific face (corresponding to how
// block face textures are organized in individual block texture files). The
// order is:
//   front, sides, top, bot/in/out
static uint16_t const FACE_TC_MAP_OFF[8] = {
  0, 1, 1, 1, 2, 3, 3, 3
};

/********************
 * Global variables *
 ********************/

// The array of dynamic texture atlases for each rendering layer:
extern dynamic_texture_atlas *LAYER_ATLASES[N_LAYERS];

/*************************
 * Structure Definitions *
 *************************/

struct dynamic_texture_atlas_s {
  size_t size; // How many block texture to accommodate (size*size total)
  bitmap *vacancies; // A bitmap of vacancies
  map *tcmap; // block id -> texture index (wrap into size*size)
  texture *atlas; // CPU-side texture data
  GLuint handle; // Handle for GPU-side texture data
};

/********************
 * Inline Functions *
 ********************/

// Takes a block and a face and computes the actual face accounting for the
// block's orientation.
static inline block actual_face(
  block b,
  block ori,
  block face
) {
  if (bi_oabl(b)) {
    return ROTATE_FACE[ori][face];
  } else {
    return face;
  }
}

// Looks up a block and returns its index, or 0 if it isn't mapped.
static inline size_t dta_get_index(dynamic_texture_atlas *dta, block b) {
  return (size_t) m1_get_value(
    dta->tcmap,
    (map_key_t) ((size_t) b_idspc(b))
  );
}

// Writes the index of the given block into the given atlas, returning the old
// value for that block (which should hopefully always be 0).
static inline size_t dta_set_index(
  dynamic_texture_atlas *dta,
  block b,
  size_t index
) {
  return (size_t) m1_put_value(
    dta->tcmap,
    (void *) index,
    (map_key_t) ((size_t) b_idspc(b))
  );
}

static inline void compute_dynamic_face_tc(
  block b,
  block face,
  tcoords *result
) {
  dynamic_texture_atlas *dta = LAYER_ATLASES[block_layer(b)];
  size_t i = dta_get_index(dta, b);
  if (bi_oabl(b)) {
    face = ROTATE_FACE[b_ori(b)][face];
  } else if (!bi_anis(b)) {
    face = 0;
  }
  if (i == 0) {
    result->s = 0;
    result->t = 0;
  } else {
    i += FACE_TC_MAP_OFF[face];
    result->s = i % dta->size;
    result->t = (i / dta->size) % dta->size;
  }
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new dynamic texture atlas of the given size.
dynamic_texture_atlas *create_dynamic_atlas(size_t size);

// Frees the data allocated for the given dynamic texture atlas.
void cleanup_dynamic_atlas(dynamic_texture_atlas *dta);

/*************
 * Functions *
 *************/

// Ensures that the given block is loaded into the appropriate texture atlas,
// loading it if it isn't already loaded.
void ensure_mapped(block b);

// Instructs the given dynamic texture atlas to (re)upload its texture data to
// the GPU, committing any changes made using dta_add/free_block.
void dta_update_texture(dynamic_texture_atlas *dta);

// Adds a block to the given dynamic texture atlas, updating the GPU texture
// after importing the given textures into the atlas. If the block is
// omnidirectional, a 32x32 texture can be used, otherwise a 128x32 texture is
// required, and each of the four 32x32 patches of this texture will be copied
// into a subsequent index in the texture atlas (compute_dynamic_face_tc
// expects this packing scheme). Returns the block within the texture atlas, or
// -1 if it fails.
ptrdiff_t dta_add_block(
  dynamic_texture_atlas *dta,
  block b,
  texture *facemap
);

// TODO: this function!
ptrdiff_t dta_free_block(dynamic_texture_atlas *dta, block b);

#endif //ifndef DTA_H
