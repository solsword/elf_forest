#if defined(EFD_REGISTER_DECLARATIONS)
/**************
 * Structures *
 **************/

struct efd_int_test_s;
typedef struct efd_int_test_s efd_int_test;

struct efd_num_test_s;
typedef struct efd_num_test_s efd_num_test;

struct efd_parse_test_s;
typedef struct efd_parse_test_s efd_parse_test;

/*************************
 * Structure Definitions *
 *************************/

struct efd_int_test_s {
  string* input;
  string* expect;
  ptrdiff_t output;
  string* remainder;
};

struct efd_num_test_s {
  string* input;
  string* expect;
  float output;
  string* remainder;
};

struct efd_parse_test_s {
  string* input;
  string* expect;
  string* remainder;
};
#elif defined(EFD_REGISTER_NAMES)
"itest",
"ntest",
"ptest",
#elif defined(EFD_REGISTER_UNPACKERS)
efd__int_test,
efd__num_test,
efd__parse_test,
#elif defined(EFD_REGISTER_PACKERS)
int_test__efd,
num_test__efd,
parse_test__efd,
#elif defined(EFD_REGISTER_COPIERS)
copy_int_test,
copy_num_test,
copy_parse_test,
#elif defined(EFD_REGISTER_DESTRUCTORS)
cleanup_v_int_test,
cleanup_v_num_test,
cleanup_v_parse_test,
#else
#ifndef INCLUDE_EFD_TESTS_H
#define INCLUDE_EFD_TESTS_H
// efd_tests.h
// Conversions efd <-> test structs

#include "efd/efd.h"
#include "efd/efd_setup.h"
#include "datatypes/string.h"

/************************
 * Conversion functions *
 ************************/

void* efd__int_test(efd_node *n) {
  efd_int_test *result = (efd_int_test*) calloc(1, sizeof(efd_int_test));
  result->input = NULL;
  result->expect = NULL;
  result->remainder = NULL;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_output, "output", 6);
  SSTR(s_remainder, "remainder", 9);

  efd_node *field;
  efd_assert_type(n, EFD_NT_CONTAINER);

  field = efd_lookup(n, s_input);
  if (field == NULL) {
    fprintf(stderr, "ERROR: itest node missing 'input' field\n");
    free(result);
    return NULL;
  }
  result->input = copy_string(*efd__s(field));

  field = efd_lookup(n, s_expect);
  if (field == NULL) {
    fprintf(stderr, "ERROR: itest node missing 'expect' field\n");
    free(result);
    return NULL;
  }
  result->expect = copy_string(*efd__s(field));

  if (s_check_bytes(result->expect, "success")) {
    field = efd_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr, "ERROR: itest 'success' node missing 'output' field\n");
      free(result);
      return NULL;
    }
    result->output = *efd__i(field);
  } else if (s_check_bytes(result->expect, "remainder")) {
    field = efd_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr, "ERROR: itest 'remainder' node missing 'output' field\n");
      free(result);
      return NULL;
    }
    result->output = *efd__i(field);

    field = efd_lookup(n, s_remainder);
    if (field == NULL) {
      fprintf(
        stderr,
        "ERROR: itest 'remainder' node missing 'remainder' field\n"
      );
      free(result);
      return NULL;
    }
    result->remainder = copy_string(*efd__s(field));
  } // "failure" -> nothing to do

  return (void*) result;
}

efd_node *int_test__efd(void *v_t) {
  efd_int_test *t = (efd_int_test*) v_t;
  efd_node *n;
  efd_node *result;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_output, "output", 6);
  SSTR(s_remainder, "remainder", 9);
  
  result = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);

  n = create_efd_node(EFD_NT_STRING, s_input);
  *efd__s(n) = copy_string(t->input);
  efd_add_child(result, n);

  n = create_efd_node(EFD_NT_STRING, s_expect);
  *efd__s(n) = copy_string(t->expect);
  efd_add_child(result, n);

  if (s_check_bytes(t->expect, "success")) {
    n = create_efd_node(EFD_NT_INTEGER, s_output);
    *efd__i(n) = t->output;
    efd_add_child(result, n);
  } else if (s_check_bytes(t->expect, "remainder")) {
    n = create_efd_node(EFD_NT_INTEGER, s_output);
    *efd__i(n) = t->output;
    efd_add_child(result, n);

    n = create_efd_node(EFD_NT_STRING, s_remainder);
    *efd__s(n) = copy_string(t->remainder);
    efd_add_child(result, n);
  }

  return result;
}

void* copy_int_test(void *v_itest) {
  efd_int_test *t = (efd_int_test*) v_itest;
  efd_int_test *result = (efd_int_test*) calloc(1, sizeof(efd_int_test));
  result->input = copy_string(t->input);
  result->expect = copy_string(t->expect);
  result->output = t->output;
  result->remainder = copy_string(t->remainder);
  return (void*) result;
}

void cleanup_v_int_test(void *v_itest) {
  efd_int_test *t = (efd_int_test*) v_itest;
  if (t->input != NULL) {
    cleanup_string(t->input);
    t->input = NULL;
  }
  if (t->expect != NULL) {
    cleanup_string(t->expect);
    t->expect = NULL;
  }
  if (t->remainder != NULL) {
    cleanup_string(t->remainder);
    t->remainder = NULL;
  }
  free(t);
}

void* efd__num_test(efd_node *n) {
  efd_num_test *result = (efd_num_test*) calloc(1, sizeof(efd_num_test));
  result->input = NULL;
  result->expect = NULL;
  result->remainder = NULL;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_output, "output", 6);
  SSTR(s_remainder, "remainder", 9);

  efd_node *field;
  efd_assert_type(n, EFD_NT_CONTAINER);

  field = efd_lookup(n, s_input);
  if (field == NULL) {
    fprintf(stderr, "ERROR: ntest node missing 'input' field\n");
    free(result);
    return NULL;
  }
  result->input = copy_string(*efd__s(field));

  field = efd_lookup(n, s_expect);
  if (field == NULL) {
    fprintf(stderr, "ERROR: ntest node missing 'expect' field\n");
    free(result);
    return NULL;
  }
  result->expect = copy_string(*efd__s(field));

  if (s_check_bytes(result->expect, "success")) {
    field = efd_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr, "ERROR: ntest 'success' node missing 'output' field\n");
      free(result);
      return NULL;
    }
    result->output = *efd__n(field);
  } else if (s_check_bytes(result->expect, "remainder")) {
    field = efd_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr, "ERROR: ntest 'remainder' node missing 'output' field\n");
      free(result);
      return NULL;
    }
    result->output = *efd__n(field);

    field = efd_lookup(n, s_remainder);
    if (field == NULL) {
      fprintf(
        stderr,
        "ERROR: ntest 'remainder' node missing 'remainder' field\n"
      );
      free(result);
      return NULL;
    }
    result->remainder = copy_string(*efd__s(field));
  } // "failure" -> nothing to do

  return (void*) result;
}

efd_node *num_test__efd(void *v_t) {
  efd_num_test *t = (efd_num_test*) v_t;
  efd_node *n;
  efd_node *result;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_output, "output", 6);
  SSTR(s_remainder, "remainder", 9);
  
  result = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);

  n = create_efd_node(EFD_NT_STRING, s_input);
  *efd__s(n) = copy_string(t->input);
  efd_add_child(result, n);

  n = create_efd_node(EFD_NT_STRING, s_expect);
  *efd__s(n) = copy_string(t->expect);
  efd_add_child(result, n);

  if (s_check_bytes(t->expect, "success")) {
    n = create_efd_node(EFD_NT_NUMBER, s_output);
    *efd__n(n) = t->output;
    efd_add_child(result, n);
  } else if (s_check_bytes(t->expect, "remainder")) {
    n = create_efd_node(EFD_NT_NUMBER, s_output);
    *efd__n(n) = t->output;
    efd_add_child(result, n);

    n = create_efd_node(EFD_NT_STRING, s_remainder);
    *efd__s(n) = copy_string(t->remainder);
    efd_add_child(result, n);
  }

  return result;
}

void* copy_num_test(void *v_ntest) {
  efd_num_test *t = (efd_num_test*) v_ntest;
  efd_num_test *result = (efd_num_test*) calloc(1, sizeof(efd_num_test));
  result->input = copy_string(t->input);
  result->expect = copy_string(t->expect);
  result->output = t->output;
  result->remainder = copy_string(t->remainder);
  return (void*) result;
}

void cleanup_v_num_test(void *v_ntest) {
  efd_num_test *t = (efd_num_test*) v_ntest;
  if (t->input != NULL) {
    cleanup_string(t->input);
    t->input = NULL;
  }
  if (t->expect != NULL) {
    cleanup_string(t->expect);
    t->expect = NULL;
  }
  if (t->remainder != NULL) {
    cleanup_string(t->remainder);
    t->remainder = NULL;
  }
  free(t);
}

void* efd__parse_test(efd_node *n) {
  efd_parse_test *result = (efd_parse_test*) calloc(1, sizeof(efd_parse_test));
  result->input = NULL;
  result->expect = NULL;
  result->remainder = NULL;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_remainder, "remainder", 9);

  efd_node *field;
  efd_assert_type(n, EFD_NT_CONTAINER);

  field = efd_lookup(n, s_input);
  if (field == NULL) {
    fprintf(stderr, "ERROR: ptest node missing 'input' field\n");
    free(result);
    return NULL;
  }
  result->input = copy_string(*efd__s(field));

  field = efd_lookup(n, s_expect);
  if (field == NULL) {
    fprintf(stderr, "ERROR: ptest node missing 'expect' field\n");
    free(result);
    return NULL;
  }
  result->expect = copy_string(*efd__s(field));

  if (s_equals(result->expect, s_remainder)) {
    field = efd_lookup(n, s_remainder);
    if (field == NULL) {
      fprintf(
        stderr,
        "ERROR: ptest 'remainder' node missing 'remainder' field\n"
      );
      free(result);
      return NULL;
    }
    result->remainder = copy_string(*efd__s(field));
  } // "failure" -> nothing to do

  return (void*) result;
}

efd_node *parse_test__efd(void *v_t) {
  efd_parse_test *t = (efd_parse_test*) v_t;
  efd_node *n;
  efd_node *result;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_remainder, "remainder", 9);
  
  result = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);

  n = create_efd_node(EFD_NT_STRING, s_input);
  *efd__s(n) = copy_string(t->input);
  efd_add_child(result, n);

  n = create_efd_node(EFD_NT_STRING, s_expect);
  *efd__s(n) = copy_string(t->expect);
  efd_add_child(result, n);

  if (s_equals(t->expect, s_remainder)) {
    n = create_efd_node(EFD_NT_STRING, s_remainder);
    *efd__s(n) = copy_string(t->remainder);
    efd_add_child(result, n);
  }

  return result;
}

void* copy_parse_test(void *v_ptest) {
  efd_parse_test *t = (efd_parse_test*) v_ptest;
  efd_parse_test *result = (efd_parse_test*) calloc(1, sizeof(efd_parse_test));
  result->input = copy_string(t->input);
  result->expect = copy_string(t->expect);
  result->remainder = copy_string(t->remainder);
  return (void*) result;
}

void cleanup_v_parse_test(void *v_ptest) {
  efd_parse_test *t = (efd_parse_test*) v_ptest;
  if (t->input != NULL) {
    cleanup_string(t->input);
    t->input = NULL;
  }
  if (t->expect != NULL) {
    cleanup_string(t->expect);
    t->expect = NULL;
  }
  if (t->remainder != NULL) {
    cleanup_string(t->remainder);
    t->remainder = NULL;
  }
  free(t);
}

#endif // INCLUDE_EFD_TESTS_H
#endif // EFD_REGISTRATION
