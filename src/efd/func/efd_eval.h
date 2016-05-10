#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "peek",    .function = &efd_fn_peek },
{ .key = "call",    .function = &efd_fn_call },
#else
#ifndef INCLUDE_EFD_EVAL_H
#define INCLUDE_EFD_EVAL_H
// efd_eval.h
// Eval functions for recursive evaluation

#include "efd/efd.h"
#include "datatypes/dictionary.h"

// Peeks at the value of the given node: the value of this node will be a copy
// of the value of the first argument. If two arguments are given, the second
// should be a local link and will be treated as a link within the value of the
// first argument.
efd_node * efd_fn_peek(efd_node const * const node, efd_value_cache *cache) {
  intptr_t count;
  efd_node *target, *index, *value, *sub;

  count = efd_normal_child_count(node);

  if (count < 1 || count > 2) {
    efd_report_error(
      node,
      s_("ERROR: Invalid number of arguments to 'peek' (needs 1 or 2).")
    );
    return NULL;
  }

  target = efd_nth(node, 0);

  value = efd_get_value(target, cache);

  if (count == 2) {
    index = efd_nth(node, 1);
    if (index->h.type != EFD_NT_LOCAL_LINK) {
      efd_report_error(
        index,
        s_("ERROR: Invalid argument to 'peek' "
           "(if given, second argument must be a local link).")
      );
      return NULL;
    }
    // lookup within the peeked value:
    sub = efd(value, index->b.as_link.target);
    if (sub == NULL) {
      efd_report_error(
        index,
        s_("ERROR: 'peek' could not find target within value:")
      );
      efd_report_error(
        index,
        s_("   ...target node...")
      );
      efd_report_error(
        value,
        s_("   ...value node.")
      );
      return NULL;
    }

    value = efd_get_value(sub, cache);

    if (value == NULL) {
      efd_report_broken_link(
        sub,
        s_("ERROR: broken link within value during 'peek' call.")
      );
    }
  }

  return value;
}

// Makes a copy of the given node (first argument), inserts a scope node
// (second argument) as its first child, and evaluates the result.
efd_node * efd_fn_call(efd_node const * const node, efd_value_cache *cache) {
  dictionary *children;
  efd_node *target, *args, *shadow, *tmp, *targs, *result;

  children = efd_children_dict(node);
  if (d_get_count(children) != 2) {
    efd_report_error(
      node,
      s_("ERROR: Invalid number of arguments to 'call' "
         "(needs 2; extra scopes not allowed).")
    );
    return NULL;
  }

  target = efd_concrete(d_get_item(children, 0));

  if (!efd_is_container_node(target)) {
    efd_report_error(
      target,
      s_("ERROR: Invalid argument to 'call' "
         "(first argument must be a container type).")
    );
    return NULL;
  }

  args = efd_get_value(d_get_item(children, 1), cache);

  if (args->h.type != EFD_NT_SCOPE) {
    efd_report_error(
      args,
      s_("ERROR: Invalid argument to 'call' "
         "(second argument must be a SCOPE node).")
    );
    return NULL;
  }

  shadow = efd_create_shadow_clone(target);
  tmp = shadow->b.as_reroute.child;
  targs = copy_efd_node(args);
  efd_prepend_child(tmp, targs); // will be cleaned up when tmp is

  // Get a flat result that doesn't contain any links & which has no parent:
  result = efd_flatten(tmp);

  // cleanup temporary stuff:
  cleanup_efd_node(shadow);

  return result;
}

#endif // INCLUDE_EFD_EVAL_H
#endif // EFD_REGISTRATION
