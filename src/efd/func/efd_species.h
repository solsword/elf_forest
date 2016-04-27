#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "weighted_el_prop";   .function = efd_fn_weighted_el_prop; },
#else
#ifndef INCLUDE_EFD_RNG_H
#define INCLUDE_EFD_RNG_H
// efd_species.h
// Eval functions for dealing with species values.

#include "efd/efd.h"

// Takes three arguments: a generic iterable of species IDs (integers), a
// generic iterable of numbers (weights), and an integer specifying the
// EL_PROPERTY_* constant attribute to be averaged. Computes the weighted
// average of the given property across the given elements, returning it as a
// number.
efd_node * efd_fn_weighted_el_prop(efd_node * node) {
  efd_assert_return_type(node, EFD_NT_NUMBER);
  efd_...
  // TODO: Generators
}

// Given a seed and a branch integer, takes a step in a pseudo-random number
// generator according to the branch. This can be used to generate multiple
// "independent" lines of random numbers from a common seed. Obviously the
// quality of the PRGN (not great) affect this somewhat.
efd_node * efd_fn_brng(efd_node * node) {
  efd_assert_return_type(node, EFD_NT_INTEGER);
  efd_int_t result = prng(
    efd__i(efd_nth(node, 0))
  + 1173 * efd__i(efd_nth(node, 1))
  );
  return construct_efd_int_node(node->h.name, result);
}

// Takes a single integer argument (the seed) and returns a corresponding
// number value in [0, 1).
efd_node * efd_fn_rng_n(efd_node *node) {
  efd_assert_return_type(node, EFD_NT_INTEGER);
  efd_int_t result = ptrf(efd__i(efd_nth(node, 0)));
  return construct_efd_int_node(node->h.name, result);
}

// Takes an integer seed value followed by numeric min and max values and
// returns a random value from the given range according to the given seed. The
// result has an approximately uniform distribution over the given range.
efd_node * efd_fn_rng_uniform(efd_node *node) {
  efd_int_t seed;
  efd_num_t min, max;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  seed = efd__i(efd_nth(node, 0));
  min = efd__n(efd_nth(node, 1));
  max = efd__n(efd_nth(node, 2));

  return construct_efd_num_node(node->h.name, randf(seed, min, max));
}

// Like rng_uniform, except that it uses a pseudo-normal distribution that
// results from randomizing three uniform numbers on the given range. As
// rng_uniform, takes an integer followed by two numbers and returns a number.
efd_node * efd_fn_rng_normal(efd_node *node) {
  efd_int_t seed;
  efd_num_t min, max;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  seed = efd__i(efd_nth(node, 0));
  min = efd__n(efd_nth(node, 1));
  max = efd__n(efd_nth(node, 2));

  return construct_efd_num_node(node->h.name, randf_pnorm(seed, min, max));
}

#endif // INCLUDE_EFD_RNG_H
#endif // EFD_REGISTRATION
