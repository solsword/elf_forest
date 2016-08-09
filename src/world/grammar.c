// grammar.c
// Cell-space grammars.

#include "grammar.h"

#include "blocks.h"
#include "world.h"

#include "datatypes/list.h"

#include "gen/biology.h"

/********************
 * Helper Functions *
 ********************/

void _iter_cleanup_cell_grammar_expansions(void* vcge) {
  cg_expansion* cge = (cg_expansion*) vcge;
  cleanup_cell_grammar_expansion(cge);
}

/********************
 * Helper Functions *
 ********************/

static inline int _check_block(
  cg_comparison_strategy strategy,
  block b,
  block compare,
  block mask
) {
  ptrdiff_t resist, strength;
  if (strategy == CGCS_CAN_GROW || strategy == CGCS_ROOT_CAN_GROW) {
    resist = get_species_growth_resist(b);
    strength = get_species_growth_strength(compare);
    return resist < strength;
  } else if (strategy == CGCS_CANT_GROW || strategy == CGCS_ROOT_CANT_GROW) {
    resist = get_species_growth_resist(b);
    strength = get_species_growth_strength(compare);
    return resist >= strength;
  } else if (strategy == CGCS_BLOCKS_GROWTH) {
    return get_species_growth_resist(b) >= compare;
  } else if (strategy == CGCS_BLOCK_INFO) {
    return (b_info(b) & mask) == (block_info) (compare & mask);
  } else {
    return (b & mask) == (compare & mask);
  }
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

void cg_add_expansion(cell_grammar *cg, cg_expansion *cge) {
  l_append_element(cg->expansions, (void*) cge);
}

void cge_add_child(cg_expansion *parent, cg_expansion *child) {
  l_append_element(parent->children, (void*) child);
}

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
  block compare;
  switch (cge->cmp_strategy) {
    default:
    case CGCS_EXACT:
    case CGCS_CAN_GROW:
    case CGCS_CANT_GROW:
    case CGCS_BLOCKS_GROWTH:
    case CGCS_BLOCK_INFO:
      compare = cge->compare;
      break;
    case CGCS_ROOT:
    case CGCS_ROOT_CAN_GROW:
    case CGCS_ROOT_CANT_GROW:
      compare = root_block;
      break;
    case CGCS_ROOT_ID:
      compare = cge->compare;
      compare &= ~BM_ID;
      compare |= (root_block & BM_ID);
      break;
    case CGCS_ROOT_SPECIES:
      compare = cge->compare;
      compare &= ~BM_SPECIES;
      compare |= (root_block & BM_SPECIES);
      break;
  }
  switch (cge->type) {
    default:
    case CGET_BLOCK_RELATIVE:
      cge->target = nb_block(nbh, cidx_add(base, cge->offset));
      b = *(cge->target);
      return _check_block(cge->cmp_strategy, b, compare, cge->cmp_mask);
    case CGET_BLOCK_EXACT:
      cge->target = &(
        nb_cell(
          nbh,
          cidx_add(base, cge->offset)
        )->blocks[cge->offset.xyz.w]
      );
      b = *(cge->target);
      return _check_block(cge->cmp_strategy, b, compare, cge->cmp_mask);
    case CGET_BLOCK_EITHER:
      cl = nb_cell(nbh, cidx_add(base, cge->offset));
      // Try the first block:
      cge->target = &(cl->blocks[cge->offset.xyz.w]);
      b = *(cge->target);
      if ((b & cge->cmp_mask) == compare) {
        return 1;
      }
      if (_check_block(cge->cmp_strategy, b, compare, cge->cmp_mask)) {
        return 1;
      }
      // Try the other block:
      cge->target = &(cl->blocks[cge->offset.xyz.w ^ 1]);
      b = *(cge->target);
      return _check_block(cge->cmp_strategy, b, compare, cge->cmp_mask);
    case CGET_LOGICAL_AND:
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
    case CGET_LOGICAL_OR:
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
    case CGET_LOGICAL_NOT:
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
  cell_grammar *cg,
  chunk_neighborhood* nbh,
  block_index base,
  ptrdiff_t seed
) {
  size_t i;
  int success;
  cg_expansion *exp;
  list *options = create_list();
  block root_block = *nb_block(nbh, base);
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
  block replace = cge->replace;
  cg_expansion* child;
  switch (cge->type) {
    default:
    case CGET_BLOCK_RELATIVE:
    case CGET_BLOCK_EXACT:
    case CGET_BLOCK_EITHER:
      if (cge->rpl_strategy == CGRS_NONE) {
        break;
      } else if (cge->rpl_strategy == CGRS_ROOT) {
        replace = root_block;
      } else if (cge->rpl_strategy == CGRS_ROOT_SPECIES) {
        replace &= ~BM_SPECIES;
        replace |= (root_block & BM_SPECIES);
      }
      *(cge->target) &= ~(cge->rpl_mask);
      *(cge->target) |= (cge->rpl_mask & cge->replace);
      break;
    case CGET_LOGICAL_AND:
      for(i = 0; i < l_get_length(cge->children); ++i) {
        apply_expansion(
          (cg_expansion*) l_get_item(cge->children, i),
          root_block
        );
      }
      break;
    case CGET_LOGICAL_OR:
      for(i = 0; i < l_get_length(cge->children); ++i) {
        child = (cg_expansion*) l_get_item(cge->children, i);
        if (child->target != NULL) {
          apply_expansion(child, root_block);
          break;
        }
      }
      break;
    case CGET_LOGICAL_NOT:
      break;
  }
}

void build_grammar_from_string(string *definition) {
}
