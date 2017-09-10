#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(ELFSCRIPT_REGISTER_FUNCTIONS)
{ .key = "DEBUG",         .function = &elfscript_fn_debug         },
#else
#ifndef INCLUDE_ELFSCRIPT_FUNC_DEBUG_H
#define INCLUDE_ELFSCRIPT_FUNC_DEBUG_H
// elfscript_eval.h
// Eval functions for recursive evaluation

#include <stdio.h>

#include "elfscript/elfscript.h"

// TODO: Make this actually useful! (print at parsing time?)

// Prints debugging information when evaluated and returns NULL.
es_val_t elfscript_fn_debug(elfscript_scope const * const scope) {
  fprintf(stderr, "ELFSCRIPT DEBUG:\n");
  // TODO: Print the whole scope here?
  s_fprintln(stderr, es_as_s(es_read_nth(scope, 0)));
  return NULL;
}

#endif // INCLUDE_ELFSCRIPT_FUNC_DEBUG_H
#endif // ELFSCRIPT_REGISTRATION
