#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "sample_rngtable",   .function = &efd_fn_sample_rngtable     },
#else
#ifndef INCLUDE_EFD_FUNC_RNGTABLE_H
#define INCLUDE_EFD_FUNC_RNGTABLE_H
// efd_rngtable.h
// Eval functions for rngtables (see also efd_rngtable.h in efd/conv/

#include "efd/efd.h"
#include "datatypes/rngtable.h"

// Calls rt_pick_result, using the first argument as a seed to pick a result
// from the second argument, which should be an Object node that contains an
// rngtable.
efd_node * efd_fn_sample_rngtable(
  efd_node const * const node,
  efd_value_cache *cache
) {
  efd_assert_return_type(node, EFD_NT_INTEGER);
  efd_int_t seed = *efd__i(efd_get_value(efd_nth(node, 0), cache));
  efd_node *obj = efd_get_value(efd_nth(node, 1), cache);
  rngtable *table = (rngtable*) obj->b.as_object.value;

  efd_int_t result = (efd_int_t) rt_pick_result(table, seed);

  return construct_efd_int_node(node->h.name, result);
}

#endif // INCLUDE_EFD_FUNC_RNGTABLE_H
#endif // EFD_REGISTRATION
