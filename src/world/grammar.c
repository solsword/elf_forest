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
}

void cleanup_cell_grammar(cell_grammar *cg) {
}

cell_grammar_expansion * create_cell_grammar_expansion(void) {
}

void cleanup_cell_grammar_expansion(cell_grammar_expansion *cge) {
}

/*************
 * Functions *
 *************/

void build_grammar_from_string(string *definition) {
}

cell_grammmar_expansion* pick_expansion(
  cell *cell_neighborhood[],
  cell_grammar *cg
) {
}

int apply_expansion(
  cell *cell_neighborhood[],
  cell_grammar_expansion *cge
) {
}
