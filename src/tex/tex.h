#ifndef TEXTURE_H
#define TEXTURE_H

// tex.h
// Texture loading and management.

#include <string.h>

#include <GL/gl.h>

#include "datatypes/map.h"
#include "datatypes/bitmap.h"

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

/*************
 * Constants *
 *************/

// Pixel dimension of each block texture:
#define BLOCK_TEXTURE_SIZE 32

// An empty (completely transparent) pixel value:
#define PX_EMPTY 0x00000000

// A black opaque pixel value:
#define PX_BLACK 0xff000000

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

// Pixel manipulation functions:

static inline channel px_red(pixel p) {
  return ((p & RED_MASK) >> RED_SHIFT);
}
static inline channel px_hue(pixel p) {
  return ((p & RED_MASK) >> RED_SHIFT);
}

static inline channel px_green(pixel p) {
  return ((p & GREEN_MASK) >> GREEN_SHIFT);
}
static inline channel px_sat(pixel p) {
  return ((p & GREEN_MASK) >> GREEN_SHIFT);
}

static inline channel px_blue(pixel p) {
  return ((p & BLUE_MASK) >> BLUE_SHIFT);
}
static inline channel px_val(pixel p) {
  return ((p & BLUE_MASK) >> BLUE_SHIFT);
}

static inline channel px_alpha(pixel p) {
  return ((p & ALPHA_MASK) >> ALPHA_SHIFT);
}

static inline float px_chroma_f(pixel p) {
  float r = px_red(p) / (float) CHANNEL_MAX;
  float g = px_green(p) / (float) CHANNEL_MAX;
  float b = px_blue(p) / (float) CHANNEL_MAX;
  float max = 0, min = CHANNEL_MAX;
  if (r > max) { max = r; }
  if (g > max) { max = g; }
  if (b > max) { max = b; }
  if (r < min) { min = r; }
  if (g < min) { min = g; }
  if (b < min) { min = b; }
  return max - min;
}

static inline float px_hue_f(pixel p) {
  float r = px_red(p) / (float) CHANNEL_MAX;
  float g = px_green(p) / (float) CHANNEL_MAX;
  float b = px_blue(p) / (float) CHANNEL_MAX;
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

static inline float px_saturation_f(pixel p) {
  float r = px_red(p) / (float) CHANNEL_MAX;
  float g = px_green(p) / (float) CHANNEL_MAX;
  float b = px_blue(p) / (float) CHANNEL_MAX;
  float max = 0, min = CHANNEL_MAX;
  float chroma = 0;
  if (r > max) { max = r; }
  if (g > max) { max = g; }
  if (b > max) { max = b; }
  if (r < min) { min = r; }
  if (g < min) { min = g; }
  if (b < min) { min = b; }
  chroma = max - min;
  if (max != 0) {
    return chroma / max;
  } else {
    return 0;
  }
}

static inline float px_value_f(pixel p) {
  float r = px_red(p) / (float) CHANNEL_MAX;
  float g = px_green(p) / (float) CHANNEL_MAX;
  float b = px_blue(p) / (float) CHANNEL_MAX;
  if (r >= g && r >= b) {
    return r;
  } else if (g >= b) {
    return g;
  } else {
    return b;
  }
}

static inline float px_intensity_f(pixel p) {
  float r = px_red(p) / (float) CHANNEL_MAX;
  float g = px_green(p) / (float) CHANNEL_MAX;
  float b = px_blue(p) / (float) CHANNEL_MAX;
  return (r + g + b) / 3.0;
}

static inline float px_lightness_f(pixel p) {
  float r = px_red(p) / (float) CHANNEL_MAX;
  float g = px_green(p) / (float) CHANNEL_MAX;
  float b = px_blue(p) / (float) CHANNEL_MAX;
  float max = 0, min = CHANNEL_MAX;
  if (r > max) { max = r; }
  if (g > max) { max = g; }
  if (b > max) { max = b; }
  if (r < min) { min = r; }
  if (g < min) { min = g; }
  if (b < min) { min = b; }
  return (max + min) / 2.0;
}

static inline float px_luma_f(pixel p) {
  float r = px_red(p) / (float) CHANNEL_MAX;
  float g = px_green(p) / (float) CHANNEL_MAX;
  float b = px_blue(p) / (float) CHANNEL_MAX;
  return 0.30 * r + 0.59 * g + 0.11 * b;
}


static inline void px_set_red(pixel *p, channel r) {
  *p &= ~RED_MASK;
  *p |= ((pixel) r) << RED_SHIFT;
}
static inline void px_set_hue(pixel *p, channel h) {
  *p &= ~RED_MASK;
  *p |= ((pixel) h) << RED_SHIFT;
}

static inline void px_set_green(pixel *p, channel g) {
  *p &= ~GREEN_MASK;
  *p |= ((pixel) g) << GREEN_SHIFT;
}
static inline void px_set_sat(pixel *p, channel s) {
  *p &= ~GREEN_MASK;
  *p |= ((pixel) s) << GREEN_SHIFT;
}

static inline void px_set_blue(pixel *p, channel b) {
  *p &= ~BLUE_MASK;
  *p |= ((pixel) b) << BLUE_SHIFT;
}
static inline void px_set_val(pixel *p, channel v) {
  *p &= ~BLUE_MASK;
  *p |= ((pixel) v) << BLUE_SHIFT;
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


// Takes RGBA floating point values in [0, 1] and returns a pixel.
static inline pixel float_color(float r, float g, float b, float a) {
  pixel result = PX_EMPTY;
  px_set_red(&result, fastfloor(CHANNEL_MAX * r));
  px_set_green(&result, fastfloor(CHANNEL_MAX * g));
  px_set_blue(&result, fastfloor(CHANNEL_MAX * b));
  px_set_alpha(&result, fastfloor(CHANNEL_MAX * a));
  return result;
}


// Takes an RGBA pixel and returns an HSVA pixel, using the red, green, and
// blue channels for hue, saturation, and value respectively:
static inline void rgb__hsv(pixel p, pixel *result) {
  *result = 0xff000000;
  float r = px_red(p) / (float) CHANNEL_MAX;
  float g = px_green(p) / (float) CHANNEL_MAX;
  float b = px_blue(p) / (float) CHANNEL_MAX;
  float max = 0, min = CHANNEL_MAX;
  float chroma = 0;
  if (r > max) { max = r; }
  if (g > max) { max = g; }
  if (b > max) { max = b; }
  if (r < min) { min = r; }
  if (g < min) { min = g; }
  if (b < min) { min = b; }
  chroma = max - min;

  // blue <-> saturation (first 'cause of the shortcut when max == 0)
  if (max == 0) {
    px_set_red(result, 0);
    px_set_blue(result, 0);
    px_set_green(result, 0);
    return;
  } else {
    px_set_blue(result, CHANNEL_MAX * (chroma / max));
  }

  // red <-> hue
  if (max == r) {
    px_set_red(
      result,
      CHANNEL_MAX * (
        (1/6.0) * (g - b) / chroma + (b > g ? 1 : 0)
      )
    );
  } else if (max == g) {
    px_set_red(
      result,
      CHANNEL_MAX * (
        (1/6.0) * (b - r) / chroma + (1/3.0)
      )
    );
  } else if (max == b) {
    px_set_red(
      result,
      CHANNEL_MAX * (
        (1/6.0) * (r - g) / chroma + (2/3.0)
      )
    );
  } else {
    px_set_red(
      result,
      CHANNEL_MAX * (
        0
      )
    );
  }

  // green <-> value
  if (r >= g && r >= b) {
    px_set_green(result, CHANNEL_MAX * r);
  } else if (g >= b) {
    px_set_green(result, CHANNEL_MAX * g);
  } else {
    px_set_green(result, CHANNEL_MAX * b);
  }
  return;
}

// Takes an HSVA pixel and returns an RGBA pixel:
static inline void hsv__rgb(pixel p, pixel *result) {
  *result = PX_BLACK;
  float h = px_red(p) / (float) CHANNEL_MAX;
  float s = px_green(p) / (float) CHANNEL_MAX;
  float v = px_blue(p) / (float) CHANNEL_MAX;
  int sector = floor(h / (1/6.0)); // which sector of the color wheel?
  float remainder = 6 * (h - (1/6.0)*sector); // [0, 1] position in sector
  // The three possible channel values (along with v):
  float c1 = v * (1 - s);
  float c2 = v * (1 - s * remainder);
  float c3 = v * (1 - s * (1 - remainder));

  // shortcut for gray:
  if (s == 0) {
    px_set_red(result, CHANNEL_MAX*v);
    px_set_green(result, CHANNEL_MAX*v);
    px_set_blue(result, CHANNEL_MAX*v);
    return;
  }
  switch (sector) {
    case 0:
      px_set_red(result, CHANNEL_MAX * v);
      px_set_green(result, CHANNEL_MAX * c3);
      px_set_blue(result, CHANNEL_MAX * c1);
      break;
    case 1:
      px_set_red(result, CHANNEL_MAX * c2);
      px_set_green(result, CHANNEL_MAX * v);
      px_set_blue(result, CHANNEL_MAX * c1);
      break;
    case 2:
      px_set_red(result, CHANNEL_MAX * c1);
      px_set_green(result, CHANNEL_MAX * v);
      px_set_blue(result, CHANNEL_MAX * c3);
      break;
    case 3:
      px_set_red(result, CHANNEL_MAX * c1);
      px_set_green(result, CHANNEL_MAX * c2);
      px_set_blue(result, CHANNEL_MAX * v);
      break;
    case 4:
      px_set_red(result, CHANNEL_MAX * c3);
      px_set_green(result, CHANNEL_MAX * c1);
      px_set_blue(result, CHANNEL_MAX * v);
      break;
    case 5:
    default:
      px_set_red(result, CHANNEL_MAX * v);
      px_set_green(result, CHANNEL_MAX * c1);
      px_set_blue(result, CHANNEL_MAX * c2);
      break;
  }
  return;
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
