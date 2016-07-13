#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "pick_element",   .function = &efd_fn_pick_element },
#else
#ifndef INCLUDE_EFD_FUNC_WORLD_MAP_H
#define INCLUDE_EFD_FUNC_WORLD_MAP_H
// efd_world_map.h
// Eval functions for dealing with world maps.

#include "efd/efd.h"

#include "world/world_map.h"

// Takes four arguments: a world map (an object), category constraint(s) (an
// integer), a list of elements to ignore (something that can generate
// element_species objects), and a seed (an integer).
efd_node * efd_fn_pick_element(
  efd_node const * const node,
  efd_value_cache *cache
) {
  intptr_t count;
  efd_node *map_node, *categories_node, *ignore_container, *seed_node;
  efd_generator_state *ignore_gen;

  world_map *wm;
  element_categorization cat_constraints;
  list *ignore_nodes, *ignore_list;
  ptrdiff_t seed;

  // TODO: REMOVE
  fprintf(stderr, "efd_fn_pick_element::start\n");

  efd_node *result;

  efd_assert_return_type(node, EFD_NT_INTEGER);

  count = efd_normal_child_count(node);

  if (count != 4) {
    efd_report_error(
      s_("ERROR: Wrong number of arguments to 'pick_element' (needs 4)."),
      node
    );
    return NULL;
  }

  // Unpack arguments:
  map_node = efd_get_value(efd_nth(node, 0), cache);
  categories_node = efd_get_value(efd_nth(node, 1), cache);
  ignore_gen = efd_generator_for(efd_get_value(efd_nth(node, 2), cache), cache);
  seed_node = efd_get_value(efd_nth(node, 3), cache);

  // The world map:
  wm = (world_map*) map_node->b.as_object.value;
  if (wm == NULL) {
    efd_report_error(
      s_("ERROR: 'pick_element' got NULL world_map argument."),
      node
    );
    if (efd_is_link_node(efd_nth(node, 0))) {
      efd_report_broken_link(
        s_("(link trace for world map argument)"),
        efd_nth(node, 0)
      );
      s_fprintln(
        stderr,
        efd_full_repr(node)
      );
      exit(EXIT_FAILURE);
    }
    return NULL;
  }

  // TODO: REMOVE
  fprintf(stderr, "efd_fn_pick_element::got_wm\n");

  // Category constraint(s):
  cat_constraints = (element_categorization) (*efd__i(categories_node));

  // The list of species to ignore:
  ignore_container = efd_gen_all(ignore_gen);
  cleanup_efd_generator_state(ignore_gen);

  // TODO: REMOVE
  fprintf(stderr, "efd_fn_pick_element::ignore_container\n");
  s_fprintln(stderr, efd_full_repr(ignore_container));

  ignore_nodes = d_as_list(efd_children_dict(ignore_container));

  // TODO: REMOVE
  fprintf(stderr, "efd_fn_pick_element::got_ignore_nodes\n");

  ignore_list = l_map(ignore_nodes, &v_efd__v_i);
  cleanup_list(ignore_nodes);
  cleanup_efd_node(ignore_container);

  // TODO: REMOVE
  fprintf(stderr, "efd_fn_pick_element::got_ignore_list\n");

  // The seed:
  seed = (ptrdiff_t) *efd__i(seed_node);

  // Construct the result node:
  result = construct_efd_int_node(
    node->h.name,
    (efd_int_t) pick_element(wm, cat_constraints, ignore_list, seed)
  );

  // TODO: REMOVE
  fprintf(stderr, "efd_fn_pick_element::built_result\n");

  // Final cleanup:
  cleanup_list(ignore_list);

  return result;
}

#endif // INCLUDE_EFD_FUNC_WORLD_MAP_H
#endif // EFD_REGISTRATION
