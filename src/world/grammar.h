#ifndef GRAMMAR_H
#define GRAMMAR_H

// grammar.c
// Cell-space grammars.

#include "blocks.h"
#include "world.h"

#include "datatypes/list.h"
#include "datatypes/string.h"

/****************
 * Enumerations *
 ****************/

// Different expansion types.
enum cg_condition_type_e {
  CGCT_EXACT,
  CGCT_EITHER_BLOCK,
  CGCT_FUZZY
};
typedef enum cg_condition_type_e cg_condition_type;

/**************
 * Structures *
 **************/

// A grammar in cell-space.
struct cell_grammar_s;
typedef struct cell_grammar_s cell_grammar;

// An individual expansion rule within a cell-space grammar.
struct cg_expansion_s;
typedef struct cg_expansion_s cg_expansion;

// An individual condition of a grammar expansion rule.
struct cg_condition_s;
typedef struct cg_condition_s cg_condition;

/*************************
 * Structure Definitions *
 *************************/

struct cell_grammar_s {
};

struct cg_expansion_s {
};

/********************
 * Inline Functions *
 ********************/

static inline void i_dont_know(void) {
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
cg_expansion * create_cell_grammar_expansion(void);

// Cleans up the memory associated with a cell grammar expansion.
void cleanup_cell_grammar_expansion(cg_expansion *cge);

/*************
 * Functions *
 *************/

// TODO: Comment this
void build_grammar_from_string(string *definition);

// TODO: Comment this
cg_expansion* pick_expansion(
  cell *cell_neighborhood[],
  cell_grammar *cg
);

// TODO: Comment this
int apply_expansion(
  cell *cell_neighborhood[],
  cg_expansion *cge
);

#endif // ifndef GRAMMAR_H
