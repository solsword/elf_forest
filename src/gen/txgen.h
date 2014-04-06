#ifndef TXGEN_H
#define TXGEN_H

// txgen.h
// Texture generation.

#include <stdint.h>

#include "graphics/tex.h"

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

typedef void (*texture_filter)(texture *, void *);

// A structure for remembering where we want to do a grammar expansion:
struct tx_grammar_expansion_site_s;
typedef struct tx_grammar_expansion_site_s tx_grammar_expansion_site;

// A structure for holding arguments to the scatter texture filter:
// x-frequency, y-frequency, and color.
struct scatter_filter_args_s;
typedef struct scatter_filter_args_s scatter_filter_args;

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

struct scatter_filter_args_s {
  size_t x_freq, y_freq;
  pixel color;
};

/**********
 * Macros *
 **********/

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

/******************************
 * Constructors & Destructors *
 ******************************/

// Cleans up any allocated result textures associated with the given grammar
// and any of its children. Leaves the grammar literal itself untouched, as
// they are generally not allocated on the heap.
void cleanup_grammar(tx_grammar_literal *lit);

// Allocates and returns a new grammar expansion site:
tx_grammar_expansion_site * create_expansion_site(void);

// Cleans up a grammar expansion site:
void cleanup_expansion_site(tx_grammar_expansion_site *ges);

/*************
 * Functions *
 *************/

// Takes a grammar literal and runs it, recursively picking children and
// running them before allocating and computing a result texture.
void run_grammar(tx_grammar_literal *lit);

/********************
 * Filter Functions *
 ********************/

// Takes a scatter_filter_args struct as args, and scatters pixels of the
// specified color over the target texture, with about the given frequencies.
// It can be used to scatter a grammar key color in a preprocessing filter,
// effectively scattering copies of that key's expansion.
void fltr_scatter(texture *tx, void *args);

#endif // ifndef TXGEN_H
