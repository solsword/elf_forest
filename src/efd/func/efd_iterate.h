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
  list *generators, *gennames;
  efd_value_cache *tmp_cache;
  string *valname;

  efd_assert_return_type(node, EFD_NT_ANY);

  generators = create_list();
  gennames = create_list();

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
    l_append_element(gennames, (void*) (efd_nth(iter_vars, i)->h.name));
  }

  valcount = efd_normal_child_count(node);

  result = create_efd_node(EFD_NT_CONTAINER, node->h.name, node);
  for (i = 0; i < valcount; ++i) {
    efd_add_child(
      result,
      create_efd_node(EFD_NT_CONTAINER, efd_nth(node, i)->h.name, NULL)
    );
  }
  while (1) {
    container = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME, result);
    scope = create_efd_node(EFD_NT_SCOPE, s_ivars, container);
    for (i = 0; i < l_get_length(generators); ++i) {
      gen = (efd_generator_state*) l_get_item(generators, i);
      valname = (string*) l_get_item(gennames, i);
      val = efd_gen_next(gen);

      if (val == NULL) {
        cleanup_efd_node(container);
        scope = NULL;
        break;
      } else {
        efd_rename(val, valname);
        efd_add_child(scope, val);
      }
    }
    if (scope == NULL) {
      break;
    }
    efd_add_child(container, scope);
    tmp_cache = create_efd_value_cache();
    // get values for each real child
    for (i = 0; i < valcount; ++i) {
      tmp = copy_efd_node(efd_nth(node, i));
      tmp->h.context = container;
      efd_add_child(container, tmp);
      val = efd_get_value(tmp, tmp_cache); // cleaned up w/ cache
      val = copy_efd_node_as(val, EFD_ANON_NAME);
      val->h.context = efd_nth(result, i);
      efd_add_child(efd_nth(result, i), val);
    }
    cleanup_efd_value_cache(tmp_cache);
    cleanup_efd_node(container);
  }

  // DEBUG: TODO: REMOVE
  //fprintf(stderr, "iterate::reuslt");
  //s_fprintln(stderr, efd_full_repr(result));

  l_foreach(generators, &cleanup_v_efd_generator_state);
  cleanup_list(generators);

  return result;
}

#endif // INCLUDE_EFD_FUNC_ITERATE_H
#endif // EFD_REGISTRATION
