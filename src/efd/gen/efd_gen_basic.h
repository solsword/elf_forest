#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_GENERATORS)
{ .key = "range",    .constructor = efd_gn_range   },
{ .key = "extend",   .constructor = efd_gn_extend  },
#else
#ifndef INCLUDE_EFD_GEN_BASIC_H
#define INCLUDE_EFD_GEN_BASIC_H
// efd_gen_basic.h
// Generator constructors for basic generator types like range.

#include "efd/efd.h"

// The generator function that implements range as described below.
efd_node * efd_gn_impl_range(efd_generator_state *state) {
  SSTR(s_start, "start", 5);
  SSTR(s_stop, "stop", 4);
  SSTR(s_step, "step", 4);

  efd_node *params, *start_node, *stop_node, *step_node;
  efd_int_t start, stop, step, result;
  int maystop = 1;

  params = (efd_node*) state->stash;

  SSTR(s_ec, "...during call to 'range' generator:", 36);
  efd_push_error_context(s_ec);

  start_node = efd_lookup(params, s_start);
  stop_node = efd_lookup(params, s_stop);
  step_node = efd_lookup(params, s_step);

  if (start_node == NULL) {
    start = 0;
  } else {
    start = efd_as_i(start_node);
  }
  if (stop_node == NULL) {
    stop = 0;
    maystop = 0;
  } else {
    stop = efd_as_i(stop_node);
  }
  if (step_node == NULL) {
    step = 1;
  } else {
    step = efd_as_i(step_node);
  }

  result = start + step * state->index;

  if (
    maystop
 && (
      (step >= 0 && result >= stop)
   || (step < 0 && result <= stop)
    )
  ) {
    efd_pop_error_context();
    return NULL; // finished
  }
  state->index += 1;
  efd_pop_error_context();
  return construct_efd_int_node(state->name, params, result);
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
efd_generator_state * efd_gn_range(efd_node const * const node) {
  SSTR(s_start, "start", 5);
  SSTR(s_stop, "stop", 4);
  SSTR(s_step, "step", 4);

  size_t child_count;
  efd_node *stash, *tmp;
  efd_generator_state *result;
  SSTR(s_ec, "...during setup of 'range' generator:", 37);
  efd_push_error_context(s_ec);

  efd_assert_return_type(node, EFD_NT_INTEGER);

  stash = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME, node);
  result = create_efd_generator_state(
    EFD_GT_FUNCTION,
    node->h.name,
    (void*) (&efd_gn_impl_range)
  );

  child_count = efd_normal_child_count(node);
  if (child_count <= 0 || child_count > 3) {
    efd_report_error(
      s_sprintf(
        "ERROR: 'range' iterator must have 1-3 arguments (had %lu):",
        child_count
      ),
      node
    );
    exit(EXIT_FAILURE);
    // TODO: Better here?
  }
  if (child_count >= 1) {
    tmp = copy_efd_node(efd_get_value(efd_nth(node, 0)));
    efd_rename(tmp, s_start);
    efd_add_child(stash, tmp);
  }
  if (child_count >= 2) {
    tmp = copy_efd_node(efd_get_value(efd_nth(node, 0)));
    efd_rename(tmp, s_stop);
    efd_add_child(stash, tmp);
  }
  if (child_count >= 3) {
    tmp = copy_efd_node(efd_get_value(efd_nth(node, 0)));
    efd_rename(tmp, s_step);
    efd_add_child(stash, tmp);
  }

  result->stash = (void*) stash;

  efd_pop_error_context();
  return result;
}

// Takes a generator and an integer node containing one of the EFD_GT_EXTEND_*
// constants and creates a generator which extends the base generator using the
// given extension method.
efd_generator_state * efd_gn_extend(efd_node const * const node) {
  efd_node *child;
  efd_generator_type type;
  efd_generator_state *sub, *result;

  SSTR(s_ec, "...during setup of 'extend' generator:", 38);
  efd_push_error_context(s_ec);

  child = efd_get_value(efd_nth(node, 0));
  efd_assert_return_type(node, efd_return_type_of(child));

  sub = efd_generator_for(child);

  type = (efd_generator_type) efd_as_i(efd_get_value(efd_nth(node, 1)));
  if (type != EFD_GT_EXTEND_RESTART && type != EFD_GT_EXTEND_HOLD) {
    efd_report_error(
      s_sprintf(
        "ERROR: 'extend' iterator has invalid extension type (%d):",
        type
      ),
      node
    );
    exit(EXIT_FAILURE);
    // TODO: better here?
  }

  result = create_efd_generator_state(
    type,
    node->h.name,
    (void*) (sub)
  );
  efd_pop_error_context();
  return result;
}

#endif // INCLUDE_EFD_GEN_BASIC_H
#endif // EFD_REGISTRATION
