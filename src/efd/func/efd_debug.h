#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "DEBUG",         .function = &efd_fn_debug         },
#else
#ifndef INCLUDE_EFD_FUNC_DEBUG_H
#define INCLUDE_EFD_FUNC_DEBUG_H
// efd_eval.h
// Eval functions for recursive evaluation

#include <stdio.h>

#include "efd/efd.h"

// TODO: Make this actually useful! (print at parsing time?)

// Prints debugging information when evaluated and returns NULL.
efd_node * efd_fn_debug(efd_node const * const node, efd_value_cache *cache) {
  fprintf(stderr, "EFD DEBUG:\n");
  s_fprintln(stderr, efd_repr(node));
  return NULL;
}

#endif // INCLUDE_EFD_FUNC_DEBUG_H
#endif // EFD_REGISTRATION
