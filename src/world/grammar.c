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
  cg_expansion *cge,
  chunk_neighborhood* nbh,
  block_index base,
  block root_block
) {
  cell *cl;
  size_t i;
  int subresult;
  block_index oidx;
  cg_expansion* child;
  block b;
  if (cge->check_relative) {
    cge->compare = root_block;
  }
  switch (cge->type) {
    default:
    case CGCT_BLOCK_RELATIVE:
      cge->target = nb_block(nbh, cidx_add(base, cge->offset));
      b = *(cge->target);
      if ((b & cge->cmp_mask) == cge->compare) {
        return 1;
      } else {
        return 0;
      }
    case CGCT_BLOCK_EXACT:
      cge->target = &(
        nb_cell(
          nbh,
          cidx_add(base, cge->offset)
        )->blocks[cge->offset.xyz.w]
      );
      b = *(cge->target);
      if ((b & cge->cmp_mask) == cge->compare) {
        return 1;
      } else {
        return 0;
      }
    case CGCT_BLOCK_EITHER:
      cl = nb_cell(nbh, cidx_add(base, cge->offset));
      // Try the first block:
      cge->target = &(cl->blocks[cge->offset.xyz.w]);
      b = *(cge->target);
      if ((b & cge->cmp_mask) == cge->compare) {
        return 1;
      }
      // Try the other block:
      cge->target = &(cl->blocks[cge->offset.xyz.w ^ 1]);
      b = *(cge->target);
      if ((b & cge->cmp_mask) == cge->compare) {
        return 1;
      } else {
        return 0;
      }
    case CGCT_LOGICAL_AND:
      oidx = cidx_add(base, cge->offset);
      cge->target = nb_block(nbh, oidx);
      for(i = 0; i < l_get_length(cge->children); ++i) {
        subresult = check_expansion(
          (cg_expansion*) l_get_item(cge->children, i),
          nbh,
          oidx,
          root_block
        );
        if (!subresult) { return 0; }
      }
      return 1;
    case CGCT_LOGICAL_OR:
      oidx = cidx_add(base, cge->offset);
      cge->target = nb_block(nbh, oidx);
      for(i = 0; i < l_get_length(cge->children); ++i) {
        child = (cg_expansion*) l_get_item(cge->children, i);
        subresult = check_expansion(
          child,
          nbh,
          oidx,
          root_block
        );
        if (subresult) {
          return 1;
        } else {
          child->target = NULL; // mark this child as failed
        }
      }
      return 0;
    case CGCT_LOGICAL_NOT:
      oidx = cidx_add(base, cge->offset);
      cge->target = nb_block(nbh, oidx);
      for(i = 0; i < l_get_length(cge->children); ++i) {
        child = (cg_expansion*) l_get_item(cge->children, i);
        subresult = check_expansion(
          child,
          nbh,
          oidx,
          root_block
        );
        if (subresult) { return 0; }
      }
      return 1;
  }
#ifdef DEBUG
  return 0; // Should be unreachable
#endif
}

cg_expansion* pick_expansion(
  chunk_neighborhood* nbh,
  block_index base,
  cell_grammar *cg,
  ptrdiff_t seed
) {
  size_t i;
  int success;
  cg_expansion *exp;
  list *options = create_list();
  block root_block = nb_block(nbh, base);
  for (i = 0; i < l_get_length(cg->expansions); ++i) {
    exp = (cg_expansion*) l_get_item(cg->expansions, i);
    success = check_expansion(exp, nbh, base, root_block);
    if (success) {
      l_append_element(options, (void*) exp);
    }
  }
  if (l_is_empty(options)) {
    cleanup_list(options);
    return NULL;
  }
  // Choose an option based on our seed:
  i = posmod(prng(seed + 819991), l_get_length(options));
  exp = (cg_expansion*) l_get_item(options, i);
  cleanup_list(options);
  return exp;
}

void apply_expansion(cg_expansion *cge, block root_block) {
  size_t i;
  cg_expansion* child;
  switch (cge->type) {
    default:
    case CGCT_BLOCK_RELATIVE:
    case CGCT_BLOCK_EXACT:
    case CGCT_BLOCK_EITHER:
      if (cge->replace_relative) {
        cge->replace = root_block;
      }
      *(cge->target) &= ~(cge->rpl_mask);
      *(cge->target) |= (cge->rpl_mask & cge->replace);
      break;
    case CGCT_LOGICAL_AND:
      for(i = 0; i < l_get_length(cge->children); ++i) {
        apply_expansion((cg_expansion*) l_get_item(cge->children, i));
      }
      break;
    case CGCT_LOGICAL_OR:
      for(i = 0; i < l_get_length(cge->children); ++i) {
        child = (cg_expansion*) l_get_item(cge->children, i);
        if (child->target != NULL) {
          apply_expansion(child);
          break;
        }
      }
      break;
    case CGCT_LOGICAL_NOT:
      break;
  }
}

void build_grammar_from_string(string *definition) {
}
