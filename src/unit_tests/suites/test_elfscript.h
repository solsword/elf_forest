#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME elfscript
#define TEST_SUITE_TESTS { \
    &setup_elfscript_tests, \
    &test_elfscript_simple_parse, \
    &test_elfscript_basic_parse, \
    &test_elfscript_real_parse, \
    &test_elfscript_defined_tests, \
    &test_elfscript_persist, \
    NULL, \
  }

#ifndef TEST_ELFSCRIPT_H
#define TEST_ELFSCRIPT_H

#include "elfscript/elfscript.h"
#include "elfscript/elfscript_parser.h"
#include "elfscript/elfscript_setup.h"

#include <stdio.h>

#include "unit_tests/test_suite.h"

size_t setup_elfscript_tests(void) {
  init_strings();
  setup_elfscript(1);
  return 0;
}

size_t test_elfscript_simple_parse(void) {
  SSTR(s_test, "test", 4);
  elfscript_node *n = create_elfscript_node(ELFSCRIPT_NT_CONTAINER, s_test);
  elfscript_index *cr = create_elfscript_index();
  if (!elfscript_parse_file(n, cr, "res/data/test/test-simple.elfscript")) {
    cleanup_elfscript_node(n);
    cleanup_elfscript_index(cr);
    return 1;
  }
  cleanup_elfscript_node(n);
  cleanup_elfscript_index(cr);
  return 0;
}

size_t test_elfscript_basic_parse(void) {
  SSTR(s_test, "test", 4);
  es_bytecode *code = es_parse_file("res/data/test/test.es");
  if (code == NULL) {
    return 1;
  }
  cleanup_es_bytecode(code);
  return 0;
}

size_t test_elfscript_real_parse(void) {
  SSTR(s_test, "test", 4);
  es_bytecode *code = es_parse_file("res/data/gen/geo.es");
  if (node == NULL) {
    return 1;
  }
  cleanup_es_bytecode(code);
  return 0;
}

// Returns 1 for success, 0 for failure.
int _test_elfscript_case(es_bytecode *code) {
  static elfscript_parse_state s;
  ptrdiff_t int_result;
  float float_result;
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
  s.error = ELFSCRIPT_PE_NO_ERROR;

  /*
   * TODO: This function!
  if (!elfscript_is_type(n, ELFSCRIPT_NT_OBJECT)) {
    fprintf(stderr, "Test ELFSCRIPT case: Bad object type!\n");
    cleanup_elfscript_index(cr);
    return 0;
  }

  if (elfscript_format_is(n, s_itest)) {
    elfscript_int_test *t = (elfscript_int_test*) (*elfscript__o(n));
    s.input = s_raw(t->input);
    s.input_length = s_count_bytes(t->input);
    int_result = elfscript_parse_int(&s);
    if (s_check_bytes(t->expect, "failure")) {
      result = elfscript_parse_failed(&s);
    } else if (s_check_bytes(t->expect, "success")) {
      result = (!elfscript_parse_failed(&s) && int_result == t->output);
    } else if (s_check_bytes(t->expect, "remainder")) {
      result = (
        !elfscript_parse_failed(&s)
     && int_result == t->output
     && s_check_bytes(t->remainder, s.input + s.pos)
      );
    }
    if (!result) {
      fprintf(stderr, "Test ELFSCRIPT case [int]: Failed.\n");
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
      if (elfscript_parse_failed(&s)) {
        fprintf(stderr, "  Result: <parse failed>\n");
        elfscript_throw_parse_error(&s);
      } else {
        fprintf(stderr, "  Result: %ld\n", int_result);
      }
      if (!elfscript_parse_atend(&s)) {
        fprintf(stderr, "  Remainder: %s\n", s.input + s.pos);
      }
    }
  } else if (elfscript_format_is(n, s_ntest)) {
    elfscript_num_test *t = (elfscript_num_test*) (*elfscript__o(n));
    s.input = s_raw(t->input);
    s.input_length = s_count_bytes(t->input);
    float_result = elfscript_parse_float(&s);
    if (s_check_bytes(t->expect, "failure")) {
      result = elfscript_parse_failed(&s);
    } else if (s_check_bytes(t->expect, "success")) {
      result = (!elfscript_parse_failed(&s) && float_result == t->output);
    } else if (s_check_bytes(t->expect, "remainder")) {
      result = (
        !elfscript_parse_failed(&s)
     && float_result == t->output
     && s_check_bytes(t->remainder, s.input + s.pos)
      );
    }
    if (!result) {
      fprintf(stderr, "Test ELFSCRIPT case [float]: Failed.\n");
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
      if (elfscript_parse_failed(&s)) {
        fprintf(stderr, "  Result: <parse failed>\n");
        elfscript_throw_parse_error(&s);
      } else {
        fprintf(stderr, "  Result: %f\n", float_result);
      }
      if (!elfscript_parse_atend(&s)) {
        fprintf(stderr, "  Remainder: %s\n", s.input + s.pos);
      }
    }
  } else if (elfscript_format_is(n, s_ptest)) {
    elfscript_parse_test *t = (elfscript_parse_test*) (*elfscript__o(n));
    s.input = s_raw(t->input);
    s.input_length = s_count_bytes(t->input);
    node_result = elfscript_parse_any(&s, cr);
    if (node_result != NULL) { cleanup_elfscript_node(node_result); }
    if (s_check_bytes(t->expect, "failure")) {
      result = elfscript_parse_failed(&s);
    } else if (s_check_bytes(t->expect, "success")) {
      result = !elfscript_parse_failed(&s);
    } else if (s_check_bytes(t->expect, "remainder")) {
      result = (
        elfscript_parse_failed(&s)
     && s_check_bytes(t->remainder, s.input + s.pos)
      );
    }
    if (!result) {
      fprintf(stderr, "Test ELFSCRIPT case [parse]: Failed.\n");
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
      if (elfscript_parse_failed(&s)) {
        fprintf(stderr, "  Result: <parse failed>\n");
        elfscript_print_parse_error(&s);
      } else {
        fprintf(stderr, "  Result: <parse succeeded>\n");
      }
      if (!elfscript_parse_atend(&s)) {
        fprintf(stderr, "  Remainder: %s\n", s.input + s.pos);
      }
    }
  } else if (elfscript_format_is(n, s_evtest)) {
    elfscript_ev_test *t = (elfscript_ev_test*) (*elfscript__o(n));
    /* DEBUG
    printf("HERE\n");
    string *cmp_r = elfscript_full_repr(t->compare);
    string *agn_r = elfscript_full_repr(t->against);
    string *trace;
    s_fprintln(stderr, cmp_r);
    if (elfscript_is_link_node(t->compare)) {
      trace = elfscript_trace_link(t->compare);
      s_fprintln(stderr, trace);
      cleanup_string(trace);
    }
    s_fprintln(stderr, agn_r);
    if (elfscript_is_link_node(t->against)) {
      trace = elfscript_trace_link(t->against);
      s_fprintln(stderr, trace);
      cleanup_string(trace);
    }
    cleanup_string(cmp_r);
    cleanup_string(agn_r);
    // END DEBUG * /
    cmp = elfscript_fresh_value(t->compare);
    agn = elfscript_fresh_value(t->against);
    result = (
      cmp != NULL
   && agn != NULL
   && elfscript_equivalent(cmp, agn)
    );
    if (!result) {
      if (cmp == NULL) {
        fprintf(stderr, "Test ELFSCRIPT case [eval]: Failed (1st eval falied).\n");
      } else if (agn == NULL) {
        fprintf(stderr, "Test ELFSCRIPT case [eval]: Failed (2nd eval failed).\n");
      } else {
        fprintf(stderr, "Test ELFSCRIPT case [eval]: Failed (results not equal).\n");
        repr = elfscript_full_repr(cmp);
        s_fprintln(stderr, repr);
        cleanup_string(repr);
        fprintf(stderr, "\n =/= \n");
        repr = elfscript_full_repr(agn);
        s_fprintln(stderr, repr);
        cleanup_string(repr);
      }
    }
    if (cmp != NULL) { cleanup_elfscript_node(cmp); }
    if (agn != NULL) { cleanup_elfscript_node(agn); }
  }
  cleanup_elfscript_index(cr);
  */
  return result;
}

size_t test_elfscript_defined_tests(void) {
  size_t i;
  SSTR(s_test, "test", 4);
  SSTR(s_itest, "itest", 5);
  SSTR(s_ntest, "ntest", 5);
  SSTR(s_ptest, "ptest", 5);
  SSTR(s_evtest, "evtest", 6);

  es_bytecode *code = es_parse_file("res/data/test/test.elfscript");
  if (code == NULL) {
    return 1;
  }

  /*
   * TODO: This function!
  children = n->b.as_container.children;
  for (i = 0; i < d_get_count(children); ++i) {
    test = (elfscript_node*) d_get_item(children, i);
    if (
      elfscript_is_type(test, ELFSCRIPT_NT_OBJECT)
   && (
        elfscript_format_is(test, s_itest)
     || elfscript_format_is(test, s_ntest)
     || elfscript_format_is(test, s_ptest)
     || elfscript_format_is(test, s_evtest)
     )
   && !_test_elfscript_case(test)
    ) {
      fqn = elfscript_build_fqn(test);
      fprintf(
        stderr,
        "Dynamic test case '%.*s' failed.\n",
        (int) s_count_bytes(fqn),
        s_raw(fqn)
      );
      return i + 2; // 2, 3, 4, etc.
    }
  }

  cleanup_elfscript_node(n);
  cleanup_elfscript_index(cr);
  */

  return 0;
}

size_t test_elfscript_persist(void) {
  // TODO: HERE
  return 0;
}

#endif //ifndef TEST_ELFSCRIPT_H
