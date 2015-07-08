#ifndef GRAMMAR_H
#define GRAMMAR_H

// grammar.c
// Cell-space grammars.

#include "blocks.h"
#include "world.h"

#include "datatypes/list.h"

/****************
 * Enumerations *
 ****************/

// Different expansion types.
enum expansion_type_e {
  ET_PRECISE,
  ET_FUZZY,
};
typedef enum expansion_type_e expansion_type;

/**************
 * Structures *
 **************/

// A grammar in cell-space.
struct cell_grammar_s;
typedef struct cell_grammar_s cell_grammar;

// An individual expansion rule within a cell-space grammar.
struct cell_grammar_expansion_s;
typedef struct cell_grammar_expansion_s cell_grammar_expansion;

/*************************
 * Structure Definitions *
 *************************/

struct cell_grammar_s {
};

struct cell_grammar_expansion_s {
};

/********************
 * Inline Functions *
 ********************/

static inline ptrdiff_t i_dont_know(void) {
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates a new cell grammar.
cell_grammar * create_cell_grammar(void);

// Cleans up memory allocated for the given cell grammar, including its
// expansions.
void cleanup_cell_grammar(cell_grammar *cg);

// Allocates a new cell grammar expansion.
cell_grammar_expansion * create_cell_grammar_expansion(void);

// Cleans up the memory associated with a cell grammar expansion.
void cleanup_cell_grammar_expansion(cell_grammar_expansion *cge);

/*************
 * Functions *
 *************/

// TODO: Comment this
void build_grammar_from_string(string *definition);

// TODO: Comment this
cell_grammmar_expansion* pick_expansion(
  cell *cell_neighborhood[],
  cell_grammar *cg
);

// TODO: Comment this
int apply_expansion(
  cell *cell_neighborhood[],
  cell_grammar_expansion *cge
);

#endif // ifndef GRAMMAR_H
