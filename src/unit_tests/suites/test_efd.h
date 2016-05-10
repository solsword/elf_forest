#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME efd
#define TEST_SUITE_TESTS { \
    &setup_efd_tests, \
    &test_efd_simple_parse, \
    &test_efd_basic_parse, \
    &test_efd_real_parse, \
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
  SSTR(s_test, "test", 4);
  efd_node *n = create_efd_node(EFD_NT_CONTAINER, s_test);
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
  SSTR(s_test, "test", 4);
  efd_node *n = create_efd_node(EFD_NT_CONTAINER, s_test);
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

size_t test_efd_real_parse(void) {
  SSTR(s_test, "test", 4);
  efd_node *n = create_efd_node(EFD_NT_CONTAINER, s_test);
  efd_index *cr = create_efd_index();
  if (!efd_parse_file(n, cr, "res/data/gen/geo.efd")) {
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
  efd_node *cmp, *agn;
  string *repr;
  int result = 0;
  SSTR(s_itest, "itest", 5);
  SSTR(s_ntest, "ntest", 5);
  SSTR(s_ptest, "ptest", 5);
  SSTR(s_evtest, "evtest", 6);

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

  if (efd_format_is(n, s_itest)) {
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
  } else if (efd_format_is(n, s_ntest)) {
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
  } else if (efd_format_is(n, s_ptest)) {
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
  } else if (efd_format_is(n, s_evtest)) {
    efd_ev_test *t = (efd_ev_test*) (*efd__o(n));
    /* DEBUG
    printf("HERE\n");
    string *cmp_r = efd_full_repr(t->compare);
    string *agn_r = efd_full_repr(t->against);
    string *trace;
    s_fprintln(stderr, cmp_r);
    if (efd_is_link_node(t->compare)) {
      trace = efd_trace_link(t->compare);
      s_fprintln(stderr, trace);
      cleanup_string(trace);
    }
    s_fprintln(stderr, agn_r);
    if (efd_is_link_node(t->against)) {
      trace = efd_trace_link(t->against);
      s_fprintln(stderr, trace);
      cleanup_string(trace);
    }
    cleanup_string(cmp_r);
    cleanup_string(agn_r);
    // END DEBUG */
    cmp = efd_fresh_value(t->compare);
    agn = efd_fresh_value(t->against);
    result = (
      cmp != NULL
   && agn != NULL
   && efd_equivalent(cmp, agn)
    );
    if (!result) {
      if (cmp == NULL) {
        fprintf(stderr, "Test EFD case [eval]: Failed (1st eval falied).\n");
      } else if (agn == NULL) {
        fprintf(stderr, "Test EFD case [eval]: Failed (2nd eval failed).\n");
      } else {
        fprintf(stderr, "Test EFD case [eval]: Failed (results not equal).\n");
        repr = efd_full_repr(cmp);
        s_fprintln(stderr, repr);
        cleanup_string(repr);
        fprintf(stderr, "\n =/= \n");
        repr = efd_full_repr(agn);
        s_fprintln(stderr, repr);
        cleanup_string(repr);
      }
    }
    if (cmp != NULL) { cleanup_efd_node(cmp); }
    if (agn != NULL) { cleanup_efd_node(agn); }
  }
  cleanup_efd_index(cr);
  return result;
}

size_t test_efd_defined_tests(void) {
  size_t i;
  dictionary *children;
  efd_node *n;
  efd_node *test;
  efd_index *cr = create_efd_index();
  string *fqn;
  SSTR(s_test, "test", 4);
  SSTR(s_itest, "itest", 5);
  SSTR(s_ntest, "ntest", 5);
  SSTR(s_ptest, "ptest", 5);
  SSTR(s_evtest, "evtest", 6);

  if (!efd_parse_file(EFD_ROOT, cr, "res/data/test/test.efd")) {
    cleanup_efd_index(cr);
    return 1;
  }

  efd_unpack_node(EFD_ROOT, cr);

  n = efdx(EFD_ROOT, s_test); // test.test

  children = n->b.as_container.children;
  for (i = 0; i < d_get_count(children); ++i) {
    test = (efd_node*) d_get_item(children, i);
    if (
      efd_is_type(test, EFD_NT_OBJECT)
   && (
        efd_format_is(test, s_itest)
     || efd_format_is(test, s_ntest)
     || efd_format_is(test, s_ptest)
     || efd_format_is(test, s_evtest)
     )
   && !_test_efd_case(test)
    ) {
      fqn = efd_build_fqn(test);
      fprintf(
        stderr,
        "Dynamic test case '%.*s' failed.\n",
        (int) s_get_length(fqn),
        s_raw(fqn)
      );
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
