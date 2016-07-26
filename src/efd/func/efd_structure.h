#if defined(EFD_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(EFD_REGISTER_FUNCTIONS)
{ .key = "index",         .function = &efd_fn_index         },
{ .key = "choose",        .function = &efd_fn_choose        },
{ .key = "index_of",      .function = &efd_fn_index_of      },
{ .key = "lookup_key",    .function = &efd_fn_lookup_key    },
#else
#ifndef INCLUDE_EFD_FUNC_STRUCTURE_H
#define INCLUDE_EFD_FUNC_STRUCTURE_H
// efd_eval.h
// Eval functions for recursive evaluation

#include "efd/efd.h"
#include "datatypes/dictionary.h"

// Uses the second argument as an index into the first, which must be an
// array-type node.
efd_node * efd_fn_index(efd_node const * const node, efd_value_cache *cache) {
  size_t acount;
  efd_int_t index;
  efd_node *array, *result;

  efd_assert_child_count(node, 2, 2);

  array = efd_get_value(efd_nth(node, 0), cache);

  switch (array->h.type) {
    default:
      efd_report_error(
        s_("ERROR: First argument to 'index' isn't an array:"),
        array
      );
      return NULL;
    case EFD_NT_ARRAY_INT:
      efd_assert_return_type(node, EFD_NT_INTEGER);
      acount = efd_array_count(array);
      break;
    case EFD_NT_ARRAY_NUM:
      efd_assert_return_type(node, EFD_NT_NUMBER);
      acount = efd_array_count(array);
      break;
    case EFD_NT_ARRAY_STR:
      efd_assert_return_type(node, EFD_NT_STRING);
      acount = efd_array_count(array);
      break;
  }

  index = efd_as_i(efd_get_value(efd_nth(node, 1), cache));
  if (index < 0) {
    efd_report_error(
      s_("ERROR: 'index' node's index argument must be >= 0."),
      node
    );
    fprintf(stderr, "(index was %ld)\n", index);
    return NULL;
  } else if (index >= acount) {
    efd_report_error(
      s_("ERROR: 'index' node's index is out of range."),
      node
    );
    fprintf(
      stderr,
      "(index was %ld but there were only %ld items)\n",
      index,
      acount
    );
    return NULL;
  }

  switch (array->h.type) {
    default:
#ifdef DEBUG
      // shouldn't be possible
      efd_report_error(
        s_("ERROR: Something seriously wrong in efd_fn_index!"),
        node
      );
      return NULL;
#endif
    case EFD_NT_ARRAY_INT:
      result = construct_efd_int_node(
        node->h.name,
        node,
        efd_as_ai(array)[index]
      );
      break;
    case EFD_NT_ARRAY_NUM:
      result = construct_efd_num_node(
        node->h.name,
        node,
        efd_as_an(array)[index]
      );
      break;
    case EFD_NT_ARRAY_STR:
      result = construct_efd_str_node(
        node->h.name,
        node,
        efd_as_as(array)[index]
      );
      break;
  }
  return result;
}

// Uses the first argument as an index among the remaining arguments, returning
// the value of the n+1st child.
efd_node * efd_fn_choose(efd_node const * const node, efd_value_cache *cache) {
  intptr_t count;
  efd_int_t index;
  efd_node *result;

  count = efd_normal_child_count(node);

  efd_assert_child_count(node, 2, -1);
  
  index = efd_as_i(efd_get_value(efd_nth(node, 0), cache));
  if (index < 0) {
    efd_report_error(
      s_("ERROR: 'choose' node's index argument must be >= 0."),
      node
    );
    fprintf(stderr, "(index was %ld)\n", index);
    return NULL;
  } else if (index >= count - 1) {
    efd_report_error(
      s_("ERROR: 'choose' node's index is out of range."),
      node
    );
    fprintf(
      stderr,
      "(index was %ld but there were only %ld items)\n",
      index,
      count - 1
    );
    return NULL;
  }
  result = efd_get_value(efd_nth(node, index + 1), cache);
  efd_rename(result, node->h.name);
  result->h.context = node;

  return result;
}

// Looks at the first argument and scans through remaining arguments until it
// finds a matching value. Returns the index of the match within the remaining
// elements (i.e. not counting the first). The sought-for value and matching
// values should all be integers.
efd_node * efd_fn_index_of(
  efd_node const * const node,
  efd_value_cache *cache
) {
  intptr_t count;
  size_t i;
  efd_int_t look_for;
  efd_node *test;

  efd_assert_return_type(node, EFD_NT_INTEGER);

  count = efd_normal_child_count(node);

  efd_assert_child_count(node, 2, -1);
  
  look_for = efd_as_i(efd_get_value(efd_nth(node, 0), cache));
  for (i = 1; i < count; ++i) {
    test = efd_get_value(efd_nth(node, i), cache);
    if (efd_as_i(test) == look_for) {
      break;
    }
  }
  if (i >= count) {
    efd_report_error(
      s_("ERROR: index_of's first argument was not found among the rest."),
      node
    );
    return NULL;
  } else {
    return construct_efd_int_node(node->h.name, node, i-1);
  }
}

// Looks at the 'input' argument and compares its value against the 'key'
// values of each 'entry' argument until it finds a match. Returns the first
// 'value' child of the matching 'entry'.
efd_node * efd_fn_lookup_key(
  efd_node const * const node,
  efd_value_cache *cache
) {
  SSTR(s_entry, "entry", 5);
  SSTR(s_key, "key", 3);
  SSTR(s_value, "value", 5);
  SSTR(s_input, "input", 5);

  size_t i;

  efd_node *look_for;
  list *entries;

  efd_node *this_entry;
  efd_node *this_key;
  efd_node *this_value;

  efd_assert_child_count(node, 2, -1);

  // Lookup first entry to test return type:
  this_entry = efd_lookup_expected(node, s_entry);

  this_key = efd_lookup(this_entry, s_key);
  if (this_key == NULL) {
    efd_report_error(
     s_("ERROR: 'lookup_key' has malformed 'entry' node (missing 'key')."),
      node
    );
    efd_report_error(
     s_("('entry' node was:)."),
      this_entry
    );
    return NULL;
  }

  this_value = efd_lookup(this_entry, s_value);
  if (this_value == NULL) {
    efd_report_error(
     s_("ERROR: 'lookup_key' has malformed 'entry' node (missing 'value')."),
      node
    );
    efd_report_error(
     s_("('entry' node was:)."),
      this_entry
    );
    return NULL;
  }

  efd_assert_return_type(node, efd_value_type_of(this_value));

  // Find our key node:
  look_for = efd_lookup_expected(node, s_input);
  look_for = efd_get_value(look_for, cache);

  // Get all entries and search for a match:
  entries = efd_lookup_all(node, s_entry);

  this_value = NULL;

  for (i = 0; i < l_get_length(entries); ++i) {
    this_entry = efd_get_value((efd_node*) l_get_item(entries, i), cache);
    this_key = efd_get_value(efd_lookup(this_entry, s_key), cache);

    if (efd_equivalent(look_for, this_key)) {
      this_value = efd_get_value(efd_lookup(this_entry, s_value), cache);
      break;
    }
  }
  cleanup_list(entries);
  // fail-to-find is not an error
  efd_rename(this_value, node->h.name);
  return this_value;
}

#endif // INCLUDE_EFD_FUNC_STRUCTURE_H
#endif // EFD_REGISTRATION
