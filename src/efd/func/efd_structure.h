#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "choose",        .function = &efd_fn_choose },
#else
#ifndef INCLUDE_EFD_FUNC_STRUCTURE_H
#define INCLUDE_EFD_FUNC_STRUCTURE_H
// efd_eval.h
// Eval functions for recursive evaluation

#include "efd/efd.h"
#include "datatypes/dictionary.h"

// Uses the first argument as an index among the remaining arguments, returning
// the n+1st child.
efd_node * efd_fn_choose(efd_node const * const node, efd_value_cache *cache) {
  // TODO: HERE
}

#endif // INCLUDE_EFD_FUNC_STRUCTURE_H
#endif // EFD_REGISTRATION
