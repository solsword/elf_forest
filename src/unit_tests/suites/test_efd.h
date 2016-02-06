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

// DEBUG:
#include "datatypes/string.h"
#include <unistr.h>
#include <uniconv.h>

#include "unit_tests/test_suite.h"

size_t setup_efd_tests(void) {
  init_strings();
  return 0;
}

size_t test_efd_simple_parse(void) {
  efd_node *n = create_efd_node(EFD_NT_CONTAINER, "test");
  if (!efd_parse_file(n, "res/data/test/test-simple.efd")) {
    cleanup_efd_node(n);
    return 1;
  }
  cleanup_efd_node(n);
  return 0;
}

size_t test_efd_basic_parse(void) {
  efd_node *n = create_efd_node(EFD_NT_CONTAINER, "test");
  if (!efd_parse_file(n, "res/data/test/test.efd")) {
    cleanup_efd_node(n);
    return 1;
  }
  cleanup_efd_node(n);
  printf("Done with basic\n.");
  return 0;
}

// Returns 1 for success, 0 for failure.
int _test_efd_case(efd_node *n) {
  static efd_parse_state s;
  ptrdiff_t int_result;
  float float_result;

  if (s.input != NULL) {
    return 0;
  }

  s.input = NULL;
  s.input_length = 0;
  s.pos = 0;
  s.filename = "<test-input>";
  s.lineno = 0;
  s.context = NULL;
  s.error = EFD_PE_NO_ERROR;

  if (!efd_is_type(n, EFD_NT_OBJECT)) {
    return 0;
  }

  if (efd_format_is(n, "itest")) {
    efd_int_test *t = (efd_int_test*) (*efd__o(n));
    s.input = s_raw(t->input);
    s.input_length = s_get_length(t->input);
    int_result = efd_parse_int(&s);
    if (s_check_bytes(t->expect, "failure")) {
      return efd_parse_failed(&s);
    } else if (s_check_bytes(t->expect, "success")) {
      return (!efd_parse_failed(&s) && int_result == t->output);
    } else if (s_check_bytes(t->expect, "remainder")) {
      return (
        !efd_parse_failed(&s)
     && int_result == t->output
     && s_check_bytes(t->remainder, s.input + s.pos)
      );
    }
  } else if (efd_format_is(n, "ntest")) {
    efd_num_test *t = (efd_num_test*) (*efd__o(n));
    s.input = s_raw(t->input);
    s.input_length = s_get_length(t->input);
    float_result = efd_parse_float(&s);
    if (s_check_bytes(t->expect, "failure")) {
      return efd_parse_failed(&s);
    } else if (s_check_bytes(t->expect, "success")) {
      return (!efd_parse_failed(&s) && float_result == t->output);
    } else if (s_check_bytes(t->expect, "remainder")) {
      return (
        !efd_parse_failed(&s)
     && float_result == t->output
     && s_check_bytes(t->remainder, s.input + s.pos)
      );
    }
  }

  // failsafe:
  return 0;
}

size_t test_efd_defined_tests(void) {
  size_t i;
  list *l;
  efd_node *n;
  efd_node *test;

  n = create_efd_node(EFD_NT_CONTAINER, "test");
  if (!efd_parse_file(n, "res/data/test/test.efd")) {
    cleanup_efd_node(n);
    return 1;
  }

  efd_unpack_node(n);

  l = n->b.as_container.children;
  for (i = 0; i < l_get_length(l); ++i) {
    test = (efd_node*) l_get_item(l, i);
    if (
      efd_is_type(test, EFD_NT_OBJECT)
   && (
        efd_format_is(test, "itest")
     || efd_format_is(test, "ntest")
     )
   && !_test_efd_case(test)
    ) {
      return i + 2; // 2, 3, 4, etc.
    }
  }

  cleanup_efd_node(n);

  return 0;
}

size_t test_efd_persist(void) {
  // TODO: HERE
  return 0;
}

#endif //ifndef TEST_EFD_H
