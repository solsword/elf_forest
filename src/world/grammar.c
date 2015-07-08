// grammar.c
// Cell-space grammars.

#include "grammar.h"

#include "blocks.h"
#include "world.h"

#include "datatypes/list.h"

/******************************
 * Constructors & Destructors *
 ******************************/

cell_grammar * create_cell_grammar(void) {
  return NULL;
}

void cleanup_cell_grammar(cell_grammar *cg) {
}

cg_expansion * create_cell_grammar_expansion(void) {
  return NULL;
}

void cleanup_cell_grammar_expansion(cg_expansion *cge) {
}

/*************
 * Functions *
 *************/

void build_grammar_from_string(string *definition) {
}

cg_expansion* pick_expansion(
  cell *cell_neighborhood[],
  cell_grammar *cg
) {
  return NULL;
}

int apply_expansion(
  cell *cell_neighborhood[],
  cg_expansion *cge
) {
  return 0;
}
