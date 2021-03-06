#ifndef TXGEN_H
#define TXGEN_H

// txgen.h
// Texture generation.

#include <stdint.h>

#include "tex/tex.h"

/************************
 * Types and Structures *
 ************************/

// A tx_grammar_literal is part of a texture grammar: it has a filename to load
// data from and possibly some children which it will expand into that file
// based on certain special pixel values (the GRAMMAR_KEYS array). It may also
// have pre- and/or post-processing functions, and it specifies an anchor
// location for use when it is expanded into a parent element. Its children are
// all grammar disjunction elements.
struct tx_grammar_literal_s;
typedef struct tx_grammar_literal_s tx_grammar_literal;

// A tx_grammar_disjunction represents a random choice between one of several
// possible grammar literals. Each disjunction is actually part of a
// singly-linked list, with each element storing a weight. Note that the
// weights shouldn't be too different from each other: any disjunction whose
// weight is less than 1/65536th of the total weight of disjunctions in its
// list will never be selected.
struct tx_grammar_disjunction_s;
typedef struct tx_grammar_disjunction_s tx_grammar_disjunction;

// A texture filter takes a texture and an additional argument and filters the
// texture somehow (one hopes).
typedef void (*texture_filter)(texture *, void const * const);

// A structure for remembering where we want to do a grammar expansion:
struct tx_grammar_expansion_site_s;
typedef struct tx_grammar_expansion_site_s tx_grammar_expansion_site;

// A gradient map is a series of up to GRADIENT_MAP_MAX_SIZE colors, each with
// its own cutoff point. It can be applied to an image, transforming each pixel
// into one of the gradient colors based on its luma value.
struct gradient_map_s;
typedef struct gradient_map_s gradient_map;

// A gradient is simpler than a gradient map: it just contains up to
// GRADIENT_MAX_SIZE colors, and implicitly maps them evenly to the range
// [0, 1]. It also has colors for out-of-bounds both below and above the
// gradient.
struct gradient_s;
typedef struct gradient_s gradient;

/******************************
 * Filter Argument Structures *
 ******************************/

// Arguments to the scatter texture filter:
// x-frequency, y-frequency, and color.
struct scatter_filter_args_s;
typedef struct scatter_filter_args_s scatter_filter_args;

// Arguments to the worley texture filter:
// frequency, and a color-map (which may be NULL).
struct worley_filter_args_s;
typedef struct worley_filter_args_s worley_filter_args;

/*************
 * Constants *
 *************/

// A table of special pixel values used for grammar expansion:
#define N_GRAMMAR_KEYS 6
extern pixel const GRAMMAR_KEYS[N_GRAMMAR_KEYS];
// For use in static contexts, we define macros for the grammar keys:
#define GRAMMAR_KEY_0 0xff0000fe
#define GRAMMAR_KEY_1 0xff00fffe
#define GRAMMAR_KEY_2 0xfffe0000
#define GRAMMAR_KEY_3 0xff8080fe
#define GRAMMAR_KEY_4 0xff80fffe
#define GRAMMAR_KEY_5 0xfffe8080

#define GRADIENT_MAP_MAX_SIZE 32

#define GRADIENT_MAX_SIZE 64

/*************************
 * Structure Definitions *
 *************************/

struct tx_grammar_literal_s {
  char const *filename;
  size_t anchor_x, anchor_y;
  texture_filter preprocess, postprocess;
  void * preargs, * postargs;
  tx_grammar_disjunction * children[N_GRAMMAR_KEYS];
  texture *result;
};

struct tx_grammar_disjunction_s {
  tx_grammar_disjunction *next;
  tx_grammar_literal *literal;
  float weight;
};

struct tx_grammar_expansion_site_s {
  size_t col, row;
  tx_grammar_literal *literal;
};

struct gradient_map_s {
  pixel colors[GRADIENT_MAP_MAX_SIZE];
  float thresholds[GRADIENT_MAP_MAX_SIZE];
};

struct gradient_s {
  size_t count;
  pixel colors[GRADIENT_MAX_SIZE];
  pixel oob_below;
  pixel oob_above;
};

struct scatter_filter_args_s {
  size_t seed;
  size_t x_freq, y_freq;
  pixel color;
};

struct worley_filter_args_s {
  float freq;
  size_t seed;
  gradient_map *grmap;
};

/**********
 * Macros *
 **********/

// A literal that just reads from a file:
#define SIMPLE_TX_LITERAL(FILENAME, ANCHOR_X, ANCHOR_Y) \
  { \
    .filename = FILENAME, \
    .anchor_x = ANCHOR_X, \
    .anchor_y = ANCHOR_Y, \
    .preprocess = NULL, .preargs = NULL, \
    .postprocess = NULL, .postargs = NULL, \
    .children = { NULL, NULL, NULL, NULL, NULL, NULL, }, \
    .result = NULL \
  }

// A literal that just runs a filter:
#define FILTER_TX_LITERAL(FILTER, ARGS, SIZE_X, SIZE_Y) \
  { \
    .filename = NULL, \
    .anchor_x = SIZE_X, \
    .anchor_y = SIZE_Y, \
    .preprocess = &FILTER, \
      .preargs = (void *) (&ARGS), \
    .postprocess = NULL, \
      .postargs = NULL, \
    .children = { \
      NULL, \
      NULL, \
      NULL, \
      NULL, \
      NULL, \
      NULL, \
    }, \
    .result = NULL \
  }

// A choice between a set of literals:
#define TX_ANY(LITERAL, N) any_ ## LITERAL ## _ ## N
#define TX_ANY_ENTRY(LITERAL, N, NXT, WEIGHT) \
  tx_grammar_disjunction TX_ANY(LITERAL, N) = { \
    .next = NXT, \
    .literal = &LITERAL ## _ ## N, \
    .weight = WEIGHT \
  }

/********************
 * Inline Functions *
 ********************/

// Returns an interpolation between the two nearest colors in the given
// gradient map.
static inline pixel gradient_map_result(
  gradient_map const * const grmap,
  float t
) {
  size_t i = 0, j;
  float f;
  for (i = 0; i < GRADIENT_MAP_MAX_SIZE - 1; ++i) {
    if (t <= grmap->thresholds[i]) {
      if (i > 0) {
        j = i - 1;
        f = (
          (t - grmap->thresholds[j])
        /
          (grmap->thresholds[i] - grmap->thresholds[j])
        );
        return px_interp(grmap->colors[j], grmap->colors[i], f);
      } else {
        return grmap->colors[i];
      }
    }
  }
  return grmap->colors[i];
}

// Returns the first color in the gradient map that has a threshold higher than
// the given value.
static inline pixel gradient_map_result_sharp(
  gradient_map const * const grmap,
  float t
) {
  size_t i = 0;
  for (i = 0; i < GRADIENT_MAP_MAX_SIZE - 1; ++i) {
    if (t <= grmap->thresholds[i]) {
      return grmap->colors[i];
    }
  }
  return grmap->colors[i];
}

// Returns a color from the given gradient.
static inline pixel gradient_result(gradient const * const gr, float t) {
  float f;
  size_t lower, upper;
  if (t < 0) {
    return gr->oob_below;
  } else if (t > 1) {
    return gr->oob_above;
  } else {
    lower = fastfloor((gr->count - 1) * t);
    upper = fastceil((gr->count - 1) * t);
    if (lower == upper) {
      upper = lower + 1;
    }
    if (upper > gr->count - 1) { // watch out for floating point error
      return gr->colors[lower];
    }
    f = ( ( (gr->count - 1) * t) - lower) / (upper - lower);
    return px_interp(gr->colors[lower], gr->colors[upper], f);
  }
}

// Tints the given pixel, darkening it if the given tint amount is less than 0
// and brightening it otherwise. The tint value given should be in [-1, 1].
static inline pixel px_tint(pixel p, float tint) {
  pixel hsv;
  hsv = rgb__hsv(p);
  float val = px_val(hsv) / (float) CHANNEL_MAX;
  if (tint < 0) {
    px_set_val(&hsv, CHANNEL_MAX * val * (1+tint));
  } else {
    px_set_val(&hsv, CHANNEL_MAX * (1 - (1 - val) * (1 - tint)));
  }
  return hsv__rgb(hsv);
}

// Relights the given pixel, lightening or darkening it by a set amount and
// clamping the result if it would be out-of-range. The given brightness value
// should be in [-1, 1];
static inline pixel px_relight(pixel p, float brightness) {
  pixel hsv;
  hsv = rgb__hsv(p);
  float val = px_val(hsv) / (float) CHANNEL_MAX;
  val += brightness;
  if (val < 0) { val = 0; } else if (val > 1) { val = 1; }
  px_set_val(&hsv, CHANNEL_MAX * val);
  return hsv__rgb(hsv);
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new grammar literal. Note that this constructor only
// allows the specification of two children: if a literal needs more children
// it should specify them manually after creation.
tx_grammar_literal * create_grammar_literal(
  char const * const fn,
  size_t ax, size_t ay,
  texture_filter prp, void *prargs,
  texture_filter psp, void *psargs,
  tx_grammar_disjunction *child0,
  tx_grammar_disjunction *child1
);

// Cleans up the data associated with the given literal, including recursively
// calling cleanup for its children. It does not free the filename or the pre-
// or post- args, as these are assumed to be non-heap data.
void cleanup_grammar_literal(tx_grammar_literal *lit);

// Allocates and returns a new grammar disjunction.
tx_grammar_disjunction * create_grammar_disjunction(
  tx_grammar_disjunction *nxt,
  tx_grammar_literal *lit,
  float w
);

// Cleans up all data associated with the given disjunction, including calling
// cleanup on its associated literal and the next disjunction in its chain.
void cleanup_grammar_disjunction(tx_grammar_disjunction *dis);

// Cleans up any allocated result textures associated with the given grammar
// and any of its children. Leaves the grammar literal itself untouched, as
// they are generally not allocated on the heap.
void cleanup_grammar_results(tx_grammar_literal *lit);

// Allocates and returns a new grammar expansion site:
tx_grammar_expansion_site * create_expansion_site(void);

// Cleans up a grammar expansion site:
void cleanup_expansion_site(tx_grammar_expansion_site *ges);

/*************
 * Functions *
 *************/
 
// Looks up and loads/generates the texture for the given block.
texture* gen_block_texture(block b);

// Takes a grammar literal and runs it, recursively picking children and
// running them before allocating and computing a result texture.
void run_grammar(tx_grammar_literal *lit);

// Calls the given function for every point on a jittered grid.
void ateach_scattered(
  size_t seed,
  size_t x_scale, // the basic spacing of the points in x/y
  size_t y_scale,
  int x_strength, // maximum offset of each point in x/y
  int y_strength,
  ptrdiff_t x_min, ptrdiff_t x_max, // bounding box of the area to iterate over
  ptrdiff_t y_min, ptrdiff_t y_max, // (min is inclusive, max is exclusive)
  void *arg, // extra argument to the function
  void (*f)(int, int, void *) // iteration function
);

/********************
 * Filter Functions *
 ********************/

// Takes a scatter_filter_args struct as args, and scatters pixels of the
// specified color over the target texture, with about the given frequencies.
// It can be used to scatter a grammar key color in a preprocessing filter,
// effectively scattering copies of that key's expansion.
void fltr_scatter(texture *tx, void const * const args);

// Applies the given gradient map to the texture.
void fltr_apply_gradient_map(texture *tx, void const * const args);

// Generates wrapped Worley noise across the entire texture.
void fltr_worley(texture *tx, void const * const args);

/*******************
 * Remix Functions *
 *******************/

static inline tx_grammar_literal * create_terminal_grammar_literal(
  char const * const fn,
  size_t ax, size_t ay
) {
  return create_grammar_literal(
    fn,
    ax, ay,
    NULL, NULL,
    NULL, NULL,
    NULL,
    NULL
  );
}

static inline tx_grammar_literal * create_filter_grammar_literal(
  size_t ax, size_t ay,
  texture_filter filter, void *fargs
) {
  return create_grammar_literal(
    NULL,
    ax, ay,
    filter, fargs,
    NULL, NULL,
    NULL,
    NULL
  );
}

#endif // ifndef TXGEN_H
