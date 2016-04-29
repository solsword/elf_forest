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

#include "world/species.h"

// Helper function for use with l_map to convert a list of element species IDs
// to a list of pointers to the corresponding element_species structs.
void * _map_v_id_to_v_element_species(void *v_id) {
  return (void*) get_element_species((species) v_id);
}

// Takes three arguments: a generic iterable of species IDs (integers), a
// generic iterable of numbers (weights), and an integer specifying the
// EL_PROPERTY_* constant attribute to be averaged. Computes the weighted
// average of the given property across the given elements, returning it as a
// number.
efd_node * efd_fn_weighted_el_prop(efd_node * node) {
  efd_generator_state *ids_gen, *weights_gen;
  list *id_nodes, *ids, *species, *weight_nodes, *weights;
  element_property prop;
  efd_node *result;

  efd_assert_return_type(node, EFD_NT_NUMBER);

  // Unpack arguments:
  ids_gen = efd_generator_for(efd_nth(node, 0));
  weights_gen = efd_generator_for(efd_nth(node, 1));
  prop = (element_property) *efd__i(efd_nth(node, 2));
  // TODO: bounds-checking here?

  // Assemble a list of species pointers:
  id_nodes = efd_gen_all(ids_gen);
  cleanup_efd_generator_state(ids_gen);
  ids = l_map(id_nodes, &v_efd__v_i);
  l_foreach(id_nodes, &cleanup_v_efd_node);
  cleanup_list(id_nodes);
  species = l_map(ids, &_map_v_id_to_v_element_species);
  cleanup_list(ids);

  // Assemble a list of weight floats:
  weight_nodes = efd_gen_all(weights_gen);
  cleanup_efd_generator_state(weights_gen);
  weights = l_map(weight_nodes, &v_efd__v_n);
  l_foreach(weight_nodes, &cleanup_v_efd_node);
  cleanup_list(weight_nodes);

  // Construct the result node:
  result = construct_efd_num_node(
    node->h.name,
    el_weighted_property(
      species,
      weights,
      prop
    )
  );

  // Final cleanup:
  cleanup_list(species);
  cleanup_list(weights);

  return result;
}

#endif // INCLUDE_EFD_SPECIES_H
#endif // EFD_REGISTRATION
