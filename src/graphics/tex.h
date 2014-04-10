#ifndef TEXTURE_H
#define TEXTURE_H

// tex.h
// Texture loading and management.

#include <GL/gl.h>

#include "datatypes/map.h"

#include "world/blocks.h"

/**************
 * Structures *
 **************/

typedef uint32_t pixel; // Pixel data packed into 32 bits
typedef uint8_t channel; // A single channel from pixel data

#define CHANNEL_BITS 8
#define CHANNEL_MAX umaxof(channel)

#define RED_SHIFT 0
#define GREEN_SHIFT 8
#define BLUE_SHIFT 16
#define ALPHA_SHIFT 24

#define RED_MASK (0xff << RED_SHIFT)
#define BLUE_MASK (0xff << BLUE_SHIFT)
#define GREEN_MASK (0xff << GREEN_SHIFT)
#define ALPHA_MASK (0xff << ALPHA_SHIFT)

// Texture dimensions and a data array
struct texture_s;
typedef struct texture_s texture;

// A pair of OpenGL texture coordinates
struct tcoords_s;
typedef struct tcoords_s tcoords;

// A texture atlas that dynamically stores 32x32 textures. It contains a map
// that maps block ids to texture coordinates, as well as a texture that holds
// an atlas of all stored sub-textures and a handle for an OpenGL texture that
// is a copy of the CPU-side texture.
struct dynamic_texture_atlas_s;
typedef struct dynamic_texture_atlas_s dynamic_texture_atlas;

/*************
 * Constants *
 *************/

// Pixel dimension of each block texture:
static uint8_t const BLOCK_TEXTURE_SIZE = 32;

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

struct texture_s {
  GLsizei width, height;
  pixel *pixels;
};

struct tcoords_s {
  uint16_t s, t;
};

struct dynamic_texture_atlas_s {
  texture *atlas; // CPU-side texture data
  GLuint handle; // Handle for GPU-side texture data
  map *tcmap; // block id -> texture coords
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

static inline float px_chroma(pixel p) {
  float r = px_red(p) / CHANNEL_MAX;
  float g = px_green(p) / CHANNEL_MAX;
  float b = px_blue(p) / CHANNEL_MAX;
  float max = 0, min = CHANNEL_MAX;
  if (r > max) { max = r; }
  if (g > max) { max = g; }
  if (b > max) { max = b; }
  if (r < min) { min = r; }
  if (g < min) { min = g; }
  if (b < min) { min = b; }
  return max - min;
}

static inline float px_hue(pixel p) {
  float r = px_red(p) / CHANNEL_MAX;
  float g = px_green(p) / CHANNEL_MAX;
  float b = px_blue(p) / CHANNEL_MAX;
  float max = 0, min = CHANNEL_MAX;
  float chroma = 0;
  if (r > max) { max = r; }
  if (g > max) { max = g; }
  if (b > max) { max = b; }
  if (r < min) { min = r; }
  if (g < min) { min = g; }
  if (b < min) { min = b; }
  chroma = max - min;
  if (max == r) {
    return (1/6.0) * (g - b) / chroma + (b > g ? 1 : 0);
  } else if (max == g) {
    return (1/6.0) * (b - r) / chroma + (1/3.0);
  } else if (max == b) {
    return (1/6.0) * (r - g) / chroma + (2/3.0);
  } else {
    return 0;
  }
}

static inline float px_intensity(pixel p) {
  float r = px_red(p) / CHANNEL_MAX;
  float g = px_green(p) / CHANNEL_MAX;
  float b = px_blue(p) / CHANNEL_MAX;
  return (r + g + b) / 3.0;
}

static inline float px_value(pixel p) {
  float r = px_red(p) / CHANNEL_MAX;
  float g = px_green(p) / CHANNEL_MAX;
  float b = px_blue(p) / CHANNEL_MAX;
  if (r >= g && r >= b) {
    return r;
  } else if (g >= b) {
    return g;
  } else {
    return b;
  }
}

static inline float px_lightness(pixel p) {
  float r = px_red(p) / CHANNEL_MAX;
  float g = px_green(p) / CHANNEL_MAX;
  float b = px_blue(p) / CHANNEL_MAX;
  float max = 0, min = CHANNEL_MAX;
  if (r > max) { max = r; }
  if (g > max) { max = g; }
  if (b > max) { max = b; }
  if (r < min) { min = r; }
  if (g < min) { min = g; }
  if (b < min) { min = b; }
  return (max + min) / 2.0;
}

static inline float px_luma(pixel p) {
  float r = px_red(p) / CHANNEL_MAX;
  float g = px_green(p) / CHANNEL_MAX;
  float b = px_blue(p) / CHANNEL_MAX;
  return 0.30 * r + 0.59 * g + 0.11 * b;
}


static inline void px_set_red(pixel *p, channel r) {
  *p &= ~RED_MASK;
  *p |= ((pixel) r) << RED_SHIFT;
}

static inline void px_set_green(pixel *p, channel g) {
  *p &= ~GREEN_MASK;
  *p |= ((pixel) g) << GREEN_SHIFT;
}

static inline void px_set_blue(pixel *p, channel b) {
  *p &= ~BLUE_MASK;
  *p |= ((pixel) b) << BLUE_SHIFT;
}

static inline void px_set_alpha(pixel *p, channel a) {
  *p &= ~ALPHA_MASK;
  *p |= ((pixel) a) << ALPHA_SHIFT;
}

static inline void px_set_gray(pixel *p, channel gray) {
  *p &= ALPHA_MASK;
  *p |= ((pixel) gray) << RED_SHIFT;
  *p |= ((pixel) gray) << GREEN_SHIFT;
  *p |= ((pixel) gray) << BLUE_SHIFT;
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
texture * create_texture(size_t width, size_t height);

// Allocates a new texture and copies the data from the given original into it,
// returning a pointer to the copied texture.
texture *duplicate_texture(texture *original);

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

// Writes the given texture in .ppm format to the given file.
void write_texture_to_ppm(texture *tx, char const * const filename);

// Writes the given texture in .png format to the given file using libpng.
void write_texture_to_png(texture *tx, char const * const filename);

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
