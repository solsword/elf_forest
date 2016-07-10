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

struct efd_ev_test_s;
typedef struct efd_ev_test_s efd_ev_test;

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

struct efd_ev_test_s {
  efd_node* compare;
  efd_node* against;
};
#elif defined(EFD_REGISTER_FORMATS)
{
  .key = "itest",
  .unpacker = efd__int_test,
  .packer = int_test__efd,
  .copier = copy_int_test,
  .destructor = cleanup_v_int_test
},
{
  .key = "ntest",
  .unpacker = efd__num_test,
  .packer = num_test__efd,
  .copier = copy_num_test,
  .destructor = cleanup_v_num_test
},
{
  .key = "ptest",
  .unpacker = efd__parse_test,
  .packer = parse_test__efd,
  .copier = copy_parse_test,
  .destructor = cleanup_v_parse_test
},
{
  .key = "evtest",
  .unpacker = efd__ev_test,
  .packer = ev_test__efd,
  .copier = copy_ev_test,
  .destructor = cleanup_v_ev_test
},
#else
#ifndef INCLUDE_EFD_CONV_TESTS_H
#define INCLUDE_EFD_CONV_TESTS_H
// efd_tests.h
// Conversions efd <-> test structs

#include "efd/efd.h"
#include "datatypes/string.h"

/************************
 * Conversion functions *
 ************************/

void* efd__int_test(efd_node *n) {
  efd_int_test *result = (efd_int_test*) malloc(sizeof(efd_int_test));
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
    fprintf(stderr, "ERROR: itest node missing 'input' field.\n");
    free(result);
    return NULL;
  }
  result->input = copy_string(*efd__s(field));

  field = efd_lookup(n, s_expect);
  if (field == NULL) {
    fprintf(stderr, "ERROR: itest node missing 'expect' field.\n");
    free(result);
    return NULL;
  }
  result->expect = copy_string(*efd__s(field));

  if (s_check_bytes(result->expect, "success")) {
    field = efd_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr, "ERROR: itest 'success' node missing 'output' field.\n");
      free(result);
      return NULL;
    }
    result->output = *efd__i(field);
  } else if (s_equals(result->expect, s_remainder)) {
    field = efd_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr,"ERROR: itest 'remainder' node missing 'output' field.\n");
      free(result);
      return NULL;
    }
    result->output = *efd__i(field);

    field = efd_lookup(n, s_remainder);
    if (field == NULL) {
      fprintf(
        stderr,
        "ERROR: itest 'remainder' node missing 'remainder' field.\n"
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

  n = construct_efd_str_node(s_input, t->input);
  efd_add_child(result, n);

  n = construct_efd_str_node(s_expect, t->expect);
  efd_add_child(result, n);

  n = construct_efd_int_node(s_output, t->output);
  efd_add_child(result, n);

  if (s_equals(t->expect, s_remainder)) {
    n = construct_efd_str_node(s_remainder, t->remainder);
    efd_add_child(result, n);
  }

  return result;
}

void* copy_int_test(void *v_itest) {
  efd_int_test *t = (efd_int_test*) v_itest;
  efd_int_test *result = (efd_int_test*) malloc(sizeof(efd_int_test));
  result->input = copy_string(t->input);
  result->expect = copy_string(t->expect);
  result->output = t->output;
  if (t->remainder == NULL) {
    result->remainder = NULL;
  } else {
    result->remainder = copy_string(t->remainder);
  }
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
  efd_num_test *result = (efd_num_test*) malloc(sizeof(efd_num_test));
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
    fprintf(stderr, "ERROR: ntest node missing 'input' field.\n");
    free(result);
    return NULL;
  }
  result->input = copy_string(*efd__s(field));

  field = efd_lookup(n, s_expect);
  if (field == NULL) {
    fprintf(stderr, "ERROR: ntest node missing 'expect' field.\n");
    free(result);
    return NULL;
  }
  result->expect = copy_string(*efd__s(field));

  if (s_check_bytes(result->expect, "success")) {
    field = efd_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr, "ERROR: ntest 'success' node missing 'output' field.\n");
      free(result);
      return NULL;
    }
    result->output = *efd__n(field);
  } else if (s_equals(result->expect, s_remainder)) {
    field = efd_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr,"ERROR: ntest 'remainder' node missing 'output' field.\n");
      free(result);
      return NULL;
    }
    result->output = *efd__n(field);

    field = efd_lookup(n, s_remainder);
    if (field == NULL) {
      fprintf(
        stderr,
        "ERROR: ntest 'remainder' node missing 'remainder' field.\n"
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

  n = construct_efd_str_node(s_input, t->input);
  efd_add_child(result, n);

  n = construct_efd_str_node(s_expect, t->expect);
  efd_add_child(result, n);

  n = construct_efd_num_node(s_output, t->output);
  efd_add_child(result, n);

  if (s_equals(t->expect, s_remainder)) {
    n = construct_efd_str_node(s_remainder, t->remainder);
    efd_add_child(result, n);
  }

  return result;
}

void* copy_num_test(void *v_ntest) {
  efd_num_test *t = (efd_num_test*) v_ntest;
  efd_num_test *result = (efd_num_test*) malloc(sizeof(efd_num_test));
  result->input = copy_string(t->input);
  result->expect = copy_string(t->expect);
  result->output = t->output;
  if (t->remainder == NULL) {
    result->remainder = NULL;
  } else {
    result->remainder = copy_string(t->remainder);
  }
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
  efd_parse_test *result = (efd_parse_test*) malloc(sizeof(efd_parse_test));
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
    fprintf(stderr, "ERROR: ptest node missing 'input' field.\n");
    free(result);
    return NULL;
  }
  result->input = copy_string(*efd__s(field));

  field = efd_lookup(n, s_expect);
  if (field == NULL) {
    fprintf(stderr, "ERROR: ptest node missing 'expect' field.\n");
    free(result);
    return NULL;
  }
  result->expect = copy_string(*efd__s(field));

  if (s_equals(result->expect, s_remainder)) {
    field = efd_lookup(n, s_remainder);
    if (field == NULL) {
      fprintf(
        stderr,
        "ERROR: ptest 'remainder' node missing 'remainder' field.\n"
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

  n = construct_efd_str_node(s_input, t->input);
  efd_add_child(result, n);

  n = construct_efd_str_node(s_expect, t->expect);
  efd_add_child(result, n);

  if (s_equals(t->expect, s_remainder)) {
    n = construct_efd_str_node(s_remainder, t->remainder);
    efd_add_child(result, n);
  }

  return result;
}

void* copy_parse_test(void *v_ptest) {
  efd_parse_test *t = (efd_parse_test*) v_ptest;
  efd_parse_test *result = (efd_parse_test*) malloc(sizeof(efd_parse_test));
  result->input = copy_string(t->input);
  result->expect = copy_string(t->expect);
  if (t->remainder == NULL) {
    result->remainder = NULL;
  } else {
    result->remainder = copy_string(t->remainder);
  }
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

void* efd__ev_test(efd_node *n) {
  efd_ev_test *result = (efd_ev_test*) malloc(sizeof(efd_ev_test));
  result->compare = NULL;
  result->against = NULL;

  efd_assert_type(n, EFD_NT_CONTAINER);

  result->compare = efd_nth(n, 0);
  if (result->compare == NULL) {
    fprintf(stderr, "ERROR: evtest node has no children.\n");
    free(result);
    return NULL;
  }
  result->compare = copy_efd_node(result->compare);

  result->against = efd_nth(n, 1);
  if (result->against == NULL) {
    fprintf(stderr, "ERROR: evtest node has only one child.\n");
    free(result);
    return NULL;
  }
  result->against = copy_efd_node(result->against);

  return (void*) result;
}

efd_node *ev_test__efd(void *v_t) {
  efd_ev_test *t = (efd_ev_test*) v_t;
  efd_node *result;

  result = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME);

  efd_add_child(result, copy_efd_node(t->compare));
  efd_add_child(result, copy_efd_node(t->against));

  return result;
}

void* copy_ev_test(void *v_ptest) {
  efd_ev_test *t = (efd_ev_test*) v_ptest;
  efd_ev_test *result = (efd_ev_test*) malloc(sizeof(efd_ev_test));
  result->compare = copy_efd_node(t->compare);
  result->against = copy_efd_node(t->against);
  return (void*) result;
}

void cleanup_v_ev_test(void *v_ptest) {
  efd_ev_test *t = (efd_ev_test*) v_ptest;
  if (t->compare != NULL) {
    cleanup_efd_node(t->compare);
    t->compare = NULL;
  }
  if (t->against != NULL) {
    cleanup_efd_node(t->against);
    t->against = NULL;
  }
  free(t);
}

#endif // INCLUDE_EFD_CONV_TESTS_H
#endif // EFD_REGISTRATION
