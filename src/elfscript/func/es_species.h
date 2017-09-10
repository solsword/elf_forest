#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(ELFSCRIPT_REGISTER_FUNCTIONS)
{ .key = "weighted_el_prop",   .function = &elfscript_fn_weighted_el_prop    },
{ .key = "get_el_attr",        .function = &elfscript_fn_get_el_attr         },
#else
#ifndef INCLUDE_ELFSCRIPT_FUNC_SPECIES_H
#define INCLUDE_ELFSCRIPT_FUNC_SPECIES_H
// elfscript_species.h
// Eval functions for dealing with species values.

#include "elfscript/elfscript.h"

#include "world/species.h"

// Helper function for use with l_map to convert a list of element species IDs
// to a list of pointers to the corresponding element_species structs.
void * _map_v_id_to_v_element_species(void *v_id) {
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
  return (void*) get_element_species((species) v_id);
#pragma GCC diagnostic warning "-Wpointer-to-int-cast"
}

// Takes three arguments: a generic iterable of species IDs (integers), a
// generic iterable of numbers (weights), and an integer specifying the
// EL_PROPERTY_* constant attribute to be averaged. Computes the weighted
// average of the given property across the given elements, returning it as a
// number.
elfscript_node * elfscript_fn_weighted_el_prop(elfscript_node const * const node) {
  elfscript_generator_state *ids_gen, *weights_gen;
  elfscript_node *id_node_container, *weight_node_container;
  list *id_nodes, *ids, *species, *weight_nodes, *weights;
  element_property prop;
  elfscript_node *result;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);

  // Unpack arguments:
  ids_gen = elfscript_generator_for(elfscript_get_value(elfscript_nth(node, 0)));
  weights_gen = elfscript_generator_for(elfscript_get_value(elfscript_nth(node, 1)));
  prop = (element_property) elfscript_as_i(elfscript_get_value(elfscript_nth(node, 2)));
  // TODO: bounds-checking here?

  // Assemble a list of species pointers:
  id_node_container = elfscript_gen_all(ids_gen);
  cleanup_elfscript_generator_state(ids_gen);

  id_nodes = d_as_list(elfscript_children_dict(id_node_container));

  ids = l_map(id_nodes, &v_elfscript__v_i);
  cleanup_elfscript_node(id_node_container);
  cleanup_list(id_nodes);

  species = l_map(ids, &_map_v_id_to_v_element_species);
  cleanup_list(ids);

  // Assemble a list of weight floats:
  weight_node_container = elfscript_gen_all(weights_gen);
  cleanup_elfscript_generator_state(weights_gen);

  weight_nodes = d_as_list(elfscript_children_dict(weight_node_container));

  weights = l_map(weight_nodes, &v_elfscript__v_n);
  cleanup_elfscript_node(weight_node_container);
  cleanup_list(weight_nodes);

  // Truncate the weight list if necessary:
  while (l_get_length(weights) > l_get_length(species)) {
    l_pop_element(weights);
  }

  // Construct the result node:
  result = construct_elfscript_num_node(
    node->h.name,
    node,
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

// Takes two arguments; an integer species ID for an element species, and an
// integer element property constant (in the EL_PRP family). Returns the value
// of the specified property of the given element, as either an integer or
// number. If the property is an integer property, the return type can be
// either number or integer, otherwise it must be number.
elfscript_node * elfscript_fn_get_el_attr(elfscript_node const * const node) {
  species element;
  element_property property;
  element_species *el;
  int ival;

  elfscript_assert_child_count(node, 2, 2);

  element = (species) elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0)));
  property = (element_property) elfscript_as_i(
    elfscript_get_value(elfscript_nth(node, 1))
  );

  if (el_prp_is_integer(property)) {
    // TODO: Better error message here
    if (elfscript_return_type_of(node) != ELFSCRIPT_NT_INTEGER) {
      elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
    }
  } else {
    elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
  }

  el = get_element_species(element);

  if (el == NULL) {
    fprintf(
      stderr,
      "ERROR: unknown element species %d in get_el_attr.\n",
      element
    );
    exit(EXIT_FAILURE);
  }

  if (el_prp_is_integer(property)) {
    ival = el_int_property(el, property);
    if (elfscript_return_type_of(node) == ELFSCRIPT_NT_INTEGER) {
      return construct_elfscript_int_node(node->h.name, node, (elfscript_int_t) ival);
    } else {
      return construct_elfscript_num_node(node->h.name, node, (elfscript_num_t) ival);
    }
  } else {
    return construct_elfscript_num_node(
      node->h.name,
      node,
      (elfscript_num_t) el_float_property(el, property)
    );
  }
}

#endif // INCLUDE_ELFSCRIPT_FUNC_SPECIES_H
#endif // ELFSCRIPT_REGISTRATION
