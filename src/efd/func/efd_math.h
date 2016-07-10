#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "add",    .function = &efd_fn_add },
{ .key = "mult",   .function = &efd_fn_mult },
{ .key = "sub",    .function = &efd_fn_sub },
{ .key = "div",    .function = &efd_fn_div },
{ .key = "scale",  .function = &efd_fn_scale },
{ .key = "weight", .function = &efd_fn_weight },
#else
#ifndef INCLUDE_EFD_FUNC_MATH_H
#define INCLUDE_EFD_FUNC_MATH_H
// efd_math.h
// Eval functions for math operations

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

// The first child divided by the second if the return type is an integer,
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

// The first argument (assumed to be in [0, 1]) scaled to be in the range
// between the second and third arguments. If the second argument is larger
// than the third, the scale will wind up inverted (no special check is done).
efd_node * efd_fn_scale(efd_node const * const node, efd_value_cache *cache) {
  efd_num_t input, max, min, result;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  input = *efd__n(efd_get_value(efd_nth(node, 0), cache));
  min = *efd__n(efd_get_value(efd_nth(node, 1), cache));
  max = *efd__n(efd_get_value(efd_nth(node, 2), cache));

  result = min + input * (max - min);

  return construct_efd_num_node(node->h.name, result);
}

// A weighted average of several numbers. The first argument must be an array
// of numbers specifying the weights of each argument, and its length must be
// equal to the number of subsequent arguments.
efd_node * efd_fn_weight(efd_node const * const node, efd_value_cache *cache) {
  efd_node *wnode;
  efd_num_t *weights;
  size_t count, i;
  efd_num_t numerator, denominator, result;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  wnode = efd_get_value(efd_nth(node, 0), cache);
  count = *efd__an_count(wnode);
  weights = *efd__an(wnode);

  for (i = 0; i < count; ++i) {
    numerator += weights[i] * (*efd__n(efd_get_value(efd_nth(node, i+1),cache)));
    denominator += weights[i];
  }
  if (denominator == 0) {
    result = 0;
  } else {
    result = numerator / denominator;
  }

  return construct_efd_num_node(node->h.name, result);
}

#endif // INCLUDE_EFD_FUNC_MATH_H
#endif // EFD_REGISTRATION
