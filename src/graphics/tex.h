#ifndef TEXTURE_H
#define TEXTURE_H

// tex.h
// Texture loading and management.

#include <GL/gl.h>

#include "world/blocks.h"

/**************
 * Structures *
 **************/

typedef uint32_t pixel; // Pixel data packed into 32 bits
typedef uint8_t channel; // A single channel from pixel data

#define CHANNEL_MAX umaxof(channel)

#define RED_SHIFT 24
#define GREEN_SHIFT 16
#define BLUE_SHIFT 8
#define ALPHA_SHIFT 0

#define RED_MASK (0xff << RED_SHIFT)
#define BLUE_MASK (0xff << BLUE_SHIFT)
#define GREEN_MASK (0xff << GREEN_SHIFT)
#define ALPHA_MASK (0xff << ALPHA_SHIFT)

typedef struct texture_s texture; // Texture dimensions and a data array
typedef struct tcoords_s tcoords; // A pair of texture coordinates

/*************
 * Constants *
 *************/

// Pixel dimension of each block texture:
static uint8_t const BLOCK_TEXTURE_SIZE = 16;

/********************
 * Global variables *
 ********************/

// The file to load block textures from:
extern char const * const BLOCK_TEXTURE_FILE;

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
extern uint16_t const BLOCK_TEXTURE_MAP[];

/*************************
 * Structure Definitions *
 *************************/

struct pixel_s {
  channel r, g, b, a;
};

struct texture_s {
  GLsizei width, height;
  pixel *pixels;
};

struct tcoords_s {
  uint16_t s, t;
};

/********************
 * Inline Functions *
 ********************/

// Pixel manipulation functions:

static inline channel px_red(pixel p) {
  return ((p & RED_MASK) >> RED_SHIFT);
}

static inline channel px_green(pixel p) {
  return ((p & GREEN_MASK) >> GREEN_SHIFT);
}

static inline channel px_blue(pixel p) {
  return ((p & BLUE_MASK) >> BLUE_SHIFT);
}

static inline channel px_alpha(pixel p) {
  return ((p & ALPHA_MASK) >> ALPHA_SHIFT);
}


static inline void px_set_red(pixel *p, channel r) {
  *p &= ~RED_MASK;
  *p &= ((pixel) r) << RED_SHIFT;
}

static inline void px_set_green(pixel *p, channel g) {
  *p &= ~GREEN_MASK;
  *p &= ((pixel) g) << GREEN_SHIFT;
}

static inline void px_set_blue(pixel *p, channel b) {
  *p &= ~BLUE_MASK;
  *p &= ((pixel) b) << BLUE_SHIFT;
}

static inline void px_set_alpha(pixel *p, channel a) {
  *p &= ~ALPHA_MASK;
  *p &= ((pixel) a) << ALPHA_SHIFT;
}

// Pixel-level texture access:

static inline pixel tx_get_px(
  texture const * const tx,
  size_t left,
  size_t top
) {
  return tx->pixels[left + tx->width * top];
}

static inline pixel * tx_get_addr(
  texture const * const tx,
  size_t left,
  size_t top
) {
  return &(tx->pixels[left + tx->width * top]);
}

static inline void tx_set_px(
  texture * const tx,
  pixel p,
  size_t left,
  size_t top
) {
  tx->pixels[left + tx->width * top] = p;
}

// Texture coordinate computation routines:

// Stores the offset into the BLOCK_TEXTURE_MAP for the given face:
static uint16_t FACE_TC_MAP_OFF[8] = {
  0, 1, 2, 3, 3, 3, 3, 3
};

// Computes the texture s coordinate for the given index into the texture
// atlas. Won't work if init_textures hasn't been called.
static inline uint16_t block_tc_s(uint16_t i) {
  return i % BLOCK_ATLAS_WIDTH;
}

// Computes the texture t coordinate for the given index into the texture
// atlas. Won't work if init_textures hasn't been called.
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

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates an empty texture (filled with 0 data).
texture * create_texture();

// Frees the data allocated for the given texture.
void cleanup_texture(texture *tx);

/*************
 * Functions *
 *************/

// Sets up the standard textures, loading data into OpenGL but then freeing the
// CPU's copy. No cleanup necessary.
void init_textures(void);

// Loads a PNG file and returns a newly-allocated texture pointer.
texture * load_texture_from_png(char const * const filename);

// Returns an OpenGL texture handle created using the given texture:
GLuint upload_texture(texture* tx);

// Loads a PNG file directly into an OpenGL texture and returns the texture
// handle. Takes care of freeing the texture struct and associated data once
// it's been loaded into OpenGL.
GLuint upload_png(char const * const filename);

// Copies a rectangle of pixels from one texture to another:
void tx_paste_region(
  texture *dst,
  texture const * const src,
  size_t dst_left,
    size_t dst_top,
  size_t src_left,
    size_t src_top,
    size_t region_width,
    size_t region_height
);

// Alias for tx_paste_region that uses the entire source.
static inline void tx_paste(
  texture *dst,
  texture const * const src,
  size_t left,
  size_t top
) {
  tx_paste_region(
    dst,
    src,
    left, top,
    0, 0,
    src->width, src->height
  );
}

// Draws a rectangle of pixels from one texture to another, using alpha
// blending:
void tx_draw_region(
  texture *dst,
  texture const * const src,
  size_t dst_left,
    size_t dst_top,
  size_t src_left,
    size_t src_top,
    size_t region_width,
    size_t region_height
);

// Alias for tx_draw_region that uses the entire source.
static inline void tx_draw(
  texture *dst,
  texture const * const src,
  size_t left,
  size_t top
) {
  tx_draw_region(
    dst,
    src,
    left, top,
    0, 0,
    src->width, src->height
  );
}

#endif //ifndef TEXTURE_H
