// efd_parser.c
// Parsing for the Elf Forest Data format.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include <string.h>

#include "util.h"

#include "efd_parser.h"
#include "efd.h"

/**********************
 * Boilerplate Macros *
 **********************/

#define SKIP_OR_ELSE(S, X) \
  efd_parse_skip(S); \
  if (efd_parse_failed(S)) { \
    if (s->context == NULL) { \
      s->context = (X); \
    } \
    return; \
  }

/*************
 * Functions *
 *************/

void efd_parse_copy_state(efd_parse_state *from, efd_parse_state *to) {
  to->pos = from->pos;
  to->lineno = from->lineno;
  to->filename = from->filename;
  to->context = from->context;
  to->error = from->error;
}

int efd_parse_file(efd_node *parent, char const * const filename) {
  static efd_parse_state s;
  char *contents;

#ifdef DEBUG
  if (s.input != NULL || s.context != NULL) {
    fprintf(stderr, "Error: efd_parse_file called w/ input already set up!\n");
    return 0;
  }
#endif
  s.input = NULL;
  s.input_length = 0;
  s.pos = 0;
  s.filename = filename;
  s.lineno = 0;
  s.context = NULL;
  s.error = EFD_PE_NO_ERROR;

  contents = load_file(filename, (size_t*) &(s.input_length));
  s.input = contents;

  efd_assert_type(parent, EFD_NT_CONTAINER);
  efd_parse_children(parent, &s);

  if (efd_parse_failed(&s)) {
    efd_throw_parse_error(&s);
    s.input = NULL;
    s.input_length = 0;
    free(contents);
    s.context = NULL;
    return 0;
  } else {
    s.input = NULL;
    s.input_length = 0;
    free(contents);
    s.context = NULL;
    return 1;
  }
}

efd_node* efd_parse_any(efd_parse_state *s) {
  efd_node_type type;
  static char name[EFD_NODE_NAME_SIZE];

  efd_parse_open(s);
  if (efd_parse_failed(s)) {
    if (efd_parse_atend(s)) {
      s->error = EFD_PE_NO_ERROR;
      // if there's nothing here (as opposed to something malformed) then we'll
      // just return NULL without an error.
    }
    return NULL;
  }

  type = efd_parse_type(s);
  if (efd_parse_failed(s)) { return NULL; }

  efd_parse_name(s, name);
  if (efd_parse_failed(s)) { return NULL; }

  efd_node *result = create_efd_node(type, name);

  switch(type) {
    default:
    case EFD_NT_INVALID:
    case EFD_NT_PROTO:
      s->error = EFD_PE_MALFORMED;
      s->context = "node (invalid type)";
      return NULL;
    case EFD_NT_CONTAINER:
      efd_parse_children(result, s);
      break;
    case EFD_NT_OBJECT:
      efd_parse_proto(result, s);
      break;
    case EFD_NT_INTEGER:
      efd_parse_integer(result, s);
      break;
    case EFD_NT_NUMBER:
      efd_parse_number(result, s);
      break;
    case EFD_NT_STRING:
      efd_parse_string(result, s);
      break;
    case EFD_NT_ARRAY_OBJ:
      efd_parse_obj_array(result, s);
      break;
    case EFD_NT_ARRAY_INT:
      efd_parse_int_array(result, s);
      break;
    case EFD_NT_ARRAY_NUM:
      efd_parse_num_array(result, s);
      break;
    case EFD_NT_ARRAY_STR:
      efd_parse_str_array(result, s);
      break;
  }
  if (efd_parse_failed(s)) {
    cleanup_efd_node(result);
    return NULL;
  }

  efd_parse_close(s);
  if (efd_parse_failed(s)) {
    cleanup_efd_node(result);
    return NULL;
  }

  return result;
}

// Parsing functions for the EFD primitive types:
//-----------------------------------------------

void efd_parse_children(efd_node *result, efd_parse_state *s) {
  static char schema[EFD_NODE_NAME_SIZE];
  efd_parse_state back;
  list *children;
  efd_node *child;

  children = result->b.as_container.children;

  efd_parse_copy_state(s, &back);
  efd_parse_schema(s, schema);
  if (efd_parse_failed(s)) {
    if (s->error == EFD_PE_MISSING) {
      efd_parse_copy_state(&back, s);
      result->b.as_container.schema = NULL;
    } else {
      return; // propagate the error upwards
    }
  } else {
    result->b.as_container.schema = efd_fetch_schema(schema);
  }

  // parse any number of children and add them:
  while (1) {
    efd_parse_copy_state(s, &back);
    child = efd_parse_any(s);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        // we've probably hit the end of this object: return w/out error
        efd_parse_copy_state(&back, s);
      } // otherwise return w/ error
      break;
    } else if (child == NULL) {
      break;
    } else {
      l_append_element(children, (void*) child);
    }
  }
}

void efd_parse_proto(efd_node *result, efd_parse_state *s) {
  static char format[EFD_NODE_NAME_SIZE];

  printf("Parse proto!\n");
  efd_parse_schema(s, format);
  if (efd_parse_failed(s)) {
    return;
  }

  strncpy(result->b.as_proto.format, format, EFD_OBJECT_FORMAT_SIZE - 1);
  result->b.as_proto.format[EFD_OBJECT_FORMAT_SIZE-1] = '\0';

  efd_node *proto = create_efd_node(EFD_NT_CONTAINER, EFD_PROTO_NAME);
  efd_parse_children(proto, s);
  if (efd_parse_failed(s)) {
    cleanup_efd_node(proto);
    return;
  }

  result->b.as_proto.input = proto;
}

void efd_parse_integer(efd_node *result, efd_parse_state *s) {
  ptrdiff_t i = efd_parse_int(s);
  if (efd_parse_failed(s)) {
    return;
  } else {
    result->b.as_integer.value = i;
  }
}

void efd_parse_number(efd_node *result, efd_parse_state *s) {
  float n = efd_parse_float(s);
  if (efd_parse_failed(s)) {
    return;
  } else {
    result->b.as_number.value = n;
  }
}

void efd_parse_string(efd_node *result, efd_parse_state *s) {
  ptrdiff_t start, end;

  efd_grab_string_limits(s, &start, &end);
  if (efd_parse_failed(s)) {
    return;
  }

  result->b.as_string.value = create_string_from_chars(
    s->input + start,
    end - start
  );
}

void efd_parse_obj_array(efd_node *result, efd_parse_state *s) {
  size_t i;
  void *val;
  list *l = create_list();

  while (1) {
    val = efd_parse_ref(s); // TODO: THIS
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        cleanup_list(l);
        return;
      }
    } else {
      l_append_element(l, val);
    }
    efd_parse_sep(s);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        cleanup_list(l);
        return;
      }
    }
  }

  result->b.as_obj_array.count = l_get_length(l);
#ifdef DEBUG
  if (result->b.as_obj_array.values != NULL) {
    fprintf(stderr, "EFD object array already had values while parsing!\n");
    exit(-1);
  }
#endif
  result->b.as_obj_array.values = (void*) malloc(
    result->b.as_obj_array.count * sizeof(void*)
  );
  for (i = 0; i < result->b.as_obj_array.count; ++i) {
    result->b.as_obj_array.values[i] = (void*) l_get_item(l, i);
  }
  cleanup_list(l);
}

void efd_parse_int_array(efd_node *result, efd_parse_state *s) {
  size_t i;
  ptrdiff_t val;
  list *l = create_list();

  while (1) {
    val = efd_parse_int(s);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        cleanup_list(l);
        return;
      }
    } else {
      l_append_element(l, (void*) val);
    }
    efd_parse_sep(s);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        cleanup_list(l);
        return;
      }
    }
  }

  result->b.as_int_array.count = l_get_length(l);
#ifdef DEBUG
  if (result->b.as_int_array.values != NULL) {
    fprintf(stderr, "EFD integer array already had values while parsing!\n");
    exit(-1);
  }
#endif
  result->b.as_int_array.values = (ptrdiff_t*) malloc(
    result->b.as_int_array.count * sizeof(ptrdiff_t)
  );
  for (i = 0; i < result->b.as_int_array.count; ++i) {
    result->b.as_int_array.values[i] = (ptrdiff_t) l_get_item(l, i);
  }
  cleanup_list(l);
}

void efd_parse_num_array(efd_node *result, efd_parse_state *s) {
  size_t i;
  float val;
  void *v;
  list *l = create_list();

  while (1) {
    val = efd_parse_float(s);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        cleanup_list(l);
        return;
      }
    } else {
      l_append_element(l, *((void**) &val));
    }
    efd_parse_sep(s);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        cleanup_list(l);
        return;
      }
    }
  }

  result->b.as_num_array.count = l_get_length(l);
#ifdef DEBUG
  if (result->b.as_num_array.values != NULL) {
    fprintf(stderr, "EFD number array already had values while parsing!\n");
    exit(-1);
  }
#endif
  result->b.as_num_array.values = (float*) malloc(
    result->b.as_num_array.count * sizeof(float)
  );
  for (i = 0; i < result->b.as_num_array.count; ++i) {
    v = l_get_item(l, i);
    result->b.as_num_array.values[i] = *((float*) &v);
  }
  cleanup_list(l);
}

// Helper if strings in a list need to be cleaned up:
void _cleanup_string_in_list(void *v_string) {
  cleanup_string((string*) v_string);
}

void efd_parse_str_array(efd_node *result, efd_parse_state *s) {
  size_t i;
  ptrdiff_t start, end;
  string *val;
  list *l = create_list();

  while (1) {
    efd_grab_string_limits(s, &start, &end);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        l_foreach(l, &_cleanup_string_in_list);
        cleanup_list(l);
        return;
      }
    } else {
      val = create_string_from_chars(
        s->input + start,
        end - start
      );
      l_append_element(l, (void*) val);
    }
    efd_parse_sep(s);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        l_foreach(l, &_cleanup_string_in_list);
        cleanup_list(l);
        return;
      }
    }
  }

  result->b.as_str_array.count = l_get_length(l);
#ifdef DEBUG
  if (result->b.as_str_array.values != NULL) {
    fprintf(stderr, "EFD string array already had values while parsing!\n");
    exit(-1);
  }
#endif
  result->b.as_str_array.values = (string**) malloc(
    result->b.as_str_array.count * sizeof(string*)
  );
  for (i = 0; i < result->b.as_str_array.count; ++i) {
    result->b.as_str_array.values[i] = (string*) l_get_item(l, i);
  }
  cleanup_list(l);
  // Don't clean up the strings, of course, as they're now in the array.
}

// Functions for parsing bits & pieces:
//-------------------------------------

void efd_parse_open(efd_parse_state *s) {
  static char* e = "opening brace";
  SKIP_OR_ELSE(s, e) //;

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }
  if (s->input[s->pos] != EFD_PARSER_OPEN_BRACE) {
    s->error = EFD_PE_MALFORMED;
    s->context = e;
    return;
  }
  s->pos += 1;
  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }
  if (s->input[s->pos] != EFD_PARSER_OPEN_BRACE) {
    s->error = EFD_PE_MALFORMED;
    s->context = e;
    return;
  }
  s->pos += 1;
  return;
}

void efd_parse_close(efd_parse_state *s) {
  static char* e = "closing brace";
  SKIP_OR_ELSE(s, e) //;

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }
  if (s->input[s->pos] != EFD_PARSER_CLOSE_BRACE) {
    s->error = EFD_PE_MALFORMED;
    s->context = e;
    return;
  }
  s->pos += 1;
  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }
  if (s->input[s->pos] != EFD_PARSER_CLOSE_BRACE) {
    s->error = EFD_PE_MALFORMED;
    s->context = e;
    return;
  }
  s->pos += 1;
  return;
}

efd_node_type efd_parse_type(efd_parse_state *s) {
  static char* e = "node type";
  char c;

  efd_parse_skip(s);
  if (efd_parse_failed(s)) {
    s->context = e;
    return EFD_NT_INVALID;
  }

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return EFD_NT_INVALID;
  }
  c = s->input[s->pos];
  switch (c) {
    default:
      s->error = EFD_PE_MALFORMED;
      s->context = e;
      return EFD_NT_INVALID;

    case 'c':
      s->pos += 1;
      return EFD_NT_CONTAINER;
    case 'o':
      s->pos += 1;
      return EFD_NT_OBJECT;
    case 'i':
      s->pos += 1;
      return EFD_NT_INTEGER;
    case 'n':
      s->pos += 1;
      return EFD_NT_NUMBER;
    case 's':
      s->pos += 1;
      return EFD_NT_STRING;
    case 'a':
      s->pos += 1;
      if (efd_parse_atend(s)) {
        s->error = EFD_PE_INCOMPLETE;
        s->context = e;
        return EFD_NT_INVALID;
      }
      c = s->input[s->pos];
      switch (c) {
        default:
          s->error = EFD_PE_MALFORMED;
          s->context = e;
          return EFD_NT_INVALID;

        case 'o':
          s->pos += 1;
          return EFD_NT_ARRAY_OBJ;
        case 'i':
          s->pos += 1;
          return EFD_NT_ARRAY_INT;
        case 'n':
          s->pos += 1;
          return EFD_NT_ARRAY_NUM;
        case 's':
          s->pos += 1;
          return EFD_NT_ARRAY_STR;
      }
      break;
  }
  // This shouldn't be reachable...
  s->error = EFD_PE_MALFORMED;
  s->context = e;
  return EFD_NT_INVALID;
}

void efd_parse_name(efd_parse_state *s, char *r_name) {
  char c;
  size_t i;

  r_name[0] = '\0';

  SKIP_OR_ELSE(s, "node name") //;

  c = '\0';

  for (i = 0; i < EFD_NODE_NAME_SIZE - 1; ++i) {
    if (efd_parse_atend(s)) {
      break;
    }
    c = s->input[s->pos];
    if (
      is_whitespace(&c)
   || c == EFD_NODE_SEP
   || (
       c >= '0'
    && c <= '9'
    && i == 0
      )
    ) {
      break;
    }
    r_name[i] = c;
    s->pos += 1;
  }
  if (i == 0) {
    if (!efd_parse_atend(s) && c >= '0' && c <= '9') {
      s->error = EFD_PE_MALFORMED;
      s->context = "node name (initial numeral not allowed)";
    } else {
      s->error = EFD_PE_INCOMPLETE;
      s->context = "node name (missing)";
    }
    return;
  }
  r_name[i] = '\0';
}

void efd_parse_schema(efd_parse_state *s, char *r_name) {
  char c;

  r_name[0] = '\0';

  SKIP_OR_ELSE(s, "node name") //;

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = "schema annotation (missing)";
    return;
  }

  c = s->input[s->pos];

  if (c != EFD_SCHEMA_INDICATOR) {
    s->error = EFD_PE_MISSING;
    s->context = "schema annotation (no indicator)";
    return;
  }
  s->pos += 1;

  efd_parse_name(s, r_name);
  if (efd_parse_failed(s)) {
    // (error from parse_name)
    s->context = "schema (invalid name)";
    return;
  }
}

ptrdiff_t efd_parse_int(efd_parse_state *s) {
  ptrdiff_t result;
  ptrdiff_t sign;
  ptrdiff_t base;
  ptrdiff_t digit;
  ptrdiff_t count;
  efd_int_state state;
  char c;

  efd_parse_skip(s);
  if (efd_parse_failed(s)) {
    s->context = "integer (pre)";
    return EFD_PARSER_INT_ERROR;
  }

  state = EFD_INT_STATE_PRE;
  result = 0;
  sign = 1;
  base = 10;
  digit = 0;
  count = 0;
  while (!efd_parse_atend(s) && state != EFD_INT_STATE_DONE) {
    c = s->input[s->pos];
    if (c == '\n') {
      s->lineno += 1;
    }
    switch (c) {
      default:
        if (state == EFD_INT_STATE_DIGITS || state == EFD_INT_STATE_BASE) {
          state = EFD_INT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        } else {
          s->error = EFD_PE_MALFORMED;
          s->context = "integer";
          return EFD_PARSER_INT_ERROR;
        }
        break;

      case '-':
        if (state == EFD_INT_STATE_PRE) {
          if (sign == 1) {
            sign = -1;
          } else {
            s->error = EFD_PE_MALFORMED;
            s->context = "integer (multiple signs)";
            return EFD_PARSER_INT_ERROR;
          }
        } else if (state == EFD_INT_STATE_DIGITS) {
          state = EFD_INT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        } else {
          s->error = EFD_PE_MALFORMED;
          s->context = "integer (minus)";
        }
        break;
      case 'x':
        if (state == EFD_INT_STATE_BASE) {
          base = 16;
          state = EFD_INT_STATE_PRE_DIGITS;
        } else if (state == EFD_INT_STATE_DIGITS) {
          state = EFD_INT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        } else {
          s->error = EFD_PE_MALFORMED;
          s->context = "integer (x)";
          return EFD_PARSER_INT_ERROR;
        }
        break;
      case '0':
        if (state == EFD_INT_STATE_PRE) {
          state = EFD_INT_STATE_BASE;
        } else if (
          state == EFD_INT_STATE_PRE_DIGITS
       || state == EFD_INT_STATE_DIGITS
        ) {
          state = EFD_INT_STATE_DIGITS;
          result *= base;
          count += 1;
        } else {
          s->error = EFD_PE_MALFORMED;
          s->context = "integer (00)";
          return EFD_PARSER_INT_ERROR;
        }
        break;
      case '1':
        digit = 1;
        goto lbl_int_digit;
      case '2':
        digit = 2;
        goto lbl_int_digit;
      case '3':
        digit = 3;
        goto lbl_int_digit;
      case '4':
        digit = 4;
        goto lbl_int_digit;
      case '5':
        digit = 5;
        goto lbl_int_digit;
      case '6':
        digit = 6;
        goto lbl_int_digit;
      case '7':
        digit = 7;
        goto lbl_int_digit;
      case '8':
        digit = 8;
        goto lbl_int_digit;
      case '9':
        digit = 9;
        goto lbl_int_digit;
      case 'a':
      case 'A':
        digit = 10;
        goto lbl_int_hex_digit;
      case 'b':
      case 'B':
        digit = 11;
        goto lbl_int_hex_digit;
      case 'c':
      case 'C':
        digit = 12;
        goto lbl_int_hex_digit;
      case 'd':
      case 'D':
        digit = 13;
        goto lbl_int_hex_digit;
      case 'e':
      case 'E':
        digit = 14;
        goto lbl_int_hex_digit;
      case 'f':
      case 'F':
        digit = 15;
        goto lbl_int_hex_digit;
      case '\0': // impossible
      lbl_int_hex_digit:
        if (base != 16) {
          state = EFD_INT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
          break;
        }
        // flow through...
      lbl_int_digit:
        if (state == EFD_INT_STATE_BASE) {
          s->error = EFD_PE_MALFORMED;
          s->context = "integer (bad base)";
          return EFD_PARSER_INT_ERROR;
        }
        state = EFD_INT_STATE_DIGITS;
        result *= base;
        result += digit;
        count += 1;
        break;
    }
    s->pos += 1;
    if (count > EFD_PARSER_MAX_DIGITS) {
      s->error = EFD_PE_MALFORMED;
      s->context = "integer (too many digits)";
      return EFD_PARSER_INT_ERROR;
    }
  }
  if (state == EFD_INT_STATE_DONE) {
    return sign * result;
  } else {
    s->error = EFD_PE_MALFORMED;
    s->context = "integer";
    return EFD_PARSER_INT_ERROR;
  }
}

float efd_parse_float(efd_parse_state *s) {
  float sign;
  float characteristic;
  float mantissa;
  float mant_div;
  float expsign;
  float exponent;
  float digit;
  ptrdiff_t count;
  efd_float_state state;
  char c;

  efd_parse_skip(s);
  if (efd_parse_failed(s)) {
    s->context = "integer (pre)";
    return EFD_PARSER_INT_ERROR;
  }

  state = EFD_FLOAT_STATE_PRE;
  sign = 1;
  characteristic = 0;
  mantissa = 0;
  mant_div = 1;
  expsign = 0;
  exponent = 0;
  digit = 0;
  count = 0;

  while (!efd_parse_atend(s) && state != EFD_FLOAT_STATE_DONE) {
    c = s->input[s->pos];
    if (c == '\n') {
      s->lineno += 1;
    }
    switch (c) {
      default:
        if (state == EFD_FLOAT_STATE_EXP_SIGN) {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 2; // skip back to the 'e' or 'E' before us
        } else if (state == EFD_FLOAT_STATE_EXP) {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 3; // skip back over the '+'/'-' to the 'e' or 'E' before
        } else if (state != EFD_FLOAT_STATE_PRE) {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        } else {
          s->error = EFD_PE_MALFORMED;
          s->context = "number";
          return EFD_PARSER_FLOAT_ERROR;
        }
        break;

      case '-':
        if (state == EFD_FLOAT_STATE_PRE) {
          if (sign == 1) {
            sign = -1;
          } else {
            s->error = EFD_PE_MALFORMED;
            s->context = "number (multiple signs)";
            return EFD_PARSER_FLOAT_ERROR;
          }
        } else if (state == EFD_FLOAT_STATE_EXP_SIGN) {
          if (expsign == 0) {
            expsign = -1;
            state = EFD_FLOAT_STATE_EXP;
          } else {
            state = EFD_FLOAT_STATE_DONE;
            s->pos -= 3; // skip back to the 'e' or 'E'
          }
        } else {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        }
        break;

      case '+':
        if (state == EFD_FLOAT_STATE_EXP_SIGN) {
          if (expsign == 0) {
            expsign = 1;
            state = EFD_FLOAT_STATE_EXP;
          } else {
            state = EFD_FLOAT_STATE_DONE;
            s->pos -= 3; // skip back to the 'e' or 'E'
          }
        } else if (state == EFD_FLOAT_STATE_PRE) {
          s->error = EFD_PE_MALFORMED;
          s->context = "number (initial '+')";
          return EFD_PARSER_FLOAT_ERROR;
        } else {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        }
        break;

      case '.':
        if (
          state == EFD_FLOAT_STATE_PRE
       || state == EFD_FLOAT_STATE_ZERO
       || state == EFD_FLOAT_STATE_CHAR
        ) {
          state = EFD_FLOAT_STATE_MANT;
          count = 0; // reset the count
        } else if (state == EFD_FLOAT_STATE_EXP_SIGN) {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 2; // back to the 'e' or 'E'
        } else if (state == EFD_FLOAT_STATE_EXP) {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 3; // back to the 'e' or 'E' before the '+' or '-'
        } else {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        }
        break;

      case 'e':
      case 'E':
        if (state == EFD_FLOAT_STATE_CHAR || state == EFD_FLOAT_STATE_MANT) {
          state = EFD_FLOAT_STATE_EXP_SIGN;
        } else {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        }
        break;

      case '0':
        if (state == EFD_FLOAT_STATE_PRE) {
          state = EFD_FLOAT_STATE_ZERO;
        } else if (state == EFD_FLOAT_STATE_ZERO) {
          s->error = EFD_PE_MALFORMED;
          s->context = "number (00)";
        } else if (
          state == EFD_FLOAT_STATE_CHAR
       || state == EFD_FLOAT_STATE_MANT
       || state == EFD_FLOAT_STATE_EXP_SIGN
       || state == EFD_FLOAT_STATE_EXP
       || state == EFD_FLOAT_STATE_EXP_DIGITS
        ) {
          digit = 0;
          goto lbl_float_digit;
        }
        break;
      case '1':
        digit = 1;
        goto lbl_float_digit;
      case '2':
        digit = 2;
        goto lbl_float_digit;
      case '3':
        digit = 3;
        goto lbl_float_digit;
      case '4':
        digit = 4;
        goto lbl_float_digit;
      case '5':
        digit = 5;
        goto lbl_float_digit;
      case '6':
        digit = 6;
        goto lbl_float_digit;
      case '7':
        digit = 7;
        goto lbl_float_digit;
      case '8':
        digit = 8;
        goto lbl_float_digit;
      case '9':
        digit = 9;
        goto lbl_float_digit;
      case '\0': // impossible
      lbl_float_digit:
        if (state == EFD_FLOAT_STATE_PRE || state == EFD_FLOAT_STATE_CHAR) {
          state = EFD_FLOAT_STATE_CHAR;
          characteristic *= 10;
          characteristic += digit;
        } else if (state == EFD_FLOAT_STATE_MANT) {
          mantissa *= 10;
          mantissa += digit;
          mant_div *= 10;
        } else if (
          state == EFD_FLOAT_STATE_EXP_SIGN
       || state == EFD_FLOAT_STATE_EXP
       || state == EFD_FLOAT_STATE_EXP_DIGITS
        ) {
          if (expsign == 0) {
            expsign = 1;
          }
          state = EFD_FLOAT_STATE_EXP_DIGITS;
          exponent *= 10;
          exponent += digit;
        }
        count += 1;
        break;
    }
    s->pos += 1;
    if (count > EFD_PARSER_MAX_DIGITS) {
      s->error = EFD_PE_MALFORMED;
      s->context = "number (too many digits)";
      return EFD_PARSER_FLOAT_ERROR;
    }
  }
  if (state == EFD_FLOAT_STATE_DONE) {
    return (
      (
        sign * characteristic
      + (mantissa / mant_div)
      )
    * pow(10, expsign * exponent)
    );
  } else {
    s->error = EFD_PE_MALFORMED;
    s->context = "number";
    return EFD_PARSER_FLOAT_ERROR;
  }
}

void* efd_parse_ref(efd_parse_state *s) {
  // TODO: HERE
  s->error = EFD_PE_UNKNOWN;
  s->context = "object arrays don't work yet";
  return NULL;
}

void efd_grab_string_limits(
  efd_parse_state *s,
  ptrdiff_t *start,
  ptrdiff_t *end
) {
  efd_quote_state state = EFD_QUOTE_STATE_NORMAL;
  char c;
  char q;

  // skip to the starting quote:
  SKIP_OR_ELSE(s, "quoted string (pre)") //;

  c = s->input[s->pos];
  q = c;
  if (!(q == '"' || q == '\'')) {
    *start = -1;
    *end = -1;
    s->error = EFD_PE_MALFORMED;
    s->context = "quoted string (open)";
    return;
  }
  s->pos += 1;
  *start = s->pos;
  while (!efd_parse_atend(s)) {
    c = s->input[s->pos];
    if (c == '\n') {
      s->lineno += 1;
    }
    switch (state) {
      default:
      case EFD_QUOTE_STATE_NORMAL:
        if (c == q) {
          state = EFD_QUOTE_STATE_MAYBE_DONE;
        }
        break;
      case EFD_QUOTE_STATE_MAYBE_DONE:
        if (c == q) {
          state = EFD_QUOTE_STATE_NORMAL;
        } else {
          state = EFD_QUOTE_STATE_DONE;
        }
        break;
    }
    if (state == EFD_QUOTE_STATE_DONE) {
      break;
    } else {
      s->pos += 1;
    }
  }
  switch (state) {
    default:
    case EFD_QUOTE_STATE_NORMAL:
      s->context = "quoted string (main)";
      s->error = EFD_PE_INCOMPLETE;
      break;
    case EFD_QUOTE_STATE_MAYBE_DONE:
    case EFD_QUOTE_STATE_DONE:
      *end = s->pos - 2;
      break;
  }
}

void efd_parse_skip(efd_parse_state *s) {
  efd_skip_state state = EFD_SKIP_STATE_NORMAL;
  char c;
  while (!efd_parse_atend(s)) {
    c = s->input[s->pos];
    if (c == '\n') {
      s->lineno += 1;
    }
    switch (state) {
      default:
      case EFD_SKIP_STATE_NORMAL:
        if (c == '/') {
          state = EFD_SKIP_STATE_MAYBE_COMMENT;
        } else if (!is_whitespace(&c)) {
          state = EFD_SKIP_STATE_DONE;
        }
        break;
      case EFD_SKIP_STATE_MAYBE_COMMENT: // maybe starting a comment
        if (c == '/') {
          state = EFD_SKIP_STATE_LINE_COMMENT;
        } else if (c == '*') {
          state = EFD_SKIP_STATE_BLOCK_COMMENT;
        } else {
          s->pos -= 1; // go back to the '/' immediately preceding us
          if (c == '\n') {
            s->lineno -= 1;
          }
          state = EFD_SKIP_STATE_DONE; // not actually the start of a comment
        }
        break;
      case EFD_SKIP_STATE_LINE_COMMENT:
        if (c == '\n') {
          state = EFD_SKIP_STATE_NORMAL;
        }
        break;
      case EFD_SKIP_STATE_BLOCK_COMMENT: // comment until */
        if (c == '*') {
          state = EFD_SKIP_STATE_MAYBE_BLOCK_END;
        }
        break;
      case EFD_SKIP_STATE_MAYBE_BLOCK_END:
        if (c == '/') {
          state = EFD_SKIP_STATE_NORMAL; // done with this block comment
        } else {
          state = EFD_SKIP_STATE_BLOCK_COMMENT; // still in a block comment
        }
        break;
    }
    if (state == EFD_SKIP_STATE_DONE) { // don't increment position
      break;
    } else { // increment our position and keep going
      s->pos += 1;
    }
  }
  if  (
    !(state == EFD_SKIP_STATE_DONE
   || state == EFD_SKIP_STATE_NORMAL
   || state == EFD_SKIP_STATE_LINE_COMMENT)
  ) {
    if (
      state == EFD_SKIP_STATE_BLOCK_COMMENT
   || state == EFD_SKIP_STATE_MAYBE_BLOCK_END
    ) {
      s->context = "block comment";
    } // otherwise leave whatever context was already there
    s->error = EFD_PE_INCOMPLETE;
  }
}

void efd_parse_sep(efd_parse_state *s) {
  SKIP_OR_ELSE(s, "separator") //;
  if (s->input[s->pos] == EFD_PARSER_ARRAY_SEP) {
    s->pos += 1;
    return;
  } else {
    s->error = EFD_PE_INCOMPLETE;
    s->context = "separator (missing)";
    return;
  }
}

// Helper functions:
//------------------

int efd_parse_failed(efd_parse_state *s) {
  return s->error != EFD_PE_NO_ERROR;
}

void efd_throw_parse_error(efd_parse_state *s) {
  if (s->error != EFD_PE_NO_ERROR) {
    fprintf(
      stderr,
      "[%.*s:%ld] ",
      EFD_PARSER_MAX_FILENAME_DISPLAY,
      s->filename,
      s->lineno
    );
    switch (s->error) {
      default:
      case EFD_PE_UNKNOWN:
        s->error = EFD_PE_UNKNOWN;
        if (s->context == NULL) {
          fprintf(
            stderr,
            "Unknown parsing error while parsing <?>.\n"
          );
        } else {
          fprintf(
            stderr,
            "Unknown parsing error while parsing %.*s.\n",
            EFD_PARSER_MAX_CONTEXT_DISPLAY,
            s->context
          );
        }
        break;
      case EFD_PE_MALFORMED:
        if (s->context == NULL) {
          if (s->input == NULL) {
            fprintf(
              stderr,
              "Malformed input while parsing <?> at <?>.\n"
            );
          } else {
            fprintf(
              stderr,
              "Malformed input while parsing <?> at '%.*s'.\n",
              EFD_PARSER_ERROR_CONTEXT, // TODO: pre-context as well?
              (s->input + s->pos)
            );
          }
        } else {
          if (s->input == NULL) {
            fprintf(
              stderr,
              "Malformed input while parsing %.*s at <?>.\n",
              EFD_PARSER_MAX_CONTEXT_DISPLAY,
              s->context
            );
          } else {
            fprintf(
              stderr,
              "Malformed input while parsing %.*s at '%.*s'.\n",
              EFD_PARSER_MAX_CONTEXT_DISPLAY,
              s->context,
              EFD_PARSER_ERROR_CONTEXT, // TODO: pre-context as well?
              (s->input + s->pos)
            );
          }
        }
        break;
      case EFD_PE_MISSING:
        if (s->context == NULL) {
          fprintf(
            stderr,
            "Missing element while parsing <?>.\n"
          );
        } else {
          fprintf(
            stderr,
            "Missing element while parsing %.*s.\n",
            EFD_PARSER_MAX_CONTEXT_DISPLAY,
            s->context
          );
        }
        break;
      case EFD_PE_INCOMPLETE:
        if (s->context == NULL) {
          fprintf(
            stderr,
            "Input ended while parsing <?>.\n"
          );
        } else {
          fprintf(
            stderr,
            "Input ended while parsing %.*s.\n",
            EFD_PARSER_MAX_CONTEXT_DISPLAY,
            s->context
          );
        }
        break;
    }
    exit(-s->error);
  }
}

int efd_test_parse_progress(efd_parse_state *s, ptrdiff_t reset_to) {
  if (s->error == EFD_PE_NO_ERROR) {
    return 1;
  } else {
    s->pos = reset_to;
    return 0;
  }
}
