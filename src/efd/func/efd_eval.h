#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "peek",          .function = &efd_fn_peek },
{ .key = "call",          .function = &efd_fn_call },
#else
#ifndef INCLUDE_EFD_FUNC_EVAL_H
#define INCLUDE_EFD_FUNC_EVAL_H
// efd_eval.h
// Eval functions for recursive evaluation

#include "efd/efd.h"
#include "datatypes/dictionary.h"

// Peeks at the value of the given node: the value of this node will be a copy
// of the value of the first argument. If two arguments are given, the second
// should be a local link and will be treated as a link within the value of the
// first argument.
efd_node * efd_fn_peek(efd_node const * const node) {
  intptr_t count;
  efd_node *target, *index, *value, *sub;

  efd_assert_child_count(node, 1, 2);
  count = efd_normal_child_count(node);

  target = efd_nth(node, 0);

  value = efd_get_value(target);

  if (value == NULL) {
    efd_report_error(
      s_("ERROR: Primary target of 'peek' node has NULL value:"),
      node
    );
    return NULL;
  }

  if (count == 2) {
    index = efd_nth(node, 1);
    if (!efd_is_type(index, EFD_NT_LOCAL_LINK)) {
      efd_report_error(
        s_("ERROR: Invalid argument to 'peek' "
           "(if given, second argument must be a local link)."),
        index
      );
      return NULL;
    }
    // lookup within the peeked value:
    sub = efd(value, index->b.as_link.target);
    if (sub == NULL) {
      efd_report_error(
        s_("ERROR: 'peek' could not find target:"),
        index
      );
      efd_report_error_full(
        s_("   ...within value:"),
        value
      );
      return NULL;
    }

    value = sub;
  }

  value = copy_efd_node(value);
  efd_rename(value, node->h.name);
  return value;
}

// Core implementation for efd_fn_call:
efd_node * _efd_call_value(
  efd_node const * const target,
  efd_node *args
) {
  efd_node *shadow, *targs, *result;

  if (!efd_is_container_node(target)) {
    efd_report_error(
      s_("ERROR: Attempt to call non-container node as function:"),
      target
    );
  }
  shadow = copy_efd_node(target);
  targs = efd_get_value(args);
  efd_prepend_child(shadow, targs); // will be cleaned up when shadow is

  // Get the result (initial value will be cleaned up when shadow is):
  result = copy_efd_node(efd_get_value(shadow));

  // cleanup temporary stuff:
  cleanup_efd_node(shadow);

  return result;
}

// Makes a copy of the given node (first argument), inserts a scope node
// (second argument) as its first child, and evaluates the result.
efd_node * efd_fn_call(efd_node const * const node) {
  dictionary *children;
  efd_node *target, *link, *args, *result;

  children = efd_children_dict(node);
  if (d_get_count(children) != 2) {
    efd_report_error(
      s_("ERROR: Invalid number of arguments to 'call' "
         "(needs 2; extra scopes not allowed)."),
      node
    );
    return NULL;
  }

  link = d_get_item(children, 0);
  target = efd_concrete(link);

  if (target == NULL) {
    efd_report_broken_link(
      s_("ERROR: Invalid argument to 'call' "
         "(couldn't resolve link)."),
      link
    );
    return NULL;
  }

  args = efd_get_value(d_get_item(children, 1));

  if (args->h.type != EFD_NT_SCOPE) {
    efd_report_error(
      s_("ERROR: Invalid argument to 'call' "
         "(second argument must be a SCOPE node)."),
      args
    );
    return NULL;
  }

  result = _efd_call_value(target, args);
  efd_rename(result, node->h.name);
  result->h.context = node;

  return result;
}

#endif // INCLUDE_EFD_FUNC_EVAL_H
#endif // EFD_REGISTRATION
