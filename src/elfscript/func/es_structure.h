#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
// nothing to declare
#elif defined(ELFSCRIPT_REGISTER_FUNCTIONS)
{ .key = "index",         .function = &elfscript_fn_index         },
{ .key = "choose",        .function = &elfscript_fn_choose        },
{ .key = "index_of",      .function = &elfscript_fn_index_of      },
{ .key = "lookup_key",    .function = &elfscript_fn_lookup_key    },
#else
#ifndef INCLUDE_ELFSCRIPT_FUNC_STRUCTURE_H
#define INCLUDE_ELFSCRIPT_FUNC_STRUCTURE_H
// elfscript_eval.h
// Eval functions for recursive evaluation

#include "elfscript/elfscript.h"
#include "datatypes/dictionary.h"

// Uses the second argument as an index into the first, which must be an
// array-type node.
elfscript_node * elfscript_fn_index(elfscript_node const * const node) {
  size_t acount;
  elfscript_int_t index;
  elfscript_node *array, *result;

  elfscript_assert_child_count(node, 2, 2);

  array = elfscript_get_value(elfscript_nth(node, 0));

  switch (array->h.type) {
    default:
      elfscript_report_error(
        s_("ERROR: First argument to 'index' isn't an array:"),
        array
      );
      return NULL;
    case ELFSCRIPT_NT_ARRAY_INT:
      elfscript_assert_return_type(node, ELFSCRIPT_NT_INTEGER);
      acount = elfscript_array_count(array);
      break;
    case ELFSCRIPT_NT_ARRAY_NUM:
      elfscript_assert_return_type(node, ELFSCRIPT_NT_NUMBER);
      acount = elfscript_array_count(array);
      break;
    case ELFSCRIPT_NT_ARRAY_STR:
      elfscript_assert_return_type(node, ELFSCRIPT_NT_STRING);
      acount = elfscript_array_count(array);
      break;
  }

  index = elfscript_as_i(elfscript_get_value(elfscript_nth(node, 1)));
  if (index < 0) {
    elfscript_report_error(
      s_("ERROR: 'index' node's index argument must be >= 0."),
      node
    );
    fprintf(stderr, "(index was %ld)\n", index);
    return NULL;
  } else if (index >= acount) {
    elfscript_report_error(
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
      elfscript_report_error(
        s_("ERROR: Something seriously wrong in elfscript_fn_index!"),
        node
      );
      return NULL;
#endif
    case ELFSCRIPT_NT_ARRAY_INT:
      result = construct_elfscript_int_node(
        node->h.name,
        node,
        elfscript_as_ai(array)[index]
      );
      break;
    case ELFSCRIPT_NT_ARRAY_NUM:
      result = construct_elfscript_num_node(
        node->h.name,
        node,
        elfscript_as_an(array)[index]
      );
      break;
    case ELFSCRIPT_NT_ARRAY_STR:
      result = construct_elfscript_str_node(
        node->h.name,
        node,
        elfscript_as_as(array)[index]
      );
      break;
  }
  return result;
}

// Uses the first argument as an index among the remaining arguments, returning
// the value of the n+1st child. If the first argument is too large, its
// modulus will be used.
elfscript_node * elfscript_fn_choose(elfscript_node const * const node) {
  intptr_t count;
  elfscript_int_t index;
  elfscript_node *result;

  count = elfscript_normal_child_count(node);

  elfscript_assert_child_count(node, 2, -1);
  
  index = elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0)));
  if (index < 0) {
    elfscript_report_error(
      s_("ERROR: 'choose' node's index argument must be >= 0."),
      node
    );
    fprintf(stderr, "(index was %ld)\n", index);
    return NULL;
  } else if (index >= count - 1) {
    index = index % (count - 1);
  }
  result = copy_elfscript_node(elfscript_get_value(elfscript_nth(node, index + 1)));
  elfscript_rename(result, node->h.name);
  result->h.context = node;

  return result;
}

// Looks at the first argument and scans through remaining arguments until it
// finds a matching value. Returns the index of the match within the remaining
// elements (i.e. not counting the first). The sought-for value and matching
// values should all be integers.
elfscript_node * elfscript_fn_index_of(elfscript_node const * const node) {
  intptr_t count;
  size_t i;
  elfscript_int_t look_for;
  elfscript_node *test;

  elfscript_assert_return_type(node, ELFSCRIPT_NT_INTEGER);

  count = elfscript_normal_child_count(node);

  elfscript_assert_child_count(node, 2, -1);
  
  look_for = elfscript_as_i(elfscript_get_value(elfscript_nth(node, 0)));
  for (i = 1; i < count; ++i) {
    test = elfscript_get_value(elfscript_nth(node, i));
    if (elfscript_as_i(test) == look_for) {
      break;
    }
  }
  if (i >= count) {
    elfscript_report_error(
      s_("ERROR: index_of's first argument was not found among the rest."),
      node
    );
    return NULL;
  } else {
    return construct_elfscript_int_node(node->h.name, node, i-1);
  }
}

// Looks at the 'input' argument and compares its value against the 'key'
// values of each 'entry' argument until it finds a match. Returns the first
// 'value' child of the matching 'entry'.
elfscript_node * elfscript_fn_lookup_key(elfscript_node const * const node) {
  SSTR(s_entry, "entry", 5);
  SSTR(s_key, "key", 3);
  SSTR(s_value, "value", 5);
  SSTR(s_input, "input", 5);

  size_t i;

  elfscript_node *look_for;
  list *entries;

  elfscript_node *this_entry;
  elfscript_node *this_key;
  elfscript_node *this_value;

  elfscript_assert_child_count(node, 2, -1);

  // Lookup first entry to test return type:
  this_entry = elfscript_lookup_expected(node, s_entry);

  this_key = elfscript_lookup(this_entry, s_key);
  if (this_key == NULL) {
    elfscript_report_error(
     s_("ERROR: 'lookup_key' has malformed 'entry' node (missing 'key')."),
      node
    );
    elfscript_report_error(
     s_("('entry' node was:)."),
      this_entry
    );
    return NULL;
  }

  this_value = elfscript_lookup(this_entry, s_value);
  if (this_value == NULL) {
    elfscript_report_error(
     s_("ERROR: 'lookup_key' has malformed 'entry' node (missing 'value')."),
      node
    );
    elfscript_report_error(
     s_("('entry' node was:)."),
      this_entry
    );
    return NULL;
  }

  elfscript_assert_return_type(node, elfscript_value_type_of(this_value));

  // Find our key node:
  look_for = elfscript_lookup_expected(node, s_input);
  look_for = elfscript_get_value(look_for);

  // Get all entries and search for a match:
  entries = elfscript_lookup_all(node, s_entry);

  this_value = NULL;

  for (i = 0; i < l_get_length(entries); ++i) {
    this_entry = elfscript_get_value((elfscript_node*) l_get_item(entries, i));
    this_key = elfscript_get_value(elfscript_lookup(this_entry, s_key));

    if (elfscript_equivalent(look_for, this_key)) {
      this_value = copy_elfscript_node(
        elfscript_get_value(elfscript_lookup(this_entry, s_value))
      );
      break;
    }
  }
  cleanup_list(entries);
  // fail-to-find is not an error
  elfscript_rename(this_value, node->h.name);
  return this_value;
}

#endif // INCLUDE_ELFSCRIPT_FUNC_STRUCTURE_H
#endif // ELFSCRIPT_REGISTRATION
