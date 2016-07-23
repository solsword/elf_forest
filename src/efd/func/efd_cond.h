#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "if_eq",        .function = &efd_fn_if_eq        },
//{ .key = "",             .function = &efd_fn_             },
#else
#ifndef INCLUDE_EFD_FUNC_COND_H
#define INCLUDE_EFD_FUNC_COND_H
// efd_cond.h
// Eval functions for conditionals

#include "efd/efd.h"

// Takes four arguments and compares the first two. If they are equivalent,
// takes on the value of its third argument, otherwise uses the value of it's
// final argument.
efd_node * efd_fn_if_eq(efd_node const * const node, efd_value_cache *cache) {
  efd_node *cmp, *agn, *yes, *no;

  if (efd_normal_child_count(node) < 3) {
    efd_report_error(
      s_("ERROR: 'if_eq' has wrong number of children (must be 4):"),
      node
    );
    exit(EXIT_FAILURE);
  }

  cmp = efd_get_value(efd_nth(node, 0), cache);
  agn = efd_get_value(efd_nth(node, 1), cache);

  yes = efd_get_value(efd_nth(node, 2), cache);
  no = efd_get_value(efd_nth(node, 3), cache);

  efd_assert_return_type(node, yes->h.type);
  efd_assert_return_type(node, no->h.type);

  if (efd_equivalent(cmp, agn)) {
    return efd_create_shadow_clone(yes);
  } else {
    return efd_create_shadow_clone(no);
  }
}

#endif // INCLUDE_EFD_FUNC_COND_H
#endif // EFD_REGISTRATION
