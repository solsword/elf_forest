#ifndef TEXTURE_H
#define TEXTURE_H

// tex.h
// Texture loading and management.

#include <string.h>

#include <GL/gl.h>

#include "datatypes/map.h"
#include "datatypes/bitmap.h"

#include "world/blocks.h"

#include "color.h"

/**************
 * Structures *
 **************/

// Texture dimensions and a data array
struct texture_s;
typedef struct texture_s texture;

// A pair of OpenGL texture coordinates
struct tcoords_s;
typedef struct tcoords_s tcoords;

/*************
 * Constants *
 *************/

// Pixel dimension of each block texture:
#define BLOCK_TEXTURE_SIZE 32

/********************
 * Global variables *
 ********************/

// The directory to load static block textures from:
extern char const * const BLOCK_TEXTURE_DIR;

/*************************
 * Structure Definitions *
 *************************/

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
  if (
    ((left % tx->width) == left)
  &&
    ((top % tx->height) == top)
  ) {
    tx->pixels[left + tx->width * top] = p;
  }
}

// Clears the given texture, setting all of its pixels to black with 0 alpha.
static inline void tx_clear(texture *tx) {
  memset(
    tx->pixels,
    0,
    tx->height * tx->width * sizeof(pixel)
  );
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates an empty texture (filled with 0 data).
texture * create_texture(size_t width, size_t height);

// Allocates a new texture and copies the data from the given original into it,
// returning a pointer to the copied texture.
texture *duplicate_texture(texture *original);

// Frees the data allocated for the given texture.
void cleanup_texture(texture *tx);

/*************
 * Functions *
 *************/

// Sets up the standard dynamic texture atlases.
void setup_textures(void);

// Cleans up the texture subsystem.
void cleanup_textures(void);

// Loads a PNG file and returns a newly-allocated texture pointer.
texture * load_texture_from_png(char const * const filename);

// Writes the given texture in .ppm format to the given file.
void write_texture_to_ppm(texture *tx, char const * const filename);

// Writes the given texture in .png format to the given file using libpng.
void write_texture_to_png(texture *tx, char const * const filename);

// Uploads the given texture to the given texture handle (which must already
// have been created). Overwrites any data that may have been stored at that
// handle.
void upload_texture_to(texture* tx, GLuint handle);

// Returns an OpenGL texture handle created using the given texture:
static inline GLuint upload_texture(texture* tx) {
  GLuint result = 0;
  // Generate a new texture handle:
  glGenTextures(1, &result);
  upload_texture_to(tx, result);
  return result;
}

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

// Draws a rectangle of pixels from one texture to another, using alpha
// blending and wrapping within both textures as necessary:
void tx_draw_region_wrapped(
  texture *dst,
  texture const * const src,
  size_t dst_left,
    size_t dst_top,
  size_t src_left,
    size_t src_top,
    size_t region_width,
    size_t region_height
);

// Alias for tx_draw_region_wrapped that uses the entire source.
static inline void tx_draw_wrapped(
  texture *dst,
  texture const * const src,
  size_t left,
  size_t top
) {
  tx_draw_region_wrapped(
    dst,
    src,
    left, top,
    0, 0,
    src->width, src->height
  );
}

#endif //ifndef TEXTURE_H
