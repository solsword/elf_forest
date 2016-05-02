#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "call",    .function = &efd_fn_call },
#else
#ifndef INCLUDE_EFD_CALL_H
#define INCLUDE_EFD_CALL_H
// efd_call.h
// Eval functions for recursive evaluation with arguments

#include "efd/efd.h"

// Evaluates the given node using a fresh environment, or if two arguments are
// given, using the second argument as an environment.
efd_node * efd_fn_call(efd_node const * const node) {
  intptr_t count = efd_normal_child_count(node);
  if (count == 1) {
    return efd_eval(efd_nth(node, 0), NULL);
  } else if (count == 2) {
    return efd_eval(efd_nth(node, 0), efd_nth(node, 1));
  } else { // invalid number of arguments
    efd_report_error(
      node,
      s_("Invalid number of arguments to 'count' (needs 1 or 2).")
    );
    return NULL;
  }
}

#endif // INCLUDE_EFD_CALL_H
#endif // EFD_REGISTRATION
