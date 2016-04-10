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

int efd_parse_file(
  efd_node *parent,
  efd_index *cr,
  char const * const filename
) {
  efd_parse_state s;
  char *contents;

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
  efd_parse_children(parent, &s, cr);

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

efd_address* efd_parse_string_address(char const * const astr) {
  size_t len;
  efd_address *result;
  efd_parse_state s;
  s.input = astr;
  for (len = 0; len < EFD_MAX_NAME_DEPTH * (EFD_NODE_NAME_SIZE + 1); ++len) {
    if (astr[len] == '\0') {
      break;
    }
  }
  s.input_length = len;
  s.pos = 0;
  s.filename = "<address string>";
  s.lineno = -1;
  s.context = "string address";
  s.error = EFD_PE_NO_ERROR;

  result = efd_parse_address(&s);
  if (efd_parse_failed(&s)) {
    efd_throw_parse_error(&s);
    return NULL;
  }
  return result;
}

efd_node* efd_parse_any(efd_parse_state *s, efd_index *cr) {
  efd_node_type type;
  efd_node* parent_node;
  char name[EFD_NODE_NAME_SIZE + 1];
  char btype;

  btype = efd_parse_open(s);
  if (efd_parse_failed(s)) {
    if (s->error == EFD_PE_MISSING) {
      s->error = EFD_PE_NO_ERROR;
      // if there's nothing here (as opposed to something malformed) then we'll
      // just return NULL without an error.
    }
    return NULL;
  }

  type = efd_parse_type(s);
  if (efd_parse_failed(s)) { return NULL; }

  if (btype == EFD_PARSER_OPEN_ANGLE) {
    if (type != EFD_NT_LINK && type != EFD_NT_LOCAL_LINK) {
      s->error = EFD_PE_MALFORMED;
      s->context = "node (angle braces are for links)";
      return NULL;
    }
    // name will be decided by efd_parse_link:
    name[0] = EFD_PROTO_NAME[0];
    name[1] = '\0';
  } else {
    if (type == EFD_NT_LINK || type == EFD_NT_LOCAL_LINK) {
      s->error = EFD_PE_MALFORMED;
      s->context = "node (links require angle braces)";
      return NULL;
    }
    // parse the name immediately:
    efd_parse_name(s, name);
    if (efd_parse_failed(s)) { return NULL; }
  }

  efd_node *result = create_efd_node(type, name);
  parent_node = s->current_node;
  s->current_node = result;
  s->current_index = -1;

  switch(type) {
    default:
    case EFD_NT_INVALID:
    case EFD_NT_OBJECT:
      s->error = EFD_PE_MALFORMED;
      s->context = "node (invalid type)";
      s->current_node = parent_node;
      return NULL;
    case EFD_NT_CONTAINER:
      efd_parse_children(result, s, cr);
      break;
    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
      efd_parse_link(result, s, cr);
      break;
    case EFD_NT_PROTO:
      efd_parse_proto(result, s, cr);
      break;
    case EFD_NT_INTEGER:
      efd_parse_integer(result, s, cr);
      break;
    case EFD_NT_NUMBER:
      efd_parse_number(result, s, cr);
      break;
    case EFD_NT_STRING:
      efd_parse_string(result, s, cr);
      break;
    case EFD_NT_ARRAY_OBJ:
      efd_parse_obj_array(result, s, cr);
      break;
    case EFD_NT_ARRAY_INT:
      efd_parse_int_array(result, s, cr);
      break;
    case EFD_NT_ARRAY_NUM:
      efd_parse_num_array(result, s, cr);
      break;
    case EFD_NT_ARRAY_STR:
      efd_parse_str_array(result, s, cr);
      break;
    case EFD_NT_GLOBAL_INT:
      efd_parse_int_global(result, s, cr);
      break;
    case EFD_NT_GLOBAL_NUM:
      efd_parse_num_global(result, s, cr);
      break;
    case EFD_NT_GLOBAL_STR:
      efd_parse_str_global(result, s, cr);
      break;
  }
  s->current_node = parent_node;
  s->current_index = -1;
  if (efd_parse_failed(s)) {
    cleanup_efd_node(result);
    return NULL;
  }

  efd_parse_close(s, btype);
  if (efd_parse_failed(s)) {
    cleanup_efd_node(result);
    return NULL;
  }

  return result;
}

// Parsing functions for the EFD primitive types:
//-----------------------------------------------

void efd_parse_children(efd_node *result, efd_parse_state *s, efd_index *cr) {
  efd_parse_state back;
  list *children;
  efd_node *child;

  switch (result->h.type) {
    default:
      fprintf(
        stderr,
        "ERROR: Attempt to parse children for non-container node.\n"
      );
      exit(-1);
      return;
    case EFD_NT_CONTAINER:
      children = result->b.as_container.children;
      break;
    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
      children = result->b.as_link.children;
      break;
  }

  // parse any number of children and add them:
  while (1) {
    efd_parse_copy_state(s, &back);
    child = efd_parse_any(s, cr);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        // we've probably hit the end of this object: return w/out error
        efd_parse_copy_state(&back, s);
      } // otherwise return w/ error
      break;
    } else if (child == NULL) {
      break;
    } else {
      if (child->h.type == EFD_NT_GLOBAL_INT) {
        efd_set_global_i(child->h.name, child->b.as_integer.value);
        cleanup_efd_node(child);
      } else if (child->h.type == EFD_NT_GLOBAL_NUM) {
        efd_set_global_n(child->h.name, child->b.as_number.value);
        cleanup_efd_node(child);
      } else if (child->h.type == EFD_NT_GLOBAL_STR) {
        efd_set_global_s(child->h.name, child->b.as_string.value);
        cleanup_efd_node(child);
      } else {
        l_append_element(children, (void*) child);
      }
    }
  }
}

void efd_parse_link(efd_node *result, efd_parse_state *s, efd_index *cr) {
  char name[EFD_NODE_NAME_SIZE + 1];
  efd_address *target;
  efd_address *tail;
  efd_parse_state back;

  target = efd_parse_address(s);
  if (efd_parse_failed(s)) { return; }

  efd_parse_copy_state(s, &back);
  efd_parse_annotation(s, EFD_PARSER_RENAME, name);
  if (efd_parse_failed(s)) {
    if (s->error == EFD_PE_MISSING) {
      // Use the target node's name as the name of this node by default:
      efd_parse_copy_state(&back, s);
      tail = target;
      while (tail->next != NULL) {
        tail = tail->next;
      }
      strncpy(name, tail->name, EFD_NODE_NAME_SIZE);
      name[EFD_NODE_NAME_SIZE] = '\0';
    } else {
      return;
    }
  }

  strncpy(result->h.name, name, EFD_NODE_NAME_SIZE);
  name[EFD_NODE_NAME_SIZE] = '\0';
  result->b.as_link.target = target;

  // Finally just parse any children that might be present:
  efd_parse_children(result, s, cr);
}

void efd_parse_proto(efd_node *result, efd_parse_state *s, efd_index *cr) {
  char format[EFD_NODE_NAME_SIZE + 1];

  efd_parse_annotation(s, EFD_PARSER_COLON, format);
  if (efd_parse_failed(s)) { return; }

  strncpy(result->b.as_proto.format, format, EFD_OBJECT_FORMAT_SIZE - 1);
  result->b.as_proto.format[EFD_OBJECT_FORMAT_SIZE-1] = '\0';

  efd_node *proto = create_efd_node(EFD_NT_CONTAINER, EFD_PROTO_NAME);
  efd_parse_children(proto, s, cr);
  if (efd_parse_failed(s)) {
    cleanup_efd_node(proto);
    return;
  }

  result->b.as_proto.input = proto;
}

void efd_parse_integer(efd_node *result, efd_parse_state *s, efd_index *cr) {
  ptrdiff_t i = efd_parse_int_or_ref(s, cr);
  if (efd_parse_failed(s)) {
    return;
  } else {
    result->b.as_integer.value = i;
  }
}

void efd_parse_number(efd_node *result, efd_parse_state *s, efd_index *cr) {
  float n = efd_parse_float_or_ref(s, cr);
  if (efd_parse_failed(s)) {
    return;
  } else {
    result->b.as_number.value = n;
  }
}

void efd_parse_string(efd_node *result, efd_parse_state *s, efd_index *cr) {
  result->b.as_string.value = efd_parse_str_or_ref(s, cr);
  if (efd_parse_failed(s)) {
    return;
  }
}

void efd_parse_obj_array(efd_node *result, efd_parse_state *s, efd_index *cr) {
  size_t i;
  void *val;
  list *values = create_list();

  s->current_index = 0;
  while (1) {
    val = efd_parse_obj_ref(s, cr);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        cleanup_list(values);
        return;
      }
    }
    l_append_element(values, val);

    efd_parse_sep(s);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        cleanup_list(values);
        return;
      }
    }
    s->current_index += 1;
  }

  result->b.as_obj_array.count = l_get_length(values);
#ifdef DEBUG
  if (result->b.as_obj_array.values != NULL) {
    fprintf(
      stderr,
      "ERROR: EFD object array already had values while parsing!\n"
    );
    exit(-1);
  }
#endif
  result->b.as_obj_array.values = (void*) malloc(
    result->b.as_obj_array.count * sizeof(void*)
  );
  for (i = 0; i < result->b.as_obj_array.count; ++i) {
    result->b.as_obj_array.values[i] = l_get_item(values, i);
  }
  cleanup_list(values);
}

void efd_parse_int_array(efd_node *result, efd_parse_state *s, efd_index *cr) {
  size_t i;
  ptrdiff_t val;
  list *l = create_list();

  s->current_index = 0;
  while (1) {
    val = efd_parse_int_or_ref(s, cr);
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
    s->current_index += 1;
  }

  result->b.as_int_array.count = l_get_length(l);
#ifdef DEBUG
  if (result->b.as_int_array.values != NULL) {
    fprintf(
      stderr,
      "ERROR: EFD integer array already had values while parsing!\n"
    );
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

void efd_parse_num_array(efd_node *result, efd_parse_state *s, efd_index *cr) {
  size_t i;
  float val;
  void *v;
  list *l = create_list();

  s->current_index = 0;
  while (1) {
    val = efd_parse_float_or_ref(s, cr);
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
    s->current_index += 1;
  }

  result->b.as_num_array.count = l_get_length(l);
#ifdef DEBUG
  if (result->b.as_num_array.values != NULL) {
    fprintf(
      stderr,
      "ERROR: EFD number array already had values while parsing!\n"
    );
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

void efd_parse_str_array(efd_node *result, efd_parse_state *s, efd_index *cr) {
  size_t i;
  string *val;
  list *l = create_list();

  s->current_index = 0;
  while (1) {
    val = efd_parse_str_or_ref(s, cr);
    if (efd_parse_failed(s)) {
      if (s->input[s->pos] == EFD_PARSER_CLOSE_BRACE) {
        s->error = EFD_PE_NO_ERROR;
        break;
      } else {
        l_foreach(l, &cleanup_v_string);
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
        l_foreach(l, &cleanup_v_string);
        cleanup_list(l);
        return;
      }
    }
    s->current_index += 1;
  }

  result->b.as_str_array.count = l_get_length(l);
#ifdef DEBUG
  if (result->b.as_str_array.values != NULL) {
    fprintf(
      stderr,
      "ERROR: EFD string array already had values while parsing!\n"
    );
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

void efd_parse_int_global(efd_node *result, efd_parse_state *s, efd_index *cr) {
  static char const * const e = "global integer";
  char c;

  SKIP_OR_ELSE(s, e);

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }

  c = s->input[s->pos];
  if (c == EFD_PARSER_EQUALS) {
    s->pos += 1;
    SKIP_OR_ELSE(s, e);
  }

  result->b.as_integer.value = efd_parse_int_or_ref(s, cr);
  // return whether or not there's an error...
}

void efd_parse_num_global(efd_node *result, efd_parse_state *s, efd_index *cr) {
  static char const * const e = "global number";
  char c;

  SKIP_OR_ELSE(s, e);

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }

  c = s->input[s->pos];
  if (c == EFD_PARSER_EQUALS) {
    s->pos += 1;
    SKIP_OR_ELSE(s, e);
  }

  result->b.as_number.value = efd_parse_float_or_ref(s, cr);
  // return whether or not there's an error...
}

void efd_parse_str_global(efd_node *result, efd_parse_state *s, efd_index *cr) {
  static char const * const e = "global string";
  char c;

  SKIP_OR_ELSE(s, e);

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }

  c = s->input[s->pos];
  if (c == EFD_PARSER_EQUALS) {
    s->pos += 1;
    SKIP_OR_ELSE(s, e);
  }

  result->b.as_string.value = efd_parse_str_or_ref(s, cr);
  if (efd_parse_failed(s)) {
    result->b.as_string.value = NULL;
    return;
  }
}

// Functions for parsing pieces that might be references:
//-------------------------------------------------------

ptrdiff_t efd_parse_int_or_ref(efd_parse_state *s, efd_index *cr) {
  efd_reference *from, *to;
  efd_bridge *bridge;
  efd_parse_state back;

  efd_parse_copy_state(s, &back);
  to = efd_parse_global_ref(s);
  if (efd_parse_failed(s)) {
    s->error = EFD_PE_NO_ERROR;
    efd_parse_copy_state(&back, s);
    return efd_parse_int(s);
  } else {
    from = construct_efd_reference_to_here(s);
    bridge = create_efd_bridge(from, to);
    if (bridge != NULL) {
      efd_add_crossref(cr, bridge);
      return EFD_PARSER_INT_REFVAL;
    } else {
      s->error = EFD_PE_MALFORMED;
      s->context = "int ref (incompatible destination type)";
      cleanup_efd_reference(from);
      cleanup_efd_reference(to);
      return EFD_PARSER_INT_ERROR;
    }
  }
}

float efd_parse_float_or_ref(efd_parse_state *s, efd_index *cr) {
  efd_reference *from, *to;
  efd_bridge *bridge;
  efd_parse_state back;

  efd_parse_copy_state(s, &back);
  to = efd_parse_global_ref(s);
  if (efd_parse_failed(s)) {
    s->error = EFD_PE_NO_ERROR;
    efd_parse_copy_state(&back, s);
    return efd_parse_float(s);
  } else {
    from = construct_efd_reference_to_here(s);
    bridge = create_efd_bridge(from, to);
    if (bridge != NULL) {
      efd_add_crossref(cr, bridge);
      return EFD_PARSER_FLOAT_REFVAL;
    } else {
      s->error = EFD_PE_MALFORMED;
      s->context = "num ref (incompatible destination type)";
      cleanup_efd_reference(from);
      cleanup_efd_reference(to);
      return EFD_PARSER_FLOAT_ERROR;
    }
  }
}

string* efd_parse_str_or_ref(efd_parse_state *s, efd_index *cr) {
  efd_reference *from, *to;
  efd_bridge *bridge;
  efd_parse_state back;

  efd_parse_copy_state(s, &back);
  to = efd_parse_global_ref(s);
  if (efd_parse_failed(s)) {
    s->error = EFD_PE_NO_ERROR;
    efd_parse_copy_state(&back, s);
    return efd_parse_str(s);
  } else {
    from = construct_efd_reference_to_here(s);
    bridge = create_efd_bridge(from, to);
    if (bridge != NULL) {
      efd_add_crossref(cr, bridge);
      return NULL;
    } else {
      s->error = EFD_PE_MALFORMED;
      s->context = "str ref (incompatible destination type)";
      cleanup_efd_reference(from);
      cleanup_efd_reference(to);
      return NULL;
    }
  }
}

void* efd_parse_obj_ref(efd_parse_state *s, efd_index *cr) {
  efd_reference *from, *to;
  efd_bridge *bridge;

  to = efd_parse_global_ref(s);
  if (efd_parse_failed(s)) {
    return NULL;
  } else {
    from = construct_efd_reference_to_here(s);
    bridge = create_efd_bridge(from, to);
    if (bridge != NULL) {
      efd_add_crossref(cr, bridge);
      return NULL;
    } else {
      s->error = EFD_PE_MALFORMED;
      s->context = "obj ref (incompatible destination type)";
      cleanup_efd_reference(from);
      cleanup_efd_reference(to);
      return NULL;
    }
  }
}

// Functions for parsing bits & pieces:
//-------------------------------------

char efd_parse_open(efd_parse_state *s) {
  static char* e = "opening brace";
  char c;
  efd_parse_skip(s);
  if (efd_parse_failed(s)) {
    s->error = EFD_PE_MALFORMED;
    s->context = e;
    return '\0';
  }

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_MISSING;
    s->context = e;
    return '\0';
  }
  c = s->input[s->pos];
  if (c != EFD_PARSER_OPEN_BRACE && c != EFD_PARSER_OPEN_ANGLE) {
    s->error = EFD_PE_MALFORMED;
    s->context = e;
    return '\0';
  }
  s->pos += 1;
  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return '\0';
  }
  c = s->input[s->pos];
  if (c != EFD_PARSER_OPEN_BRACE && c != EFD_PARSER_OPEN_ANGLE) {
    s->error = EFD_PE_MALFORMED;
    s->context = e;
    return '\0';
  }
  s->pos += 1;
  return c;
}

void efd_parse_close(efd_parse_state *s, char otype) {
  static char* e = "closing brace";
  char ctype;
  SKIP_OR_ELSE(s, e) //;

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_MISSING;
    s->context = e;
    return;
  }
  if (otype == EFD_PARSER_OPEN_ANGLE) {
    ctype = EFD_PARSER_CLOSE_ANGLE;
  } else {
    ctype = EFD_PARSER_CLOSE_BRACE;
  }
  if (s->input[s->pos] != ctype) {
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
  if (s->input[s->pos] != ctype) {
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
      return EFD_NT_PROTO;
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
    case 'G':
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

        case 'i':
          s->pos += 1;
          return EFD_NT_GLOBAL_INT;
        case 'n':
          s->pos += 1;
          return EFD_NT_GLOBAL_NUM;
        case 's':
          s->pos += 1;
          return EFD_NT_GLOBAL_STR;
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

  SKIP_OR_ELSE(s, "identifier") //;

  c = '\0';

  for (i = 0; i < EFD_NODE_NAME_SIZE; ++i) {
    if (efd_parse_atend(s)) {
      break;
    }
    c = s->input[s->pos];
    if (
      is_whitespace(&c)
   || is_special(&c)
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
      s->context = "identifier (initial numeral not allowed)";
    } else {
      s->error = EFD_PE_INCOMPLETE;
      s->context = "identifier (missing)";
    }
    return;
  }
  if (i == EFD_NODE_NAME_SIZE && !efd_parse_atend(s)) {
    c = s->input[s->pos];
    if (!is_whitespace(&c) && !is_special(&c)) {
      s->error = EFD_PE_MALFORMED;
      s->context = "identifier (identifier too long)";
      return;
    }
  }
  r_name[i] = '\0';
}

void efd_parse_annotation(efd_parse_state *s, char sep, char *r_name) {
  char c;

  r_name[0] = '\0';

  SKIP_OR_ELSE(s, "annotation") //;

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_MISSING;
    s->context = "annotation (missing)";
    return;
  }

  c = s->input[s->pos];

  if (c != sep) {
    s->error = EFD_PE_MISSING;
    s->context = "annotation (no indicator)";
    return;
  }
  s->pos += 1;

  efd_parse_name(s, r_name);
  if (efd_parse_failed(s)) {
    // (error from parse_name)
    s->context = "annotation (invalid name)";
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
        } else if (state == EFD_INT_STATE_BASE) {
          state = EFD_INT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        } else if (state == EFD_INT_STATE_PRE_DIGITS) {
          state = EFD_INT_STATE_DONE;
          s->pos -= 2; // about to be incremented before leaving while
        } else {
          s->error = EFD_PE_MALFORMED;
          s->context = "integer (minus)";
          return EFD_PARSER_INT_ERROR;
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
          if (state == EFD_INT_STATE_DIGITS) {
            state = EFD_INT_STATE_DONE;
            s->pos -= 1; // about to be incremented before leaving while
            break;
          } else {
            s->error = EFD_PE_MALFORMED;
            s->context = "integer (unexpected hex digit)";
            return EFD_PARSER_INT_ERROR;
          }
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
  if (
    state == EFD_INT_STATE_DONE
 || state == EFD_INT_STATE_BASE
 || state == EFD_INT_STATE_DIGITS
  ) {
    return sign * result;
  } else if (state == EFD_INT_STATE_PRE_DIGITS) {
    s->pos -= 1;
    return 0;
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
        } else if (state == EFD_FLOAT_STATE_EXP && count == 0) {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 3; // skip back to the 'e' or 'E'
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
        } else if (state == EFD_FLOAT_STATE_EXP && count == 0) {
          state = EFD_FLOAT_STATE_DONE;
          s->pos -= 3; // skip back to the 'e' or 'E'
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
          count = 0;
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
  if (
    state == EFD_FLOAT_STATE_DONE
 || state == EFD_FLOAT_STATE_ZERO
 || (state == EFD_FLOAT_STATE_CHAR && count > 0)
 || state == EFD_FLOAT_STATE_MANT
 || (state == EFD_FLOAT_STATE_EXP_DIGITS && count > 0)
  ) {
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

string* efd_parse_str(efd_parse_state *s) {
  ptrdiff_t start, end;
  char *pure;
  string *result;

  efd_grab_string_limits(s, &start, &end);
  if (efd_parse_failed(s)) {
    return NULL;
  }

  pure = efd_purify_string(s->input, start, end);

  result = create_string_from_ntchars(pure);

  free(pure);

  return result;
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
  *start = s->pos;
  s->pos += 1;
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
      *end = s->pos - 1;
      break;
  }
}

char * efd_purify_string(
  char const * const input,
  ptrdiff_t start,
  ptrdiff_t end
) {
  ptrdiff_t len = end - start - 1;
  char *result = (char*) malloc(sizeof(char) * (len + 1));
  ptrdiff_t i, offset;
  char q, c;
  int skip = 0;
  q = input[start];
  i = 0;
  for (offset = start + 1; offset < end; ++offset) {
    c = input[offset];
    if (c == q) {
      if (skip) {
        skip = 0;
        i -= 1; // counteract the increment
      } else {
        skip = 1;
        result[i] = c;
      }
#ifdef DEBUG
    } else if (skip) {
      fprintf(stderr, "Warning: bad quote found in string while purifying.\n");
      fprintf(stderr, "  String: %.*s\n", (int) len, input + start + 1);
      skip = 0;
      result[i] = c;
#endif
    } else {
      result[i] = c;
    }
    i += 1;
  }
  result[i] = '\0';
  return result;
}

efd_reference* construct_efd_reference_to_here(efd_parse_state* s) {
  efd_ref_type type = efd_nt__rt(s->current_node->h.type);
  efd_address *addr = construct_efd_address_of_node(s->current_node);
  ptrdiff_t idx = s->current_index;
  return create_efd_reference(type, addr, idx);
}

efd_reference* efd_parse_global_ref(efd_parse_state *s) {
  char name[EFD_NODE_NAME_SIZE + 1];
  efd_node_type n_type;
  efd_ref_type type;
  efd_address *addr;
  efd_reference *result;
  char c;

  efd_parse_skip(s);
  if (efd_parse_failed(s)) {
    s->error = EFD_PE_MISSING;
    s->context = "global reference (missing)";
    return NULL;
  }

  c = s->input[s->pos];
  if (c != EFD_PARSER_HASH) {
    s->error = EFD_PE_MALFORMED;
    s->context = "global reference (indicator)";
    return NULL;
  }

  n_type = efd_parse_type(s);
  if (efd_parse_failed(s)) {
    return NULL;
  }
  type = efd_nt__rt(n_type);
  switch (type) {
    case EFD_RT_INT:
      type = EFD_RT_GLOBAL_INT;
      break;
    case EFD_RT_NUM:
      type = EFD_RT_GLOBAL_NUM;
      break;
    case EFD_RT_STR:
      type = EFD_RT_GLOBAL_STR;
      break;
    default:
      s->error = EFD_PE_MALFORMED;
      s->context = "global reference (invalid type)";
      return NULL;
  }

  c = s->input[s->pos];
  if (c != EFD_PARSER_COLON) {
    s->error = EFD_PE_MALFORMED;
    s->context = "global reference (type delimiter)";
    return NULL;
  }
  s->pos += 1;

  efd_parse_name(s, name);
  if (efd_parse_failed(s)) {
    return NULL;
  }

  addr = create_efd_address(NULL, name);

  result = create_efd_reference(
    type,
    addr,
    -1
  );

  return result;
}

efd_address* efd_parse_address(efd_parse_state *s) {
  char name[EFD_NODE_NAME_SIZE + 1];
  efd_address *result;
  efd_address *tail;
  char c;

  efd_parse_skip(s);
  if (efd_parse_failed(s)) {
    s->error = EFD_PE_MISSING;
    s->context = "address (missing)";
    return NULL;
  }

  // pos will be incremented at start of while loop, so pre-decrement it here
  s->pos -= 1;

  result = NULL;
  tail = NULL;
  c = EFD_NODE_SEP;
  while (c == EFD_NODE_SEP) {
    // TODO: respect max depth here?
    s->pos += 1;
    efd_parse_skip(s);
    if (efd_parse_failed(s)) {
      if (result != NULL) {
        cleanup_efd_address(result);
      }
      s->error = EFD_PE_MALFORMED;
      s->context = "address (path list)";
      return NULL;
    }
    efd_parse_name(s, name);
    if (efd_parse_failed(s)) {
      if (!efd_parse_atend(s)) {
        // A special exception for the EFD_ADDR_PARENT character, which is not
        // normally a valid name but may appear in addresses.
        c = s->input[s->pos];
        if (c == EFD_ADDR_PARENT) {
          name[0] = EFD_ADDR_PARENT;
          name[1] = '\0';
          s->pos += 1;
        } else {
          if (result != NULL) {
            cleanup_efd_address(result);
          }
          return NULL; // error message from efd_parse_name
        }
      } else {
        if (result != NULL) {
          cleanup_efd_address(result);
        }
        return NULL; // error message from efd_parse_name
      }
    }
    if (result == NULL) {
      result = create_efd_address(NULL, name);
      tail = result;
    } else {
      tail->next = create_efd_address(tail, name);
      tail = tail->next;
    }
    efd_parse_skip(s);
    if (!efd_parse_atend(s)) {
      if (efd_parse_failed(s)) {
        // we failed to find empty space
        if (result != NULL) {
          cleanup_efd_address(result);
        }
        s->error = EFD_PE_MALFORMED;
        s->context = "address (path list)";
        return NULL;
      } // else

      c = s->input[s->pos];

    } else {
      if (result == NULL) { // shouldn't be possible
        s->error = EFD_PE_MALFORMED;
        s->context = "address (ended prematurely)";
        return NULL;
      } // else

      break;

    }
  }

  return result;
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

void efd_print_parse_error(efd_parse_state *s) {
  char const *context;
  char before[EFD_PARSER_ERROR_LINE+1];
  char after[EFD_PARSER_ERROR_LINE+1];
  char above[EFD_PARSER_ERROR_LINE+1];
  char below[EFD_PARSER_ERROR_LINE+1];
  ptrdiff_t offset;
  ptrdiff_t sursize;
  if (s->error != EFD_PE_NO_ERROR) {
    // figure out our surroundings:
    memset(before, ' ', EFD_PARSER_ERROR_LINE / 2);
    memset(after, '~', EFD_PARSER_ERROR_LINE / 2);
    memset(before + (EFD_PARSER_ERROR_LINE /2), '~', EFD_PARSER_ERROR_LINE / 2);
    memset(after + (EFD_PARSER_ERROR_LINE / 2), ' ', EFD_PARSER_ERROR_LINE / 2);
    memset(above, '-', EFD_PARSER_ERROR_LINE);
    memset(below, '-', EFD_PARSER_ERROR_LINE);
    before[EFD_PARSER_ERROR_LINE] = '\0';
    after[EFD_PARSER_ERROR_LINE] = '\0';
    above[EFD_PARSER_ERROR_LINE / 2] = 'v';
    strncpy(above + (EFD_PARSER_ERROR_LINE / 2) + 2, "here", 4);
    below[EFD_PARSER_ERROR_LINE / 2] = '^';
    strncpy(below + (EFD_PARSER_ERROR_LINE / 2) - 5, "here", 4);
    above[EFD_PARSER_ERROR_LINE] = '\0';
    below[EFD_PARSER_ERROR_LINE] = '\0';

    // before:
    offset = EFD_PARSER_ERROR_LINE / 2;
    offset -= s->pos;
    if (offset < (EFD_PARSER_ERROR_LINE / 2) - EFD_PARSER_ERROR_BEFORE) {
      offset = (EFD_PARSER_ERROR_LINE / 2) - EFD_PARSER_ERROR_BEFORE;
      before[offset-3] = '.';
      before[offset-2] = '.';
      before[offset-1] = '.';
    }
    sursize = (EFD_PARSER_ERROR_LINE / 2) - offset;
    strncpy(before + offset, s->input + (s->pos - sursize), sursize);

    // after
    for (sursize = 0; sursize < EFD_PARSER_ERROR_AFTER; ++sursize) {
      if (
        s->pos + sursize >= s->input_length
     || s->input[s->pos + sursize] == '\0'
      ) {
        break;
      }
    }
    if (
      s->pos + sursize < s->input_length
   && s->input[s->pos + sursize] != '\0'
    ) {
      after[(EFD_PARSER_ERROR_LINE / 2) + sursize] = '.';
      after[(EFD_PARSER_ERROR_LINE / 2) + sursize + 1] = '.';
      after[(EFD_PARSER_ERROR_LINE / 2) + sursize + 2] = '.';
    }
    strncpy(after + (EFD_PARSER_ERROR_LINE / 2), s->input + s->pos, sursize);

    // replace newlines, tabs, and carriage returns:
    for (offset = 0; offset < EFD_PARSER_ERROR_LINE / 2; ++offset) {
      if (
        before[offset] == '\n'
     || before[offset] == '\r'
     || before[offset] == '\t'
      ) {
        before[offset] = '';
      }
      if (
        after[(EFD_PARSER_ERROR_LINE / 2) + offset] == '\n'
     || after[(EFD_PARSER_ERROR_LINE / 2) + offset] == '\r'
     || after[(EFD_PARSER_ERROR_LINE / 2) + offset] == '\t'
      ) {
        after[(EFD_PARSER_ERROR_LINE / 2) + offset] = '';
      }
    }

    // figure out our context:
    context = s->context;
    if (context == NULL) {
      context = "<unknown>";
    }

    // print the file and line number:
    fprintf(
      stderr,
      "[%.*s:%ld] ",
      EFD_PARSER_MAX_FILENAME_DISPLAY,
      s->filename,
      s->lineno
    );

    // print the error:
    switch (s->error) {
      default:
      case EFD_PE_UNKNOWN:
        s->error = EFD_PE_UNKNOWN;
        fprintf(
          stderr,
          "Unknown parsing error while parsing %.*s.\n",
          EFD_PARSER_MAX_CONTEXT_DISPLAY,
          context
        );
        break;
      case EFD_PE_MALFORMED:
        fprintf(
          stderr,
          "Malformed input while parsing %.*s.\n",
          EFD_PARSER_MAX_CONTEXT_DISPLAY,
          context
        );
        break;
      case EFD_PE_MISSING:
        fprintf(
          stderr,
          "Missing element while parsing %.*s.\n",
          EFD_PARSER_MAX_CONTEXT_DISPLAY,
          context
        );
        break;
      case EFD_PE_INCOMPLETE:
        fprintf(
          stderr,
          "Input ended while parsing %.*s.\n",
          EFD_PARSER_MAX_CONTEXT_DISPLAY,
          context
        );
        break;
    }

    // print the surroundings:
    fprintf(stderr, "%.*s\n", EFD_PARSER_ERROR_LINE, above);
    fprintf(stderr, "%.*s\n", EFD_PARSER_ERROR_LINE, before);
    fprintf(stderr, "%.*s\n", EFD_PARSER_ERROR_LINE, after);
    fprintf(stderr, "%.*s\n", EFD_PARSER_ERROR_LINE, below);
  }
}

void efd_throw_parse_error(efd_parse_state *s) {
  if (s->error != EFD_PE_NO_ERROR) {
    efd_print_parse_error(s);
    exit(-s->error);
  }
}
