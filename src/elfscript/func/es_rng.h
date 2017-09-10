#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(ELFSCRIPT_REGISTER_FUNCTIONS)
{ .key = "prng",         .function = &elfscript_fn_prng         },
{ .key = "brng",         .function = &elfscript_fn_brng         },
{ .key = "rng_n",        .function = &elfscript_fn_rng_n        },
{ .key = "rng_uniform",  .function = &elfscript_fn_rng_uniform  },
{ .key = "rng_normal",   .function = &elfscript_fn_rng_normal   },
#else
#ifndef INCLUDE_ELFSCRIPT_FUNC_RNG_H
#define INCLUDE_ELFSCRIPT_FUNC_RNG_H
// elfscript_rng.h
// Eval functions for random number generation

#include "elfscript/elfscript.h"

// The next step in a simple pseudo-random number generator given a single
// argument (which must be an integer).
elfscript_node * elfscript_fn_prng(elfscript_node const * const node) {
  elfscript_assert_return_type(node, ELFSCRIPT_NT_INTEGER);
  elfscript_assert_child_count(node, 1, 1);
  elfscript_int_t result = prng(elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0))));
  return construct_elfscript_int_node(node->h.name, node, result);
}

// Given a seed and a branch integer, takes a step in a pseudo-random number
// generator according to the branch. This can be used to generate multiple
// "independent" lines of random numbers from a common seed. Obviously the
// quality of the PRGN (not great) affect this somewhat.
elfscript_node * elfscript_fn_brng(elfscript_node const * const node) {
  elfscript_assert_return_type(node, ELFSCRIPT_NT_INTEGER);
  elfscript_assert_child_count(node, 2, 2);
  elfscript_int_t result = prng(
    elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0)))
  + 1173 * (elfscript_as_i(elfscript_get_value(elfscript_nth(node, 1))))
  );
  return construct_elfscript_int_node(node->h.name, node, result);
}

// Takes a single integer argument (the seed) and returns a corresponding
// number value in [0, 1).
elfscript_node * elfscript_fn_rng_n(elfscript_node const * const node) {
  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
  elfscript_assert_child_count(node, 1, 1);
  elfscript_num_t result = ptrf(elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0))));
  return construct_elfscript_num_node(node->h.name, node, result);
}

// Takes an integer seed value followed by numeric min and max values and
// returns a random value from the given range according to the given seed. The
// result has an approximately uniform distribution over the given range.
elfscript_node * elfscript_fn_rng_uniform(elfscript_node const * const node) {
  elfscript_int_t seed;
  elfscript_num_t min, max;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
  elfscript_assert_child_count(node, 3, 3);

  seed = elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0)));
  min = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 1)));
  max = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 2)));

  return construct_elfscript_num_node(node->h.name, node, randf(seed, min, max));
}

// Like rng_uniform, except that it uses a pseudo-normal distribution that
// results from randomizing three uniform numbers on the given range. As
// rng_uniform, takes an integer followed by two numbers and returns a number.
elfscript_node * elfscript_fn_rng_normal(elfscript_node const * const node) {
  elfscript_int_t seed;
  elfscript_num_t min, max;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
  elfscript_assert_child_count(node, 3, 3);

  seed = elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0)));
  min = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 1)));
  max = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 2)));

  return construct_elfscript_num_node(node->h.name, node,randf_pnorm(seed, min, max));
}

#endif // INCLUDE_ELFSCRIPT_FUNC_RNG_H
#endif // ELFSCRIPT_REGISTRATION
