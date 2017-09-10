#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(ELFSCRIPT_REGISTER_FUNCTIONS)
{ .key = "add",          .function = &elfscript_fn_add          },
{ .key = "mult",         .function = &elfscript_fn_mult         },
{ .key = "sub",          .function = &elfscript_fn_sub          },
{ .key = "div",          .function = &elfscript_fn_div          },
{ .key = "pow",          .function = &elfscript_fn_pow          },
{ .key = "exp",          .function = &elfscript_fn_exp          },
{ .key = "sqrt",         .function = &elfscript_fn_sqrt         },
{ .key = "sine",         .function = &elfscript_fn_sine         },
{ .key = "cosine",       .function = &elfscript_fn_cosine       },
{ .key = "atan2",        .function = &elfscript_fn_atan2        },
{ .key = "avg",          .function = &elfscript_fn_avg          },
{ .key = "scale",        .function = &elfscript_fn_scale        },
{ .key = "constrain",    .function = &elfscript_fn_constrain    },
{ .key = "weight",       .function = &elfscript_fn_weight       },
{ .key = "expdist",      .function = &elfscript_fn_expdist      },
{ .key = "logdist",      .function = &elfscript_fn_logdist      },
#else
#ifndef INCLUDE_ELFSCRIPT_FUNC_MATH_H
#define INCLUDE_ELFSCRIPT_FUNC_MATH_H
// elfscript_math.h
// Eval functions for math operations

#include <math.h>
#include "math/functions.h"

#include "elfscript/elfscript.h"

// The sum of all children. If an integer return type is specified, the values
// are interpreted as integers before addition, otherwise they're interpreted
// as numbers.
elfscript_node * elfscript_fn_add(elfscript_node const * const node) {
  size_t i, count;
  elfscript_int_t r_int = 0;
  elfscript_num_t r_num = 0;
  int use_ints = elfscript_return_type_of(node) == ELFSCRIPT_NT_INTEGER;
  if (!use_ints) { elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER); }

  count = elfscript_normal_child_count(node);
  for (i = 0; i < count; ++i) {
    if (use_ints) {
      r_int += elfscript_as_i(elfscript_get_value(elfscript_nth(node, i)));
    } else {
      r_num += elfscript_as_n(elfscript_get_value(elfscript_nth(node, i)));
    }
  }

  if (use_ints) {
    return construct_elfscript_int_node(node->h.name, node, r_int);
  } else {
    return construct_elfscript_num_node(node->h.name, node, r_num);
  }
}

// The product of all children. If an integer return type is specified, each
// value is interpreted as an integer before multiplication; if a number return
// type is specified, each value is instead interpreted as a number.
elfscript_node * elfscript_fn_mult(elfscript_node const * const node) {
  size_t i, count;
  elfscript_int_t r_int = 1;
  elfscript_num_t r_num = 1;
  int use_ints = elfscript_return_type_of(node) == ELFSCRIPT_NT_INTEGER;
  if (!use_ints) { elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER); }

  count = elfscript_normal_child_count(node);
  for (i = 0; i < count; ++i) {
    if (use_ints) {
      r_int *= elfscript_as_i(elfscript_get_value(elfscript_nth(node, i)));
    } else {
      r_num *= elfscript_as_n(elfscript_get_value(elfscript_nth(node, i)));
    }
  }

  if (use_ints) {
    return construct_elfscript_int_node(node->h.name, node, r_int);
  } else {
    return construct_elfscript_num_node(node->h.name, node, r_num);
  }
}

// The first child minus the second. If an integer return type is specified,
// the arguments will be cast to integers before subtraction, otherwise they
// will be cast to numbers first.
elfscript_node * elfscript_fn_sub(elfscript_node const * const node) {
  elfscript_int_t i_first, i_second;
  elfscript_num_t n_first, n_second;
  if (elfscript_return_type_of(node) == ELFSCRIPT_NT_INTEGER) {
    i_first = elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0)));
    i_second = elfscript_as_i(elfscript_get_value(elfscript_nth(node, 1)));
    return construct_elfscript_int_node(node->h.name, node, i_first - i_second);
  } else {
    elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
    n_first = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0)));
    n_second = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 1)));
    return construct_elfscript_num_node(node->h.name, node, n_first - n_second);
  }
}

// The first child divided by the second. If the return type is an integer,
// integer division is performed after casting the arguments to integers,
// otherwise normal division is performed after casting the arguments to
// numbers. Thus only the return type specified and not the types of the
// arguments is used to determine what kind of division to perform.
elfscript_node * elfscript_fn_div(elfscript_node const * const node) {
  elfscript_int_t i_num, i_den;
  elfscript_num_t n_num, n_den;
  if (elfscript_return_type_of(node) == ELFSCRIPT_NT_INTEGER) {
    i_num = elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0)));
    i_den = elfscript_as_i(elfscript_get_value(elfscript_nth(node, 1)));
    return construct_elfscript_int_node(node->h.name, node, i_num / i_den);
  } else {
    elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
    n_num = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0)));
    n_den = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 1)));
    return construct_elfscript_num_node(node->h.name, node, n_num / n_den);
  }
}

// The first child to the power of the second.
elfscript_node * elfscript_fn_pow(elfscript_node const * const node) {
  elfscript_num_t base, exp;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);

  base = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0)));
  exp = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 1)));

  return construct_elfscript_num_node(node->h.name, node, pow(base, exp));
}

// e to the power of the first child.
elfscript_node * elfscript_fn_exp(elfscript_node const * const node) {
  elfscript_num_t exp;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);

  exp = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0)));

  return construct_elfscript_num_node(node->h.name, node, pow(M_E, exp));
}

// The square root of the first child.
elfscript_node * elfscript_fn_sqrt(elfscript_node const * const node) {
  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
  return construct_elfscript_num_node(
    node->h.name,
    node,
    sqrt(elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0))))
  );
}

// The sine of the first child.
elfscript_node * elfscript_fn_sine(elfscript_node const * const node) {
  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
  return construct_elfscript_num_node(
    node->h.name,
    node,
    sin(elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0))))
  );
}

// The cosine of the first child.
elfscript_node * elfscript_fn_cosine(elfscript_node const * const node) {
  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
  return construct_elfscript_num_node(
    node->h.name,
    node,
    cos(elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0))))
  );
}

// atan2 of the first two children (first argument is y, second is x).
elfscript_node * elfscript_fn_atan2(elfscript_node const * const node) {
  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
  return construct_elfscript_num_node(
    node->h.name,
    node,
    atan2(
      elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0))),
      elfscript_as_n(elfscript_get_value(elfscript_nth(node, 1)))
    )
  );
}

// The average of the the argument (which should be able to generate number
// values).
elfscript_node * elfscript_fn_avg(elfscript_node const * const node) {
  size_t i, count;
  elfscript_num_t sum;
  elfscript_generator_state *values_gen;
  elfscript_node *values_node, *values_container, *child;
  dictionary *values;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);

  /* DEBUG: TODO: REMOVE
  fprintf(stderr, "avg::node\n");
  s_fprintln(stderr, elfscript_repr(node));
  */

  values_node = elfscript_concrete(elfscript_get_value(elfscript_nth(node, 0)));

  /* DEBUG: TODO: REMOVE
  elfscript_report_broken_link(s_("avg::link"), elfscript_nth(node, 0));
  fprintf(stderr, "avg::values\n");
  s_fprintln(stderr, elfscript_repr(values_node));
  */

  values_gen = elfscript_generator_for(values_node);
  if (values_gen == NULL) {
    elfscript_report_error(
      s_("ERROR: invalid 'avg' call: couldn't construct generator from:"),
      values_node
    );
  }
  values_container = elfscript_gen_all(values_gen);
  cleanup_elfscript_generator_state(values_gen);

  values = elfscript_children_dict(values_container);

  count = d_get_count(values);
  sum = 0;
  for (i = 0; i < count; ++i) {
    child = (elfscript_node*) d_get_item(values, i);
    sum += elfscript_as_n(elfscript_get_value(child));
  }

  cleanup_elfscript_node(values_container);
  // (values is cleaned up as part of values_container)

  return construct_elfscript_num_node(node->h.name, node, sum / (elfscript_num_t) count);
}

// The first argument (assumed to be in [0, 1]) scaled to be in the range
// between the second and third arguments. If the second argument is larger
// than the third, the scale will wind up inverted (no special check is done).
// Results are truncated if the return type is integer.
elfscript_node * elfscript_fn_scale(elfscript_node const * const node) {
  elfscript_num_t input, max, min, result;

  input = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0)));
  min = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 1)));
  max = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 2)));

  result = min + input * (max - min);

  if (elfscript_return_type_of(node) == ELFSCRIPT_NT_INTEGER) {
    return construct_elfscript_int_node(node->h.name, node, (elfscript_int_t) result);
  } else {
    elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
    return construct_elfscript_num_node(node->h.name, node, result);
  }
}

// Takes the first argument and looks for "max" and/or "min" children. The
// return value will be the value of the first argument if it's between the
// given limits, or one of the limits if it's not. Truncation to an integer is
// performed after limiting if the return type is integer. The first argument
// should not be named "min" or "max", otherwise it will be used both as input
// and as one of the limits.
elfscript_node * elfscript_fn_constrain(elfscript_node const * const node) {
  SSTR(s_max, "max", 3);
  SSTR(s_min, "min", 3);
  elfscript_node *min_node, *max_node;
  elfscript_num_t input, max, min, result;

  input = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0)));
  min_node = elfscript_lookup(node, s_min);
  if (min_node == NULL) {
    min = input;
  } else {
    min = elfscript_as_n(elfscript_get_value(min_node));
  }
  max_node = elfscript_lookup(node, s_max);
  if (max_node == NULL) {
    max = input;
  } else {
    max = elfscript_as_n(elfscript_get_value(max_node));
  }

  result = input;
  if (result < min) { result = min; }
  if (result > max) { result = max; }

  if (elfscript_return_type_of(node) == ELFSCRIPT_NT_INTEGER) {
    return construct_elfscript_int_node(node->h.name, node, (elfscript_int_t) result);
  } else {
    elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
    return construct_elfscript_num_node(node->h.name, node, result);
  }
}

// A weighted average of several numbers. The first argument must be an
// collection of numbers specifying the weights of each remaining argument, and
// its length must be equal to the number of subsequent arguments.
elfscript_node * elfscript_fn_weight(elfscript_node const * const node) {
  elfscript_generator_state *weights_gen;
  elfscript_node *weights_container;
  list *weights_list;
  size_t count, i;
  elfscript_num_t weight, numerator, denominator, result;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);

  weights_gen = elfscript_generator_for(elfscript_get_value(elfscript_nth(node, 0)));
  weights_container = elfscript_gen_all(weights_gen);
  cleanup_elfscript_generator_state(weights_gen);

  weights_list = d_as_list(elfscript_children_dict(weights_container));

  count = l_get_length(weights_list);

  for (i = 0; i < count; ++i) {
    weight = elfscript_as_n((elfscript_node*) l_get_item(weights_list, i));
    numerator += weight * elfscript_as_n(elfscript_get_value(elfscript_nth(node, i+1)));
    denominator += weight;
  }

  cleanup_list(weights_list);
  cleanup_elfscript_node(weights_container);

  if (denominator == 0) {
    result = 0;
  } else {
    result = numerator / denominator;
  }

  return construct_elfscript_num_node(node->h.name, node, result);
}

// The expdist function from math/functions.h
elfscript_node * elfscript_fn_expdist(elfscript_node const * const node) {
  elfscript_num_t base, exp;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);

  base = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0)));
  exp = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 1)));

  return construct_elfscript_num_node(node->h.name, node, expdist(base, exp));
}

// The logdist function from math/functions.h
elfscript_node * elfscript_fn_logdist(elfscript_node const * const node) {
  elfscript_num_t base, exp;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);

  base = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 0)));
  exp = elfscript_as_n(elfscript_get_value(elfscript_nth(node, 1)));

  return construct_elfscript_num_node(node->h.name, node, logdist(base, exp));
}

#endif // INCLUDE_ELFSCRIPT_FUNC_MATH_H
#endif // ELFSCRIPT_REGISTRATION
