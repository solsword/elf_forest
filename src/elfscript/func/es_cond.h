#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// Nothing to declare
#elif defined(ELFSCRIPT_REGISTER_FUNCTIONS)
{ .key = "if",                 .function = &elfscript_fn_if            },
{ .key = "filter_keep",        .function = &elfscript_fn_filter_keep   },
#else
#ifndef INCLUDE_ELFSCRIPT_FUNC_COND_H
#define INCLUDE_ELFSCRIPT_FUNC_COND_H
// elfscript_cond.h
// Eval functions for conditionals

#include "elfscript/elfscript.h"

// Takes a condition function and creates an immediately-resolving promise
// object with .then and .else methods.
es_var * elfscript_fn_if(es_var * function) {

  if (elfscript_condition_holds(cond, arg)) {
    return copy_elfscript_node(yes);
  } else {
    return copy_elfscript_node(no);
  }
}

// Takes two nodes, a condition, and a generatable. Applies the condition to
// every generatable item, resulting in a new container consisting of just the
// items that passed the condition.
elfscript_node * elfscript_fn_filter_keep(elfscript_node const * const node) {
  elfscript_node *cond, *container, *val, *result;
  size_t i, count;
  elfscript_generator_state *gen;

  elfscript_assert_child_count(node, 2, 2);

  cond = elfscript_get_value(elfscript_nth(node, 0));

  gen = elfscript_generator_for(elfscript_get_value(elfscript_nth(node, 1)));
  container = elfscript_gen_all(gen);

  result = create_elfscript_node(ELFSCRIPT_NT_CONTAINER, node->h.name, node);

  count = elfscript_normal_child_count(container);
  for (i = 0; i < count; ++i) {
    val = elfscript_get_value(elfscript_nth(container, i));
    if (elfscript_condition_holds(cond, val)) {
      elfscript_add_child(result, val);
    }
  }

  return result;
}

#endif // INCLUDE_ELFSCRIPT_FUNC_COND_H
#endif // ELFSCRIPT_REGISTRATION
