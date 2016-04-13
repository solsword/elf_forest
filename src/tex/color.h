#ifndef COLOR_H
#define COLOR_H

// color.h
// Color manipulation and format conversion.

#include <stdint.h>

#include "util.h"

/*********
 * Enums *
 *********/

enum {
  CFMT_INVALID = 0,
  CFMT_RGB,
  CFMT_XYZ,
  CFMT_LAB,
  CFMT_LCH
} color_format_e;
typedef enum color_format_e color_format;

/*********
 * Types *
 *********/

typedef uint32_t pixel; // Pixel data packed into 32 bits
typedef uint8_t channel; // A single channel from pixel data

/**************
 * Structures *
 **************/

// A precisely-represented color using 3 floats in the CIE 1931 XYZ color space
// (no alpha). Can also be used to represent CIE-L*a*b* colors (x=L, y=a, z=b).
struct precise_color_s;
typedef struct precise_color_s precise_color;

/*************************
 * Structure Definitions *
 *************************/

struct precise_color_s {
  float x, y, z;
  float alpha;
};

/*************
 * Constants *
 *************/

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

// An empty (completely transparent) pixel value:
#define PX_EMPTY 0x00000000

// A black opaque pixel value:
#define PX_BLACK 0xff000000

// A white opaque pixel value:
#define PX_WHITE 0xffffffff

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

// Takes two pixels and a floating point interpolant and returns a color
// between the two.
static inline pixel px_interp(pixel from, pixel to, float interp) {
  pixel result = 0xff000000;
  px_set_red(&result, interp * px_red(to) + (1 - interp) * px_red(from));
  px_set_green(&result, interp * px_green(to) + (1 - interp) * px_green(from));
  px_set_blue(&result, interp * px_blue(to) + (1 - interp) * px_blue(from));
  px_set_alpha(&result, interp * px_alpha(to) + (1 - interp) * px_alpha(from));
  return result;
}

/*************
 * Functions *
 *************/

// Takes RGBA floating point values in [0, 1] and returns a pixel. Out-of-range
// values are clamped.
pixel float_color(float r, float g, float b, float a);

// Format conversion functions:

// Takes an RGBA pixel and returns an HSVA pixel, using the red, green, and
// blue channels for hue, saturation, and value respectively.
pixel rgb__hsv(pixel p);

// Takes an HSVA pixel and returns an RGBA pixel.
pixel hsv__rgb(pixel p);

// Converts to XYZ colorspace using a 2-degree observer and a D-65 illuminant.
// Code modified from: http://www.easyrgb.com/index.php?X=MATH&H=02#text2
void rgb__xyz(pixel p, precise_color *result);
pixel xyz__rgb(precise_color* color);

// Conversions between XYZ and CIE L*a*b* color space for perceptually linear
// color transformations. These functions modify the given color in-place.
// Code modified from the same place as above.
void xyz__lab(precise_color *color);
void lab__xyz(precise_color *color);

// Conversions between L*a*b* and L*c*h* color spaces for even better color
// gradients. Same code source as above. Note that these functions cheat: our
// internal L*c*h* representations uses radians for h instead of degrees to
// avoid pointless conversion math. Don't be surprised if h* values don't line
// up with results from other sources.
void lab__lch(precise_color *color);
void lch__lab(precise_color *color);

// Takes two RGB pixels and blends them precisely using:
//   blend * a + (1 - blend) * b
// in CIE L*c*h* color space.
pixel blend_precisely(pixel a, pixel b, float blend);

#endif //ifndef COLOR_H
