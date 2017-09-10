#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(ELFSCRIPT_REGISTER_GENERATORS)
{ .key = "range",    .constructor = elfscript_gn_range   },
{ .key = "extend",   .constructor = elfscript_gn_extend  },
#else
#ifndef INCLUDE_ELFSCRIPT_GEN_BASIC_H
#define INCLUDE_ELFSCRIPT_GEN_BASIC_H
// elfscript_gen_basic.h
// Generator constructors for basic generator types like range.

#include "elfscript/elfscript.h"

// The generator function that implements range as described below.
es_bytecode * elfscript_gn_impl_range(es_generator_state *state) {
  SSTR(s_start, "start", 5);
  SSTR(s_stop, "stop", 4);
  SSTR(s_step, "step", 4);

  elfscript_var *params, *start_node, *stop_node, *step_node;
  elfscript_int_t start, stop, step, result;
  int maystop = 1;

  params = (elfscript_scope*) state->stash;

  SSTR(s_ec, "...during call to 'range' generator:", 36);
  elfscript_push_error_context(s_ec);

  start_node = es_read_var(params, s_start);
  stop_node = es_read_var(params, s_stop);
  step_node = es_read_var(params, s_step);

  if (start_node == NULL) {
    start = 0;
  } else {
    start = es_as_i(start_node);
  }
  if (stop_node == NULL) {
    stop = 0;
    maystop = 0;
  } else {
    stop = es_as_i(stop_node);
  }
  if (step_node == NULL) {
    step = 1;
  } else {
    step = es_as_i(step_node);
  }

  result = start + step * state->index;

  if (
    maystop
 && (
      (step >= 0 && result >= stop)
   || (step < 0 && result <= stop)
    )
  ) {
    es_pop_error_context();
    return NULL; // finished
  }
  state->index += 1;
  es_pop_error_context();
  return create_es_int_var(result);
}

// Takes up to three arguments: the start, stop, and step values. Creates a
// generator which generates integers starting with the start value and
// continuing up until just before the stop value, moving by the step value
// each time. All arguments must be integers, if given. If only one argument is
// given, it is used as the start value, with no stop and a step of 1. If two
// values are given, they are used as start and stop, with a step value of 1.
// If three values are given they are used as start, stop, and step. A
// container node may be used in place of the stop value to specify no stop
// value (an infinite generator) when a step value is to be specified.
es_generator_state * elfscript_gn_range(
  elfscript_scope const * const scope
) {
  SSTR(s_start, "start", 5);
  SSTR(s_stop, "stop", 4);
  SSTR(s_step, "step", 4);
  SSTR(s_name, "name", 4);

  size_t args_count;
  elfscript_scope *stash;
  es_generator_state *result;
  SSTR(s_ec, "...during setup of 'range' generator:", 37);
  elfscript_push_error_context(s_ec);

  elfscript_assert_return_type(node, ELFSCRIPT_NT_INTEGER);

  stash = create_es_scope();
  result = create_es_generator_state(
    ELFSCRIPT_GT_FUNCTION,
    es_as_s(es_read_var(scope, s_name)),
    (void*) (&elfscript_gn_impl_range)
  );

  args_count = es_scope_size(scope);
  if (args_count <= 0 || args_count > 3) {
    elfscript_report_error(
      s_sprintf(
        "ERROR: 'range' iterator must have 1--3 arguments (had %lu):",
        args_count
      ),
      node
    );
    exit(EXIT_FAILURE);
    // TODO: Better here?
  }
  if (args_count >= 1) {
    es_write_var(stash, s_start, es_read_nth(scope, 0));
  }
  if (args_count >= 2) {
    es_write_var(stash, s_stop, es_read_nth(scope, 1));
  }
  if (args_count >= 3) {
    es_write_var(stash, s_step, es_read_nth(scope, 2));
  }

  result->stash = (void*) stash;

  elfscript_pop_error_context();
  return result;
}

// Takes a scope containing a generator and one of the ES_GT_EXTEND_* constants
// and creates a generator which extends the base generator using the given
// extension method.
es_generator_state * elfscript_gn_extend(
  elfscript_scope const * const node
) {
  es_generator_type type;
  es_generator_state *sub, *result;

  SSTR(s_ec, "...during setup of 'extend' generator:", 38);
  elfscript_push_error_context(s_ec);

  sub = es_as_gen(es_read_nth(node, 0));
  type = (es_generator_type) es_as_i(es_read_nth(node, 1));

  if (type != ELFSCRIPT_GT_EXTEND_RESTART && type != ELFSCRIPT_GT_EXTEND_HOLD) {
    es_report_error(
      s_sprintf(
        "ERROR: 'extend' iterator has invalid extension type (%d):",
        type
      )
    );
    exit(EXIT_FAILURE);
    // TODO: better here?
  }

  result = create_es_generator_state(
    type,
    sub->name,
    (void*) (sub)
  );
  elfscript_pop_error_context();
  return result;
}

#endif // INCLUDE_ELFSCRIPT_GEN_BASIC_H
#endif // ELFSCRIPT_REGISTRATION
