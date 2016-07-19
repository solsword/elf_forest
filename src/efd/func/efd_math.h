#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "add",          .function = &efd_fn_add          },
{ .key = "mult",         .function = &efd_fn_mult         },
{ .key = "sub",          .function = &efd_fn_sub          },
{ .key = "div",          .function = &efd_fn_div          },
{ .key = "pow",          .function = &efd_fn_pow          },
{ .key = "sqrt",         .function = &efd_fn_sqrt         },
{ .key = "avg",          .function = &efd_fn_avg          },
{ .key = "scale",        .function = &efd_fn_scale        },
{ .key = "constrain",    .function = &efd_fn_constrain    },
{ .key = "weight",       .function = &efd_fn_weight       },
{ .key = "expdist",      .function = &efd_fn_expdist      },
{ .key = "logdist",      .function = &efd_fn_logdist      },
#else
#ifndef INCLUDE_EFD_FUNC_MATH_H
#define INCLUDE_EFD_FUNC_MATH_H
// efd_math.h
// Eval functions for math operations

#include <math.h>
#include "math/functions.h"

#include "efd/efd.h"

// The sum of all children. If an integer return type is specified, the values
// are interpreted as integers before addition, otherwise they're interpreted
// as numbers.
efd_node * efd_fn_add(efd_node const * const node, efd_value_cache *cache) {
  size_t i, count;
  efd_int_t r_int = 0;
  efd_num_t r_num = 0;
  int use_ints = efd_return_type_of(node) == EFD_NT_INTEGER;
  if (!use_ints) { efd_assert_return_type(node, EFD_NT_NUMBER); }

  count = efd_normal_child_count(node);
  for (i = 0; i < count; ++i) {
    if (use_ints) {
      r_int += efd_as_i(efd_get_value(efd_nth(node, i), cache));
    } else {
      r_num += efd_as_n(efd_get_value(efd_nth(node, i), cache));
    }
  }

  if (use_ints) {
    return construct_efd_int_node(node->h.name, r_int);
  } else {
    return construct_efd_num_node(node->h.name, r_num);
  }
}

// The product of all children. If an integer return type is specified, each
// value is interpreted as an integer before multiplication; if a number return
// type is specified, each value is instead interpreted as a number.
efd_node * efd_fn_mult(efd_node const * const node, efd_value_cache *cache) {
  size_t i, count;
  efd_int_t r_int = 1;
  efd_num_t r_num = 1;
  int use_ints = efd_return_type_of(node) == EFD_NT_INTEGER;
  if (!use_ints) { efd_assert_return_type(node, EFD_NT_NUMBER); }

  count = efd_normal_child_count(node);
  for (i = 0; i < count; ++i) {
    if (use_ints) {
      r_int *= efd_as_i(efd_get_value(efd_nth(node, i), cache));
    } else {
      r_num *= efd_as_n(efd_get_value(efd_nth(node, i), cache));
    }
  }

  if (use_ints) {
    return construct_efd_int_node(node->h.name, r_int);
  } else {
    return construct_efd_num_node(node->h.name, r_num);
  }
}

// The first child minus the second. If an integer return type is specified,
// the arguments will be cast to integers before subtraction, otherwise they
// will be cast to numbers first.
efd_node * efd_fn_sub(efd_node const * const node, efd_value_cache *cache) {
  efd_int_t i_first, i_second;
  efd_num_t n_first, n_second;
  if (efd_return_type_of(node) == EFD_NT_INTEGER) {
    i_first = efd_as_i(efd_get_value(efd_nth(node, 0), cache));
    i_second = efd_as_i(efd_get_value(efd_nth(node, 1), cache));
    return construct_efd_int_node(node->h.name, i_first - i_second);
  } else {
    efd_assert_return_type(node, EFD_NT_NUMBER);
    n_first = efd_as_n(efd_get_value(efd_nth(node, 0), cache));
    n_second = efd_as_n(efd_get_value(efd_nth(node, 1), cache));
    return construct_efd_num_node(node->h.name, n_first - n_second);
  }
}

// The first child divided by the second. If the return type is an integer,
// integer division is performed after casting the arguments to integers,
// otherwise normal division is performed after casting the arguments to
// numbers. Thus only the return type specified and not the types of the
// arguments is used to determine what kind of division to perform.
efd_node * efd_fn_div(efd_node const * const node, efd_value_cache *cache) {
  efd_int_t i_num, i_den;
  efd_num_t n_num, n_den;
  if (efd_return_type_of(node) == EFD_NT_INTEGER) {
    i_num = efd_as_i(efd_get_value(efd_nth(node, 0), cache));
    i_den = efd_as_i(efd_get_value(efd_nth(node, 1), cache));
    return construct_efd_int_node(node->h.name, i_num / i_den);
  } else {
    efd_assert_return_type(node, EFD_NT_NUMBER);
    n_num = efd_as_n(efd_get_value(efd_nth(node, 0), cache));
    n_den = efd_as_n(efd_get_value(efd_nth(node, 1), cache));
    return construct_efd_num_node(node->h.name, n_num / n_den);
  }
}

// The first child to the power of the second.
efd_node * efd_fn_pow(efd_node const * const node, efd_value_cache *cache) {
  efd_num_t base, exp;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  base = efd_as_n(efd_get_value(efd_nth(node, 0), cache));
  exp = efd_as_n(efd_get_value(efd_nth(node, 1), cache));

  return construct_efd_num_node(node->h.name, pow(base, exp));
}

// The square root of the first child.
efd_node * efd_fn_sqrt(efd_node const * const node, efd_value_cache *cache) {
  efd_assert_return_type(node, EFD_NT_NUMBER);
  return construct_efd_num_node(
    node->h.name,
    sqrt(efd_as_n(efd_get_value(efd_nth(node, 0), cache)))
  );
}

// The average of all arguments.
efd_node * efd_fn_avg(efd_node const * const node, efd_value_cache *cache) {
  size_t i, count;
  efd_num_t sum;
  dictionary *children;
  efd_node *child;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  children = efd_children_dict(node);
  count = d_get_count(children);
  sum = 0;
  for (i = 0; i < count; ++i) {
    child = (efd_node*) d_get_item(children, i);
    sum += efd_as_n(efd_get_value(child, cache));
  }

  return construct_efd_num_node(node->h.name, sum / (efd_num_t) count);
}

// The first argument (assumed to be in [0, 1]) scaled to be in the range
// between the second and third arguments. If the second argument is larger
// than the third, the scale will wind up inverted (no special check is done).
// Results are truncated if the return type is integer.
efd_node * efd_fn_scale(efd_node const * const node, efd_value_cache *cache) {
  efd_num_t input, max, min, result;

  input = efd_as_n(efd_get_value(efd_nth(node, 0), cache));
  min = efd_as_n(efd_get_value(efd_nth(node, 1), cache));
  max = efd_as_n(efd_get_value(efd_nth(node, 2), cache));

  result = min + input * (max - min);

  if (efd_return_type_of(node) == EFD_NT_INTEGER) {
    return construct_efd_int_node(node->h.name, (efd_int_t) result);
  } else {
    efd_assert_return_type(node, EFD_NT_NUMBER);
    return construct_efd_num_node(node->h.name, result);
  }
}

// Takes the first argument and looks for "max" and/or "min" children. The
// return value will be the value of the first argument if it's between the
// given limits, or one of the limits if it's not. Truncation to an integer is
// performed after limiting if the return type is integer. The first argument
// should not be named "min" or "max", otherwise it will be used both as input
// and as one of the limits.
efd_node * efd_fn_constrain(
  efd_node const * const node,
  efd_value_cache *cache
) {
  SSTR(s_max, "max", 3);
  SSTR(s_min, "min", 3);
  efd_node *min_node, *max_node;
  efd_num_t input, max, min, result;

  input = efd_as_n(efd_get_value(efd_nth(node, 0), cache));
  min_node = efd_lookup(node, s_min);
  if (min_node == NULL) {
    min = input;
  } else {
    min = efd_as_n(efd_get_value(min_node, cache));
  }
  max_node = efd_lookup(node, s_max);
  if (max_node == NULL) {
    max = input;
  } else {
    max = efd_as_n(efd_get_value(max_node, cache));
  }

  result = input;
  if (result < min) { result = min; }
  if (result > max) { result = max; }

  if (efd_return_type_of(node) == EFD_NT_INTEGER) {
    return construct_efd_int_node(node->h.name, (efd_int_t) result);
  } else {
    efd_assert_return_type(node, EFD_NT_NUMBER);
    return construct_efd_num_node(node->h.name, result);
  }
}

// A weighted average of several numbers. The first argument must be an
// collection of numbers specifying the weights of each remaining argument, and
// its length must be equal to the number of subsequent arguments.
efd_node * efd_fn_weight(efd_node const * const node, efd_value_cache *cache) {
  efd_generator_state *weights_gen;
  efd_node *weights_container;
  list *weights_list;
  size_t count, i;
  efd_num_t weight, numerator, denominator, result;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  weights_gen = efd_generator_for(
    efd_get_value(efd_nth(node, 0), cache),
    cache
  );
  weights_container = efd_gen_all(weights_gen);
  cleanup_efd_generator_state(weights_gen);

  weights_list = d_as_list(efd_children_dict(weights_container));

  count = l_get_length(weights_list);

  for (i = 0; i < count; ++i) {
    weight = efd_as_n((efd_node*) l_get_item(weights_list, i));
    numerator += weight * efd_as_n(efd_get_value(efd_nth(node, i+1),cache));
    denominator += weight;
  }

  cleanup_list(weights_list);
  cleanup_efd_node(weights_container);

  if (denominator == 0) {
    result = 0;
  } else {
    result = numerator / denominator;
  }

  return construct_efd_num_node(node->h.name, result);
}

// The expdist function from math/functions.h
efd_node * efd_fn_expdist(efd_node const * const node, efd_value_cache *cache) {
  efd_num_t base, exp;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  base = efd_as_n(efd_get_value(efd_nth(node, 0), cache));
  exp = efd_as_n(efd_get_value(efd_nth(node, 1), cache));

  return construct_efd_num_node(node->h.name, expdist(base, exp));
}

// The logdist function from math/functions.h
efd_node * efd_fn_logdist(efd_node const * const node, efd_value_cache *cache) {
  efd_num_t base, exp;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  base = efd_as_n(efd_get_value(efd_nth(node, 0), cache));
  exp = efd_as_n(efd_get_value(efd_nth(node, 1), cache));

  return construct_efd_num_node(node->h.name, logdist(base, exp));
}

#endif // INCLUDE_EFD_FUNC_MATH_H
#endif // EFD_REGISTRATION
