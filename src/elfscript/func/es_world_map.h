#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(ELFSCRIPT_REGISTER_FUNCTIONS)
{ .key = "pick_element",   .function = &elfscript_fn_pick_element },
#else
#ifndef INCLUDE_ELFSCRIPT_FUNC_WORLD_MAP_H
#define INCLUDE_ELFSCRIPT_FUNC_WORLD_MAP_H
// elfscript_world_map.h
// Eval functions for dealing with world maps.

#include "elfscript/elfscript.h"

#include "world/world_map.h"

// Takes four arguments: a world map (an object), category constraint(s) (an
// integer), a list of elements to ignore (something that can generate
// element_species objects), and a seed (an integer).
elfscript_node * elfscript_fn_pick_element(elfscript_node const * const node) {
  intptr_t count;
  elfscript_node *map_node, *categories_node, *ignore_container, *seed_node;
  elfscript_generator_state *ignore_gen;

  world_map *wm;
  element_categorization cat_constraints;
  list *ignore_nodes, *ignore_list;
  ptrdiff_t seed;

  elfscript_node *result;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_INTEGER);

  count = elfscript_normal_child_count(node);

  if (count != 4) {
    elfscript_report_error(
      s_("ERROR: Wrong number of arguments to 'pick_element' (needs 4)."),
      node
    );
    return NULL;
  }

  // Unpack arguments:
  map_node = elfscript_get_value(elfscript_nth(node, 0));
  categories_node = elfscript_get_value(elfscript_nth(node, 1));
  ignore_gen = elfscript_generator_for(elfscript_get_value(elfscript_nth(node, 2)));
  seed_node = elfscript_get_value(elfscript_nth(node, 3));

  // The world map:
  wm = (world_map*) elfscript_as_o(map_node);
  if (wm == NULL) {
    elfscript_report_error(
      s_("ERROR: 'pick_element' got NULL world_map argument."),
      node
    );
    if (elfscript_is_link_node(elfscript_nth(node, 0))) {
      elfscript_report_broken_link(
        s_("(link trace for world map argument)"),
        elfscript_nth(node, 0)
      );
      fprintf(stderr, "Full 'pick_element' node:\n");
      s_fprintln(
        stderr,
        elfscript_full_repr(node)
      );
      exit(EXIT_FAILURE);
    }
    return NULL;
  }

  // Category constraint(s):
  cat_constraints = (element_categorization) elfscript_as_i(categories_node);

  // The list of species to ignore:
  ignore_container = elfscript_gen_all(ignore_gen);
  cleanup_elfscript_generator_state(ignore_gen);

  ignore_nodes = d_as_list(elfscript_children_dict(ignore_container));

  ignore_list = l_map(ignore_nodes, &v_elfscript__v_i);
  cleanup_list(ignore_nodes);
  cleanup_elfscript_node(ignore_container);

  // The seed:
  seed = (ptrdiff_t) elfscript_as_i(seed_node);

  // Construct the result node:
  result = construct_elfscript_int_node(
    node->h.name,
    node,
    (elfscript_int_t) pick_element(wm, cat_constraints, ignore_list, seed)
  );

  // Final cleanup:
  cleanup_list(ignore_list);

  return result;
}

#endif // INCLUDE_ELFSCRIPT_FUNC_WORLD_MAP_H
#endif // ELFSCRIPT_REGISTRATION
