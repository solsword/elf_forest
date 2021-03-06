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
  CGET_BLOCK_RELATIVE,
  CGET_BLOCK_EXACT,
  CGET_BLOCK_EITHER,
  CGET_LOGICAL_AND,
  CGET_LOGICAL_OR,
  CGET_LOGICAL_NOT
};
typedef enum cg_expansion_type_e cg_expansion_type;

// Different comparison strategies.
enum cg_comparison_strategy_e {
  CGCS_EXACT,
  CGCS_CAN_GROW,
  CGCS_CANT_GROW,
  CGCS_BLOCKS_GROWTH,
  CGCS_ROOT,
  CGCS_ROOT_ID,
  CGCS_ROOT_SPECIES,
  CGCS_ROOT_CAN_GROW,
  CGCS_ROOT_CANT_GROW,
  CGCS_BLOCK_INFO
};
typedef enum cg_comparison_strategy_e cg_comparison_strategy;

// Different replacement strategies.
enum cg_replacement_strategy_e {
  CGRS_NONE,
  CGRS_EXACT,
  CGRS_ROOT,
  CGRS_ROOT_SPECIES
};
typedef enum cg_replacement_strategy_e cg_replacement_strategy;

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
  block_index offset;

  // For BLOCK-type expansions:
  cg_comparison_strategy cmp_strategy;
  block cmp_mask;
  block compare;

  cg_replacement_strategy rpl_strategy;
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

// Adds the given expansion to the given grammar.
void cg_add_expansion(cell_grammar *cg, cg_expansion *cge);

// Adds the given expansion to the children list of the given parent.
void cge_add_child(cg_expansion *parent, cg_expansion *child);

// Checks whether the given expansion fits at the given location. As it does so
// it sets target blocks for the members of that expansion tree. It returns 1
// on success and 0 on failure.
int check_expansion(
  cg_expansion *cge,
  chunk_neighborhood* nbh,
  block_index base,
  block root_block
);

// Chooses a random expansion from the given grammar that's valid at the given
// location. Calls check_expansion to do so, so the returned expansion has its
// target blocks bound at that location.
cg_expansion* pick_expansion(
  cell_grammar *cg,
  chunk_neighborhood* nbh,
  block_index base,
  ptrdiff_t seed
);

// Takes a bound grammar expansion and a root block and applies the expansion.
// This will cause a segfault if any member of the grammar is not bound to a
// block.
void apply_expansion(cg_expansion *cge, block root_block);

// TODO: Comment this
void build_grammar_from_string(string *definition);

#endif // ifndef GRAMMAR_H
