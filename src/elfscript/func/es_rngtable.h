#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(ELFSCRIPT_REGISTER_FUNCTIONS)
{ .key = "sample_rngtable",   .function = &elfscript_fn_sample_rngtable     },
#else
#ifndef INCLUDE_ELFSCRIPT_FUNC_RNGTABLE_H
#define INCLUDE_ELFSCRIPT_FUNC_RNGTABLE_H
// elfscript_rngtable.h
// Eval functions for rngtables (see also elfscript_rngtable.h in elfscript/conv/

#include "elfscript/elfscript.h"
#include "datatypes/rngtable.h"

// Calls rt_pick_result, using the first argument as a seed to pick a result
// from the second argument, which should be an Object node that contains an
// rngtable.
elfscript_node * elfscript_fn_sample_rngtable(elfscript_node const * const node) {
  SSTR(fmt_rngtable, "rngtable", 8);
  elfscript_int_t seed, result;
  elfscript_node *obj;
  rngtable *table;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_INTEGER);

  seed = elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0)));
  obj = elfscript_get_value(elfscript_nth(node, 1));
  table = (rngtable*) elfscript_as_o_fmt(obj, fmt_rngtable);

  result = (elfscript_int_t) rt_pick_result(table, seed);

  return construct_elfscript_int_node(node->h.name, node, result);
}

#endif // INCLUDE_ELFSCRIPT_FUNC_RNGTABLE_H
#endif // ELFSCRIPT_REGISTRATION
