#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "weighted_el_prop";   .function = efd_fn_weighted_el_prop; },
#else
#ifndef INCLUDE_EFD_SPECIES_H
#define INCLUDE_EFD_SPECIES_H
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

  efd_
}

#endif // INCLUDE_EFD_SPECIES_H
#endif // EFD_REGISTRATION
