#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_GENERATORS)
{ .key = "range";    .function = efd_gn_range; },
{ .key = "extend";   .function = efd_gn_extend; },
#else
#ifndef INCLUDE_EFD_GEN_BASIC_H
#define INCLUDE_EFD_GEN_BASIC_H
// efd_gen_basic.h
// Generator constructors for basic generator types like range.

#include "efd/efd.h"

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
  size_t child_count;
  efd_node *stash;
  efd_generator_state *result;
  SSTR(s_start, "start");
  SSTR(s_stop, "stop");
  SSTR(s_step, "step");

  efd_assert_return_type(node, EFD_NT_INTEGER);

  stash = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);
  result = create_efd_generator_state(
    EFD_GT_FUNCTION,
    node->h.name,
    (void*) (&efd_gn_impl_range)
  );

  child_count = efd_normal_child_count(node);
  if (child_count == 1) {
    efd_add_child(stash, copy_efd_node(efd_nth(node, 0)));
    efd_add_child(stash, create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME));
    efd_add_child(stash, construct_efd_int_node(EFD_ANON_NAME, 1));
  } else if (child_count == 2) {
    efd_add_child(stash, copy_efd_node(efd_nth(node, 0)));
    efd_add_child(stash, copy_efd_node(efd_nth(node, 1)));
    efd_add_child(stash, construct_efd_int_node(EFD_ANON_NAME, 1));
  } else if (child_count == 3) {
    efd_add_child(stash, copy_efd_node(efd_nth(node, 0)));
    efd_add_child(stash, copy_efd_node(efd_nth(node, 1)));
    efd_add_child(stash, copy_efd_node(efd_nth(node, 2)));
  } else {
    // TODO: Better & standardized error behavior
    fprintf(
      stderr, 
      "ERROR: 'range' iterator must have 1-3 arguments (%lu given).\n",
      child_count
    );
  }

  result->stash = (void*) stash;

  return result;
}

// The generator function that implements range as described above.
efd_node * efd_gn_impl_range(efd_generator_state *state) {
  efd_node *params = (efd_node*) state->stash;
  efd_int_t start, stop, step, result;
  SSTR(s_start, "start");
  SSTR(s_stop, "stop");
  SSTR(s_step, "step");

  start = efd__i(efd_lookup(params, s_start));
  stop = efd__i(efd_lookup(params, s_stop));
  step = efd__i(efd_lookup(params, s_step));

  result = start + step * state->index;
  if (
    (step >= 0 && result >= stop)
 || (step < 0 && result <= stop)
  ) {
    return NULL; // finished
  }
  state->index += 1;
  return construct_efd_int_node(state->name, result);
}

// Takes a generator and an integer node containing one of the EFD_GT_EXTEND_*
// constants and creates a generator which extends the base generator using the
// given extension method.
efd_generator_state * efd_gn_extend(efd_node const * const node) {
  efd_node *child;
  efd_generator_type type;
  efd_generator_state *sub;

  child = efd_nth(node, 0);
  efd_assert_return_type(node, efd_return_type_of(child));

  sub = efd_generator_for(child);

  type = (efd_generator_type) efd__i(efd_nth(node, 1));
  if (type != EFD_GT_EXTEND_RESTART && type != EFD_GT_EXTEND_HOLD) {
    // TODO: Better & standardized error behavior
    fprintf(
      stderr, 
      "ERROR: 'extend' iterator has invalid type argument (%d).\n",
      type
    );
  }

  return create_efd_generator_state(
    type,
    node->h.name,
    (void*) (sub)
  );
}

#endif // INCLUDE_EFD_GEN_BASIC_H
#endif // EFD_REGISTRATION
