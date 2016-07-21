#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "iterate",   .function = &efd_fn_iterate },
#else
#ifndef INCLUDE_EFD_FUNC_ITERATE_H
#define INCLUDE_EFD_FUNC_ITERATE_H
// efd_iterate.h
// Eval functions for iteration

// The iterate function looks at all of the normal children of the given node
// and evaluates each multiple times, produce a container with multiple values
// in it for each original child. During these evaluations, it sets up a local
// scope that contains values from the 'iter_vars' scope argument (which must
// be present). Each varaible in the `iter_vars' scope is treated as a
// generator, and iteration stops when any of the generators is exhausted.
// To illusrate, the folowing node:
//
//   [[ff example:iterate
//     [[V iter_vars
//        [[c *static (i 2) (i 3) (i 4) ]]
//        [[gi *iter:range (i 1) ]]
//     ]]
//     <<v *static@simple>>
//     [[fn times:mult <<v *static>> <<v *iter>> ]]
//   ]]
//
// ...produces the follwing value:
//
//   [[c example
//     [[c simple (i 2) (i 3) (i 4) ]]
//     [[c times (i 2) (i 6) (i 12) ]]
//   ]]
//
// Note that the children of each value node are anonymous. Here the values 2,
// 6, and 12 result because the *static iterator produces values 2, 3, and 4,
// while the *iter iterator produces values 1, 2, and 3, so the value of the
// 'times' node is 2*1, then 3*2, then 4*3. The *iter iterator here is
// infinite, but the *static iterator is finite, and iteration stops when it is
// exhausted.
efd_node * efd_fn_iterate(
  efd_node const * const node,
  efd_value_cache *cache
) {
  SSTR(s_ivars, "iter_vars", 9);

  size_t varcount, valcount, i;
  efd_generator_state *gen;
  efd_node *iter_vars, *container, *scope, *tmp, *val, *result;
  list *generators;

  efd_assert_return_type(node, EFD_NT_ANY);

  generators = create_list();
  iter_vars = efd_get_value(efd_lookup(node, s_ivars), cache);
  if (iter_vars == NULL || !efd_is_type(iter_vars, EFD_NT_SCOPE)) {
    efd_report_error(
      s_("ERROR: 'iterate' node has no 'iter_vars' scope:"),
      node
    );
    exit(EXIT_FAILURE);
  }

  varcount = efd_normal_child_count(iter_vars);
  for (i = 0; i < varcount; ++i) {
    gen = efd_generator_for(efd_get_value(efd_nth(iter_vars, i), cache), cache);
    l_append_element(generators, gen);
  }

  valcount = efd_normal_child_count(node);

  result = create_efd_node(EFD_NT_CONTAINER, node->h.name);
  for (i = 0; i < valcount; ++i) {
    efd_add_child(
      result,
      create_efd_node(EFD_NT_CONTAINER, efd_nth(node, i)->h.name)
    );
  }
  while (1) {
    container = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);
    scope = create_efd_node(EFD_NT_SCOPE, s_ivars);
    for (i = 0; i < l_get_length(generators); ++i) {
      gen = (efd_generator_state*) l_get_item(generators, i);
      val = efd_gen_next(gen);
      if (val == NULL) {
        cleanup_efd_node(container);
        scope = NULL;
        break;
      } else {
        efd_add_child(scope, val);
      }
    }
    if (scope == NULL) {
      break;
    }
    efd_add_child(container, scope);
    // get values for each real child
    for (i = 0; i < valcount; ++i) {
      tmp = copy_efd_node(efd_nth(node, i));
      efd_add_child(container, tmp);
      val = efd_fresh_value(tmp);
      cleanup_efd_node(tmp);
      efd_rename(val, EFD_ANON_NAME);
      efd_add_child(efd_nth(result, i), val);
    }
    cleanup_efd_node(container);
  }

  l_foreach(generators, &cleanup_v_efd_generator_state);
  cleanup_list(generators);

  return result;
}

#endif // INCLUDE_EFD_FUNC_ITERATE_H
#endif // EFD_REGISTRATION
