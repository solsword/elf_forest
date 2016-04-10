#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME efd
#define TEST_SUITE_TESTS { \
    &setup_efd_tests, \
    &test_efd_simple_parse, \
    &test_efd_basic_parse, \
    &test_efd_defined_tests, \
    &test_efd_persist, \
    NULL, \
  }

#ifndef TEST_EFD_H
#define TEST_EFD_H

#include "efd/efd.h"
#include "efd/efd_parser.h"
#include "efd/efd_setup.h"

#include <stdio.h>

#include "unit_tests/test_suite.h"

size_t setup_efd_tests(void) {
  init_strings();
  setup_elf_forest_data();
  return 0;
}

size_t test_efd_simple_parse(void) {
  efd_node *n = create_efd_node(EFD_NT_CONTAINER, "test");
  efd_index *cr = create_efd_index();
  if (!efd_parse_file(n, cr, "res/data/test/test-simple.efd")) {
    cleanup_efd_node(n);
    cleanup_efd_index(cr);
    return 1;
  }
  cleanup_efd_node(n);
  cleanup_efd_index(cr);
  return 0;
}

size_t test_efd_basic_parse(void) {
  efd_node *n = create_efd_node(EFD_NT_CONTAINER, "test");
  efd_index *cr = create_efd_index();
  if (!efd_parse_file(n, cr, "res/data/test/test.efd")) {
    cleanup_efd_node(n);
    cleanup_efd_index(cr);
    return 1;
  }
  cleanup_efd_node(n);
  cleanup_efd_index(cr);
  return 0;
}

// Returns 1 for success, 0 for failure.
int _test_efd_case(efd_node *n) {
  static efd_parse_state s;
  efd_index *cr = create_efd_index();
  ptrdiff_t int_result;
  float float_result;
  efd_node *node_result;
  int result = 0;

  s.input = NULL;
  s.input_length = 0;
  s.pos = 0;
  s.filename = "<test-input>";
  s.lineno = 0;
  s.context = NULL;
  s.error = EFD_PE_NO_ERROR;
  s.current_node = NULL;
  s.current_index = -1;

  if (!efd_is_type(n, EFD_NT_OBJECT)) {
    fprintf(stderr, "Test EFD case: Bad object type!\n");
    cleanup_efd_index(cr);
    return 0;
  }

  if (efd_format_is(n, "itest")) {
    efd_int_test *t = (efd_int_test*) (*efd__o(n));
    s.input = s_raw(t->input);
    s.input_length = s_get_length(t->input);
    int_result = efd_parse_int(&s);
    if (s_check_bytes(t->expect, "failure")) {
      result = efd_parse_failed(&s);
    } else if (s_check_bytes(t->expect, "success")) {
      result = (!efd_parse_failed(&s) && int_result == t->output);
    } else if (s_check_bytes(t->expect, "remainder")) {
      result = (
        !efd_parse_failed(&s)
     && int_result == t->output
     && s_check_bytes(t->remainder, s.input + s.pos)
      );
    }
    if (!result) {
      fprintf(stderr, "Test EFD case [int]: Failed.\n");
      fprintf(stderr, "  Input: %s\n", s_raw(t->input));
      if (s_check_bytes(t->expect, "remainder")) {
        fprintf(
          stderr,
          "  Expected: %s (%ld/\"%s\")\n",
          s_raw(t->expect),
          t->output,
          s_raw(t->remainder)
        );
      } else {
        fprintf(stderr, "  Expected: %s (%ld)\n", s_raw(t->expect), t->output);
      }
      if (efd_parse_failed(&s)) {
        fprintf(stderr, "  Result: <parse failed>\n");
        efd_throw_parse_error(&s);
      } else {
        fprintf(stderr, "  Result: %ld\n", int_result);
      }
      if (!efd_parse_atend(&s)) {
        fprintf(stderr, "  Remainder: %s\n", s.input + s.pos);
      }
    }
  } else if (efd_format_is(n, "ntest")) {
    efd_num_test *t = (efd_num_test*) (*efd__o(n));
    s.input = s_raw(t->input);
    s.input_length = s_get_length(t->input);
    float_result = efd_parse_float(&s);
    if (s_check_bytes(t->expect, "failure")) {
      result = efd_parse_failed(&s);
    } else if (s_check_bytes(t->expect, "success")) {
      result = (!efd_parse_failed(&s) && float_result == t->output);
    } else if (s_check_bytes(t->expect, "remainder")) {
      result = (
        !efd_parse_failed(&s)
     && float_result == t->output
     && s_check_bytes(t->remainder, s.input + s.pos)
      );
    }
    if (!result) {
      fprintf(stderr, "Test EFD case [float]: Failed.\n");
      fprintf(stderr, "  Input: %s\n", s_raw(t->input));
      if (s_check_bytes(t->expect, "remainder")) {
        fprintf(
          stderr,
          "  Expected: %s (%f/\"%s\")\n",
          s_raw(t->expect),
          t->output,
          s_raw(t->remainder)
        );
      } else {
        fprintf(stderr, "  Expected: %s (%f)\n", s_raw(t->expect), t->output);
      }
      if (efd_parse_failed(&s)) {
        fprintf(stderr, "  Result: <parse failed>\n");
        efd_throw_parse_error(&s);
      } else {
        fprintf(stderr, "  Result: %f\n", float_result);
      }
      if (!efd_parse_atend(&s)) {
        fprintf(stderr, "  Remainder: %s\n", s.input + s.pos);
      }
    }
  } else if (efd_format_is(n, "ptest")) {
    efd_parse_test *t = (efd_parse_test*) (*efd__o(n));
    s.input = s_raw(t->input);
    s.input_length = s_get_length(t->input);
    node_result = efd_parse_any(&s, cr);
    if (node_result != NULL) { cleanup_efd_node(node_result); }
    if (s_check_bytes(t->expect, "failure")) {
      result = efd_parse_failed(&s);
    } else if (s_check_bytes(t->expect, "success")) {
      result = !efd_parse_failed(&s);
    } else if (s_check_bytes(t->expect, "remainder")) {
      result = (
        efd_parse_failed(&s)
     && s_check_bytes(t->remainder, s.input + s.pos)
      );
    }
    if (!result) {
      fprintf(stderr, "Test EFD case [parse]: Failed.\n");
      fprintf(stderr, "  Input: %s\n", s_raw(t->input));
      if (s_check_bytes(t->expect, "remainder")) {
        fprintf(
          stderr,
          "  Expected: %s (\"%s\")\n",
          s_raw(t->expect),
          s_raw(t->remainder)
        );
      } else {
        fprintf(stderr, "  Expected: %s\n", s_raw(t->expect));
      }
      if (efd_parse_failed(&s)) {
        fprintf(stderr, "  Result: <parse failed>\n");
        efd_print_parse_error(&s);
      } else {
        fprintf(stderr, "  Result: <parse succeeded>\n");
      }
      if (!efd_parse_atend(&s)) {
        fprintf(stderr, "  Remainder: %s\n", s.input + s.pos);
      }
    }
  }
  cleanup_efd_index(cr);
  return result;
}

size_t test_efd_defined_tests(void) {
  size_t i;
  list *l;
  efd_node *n;
  efd_node *test;
  efd_index *cr = create_efd_index();

  n = create_efd_node(EFD_NT_CONTAINER, "test");
  if (!efd_parse_file(n, cr, "res/data/test/test.efd")) {
    cleanup_efd_node(n);
    cleanup_efd_index(cr);
    return 1;
  }

  efd_unpack_node(n, cr);

  l = n->b.as_container.children;
  for (i = 0; i < l_get_length(l); ++i) {
    test = (efd_node*) l_get_item(l, i);
    if (
      efd_is_type(test, EFD_NT_OBJECT)
   && (
        efd_format_is(test, "itest")
     || efd_format_is(test, "ntest")
     || efd_format_is(test, "ptest")
     )
   && !_test_efd_case(test)
    ) {
      return i + 2; // 2, 3, 4, etc.
    }
  }

  cleanup_efd_node(n);
  cleanup_efd_index(cr);

  return 0;
}

size_t test_efd_persist(void) {
  // TODO: HERE
  return 0;
}

#endif //ifndef TEST_EFD_H
