// grammar.c
// Cell-space grammars.

#include "grammar.h"

#include "blocks.h"
#include "world.h"

#include "datatypes/list.h"

/********************
 * Helper Functions *
 ********************/

void _iter_cleanup_cell_grammar_expansions(void* vcge) {
  cg_expansion* cge = (cg_expansion*) vcge;
  cleanup_cell_grammar_expansion(cge);
}

/******************************
 * Constructors & Destructors *
 ******************************/

cell_grammar* create_cell_grammar(void) {
  cell_grammar* result = (cell_grammar*) malloc(sizeof(cell_grammar));
  result->expansions = create_list();
  return result;
}

void cleanup_cell_grammar(cell_grammar *cg) {
  l_foreach(cg->expansions, &_iter_cleanup_cell_grammar_expansions);
  cleanup_list(cg->expansions);
  free(cg);
}

cg_expansion* create_cell_grammar_expansion(void) {
  cg_expansion* result = (cg_expansion*) malloc(sizeof(cg_expansion));
  result->children = create_list();
  return result;
}

void cleanup_cell_grammar_expansion(cg_expansion *cge) {
  l_foreach(cge->children, &_iter_cleanup_cell_grammar_expansions);
  cleanup_list(cge->children);
  free(cge);
}

cg_expansion* copy_cell_grammar_expansion(cg_expansion *cge) {
  size_t i;
  cg_expansion* result = create_cell_grammar_expansion();
  result->type = cge->type;
  result->offset = cge->offset;
  result->cmp_mask = cge->cmp_mask;
  result->compare = cge->compare;
  result->rpl_mask = cge->rpl_mask;
  result->replace = cge->replace;
  result->type = cge->type;
  result->target = cge->target;
  for (i = 0; i < l_get_length(cge->children); ++i) {
    l_append_element(
      result->children,
      copy_cell_grammar_expansion(
        (cg_expansion*) l_get_item(cge->children, i)
      )
    );
  }
  return result;
}

/*************
 * Functions *
 *************/

int check_expansion(
  chunk_neighborhood* nbh,
  chunk_index root,
  cg_expansion *cge
) {
  switch (cge->type) {
    default:
    case CGCT_BLOCK_RELATIVE:
      // TODO: HERE
      //cge->target = cidx_add(
      break;
    case CGCT_BLOCK_EXACT:
      break;
    case CGCT_BLOCK_EITHER:
      break;
    case CGCT_LOGICAL_AND:
      break;
    case CGCT_LOGICAL_OR:
      break;
    case CGCT_LOGICAL_NOT:
      break;
  }
  return 1;
}

cg_expansion* pick_expansion(
  chunk_neighborhood* nbh,
  chunk_index root,
  cell_grammar *cg
) {
  return NULL;
}

int apply_expansion(
  chunk_neighborhood* nbh,
  chunk_index root,
  cg_expansion *cge
) {
  return 0;
}

void build_grammar_from_string(string *definition) {
}
