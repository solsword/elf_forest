#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "add";    .function = efd_f_add; },
{ .key = "mult";   .function = efd_f_mult; },
{ .key = "sub";    .function = efd_f_sub; },
{ .key = "div";    .function = efd_f_div; },
{ .key = "scale";  .function = efd_f_scale; },
{ .key = "weight"; .function = efd_f_weight; },
#else
#ifndef INCLUDE_EFD_MATH_H
#define INCLUDE_EFD_MATH_H
// efd_math.h
// Eval functions for basic math operations

#include "efd/efd.h"
#include "efd/efd_setup.h"
#include "datatypes/string.h"

// The sum of all children
efd_node * efd_f_add(efd_node * node) {
  efd_num_t sum = 0;
}

// The product of all children
efd_node * efd_f_mult(efd_node * node) {
}

// The first child minus the second
efd_node * efd_f_sub(efd_node * node) {
}

// The first child divided by the second
efd_node * efd_f_div(efd_node * node) {
}

// The first argument (assumed to be in [0, 1]) scaled to be in the range
// between the second and third arguments. If the second argument is larger
// than the third, the scale will wind up inverted (no special check is done).
efd_node * efd_f_scale(efd_node * node) {
  efd_num_t input = *efd_n(efd_nth(node, 0));
  efd_num_t min = *efd__n(efd_nth(node, 1));
  efd_num_t max = *efd__n(efd_nth(node, 2));

  efd_num_t result = min + input * (max - min);

  return construct_efd_num_node(EFD_ANON_NAME, result);
}

efd_node * efd_f_weight(efd_node * node) {
}

#endif // INCLUDE_EFD_MATH_H
#endif // EFD_REGISTRATION
