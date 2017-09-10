#if defined(ELFSCRIPT_REGISTER_DECLARATIONS)
/**************
 * Structures *
 **************/

struct elfscript_int_test_s;
typedef struct elfscript_int_test_s elfscript_int_test;

struct elfscript_num_test_s;
typedef struct elfscript_num_test_s elfscript_num_test;

struct elfscript_parse_test_s;
typedef struct elfscript_parse_test_s elfscript_parse_test;

struct elfscript_ev_test_s;
typedef struct elfscript_ev_test_s elfscript_ev_test;

/*************************
 * Structure Definitions *
 *************************/

struct elfscript_int_test_s {
  string* input;
  string* expect;
  ptrdiff_t output;
  string* remainder;
};

struct elfscript_num_test_s {
  string* input;
  string* expect;
  float output;
  string* remainder;
};

struct elfscript_parse_test_s {
  string* input;
  string* expect;
  string* remainder;
};

struct elfscript_ev_test_s {
  elfscript_node* compare;
  elfscript_node* against;
};
#elif defined(ELFSCRIPT_REGISTER_FORMATS)
{
  .key = "itest",
  .unpacker = elfscript__int_test,
  .packer = int_test__elfscript,
  .copier = copy_int_test,
  .destructor = cleanup_v_int_test
},
{
  .key = "ntest",
  .unpacker = elfscript__num_test,
  .packer = num_test__elfscript,
  .copier = copy_num_test,
  .destructor = cleanup_v_num_test
},
{
  .key = "ptest",
  .unpacker = elfscript__parse_test,
  .packer = parse_test__elfscript,
  .copier = copy_parse_test,
  .destructor = cleanup_v_parse_test
},
{
  .key = "evtest",
  .unpacker = elfscript__ev_test,
  .packer = ev_test__elfscript,
  .copier = copy_ev_test,
  .destructor = cleanup_v_ev_test
},
#else
#ifndef INCLUDE_ELFSCRIPT_CONV_TESTS_H
#define INCLUDE_ELFSCRIPT_CONV_TESTS_H
// elfscript_tests.h
// Conversions elfscript <-> test structs

#include "elfscript/elfscript.h"
#include "datatypes/string.h"

/************************
 * Conversion functions *
 ************************/

void* elfscript__int_test(elfscript_node *n) {
  elfscript_int_test *result = (elfscript_int_test*) malloc(sizeof(elfscript_int_test));
  result->input = NULL;
  result->expect = NULL;
  result->remainder = NULL;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_output, "output", 6);
  SSTR(s_remainder, "remainder", 9);

  elfscript_node *field;
  elfscript_assert_type(n, ELFSCRIPT_NT_CONTAINER);

  field = elfscript_lookup(n, s_input);
  if (field == NULL) {
    fprintf(stderr, "ERROR: itest node missing 'input' field.\n");
    free(result);
    return NULL;
  }
  result->input = copy_string(*elfscript__s(field));

  field = elfscript_lookup(n, s_expect);
  if (field == NULL) {
    fprintf(stderr, "ERROR: itest node missing 'expect' field.\n");
    free(result);
    return NULL;
  }
  result->expect = copy_string(*elfscript__s(field));

  if (s_check_bytes(result->expect, "success")) {
    field = elfscript_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr, "ERROR: itest 'success' node missing 'output' field.\n");
      free(result);
      return NULL;
    }
    result->output = *elfscript__i(field);
  } else if (s_equals(result->expect, s_remainder)) {
    field = elfscript_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr,"ERROR: itest 'remainder' node missing 'output' field.\n");
      free(result);
      return NULL;
    }
    result->output = *elfscript__i(field);

    field = elfscript_lookup(n, s_remainder);
    if (field == NULL) {
      fprintf(
        stderr,
        "ERROR: itest 'remainder' node missing 'remainder' field.\n"
      );
      free(result);
      return NULL;
    }
    result->remainder = copy_string(*elfscript__s(field));
  } // "failure" -> nothing to do

  return (void*) result;
}

elfscript_node *int_test__elfscript(void *v_t) {
  elfscript_int_test *t = (elfscript_int_test*) v_t;
  elfscript_node *result;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_output, "output", 6);
  SSTR(s_remainder, "remainder", 9);
  
  result = create_elfscript_node(ELFSCRIPT_NT_CONTAINER, ELFSCRIPT_ANON_NAME, NULL);

  elfscript_add_child(result, construct_elfscript_str_node(s_input, NULL, t->input));

  elfscript_add_child(result, construct_elfscript_str_node(s_expect, NULL, t->expect));

  elfscript_add_child(result, construct_elfscript_int_node(s_output, NULL, t->output));

  if (s_equals(t->expect, s_remainder)) {
    elfscript_add_child(
      result,
      construct_elfscript_str_node(s_remainder, NULL, t->remainder)
    );
  }

  return result;
}

void* copy_int_test(void *v_itest) {
  elfscript_int_test *t = (elfscript_int_test*) v_itest;
  elfscript_int_test *result = (elfscript_int_test*) malloc(sizeof(elfscript_int_test));
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
  elfscript_int_test *t = (elfscript_int_test*) v_itest;
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

void* elfscript__num_test(elfscript_node *n) {
  elfscript_num_test *result = (elfscript_num_test*) malloc(sizeof(elfscript_num_test));
  result->input = NULL;
  result->expect = NULL;
  result->remainder = NULL;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_output, "output", 6);
  SSTR(s_remainder, "remainder", 9);

  elfscript_node *field;
  elfscript_assert_type(n, ELFSCRIPT_NT_CONTAINER);

  field = elfscript_lookup(n, s_input);
  if (field == NULL) {
    fprintf(stderr, "ERROR: ntest node missing 'input' field.\n");
    free(result);
    return NULL;
  }
  result->input = copy_string(*elfscript__s(field));

  field = elfscript_lookup(n, s_expect);
  if (field == NULL) {
    fprintf(stderr, "ERROR: ntest node missing 'expect' field.\n");
    free(result);
    return NULL;
  }
  result->expect = copy_string(*elfscript__s(field));

  if (s_check_bytes(result->expect, "success")) {
    field = elfscript_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr, "ERROR: ntest 'success' node missing 'output' field.\n");
      free(result);
      return NULL;
    }
    result->output = *elfscript__n(field);
  } else if (s_equals(result->expect, s_remainder)) {
    field = elfscript_lookup(n, s_output);
    if (field == NULL) {
      fprintf(stderr,"ERROR: ntest 'remainder' node missing 'output' field.\n");
      free(result);
      return NULL;
    }
    result->output = *elfscript__n(field);

    field = elfscript_lookup(n, s_remainder);
    if (field == NULL) {
      fprintf(
        stderr,
        "ERROR: ntest 'remainder' node missing 'remainder' field.\n"
      );
      free(result);
      return NULL;
    }
    result->remainder = copy_string(*elfscript__s(field));
  } // "failure" -> nothing to do

  return (void*) result;
}

elfscript_node *num_test__elfscript(void *v_t) {
  elfscript_num_test *t = (elfscript_num_test*) v_t;
  elfscript_node *result;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_output, "output", 6);
  SSTR(s_remainder, "remainder", 9);
  
  result = create_elfscript_node(ELFSCRIPT_NT_CONTAINER, ELFSCRIPT_ANON_NAME, NULL);

  elfscript_add_child(result, construct_elfscript_str_node(s_input, NULL, t->input));

  elfscript_add_child(result, construct_elfscript_str_node(s_expect, NULL, t->expect));

  elfscript_add_child(result, construct_elfscript_num_node(s_output, NULL, t->output));

  if (s_equals(t->expect, s_remainder)) {
    elfscript_add_child(
      result,
      construct_elfscript_str_node(s_remainder, NULL, t->remainder)
    );
  }

  return result;
}

void* copy_num_test(void *v_ntest) {
  elfscript_num_test *t = (elfscript_num_test*) v_ntest;
  elfscript_num_test *result = (elfscript_num_test*) malloc(sizeof(elfscript_num_test));
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
  elfscript_num_test *t = (elfscript_num_test*) v_ntest;
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

void* elfscript__parse_test(elfscript_node *n) {
  elfscript_parse_test *result = (elfscript_parse_test*) malloc(sizeof(elfscript_parse_test));
  result->input = NULL;
  result->expect = NULL;
  result->remainder = NULL;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_remainder, "remainder", 9);

  elfscript_node *field;
  elfscript_assert_type(n, ELFSCRIPT_NT_CONTAINER);

  field = elfscript_lookup(n, s_input);
  if (field == NULL) {
    fprintf(stderr, "ERROR: ptest node missing 'input' field.\n");
    free(result);
    return NULL;
  }
  result->input = copy_string(*elfscript__s(field));

  field = elfscript_lookup(n, s_expect);
  if (field == NULL) {
    fprintf(stderr, "ERROR: ptest node missing 'expect' field.\n");
    free(result);
    return NULL;
  }
  result->expect = copy_string(*elfscript__s(field));

  if (s_equals(result->expect, s_remainder)) {
    field = elfscript_lookup(n, s_remainder);
    if (field == NULL) {
      fprintf(
        stderr,
        "ERROR: ptest 'remainder' node missing 'remainder' field.\n"
      );
      free(result);
      return NULL;
    }
    result->remainder = copy_string(*elfscript__s(field));
  } // "failure" -> nothing to do

  return (void*) result;
}

elfscript_node *parse_test__elfscript(void *v_t) {
  elfscript_parse_test *t = (elfscript_parse_test*) v_t;
  elfscript_node *result;
  SSTR(s_input, "input", 5);
  SSTR(s_expect, "expect", 6);
  SSTR(s_remainder, "remainder", 9);
  
  result = create_elfscript_node(ELFSCRIPT_NT_CONTAINER, ELFSCRIPT_ANON_NAME, NULL);

  elfscript_add_child(result, construct_elfscript_str_node(s_input, NULL, t->input));

  elfscript_add_child(result, construct_elfscript_str_node(s_expect, NULL, t->expect));

  if (s_equals(t->expect, s_remainder)) {
    elfscript_add_child(
      result,
      construct_elfscript_str_node(s_remainder, NULL, t->remainder)
    );
  }

  return result;
}

void* copy_parse_test(void *v_ptest) {
  elfscript_parse_test *t = (elfscript_parse_test*) v_ptest;
  elfscript_parse_test *result = (elfscript_parse_test*) malloc(sizeof(elfscript_parse_test));
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
  elfscript_parse_test *t = (elfscript_parse_test*) v_ptest;
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

void* elfscript__ev_test(elfscript_node *n) {
  elfscript_ev_test *result = (elfscript_ev_test*) malloc(sizeof(elfscript_ev_test));
  result->compare = NULL;
  result->against = NULL;

  elfscript_assert_type(n, ELFSCRIPT_NT_CONTAINER);

  result->compare = elfscript_nth(n, 0);
  if (result->compare == NULL) {
    fprintf(stderr, "ERROR: evtest node has no children.\n");
    free(result);
    return NULL;
  }
  result->compare = copy_elfscript_node(result->compare);

  result->against = elfscript_nth(n, 1);
  if (result->against == NULL) {
    fprintf(stderr, "ERROR: evtest node has only one child.\n");
    free(result);
    return NULL;
  }
  result->against = copy_elfscript_node(result->against);

  return (void*) result;
}

elfscript_node *ev_test__elfscript(void *v_t) {
  elfscript_ev_test *t = (elfscript_ev_test*) v_t;
  elfscript_node *result;

  result = create_elfscript_node(ELFSCRIPT_NT_CONTAINER, ELFSCRIPT_ANON_NAME, NULL);

  elfscript_add_child(result, copy_elfscript_node(t->compare));
  elfscript_add_child(result, copy_elfscript_node(t->against));

  return result;
}

void* copy_ev_test(void *v_ptest) {
  elfscript_ev_test *t = (elfscript_ev_test*) v_ptest;
  elfscript_ev_test *result = (elfscript_ev_test*) malloc(sizeof(elfscript_ev_test));
  result->compare = copy_elfscript_node(t->compare);
  result->against = copy_elfscript_node(t->against);
  return (void*) result;
}

void cleanup_v_ev_test(void *v_ptest) {
  elfscript_ev_test *t = (elfscript_ev_test*) v_ptest;
  if (t->compare != NULL) {
    cleanup_elfscript_node(t->compare);
    t->compare = NULL;
  }
  if (t->against != NULL) {
    cleanup_elfscript_node(t->against);
    t->against = NULL;
  }
  free(t);
}

#endif // INCLUDE_ELFSCRIPT_CONV_TESTS_H
#endif // ELFSCRIPT_REGISTRATION
