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
enum cg_expansion_type_e {
  CGCT_BLOCK_RELATIVE,
  CGCT_BLOCK_EXACT,
  CGCT_BLOCK_EITHER,
  CGCT_LOGICAL_AND,
  CGCT_LOGICAL_OR,
  CGCT_LOGICAL_NOT
};
typedef enum cg_expansion_type_e cg_expansion_type;

/**************
 * Structures *
 **************/

// A grammar in cell-space.
struct cell_grammar_s;
typedef struct cell_grammar_s cell_grammar;

// An individual expansion rule within a cell-space grammar.
struct cg_expansion_s;
typedef struct cg_expansion_s cg_expansion;

/*************************
 * Structure Definitions *
 *************************/

struct cell_grammar_s {
  list* expansions;
};

struct cg_expansion_s {
  cg_expansion_type type;
  // For all expansions:
  chunk_index offset;
  // For BLOCK-type expansions:
  block cmp_mask;
  block compare;
  block rpl_mask;
  block replace;
  // For LOGICAL-type expansions:
  list* children;
  // For use during expansion:
  block* target;
};

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates a new cell grammar.
cell_grammar* create_cell_grammar(void);

// Cleans up memory allocated for the given cell grammar, including its
// expansions.
void cleanup_cell_grammar(cell_grammar *cg);

// Allocates a new cell grammar expansion.
cg_expansion* create_cell_grammar_expansion(void);

// Cleans up the memory associated with a cell grammar expansion.
void cleanup_cell_grammar_expansion(cg_expansion *cge);

// Allocates and returns a new copy of the given cell grammar expansion.
// Recursively copies any children of the original as well, so it's a deep copy
// operation. Note that current block target information is copied as well.
cg_expansion* copy_cell_grammar_expansion(cg_expansion *cge);

/*************
 * Functions *
 *************/

// Checks whether the given expansion fits at the given location. As it does so
// it sets target blocks for the members of that expansion tree. It returns 1
// on success and 0 on failure.
int check_expansion(
  chunk_neighborhood* nbh,
  chunk_index root,
  cg_expansion *cge
);

// TODO: Comment this
cg_expansion* pick_expansion(
  chunk_neighborhood* nbh,
  chunk_index root,
  cell_grammar *cg
);

// TODO: Comment this
int apply_expansion(
  chunk_neighborhood* nbh,
  chunk_index root,
  cg_expansion *cge
);

// TODO: Comment this
void build_grammar_from_string(string *definition);

#endif // ifndef GRAMMAR_H
