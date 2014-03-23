#ifndef TEXTURE_H
#define TEXTURE_H

// tex.h
// Texture loading and management.

#include "world/blocks.h"

/**************
 * Structures *
 **************/

typedef struct txinfo_s txinfo; // Texture dimensions & data (used for loading)
typedef struct tcoords_s tcoords; // A pair of texture coordinates

/*************
 * Constants *
 *************/

// Pixel dimension of each block texture:
static const uint8_t BLOCK_TEXTURE_SIZE = 16;

/********************
 * Global variables *
 ********************/

// The file to load block textures from:
extern const char *BLOCK_TEXTURE_FILE;

// The OpenGL texture ID for the blocks texture atlas:
extern GLuint BLOCK_ATLAS;
// The size of the blocks atlas in terms of # of textures:
extern uint16_t BLOCK_ATLAS_WIDTH;
extern uint16_t BLOCK_ATLAS_HEIGHT;

/********
 * Data *
 ********/

// Contains top, front, sides, and bottom atlas indices for each texture ID,
// running from index [id << 2] to index [(id << 2) + 3].
extern const uint16_t BLOCK_TEXTURE_MAP[];

/*************************
 * Structure Definitions *
 *************************/

struct txinfo_s {
  GLsizei width, height;
  GLvoid *data;
};

struct tcoords_s {
  uint16_t s, t;
};

/********************
 * Inline Functions *
 ********************/

// Stores the offset into the BLOCK_TEXTURE_MAP for the given face:
static uint16_t FACE_TC_MAP_OFF[8] = {
  0, 1, 2, 3, 3, 3, 3, 3
};

// Computes the texture s coordinate for the given index into the texture
// atlas. Won't work if setup_textures hasn't been called.
static inline uint16_t block_tc_s(uint16_t i) {
  return i % BLOCK_ATLAS_WIDTH;
}

// Computes the texture t coordinate for the given index into the texture
// atlas. Won't work if setup_textures hasn't been called.
static inline uint16_t block_tc_t(uint16_t i) {
  return (i / BLOCK_ATLAS_WIDTH) % BLOCK_ATLAS_HEIGHT;
}

// Uses block_tc_[s|t] to compute texture coordinates but accounts for rotation
// and facing first.
static inline void compute_face_tc(block b, block_data face, tcoords *result) {
  uint16_t orientable = (b & BF_ORIENTABLE) >> BFS_ORIENTABLE_SHIFT;
  // Duplicate out to 3 bits:
  orientable &= (orientable << 1) & (orientable << 2);
  face = 
    ((~orientable) & face) +
    (orientable & ROTATE_FACE[(b & BR_DATA)][face]);
  uint16_t i = BLOCK_TEXTURE_MAP[(just_id(b) << 2) + FACE_TC_MAP_OFF[face]];
  //uint16_t i = BLOCK_TEXTURE_MAP[(just_id(b) << 2)];
  result->s = block_tc_s(i);
  result->t = block_tc_t(i);
}

/*************
 * Functions *
 *************/

// Sets up the standard textures.
void setup_textures(void);

// Loads a PNG file into an OpenGL texture and returns the texture handle.
// Takes care of freeing the texture info struct and associated data once it's
// been loaded into OpenGL.
GLuint load_texture(const char *filename);

// Loads a PNG file and returns a newly-allocated txinfo pointer.
txinfo* loadPNG(const char *filename);

// Returns an OpenGL texture handle created using the given texture info:
GLuint create_texture(txinfo* info);

#endif //ifndef TEXTURE_H
