#if defined(EFD_REGISTER_DECLARATIONS)
// Nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "if",                 .function = &efd_fn_if            },
{ .key = "filter_keep",        .function = &efd_fn_filter_keep   },
#else
#ifndef INCLUDE_EFD_FUNC_COND_H
#define INCLUDE_EFD_FUNC_COND_H
// efd_cond.h
// Eval functions for conditionals

#include "efd/efd.h"

// Takes four arguments and interprets the first as a condition to be applied
// to the second. If the condition holds, takes on the value of its third
// argument, otherwise uses the value of its final argument.
efd_node * efd_fn_if(efd_node const * const node, efd_value_cache *cache) {
  efd_node *cond, *arg, *yes, *no;

  efd_assert_child_count(node, 4, 4);

  cond = efd_get_value(efd_nth(node, 0), cache);
  arg = efd_get_value(efd_nth(node, 1), cache);

  yes = efd_get_value(efd_nth(node, 2), cache);
  no = efd_get_value(efd_nth(node, 3), cache);

  efd_assert_return_type(node, yes->h.type);
  efd_assert_return_type(node, no->h.type);

  if (efd_condition_holds(cond, arg, cache)) {
    return copy_efd_node(yes);
  } else {
    return copy_efd_node(no);
  }
}

// Takes two nodes, a condition, and a generatable. Applies the condition to
// every generatable item, resulting in a new container consisting of just the
// items that passed the condition.
efd_node * efd_fn_filter_keep(
  efd_node const * const node,
  efd_value_cache *cache
) {
  efd_node *cond, *container, *val, *result;
  size_t i, count;
  efd_generator_state *gen;

  efd_assert_child_count(node, 2, 2);

  cond = efd_get_value(efd_nth(node, 0), cache);

  gen = efd_generator_for(efd_get_value(efd_nth(node, 1), cache), cache);
  container = efd_gen_all(gen);

  result = create_efd_node(EFD_NT_CONTAINER, node->h.name, node);

  count = efd_normal_child_count(container);
  for (i = 0; i < count; ++i) {
    val = efd_get_value(efd_nth(container, i), cache);
    if (efd_condition_holds(cond, val, cache)) {
      efd_add_child(result, val);
    }
  }

  return result;
}

#endif // INCLUDE_EFD_FUNC_COND_H
#endif // EFD_REGISTRATION
