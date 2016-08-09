#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "add",          .function = &efd_fn_add          },
{ .key = "mult",         .function = &efd_fn_mult         },
{ .key = "sub",          .function = &efd_fn_sub          },
{ .key = "div",          .function = &efd_fn_div          },
{ .key = "pow",          .function = &efd_fn_pow          },
{ .key = "exp",          .function = &efd_fn_exp          },
{ .key = "sqrt",         .function = &efd_fn_sqrt         },
{ .key = "sine",         .function = &efd_fn_sine         },
{ .key = "cosine",       .function = &efd_fn_cosine       },
{ .key = "atan2",        .function = &efd_fn_atan2        },
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
efd_node * efd_fn_add(efd_node const * const node) {
  size_t i, count;
  efd_int_t r_int = 0;
  efd_num_t r_num = 0;
  int use_ints = efd_return_type_of(node) == EFD_NT_INTEGER;
  if (!use_ints) { efd_assert_return_type(node, EFD_NT_NUMBER); }

  count = efd_normal_child_count(node);
  for (i = 0; i < count; ++i) {
    if (use_ints) {
      r_int += efd_as_i(efd_get_value(efd_nth(node, i)));
    } else {
      r_num += efd_as_n(efd_get_value(efd_nth(node, i)));
    }
  }

  if (use_ints) {
    return construct_efd_int_node(node->h.name, node, r_int);
  } else {
    return construct_efd_num_node(node->h.name, node, r_num);
  }
}

// The product of all children. If an integer return type is specified, each
// value is interpreted as an integer before multiplication; if a number return
// type is specified, each value is instead interpreted as a number.
efd_node * efd_fn_mult(efd_node const * const node) {
  size_t i, count;
  efd_int_t r_int = 1;
  efd_num_t r_num = 1;
  int use_ints = efd_return_type_of(node) == EFD_NT_INTEGER;
  if (!use_ints) { efd_assert_return_type(node, EFD_NT_NUMBER); }

  count = efd_normal_child_count(node);
  for (i = 0; i < count; ++i) {
    if (use_ints) {
      r_int *= efd_as_i(efd_get_value(efd_nth(node, i)));
    } else {
      r_num *= efd_as_n(efd_get_value(efd_nth(node, i)));
    }
  }

  if (use_ints) {
    return construct_efd_int_node(node->h.name, node, r_int);
  } else {
    return construct_efd_num_node(node->h.name, node, r_num);
  }
}

// The first child minus the second. If an integer return type is specified,
// the arguments will be cast to integers before subtraction, otherwise they
// will be cast to numbers first.
efd_node * efd_fn_sub(efd_node const * const node) {
  efd_int_t i_first, i_second;
  efd_num_t n_first, n_second;
  if (efd_return_type_of(node) == EFD_NT_INTEGER) {
    i_first = efd_as_i(efd_get_value(efd_nth(node, 0)));
    i_second = efd_as_i(efd_get_value(efd_nth(node, 1)));
    return construct_efd_int_node(node->h.name, node, i_first - i_second);
  } else {
    efd_assert_return_type(node, EFD_NT_NUMBER);
    n_first = efd_as_n(efd_get_value(efd_nth(node, 0)));
    n_second = efd_as_n(efd_get_value(efd_nth(node, 1)));
    return construct_efd_num_node(node->h.name, node, n_first - n_second);
  }
}

// The first child divided by the second. If the return type is an integer,
// integer division is performed after casting the arguments to integers,
// otherwise normal division is performed after casting the arguments to
// numbers. Thus only the return type specified and not the types of the
// arguments is used to determine what kind of division to perform.
efd_node * efd_fn_div(efd_node const * const node) {
  efd_int_t i_num, i_den;
  efd_num_t n_num, n_den;
  if (efd_return_type_of(node) == EFD_NT_INTEGER) {
    i_num = efd_as_i(efd_get_value(efd_nth(node, 0)));
    i_den = efd_as_i(efd_get_value(efd_nth(node, 1)));
    return construct_efd_int_node(node->h.name, node, i_num / i_den);
  } else {
    efd_assert_return_type(node, EFD_NT_NUMBER);
    n_num = efd_as_n(efd_get_value(efd_nth(node, 0)));
    n_den = efd_as_n(efd_get_value(efd_nth(node, 1)));
    return construct_efd_num_node(node->h.name, node, n_num / n_den);
  }
}

// The first child to the power of the second.
efd_node * efd_fn_pow(efd_node const * const node) {
  efd_num_t base, exp;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  base = efd_as_n(efd_get_value(efd_nth(node, 0)));
  exp = efd_as_n(efd_get_value(efd_nth(node, 1)));

  return construct_efd_num_node(node->h.name, node, pow(base, exp));
}

// e to the power of the first child.
efd_node * efd_fn_exp(efd_node const * const node) {
  efd_num_t exp;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  exp = efd_as_n(efd_get_value(efd_nth(node, 0)));

  return construct_efd_num_node(node->h.name, node, pow(M_E, exp));
}

// The square root of the first child.
efd_node * efd_fn_sqrt(efd_node const * const node) {
  efd_assert_return_type(node, EFD_NT_NUMBER);
  return construct_efd_num_node(
    node->h.name,
    node,
    sqrt(efd_as_n(efd_get_value(efd_nth(node, 0))))
  );
}

// The sine of the first child.
efd_node * efd_fn_sine(efd_node const * const node) {
  efd_assert_return_type(node, EFD_NT_NUMBER);
  return construct_efd_num_node(
    node->h.name,
    node,
    sin(efd_as_n(efd_get_value(efd_nth(node, 0))))
  );
}

// The cosine of the first child.
efd_node * efd_fn_cosine(efd_node const * const node) {
  efd_assert_return_type(node, EFD_NT_NUMBER);
  return construct_efd_num_node(
    node->h.name,
    node,
    cos(efd_as_n(efd_get_value(efd_nth(node, 0))))
  );
}

// atan2 of the first two children (first argument is y, second is x).
efd_node * efd_fn_atan2(efd_node const * const node) {
  efd_assert_return_type(node, EFD_NT_NUMBER);
  return construct_efd_num_node(
    node->h.name,
    node,
    atan2(
      efd_as_n(efd_get_value(efd_nth(node, 0))),
      efd_as_n(efd_get_value(efd_nth(node, 1)))
    )
  );
}

// The average of the the argument (which should be able to generate number
// values).
efd_node * efd_fn_avg(efd_node const * const node) {
  size_t i, count;
  efd_num_t sum;
  efd_generator_state *values_gen;
  efd_node *values_node, *values_container, *child;
  dictionary *values;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  /* DEBUG: TODO: REMOVE
  fprintf(stderr, "avg::node\n");
  s_fprintln(stderr, efd_repr(node));
  */

  values_node = efd_concrete(efd_get_value(efd_nth(node, 0)));

  /* DEBUG: TODO: REMOVE
  efd_report_broken_link(s_("avg::link"), efd_nth(node, 0));
  fprintf(stderr, "avg::values\n");
  s_fprintln(stderr, efd_repr(values_node));
  */

  values_gen = efd_generator_for(values_node);
  if (values_gen == NULL) {
    efd_report_error(
      s_("ERROR: invalid 'avg' call: couldn't construct generator from:"),
      values_node
    );
  }
  values_container = efd_gen_all(values_gen);
  cleanup_efd_generator_state(values_gen);

  values = efd_children_dict(values_container);

  count = d_get_count(values);
  sum = 0;
  for (i = 0; i < count; ++i) {
    child = (efd_node*) d_get_item(values, i);
    sum += efd_as_n(efd_get_value(child));
  }

  cleanup_efd_node(values_container);
  // (values is cleaned up as part of values_container)

  return construct_efd_num_node(node->h.name, node, sum / (efd_num_t) count);
}

// The first argument (assumed to be in [0, 1]) scaled to be in the range
// between the second and third arguments. If the second argument is larger
// than the third, the scale will wind up inverted (no special check is done).
// Results are truncated if the return type is integer.
efd_node * efd_fn_scale(efd_node const * const node) {
  efd_num_t input, max, min, result;

  input = efd_as_n(efd_get_value(efd_nth(node, 0)));
  min = efd_as_n(efd_get_value(efd_nth(node, 1)));
  max = efd_as_n(efd_get_value(efd_nth(node, 2)));

  result = min + input * (max - min);

  if (efd_return_type_of(node) == EFD_NT_INTEGER) {
    return construct_efd_int_node(node->h.name, node, (efd_int_t) result);
  } else {
    efd_assert_return_type(node, EFD_NT_NUMBER);
    return construct_efd_num_node(node->h.name, node, result);
  }
}

// Takes the first argument and looks for "max" and/or "min" children. The
// return value will be the value of the first argument if it's between the
// given limits, or one of the limits if it's not. Truncation to an integer is
// performed after limiting if the return type is integer. The first argument
// should not be named "min" or "max", otherwise it will be used both as input
// and as one of the limits.
efd_node * efd_fn_constrain(efd_node const * const node) {
  SSTR(s_max, "max", 3);
  SSTR(s_min, "min", 3);
  efd_node *min_node, *max_node;
  efd_num_t input, max, min, result;

  input = efd_as_n(efd_get_value(efd_nth(node, 0)));
  min_node = efd_lookup(node, s_min);
  if (min_node == NULL) {
    min = input;
  } else {
    min = efd_as_n(efd_get_value(min_node));
  }
  max_node = efd_lookup(node, s_max);
  if (max_node == NULL) {
    max = input;
  } else {
    max = efd_as_n(efd_get_value(max_node));
  }

  result = input;
  if (result < min) { result = min; }
  if (result > max) { result = max; }

  if (efd_return_type_of(node) == EFD_NT_INTEGER) {
    return construct_efd_int_node(node->h.name, node, (efd_int_t) result);
  } else {
    efd_assert_return_type(node, EFD_NT_NUMBER);
    return construct_efd_num_node(node->h.name, node, result);
  }
}

// A weighted average of several numbers. The first argument must be an
// collection of numbers specifying the weights of each remaining argument, and
// its length must be equal to the number of subsequent arguments.
efd_node * efd_fn_weight(efd_node const * const node) {
  efd_generator_state *weights_gen;
  efd_node *weights_container;
  list *weights_list;
  size_t count, i;
  efd_num_t weight, numerator, denominator, result;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  weights_gen = efd_generator_for(efd_get_value(efd_nth(node, 0)));
  weights_container = efd_gen_all(weights_gen);
  cleanup_efd_generator_state(weights_gen);

  weights_list = d_as_list(efd_children_dict(weights_container));

  count = l_get_length(weights_list);

  for (i = 0; i < count; ++i) {
    weight = efd_as_n((efd_node*) l_get_item(weights_list, i));
    numerator += weight * efd_as_n(efd_get_value(efd_nth(node, i+1)));
    denominator += weight;
  }

  cleanup_list(weights_list);
  cleanup_efd_node(weights_container);

  if (denominator == 0) {
    result = 0;
  } else {
    result = numerator / denominator;
  }

  return construct_efd_num_node(node->h.name, node, result);
}

// The expdist function from math/functions.h
efd_node * efd_fn_expdist(efd_node const * const node) {
  efd_num_t base, exp;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  base = efd_as_n(efd_get_value(efd_nth(node, 0)));
  exp = efd_as_n(efd_get_value(efd_nth(node, 1)));

  return construct_efd_num_node(node->h.name, node, expdist(base, exp));
}

// The logdist function from math/functions.h
efd_node * efd_fn_logdist(efd_node const * const node) {
  efd_num_t base, exp;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  base = efd_as_n(efd_get_value(efd_nth(node, 0)));
  exp = efd_as_n(efd_get_value(efd_nth(node, 1)));

  return construct_efd_num_node(node->h.name, node, logdist(base, exp));
}

#endif // INCLUDE_EFD_FUNC_MATH_H
#endif // EFD_REGISTRATION
