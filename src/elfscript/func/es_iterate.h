#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(ELFSCRIPT_REGISTER_FUNCTIONS)
{ .key = "map",   .function = &elfscript_fn_map },
#else
#ifndef INCLUDE_ELFSCRIPT_FUNC_ITERATE_H
#define INCLUDE_ELFSCRIPT_FUNC_ITERATE_H
// elfscript_iterate.h
// Eval functions for iteration

// Takes a scope containing an iterable and a function to apply to elements
// from that iterable, and applies the function to each element in turn,
// returning a scope full of anonymous values containing the results.
es_val_t * elfscript_fn_map(es_scope const * const scope) {
  es_generator_state *gs = es_as_gen(es_read_nth(scope, 0));
  es_bytecode *fcn = es_as_func(es_read_nth(scope, 1));

  // TODO: HERE
  return NULL;
}

#endif // INCLUDE_ELFSCRIPT_FUNC_ITERATE_H
#endif // ELFSCRIPT_REGISTRATION
