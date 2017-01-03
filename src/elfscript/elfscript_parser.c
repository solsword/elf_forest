// elfscript_parser.c
// Parsing for ElfScript.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include <string.h>

#include "util.h"

#include "filesys/filesys.h"

#include "elfscript_parser.h"
#include "elfscript.h"

/**********************
 * Boilerplate Macros *
 **********************/

#define SKIP_OR_ELSE(S, X) \
  elfscript_parse_skip(S); \
  if (elfscript_parse_failed(S)) { \
    if (s->context == NULL) { \
      s->context = (X); \
    } \
    return; \
  }

/*************
 * Functions *
 *************/

void elfscript_parse_copy_state(
  elfscript_parse_state *from,
  elfscript_parse_state *to) {
  to->pos = from->pos;
  to->filename = from->filename;
  to->lineno = from->lineno;
  to->next_closing_brace = from->next_closing_brace;
  to->context = from->context;
  to->error = from->error;
  if (to->current_address != NULL) {
    cleanup_elfscript_address(to->current_address);
  }
  to->current_address = copy_efd_address(from->current_address);
  to->current_node = from->current_node;
  to->current_index = from->current_index;
}

void efd_parse_scrub_state(efd_parse_state *state) {
  if (state->current_address != NULL) {
    cleanup_efd_address(state->current_address);
    state->current_address = NULL;
  }
}

int efd_parse_file(
  efd_node *parent,
  char const * const filename
) {
  efd_parse_state s;
  char *contents;

  s.input = NULL;
  s.input_length = 0;
  s.pos = 0;
  s.filename = filename;
  s.lineno = 0;
  s.next_closing_brace = '\0';
  s.context = NULL;
  s.error = EFD_PE_NO_ERROR;
  s.current_address = construct_efd_address_of_node(parent);
  s.current_node = parent;
  s.current_index = -1;

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
    efd_parse_scrub_state(&s);
    return 0;
  } else {
    s.input = NULL;
    s.input_length = 0;
    free(contents);
    s.context = NULL;
    efd_parse_scrub_state(&s);
    return 1;
  }
}

efd_address* efd_parse_string_address(string const * const astr) {
  efd_address *result;
  efd_parse_state s;
  s.input = s_raw(astr);
  s.input_length = s_count_bytes(astr);
  s.pos = 0;
  s.filename = "<address string>";
  s.lineno = -1;
  s.next_closing_brace = '\0';
  s.context = "string address";
  s.error = EFD_PE_NO_ERROR;
  s.current_address = NULL;
  s.current_node = NULL;
  s.current_index = -1;

  result = efd_parse_address(&s);
  if (efd_parse_failed(&s)) {
    efd_throw_parse_error(&s);
    return NULL;
  }
  return result;
}

// Private helper for adding global nodes:
void _efd_add_v_global(void *v_node) {
  efd_node *n = (efd_node*) v_node;
  n = copy_efd_node(n); // original will be cleaned up with the GLOBAL node
  efd_set_global(n->h.name, n);
}

efd_node* efd_parse_any(efd_parse_state *s) {
  efd_node_type type;
  efd_node *parent;
  efd_node *existing_node = NULL;
  efd_node *result;
  string *name;
  char btype, old_ctype;

  btype = efd_parse_open(s);
  if (efd_parse_failed(s)) {
    if (s->error == EFD_PE_MISSING) {
      // if there's nothing here (as opposed to something malformed) then we'll
      // just let efd_parse_children know that it's done
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(stderr, "* EFD parsing:?:?:node-missing\n");
#endif
      s->error = EFD_PE_CHILDREN_DONE;
    } else if (
      s->error == EFD_PE_MALFORMED
   && is_closing_brace(s->input[s->pos])
    ) {
      // if there's a closing brace instead of an opening one, let
      // efd_parse_children know it should rewind.
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(stderr, "* EFD parsing:?:?:closing-brace\n");
#endif
      s->error = EFD_PE_CLOSING_BRACE;
    }
    // in any case if there's an error return NULL
    return NULL;
  }
  if (s->next_closing_brace == EFD_PARSER_HASH && btype == EFD_PARSER_HASH) {
    // this is the expected closing brace rather than the opening brace of a
    // child: let efd_parse_children know that it should rewind.
#ifdef DEBUG_TRACE_EFD_PARSING
    fprintf(stderr, "* EFD parsing:?:?:closing-hash\n");
#endif
    s->error = EFD_PE_CLOSING_BRACE;
    return NULL;
  }

  old_ctype = s->next_closing_brace;
  s->next_closing_brace = closing_brace_for(btype);

  type = efd_parse_type(s);
  if (efd_parse_failed(s)) {
#ifdef DEBUG_TRACE_EFD_PARSING
    fprintf(stderr, "! EFD parsing:?:?:bad-type\n");
#endif
    return NULL;
  }

  if (btype == EFD_PARSER_OPEN_ANGLE) {
    if (!efd_is_link_type(type)) {
      s->error = EFD_PE_MALFORMED;
      s->context = "node (angle braces are for links)";
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "! EFD parsing:%s:?:non-link-angle-braces\n",
        EFD_NT_ABBRS[type]
      );
#endif
      return NULL;
    }
    // name will be decided by efd_parse_link:
    name = copy_string(EFD_ANON_NAME);
  } else if (btype == EFD_PARSER_OPEN_CURLY) {
    if (type != EFD_NT_CONTAINER) {
      s->error = EFD_PE_MALFORMED;
      s->context = "node (curly braces are for containers only)";
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "! EFD parsing:%s:?:non-container-curly-braces\n",
        EFD_NT_ABBRS[type]
      );
#endif
      return NULL;
    }
    // parse the name immediately:
    name = efd_parse_name(s);
    if (efd_parse_failed(s)) { return NULL; }
  } else if (btype == EFD_PARSER_HASH) {
    if (type != EFD_NT_GLOBAL) {
      s->error = EFD_PE_MALFORMED;
      s->context = "node (hashes are for global declarations only)";
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "! EFD parsing:%s:?:non-global-hashes\n",
        EFD_NT_ABBRS[type]
      );
#endif
      return NULL;
    }
    // parse the name immediately:
    name = efd_parse_name(s);
    if (efd_parse_failed(s)) { return NULL; }
  } else {
    if (efd_is_link_type(type)) {
      s->error = EFD_PE_MALFORMED;
      s->context = "node (links require angle braces)";
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "! EFD parsing:%s:?:link-without-angle-braces\n",
        EFD_NT_ABBRS[type]
      );
#endif
      return NULL;
    }
    if (type == EFD_NT_GLOBAL) {
      s->error = EFD_PE_MALFORMED;
      s->context = "node (global declarations require octothorpes)";
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "! EFD parsing:%s:?:global-without-hashes\n",
        EFD_NT_ABBRS[type]
      );
#endif
      return NULL;
    }
    if (btype == EFD_PARSER_OPEN_PAREN) {
      // an anonymous node:
      name = copy_string(EFD_ANON_NAME);
    } else {
      // parse the name immediately:
      name = efd_parse_name(s);
      if (efd_parse_failed(s)) {
#ifdef DEBUG_TRACE_EFD_PARSING
        fprintf(
          stderr,
          "! EFD parsing:%s:?:bad-name\n",
          EFD_NT_ABBRS[type]
        );
#endif
        return NULL;
      }
    }
  }

  if (btype == EFD_PARSER_OPEN_CURLY) {
    existing_node = efdx(s->current_node, name);
    if (existing_node != NULL) {
      result = existing_node;
    } else {
      result = create_efd_node(type, name, NULL);
    }
  } else {
    result = create_efd_node(type, name, NULL);
  }
  cleanup_string(name);
  parent = s->current_node;
  efd_append_address(s->current_address, result->h.name);
  s->current_node = result;
  s->current_index = -1;

  switch(type) {
    default:
    case EFD_NT_INVALID:
    case EFD_NT_OBJECT:
      s->error = EFD_PE_MALFORMED;
      s->context = "node (invalid type)";
      s->current_node = parent;
      cleanup_efd_address(efd_pop_address(s->current_address));
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "! EFD parsing:%s:%s:invalid-type\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      return NULL;

    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
    case EFD_NT_GLOBAL:
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:container\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      efd_parse_children(result, s);
      break;

    case EFD_NT_ROOT_LINK:
    case EFD_NT_GLOBAL_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:link\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      efd_parse_link(result, s);
      break;

    case EFD_NT_FUNCTION:
    case EFD_NT_FN_OBJ:
    case EFD_NT_FN_INT:
    case EFD_NT_FN_NUM:
    case EFD_NT_FN_STR:
    case EFD_NT_FN_AR_INT:
    case EFD_NT_FN_AR_NUM:
    case EFD_NT_FN_AR_STR:
    case EFD_NT_GENERATOR:
    case EFD_NT_GN_OBJ:
    case EFD_NT_GN_INT:
    case EFD_NT_GN_NUM:
    case EFD_NT_GN_STR:
    case EFD_NT_GN_AR_INT:
    case EFD_NT_GN_AR_NUM:
    case EFD_NT_GN_AR_STR:
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:function\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      efd_parse_function(result, s);
      break;

    case EFD_NT_PROTO:
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:proto\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      efd_parse_proto(result, s);
      break;

    case EFD_NT_INTEGER:
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:integer\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      efd_parse_integer(result, s);
      break;

    case EFD_NT_NUMBER:
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:number\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      efd_parse_number(result, s);
      break;

    case EFD_NT_STRING:
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:string\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      efd_parse_string(result, s);
      break;

    case EFD_NT_ARRAY_INT:
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:int-array\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      efd_parse_int_array(result, s);
      break;

    case EFD_NT_ARRAY_NUM:
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:num-array\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      efd_parse_num_array(result, s);
      break;

    case EFD_NT_ARRAY_STR:
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:str-array\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
      efd_parse_str_array(result, s);
      break;
  }
  if (efd_parse_failed(s)) {
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "! EFD parsing:%s:%s:parse-failed\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
#ifndef DEBUG
#ifdef DEBUG_TRACE_EFD_CLEANUP
    fprintf(
      stderr,
      "~ EFD cleanup[%p]:%s:failed-parse\n",
      result,
      s_raw(result->h.name)
    );
#endif
    cleanup_efd_node(result);
#endif
    return NULL;
  }

  efd_parse_close(s);
  if (efd_parse_failed(s)) {
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "! EFD parsing:%s:%s:unclosed\n",
        EFD_NT_ABBRS[type],
        s_raw(result->h.name)
      );
#endif
#ifndef DEBUG
#ifdef DEBUG_TRACE_EFD_CLEANUP
    fprintf(
      stderr,
      "~ EFD cleanup[%p]:%s:bad-closure.\n",
      result,
      s_raw(result->h.name)
    );
#endif
    cleanup_efd_node(result);
#endif
    return NULL;
  }

  s->next_closing_brace = old_ctype;
  cleanup_efd_address(efd_pop_address(s->current_address));
  s->current_node = parent;
  s->current_index = -1;

  if (type == EFD_NT_GLOBAL) {
    // Add our children to the globals dictionary, clean ourselves up, and
    // signal efd_parse_children that it should skip this result.
#ifdef DEBUG_TRACE_EFD_PARSING
    fprintf(
      stderr,
      "  EFD parsing:%s:%s:handle-globals\n",
      EFD_NT_ABBRS[type],
      s_raw(result->h.name)
    );
#endif
    d_foreach(efd_children_dict(result), &_efd_add_v_global);
    cleanup_efd_node(result);
    s->error = EFD_PE_SKIP_CHILD;
    return NULL;
  }

#ifdef DEBUG_TRACE_EFD_PARSING
  fprintf(
    stderr,
    "  EFD parsing:%s:%s:done\n",
    EFD_NT_ABBRS[type],
    s_raw(result->h.name)
  );
#endif
  if (existing_node != NULL) {
    // If we added to an existing node, let efd_parse_children know that we
    // shouldn't give that node an extra entry among its parents' children.
    s->error = EFD_PE_SKIP_CHILD;
  }
  return result;
}

// Parsing functions for the EFD primitive types:
//-----------------------------------------------

void efd_parse_children(efd_node *result, efd_parse_state *s) {
  efd_parse_state back;
  back.current_address = NULL;
  efd_node *child;

  if (!efd_is_container_node(result)) {
    // TODO: Context here!
    fprintf(
      stderr,
      "ERROR: Attempt to parse children for non-container node.\n"
    );
    exit(EXIT_FAILURE);
  }

  // parse any number of children and add them:
  while (1) {
    efd_parse_copy_state(s, &back);
    child = efd_parse_any(s);
    if (efd_parse_failed(s)) {
      if (s->error == EFD_PE_SKIP_CHILD) {
        // we need to skip this child
#ifdef DEBUG_TRACE_EFD_PARSING
        fprintf(
          stderr,
          "  EFD parsing:%s:%s:skip\n",
          EFD_NT_ABBRS[result->h.type],
          s_raw(result->h.name)
        );
#endif
        s->error = EFD_PE_NO_ERROR;
        continue;
      } else if (s->error == EFD_PE_CHILDREN_DONE) {
        // we've hit the end of this object: return
#ifdef DEBUG_TRACE_EFD_PARSING
        fprintf(
          stderr,
          "  EFD parsing:%s:%s:children-done\n",
          EFD_NT_ABBRS[result->h.type],
          s_raw(result->h.name)
        );
#endif
        s->error = EFD_PE_NO_ERROR;
        break;
      } else if (s->error == EFD_PE_CLOSING_BRACE) {
        // we've probably hit the end of this object: rewind and return
#ifdef DEBUG_TRACE_EFD_PARSING
        fprintf(
          stderr,
          "  EFD parsing:%s:%s:closing-brace\n",
          EFD_NT_ABBRS[result->h.type],
          s_raw(result->h.name)
        );
#endif
        s->error = EFD_PE_NO_ERROR;
        efd_parse_copy_state(&back, s);
        break;
      } else {
        // a real error that we should throw up the stack
#ifdef DEBUG_TRACE_EFD_PARSING
        fprintf(
          stderr,
          "  EFD parsing:%s:%s:error-up\n",
          EFD_NT_ABBRS[result->h.type],
          s_raw(result->h.name)
        );
#endif
        break;
      }
    } else {
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "  EFD parsing:%s:%s:child\n",
        EFD_NT_ABBRS[result->h.type],
        s_raw(result->h.name)
      );
#endif
      efd_add_child(result, child);
    }
  }
  efd_parse_scrub_state(&back);
}

void efd_parse_link(efd_node *result, efd_parse_state *s) {
  string *name;
  efd_address *target;
  efd_address *tail;
  efd_parse_state back;
  back.current_address = NULL;

  target = efd_parse_address(s);
  if (efd_parse_failed(s)) {
#ifdef DEBUG_TRACE_EFD_PARSING
      fprintf(
        stderr,
        "! EFD parsing:%s:%s:bad-address\n",
        EFD_NT_ABBRS[result->h.type],
        s_raw(result->h.name)
      );
#endif
    return;
  }

  efd_parse_copy_state(s, &back);
  name = efd_parse_annotation(s, EFD_PARSER_RENAME);
  if (efd_parse_failed(s)) {
    if (s->error == EFD_PE_MISSING) {
      // Use the target node's name as the name of this node by default:
      efd_parse_copy_state(&back, s);
      tail = target;
      while (tail->next != NULL) {
        tail = tail->next;
      }
      name = copy_string(tail->name);
    } else {
      efd_parse_scrub_state(&back);
      return;
    }
  }

  cleanup_string(result->h.name);
  result->h.name = name;
  result->b.as_link.target = target;
  efd_parse_scrub_state(&back);
}

void efd_parse_function(efd_node *result, efd_parse_state *s) {
  result->b.as_function.function = efd_parse_annotation(s, EFD_PARSER_COLON);
  if (efd_parse_failed(s)) { return; }

  efd_parse_children(result, s);
}

void efd_parse_proto(efd_node *result, efd_parse_state *s) {
  efd_node *proto;
  result->b.as_proto.format = efd_parse_annotation(s, EFD_PARSER_COLON);
  if (efd_parse_failed(s)) { return; }

  proto = create_efd_node(EFD_NT_CONTAINER, EFD_ANON_NAME, result);
  efd_parse_children(proto, s);
  if (efd_parse_failed(s)) {
#ifdef DEBUG_TRACE_EFD_CLEANUP
    fprintf(
      stderr,
      "~ EFD cleanup[%p]:%s:proto-parse-failure\n",
      proto,
      s_raw(proto->h.name)
    );
#endif
    cleanup_efd_node(proto);
    return;
  }

  result->b.as_proto.input = proto;
}

void efd_parse_integer(efd_node *result, efd_parse_state *s) {
  efd_int_t i = efd_parse_int_or_ref(s);
  if (efd_parse_failed(s)) {
    return;
  } else {
    result->b.as_integer.value = i;
  }
}

void efd_parse_number(efd_node *result, efd_parse_state *s) {
  efd_num_t n = efd_parse_float_or_ref(s);
  if (efd_parse_failed(s)) {
    return;
  } else {
    result->b.as_number.value = n;
  }
}

void efd_parse_string(efd_node *result, efd_parse_state *s) {
  result->b.as_string.value = efd_parse_str_or_ref(s);
  if (efd_parse_failed(s)) {
    return;
  }
}

void efd_parse_int_array(efd_node *result, efd_parse_state *s) {
  size_t i;
  efd_int_t val;
  list *l = create_list();

  s->current_index = 0;
  while (1) {
    val = efd_parse_int_or_ref(s);
    if (efd_parse_failed(s)) {
      if (is_closing_brace(s->input[s->pos])) {
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
      if (is_closing_brace(s->input[s->pos])) {
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
    exit(EXIT_FAILURE);
  }
#endif
  result->b.as_int_array.values = (efd_int_t*) malloc(
    result->b.as_int_array.count * sizeof(efd_int_t)
  );
  for (i = 0; i < result->b.as_int_array.count; ++i) {
    result->b.as_int_array.values[i] = (efd_int_t) l_get_item(l, i);
  }
  cleanup_list(l);
}

void efd_parse_num_array(efd_node *result, efd_parse_state *s) {
  size_t i;
  efd_num_t val;
  void *v;
  list *l = create_list();

  s->current_index = 0;
  while (1) {
    val = efd_parse_float_or_ref(s);
    if (efd_parse_failed(s)) {
      if (is_closing_brace(s->input[s->pos])) {
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
      if (is_closing_brace(s->input[s->pos])) {
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
    exit(EXIT_FAILURE);
  }
#endif
  result->b.as_num_array.values = (efd_num_t*) malloc(
    result->b.as_num_array.count * sizeof(efd_num_t)
  );
  for (i = 0; i < result->b.as_num_array.count; ++i) {
    v = l_get_item(l, i);
    result->b.as_num_array.values[i] = *((efd_num_t*) &v);
  }
  cleanup_list(l);
}

void efd_parse_str_array(efd_node *result, efd_parse_state *s) {
  size_t i;
  string *val;
  list *l = create_list();

  s->current_index = 0;
  while (1) {
    val = efd_parse_str_or_ref(s);
    if (efd_parse_failed(s)) {
      if (is_closing_brace(s->input[s->pos])) {
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
      if (is_closing_brace(s->input[s->pos])) {
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
    exit(EXIT_FAILURE);
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

void efd_parse_int_global(efd_node *result, efd_parse_state *s) {
  static char const * const e = "global integer";
  char c;

  SKIP_OR_ELSE(s, e) //;

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }

  c = s->input[s->pos];
  if (c == EFD_PARSER_EQUALS) {
    s->pos += 1;
    SKIP_OR_ELSE(s, e) //;
  }

  result->b.as_integer.value = efd_parse_int_or_ref(s);
  // return whether or not there's an error...
}

void efd_parse_num_global(efd_node *result, efd_parse_state *s) {
  static char const * const e = "global number";
  char c;

  SKIP_OR_ELSE(s, e) //;

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }

  c = s->input[s->pos];
  if (c == EFD_PARSER_EQUALS) {
    s->pos += 1;
    SKIP_OR_ELSE(s, e) //;
  }

  result->b.as_number.value = efd_parse_float_or_ref(s);
  // return whether or not there's an error...
}

void efd_parse_str_global(efd_node *result, efd_parse_state *s) {
  static char const * const e = "global string";
  char c;

  SKIP_OR_ELSE(s, e) //;

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }

  c = s->input[s->pos];
  if (c == EFD_PARSER_EQUALS) {
    s->pos += 1;
    SKIP_OR_ELSE(s, e) //;
  }

  result->b.as_string.value = efd_parse_str_or_ref(s);
  if (efd_parse_failed(s)) {
    result->b.as_string.value = NULL;
    return;
  }
}

// Functions for parsing pieces that might be references:
//-------------------------------------------------------

efd_int_t efd_parse_int_or_ref(efd_parse_state *s) {
  efd_int_t result;
  efd_parse_state back;
  back.current_address = NULL;
  efd_reference *to;
  efd_node *gl, *glv;

  efd_parse_copy_state(s, &back);
  to = efd_parse_global_ref(s);
  if (efd_parse_failed(s)) {
    s->error = EFD_PE_NO_ERROR;
    efd_parse_copy_state(&back, s);
    efd_parse_scrub_state(&back);
    return efd_parse_int(s);
  } else {
    efd_parse_scrub_state(&back);
    if (to->type == EFD_RT_GLOBAL_INT) {
      gl = efd_get_global(to->addr->name);
      if (gl == NULL) {
        s->error = EFD_PE_MALFORMED;
        s->context = "int ref (destination does not exist)";
        cleanup_efd_reference(to);
        return EFD_PARSER_INT_ERROR;
      }
      if (gl->h.type == EFD_NT_INTEGER || gl->h.type == EFD_NT_NUMBER) {
        cleanup_efd_reference(to);
        return efd_as_i(gl);
      } else if (gl->h.type == EFD_NT_FN_INT || gl->h.type == EFD_NT_FN_NUM) {
        glv = efd_get_value(gl);
        result = efd_as_i(glv);
        cleanup_efd_node(glv);
        cleanup_efd_reference(to);
        return result;
      } else {
        s->error = EFD_PE_MALFORMED;
        s->context = "int ref (found global with incompatible type)";
        cleanup_efd_reference(to);
        return EFD_PARSER_INT_ERROR;
      }
    } else {
      s->error = EFD_PE_MALFORMED;
      s->context = "int ref (incompatible destination type)";
      cleanup_efd_reference(to);
      return EFD_PARSER_INT_ERROR;
    }
  }
}

efd_num_t efd_parse_float_or_ref(efd_parse_state *s) {
  efd_num_t result;
  efd_parse_state back;
  back.current_address = NULL;
  efd_reference *to;
  efd_node *gl, *glv;

  efd_parse_copy_state(s, &back);
  to = efd_parse_global_ref(s);
  if (efd_parse_failed(s)) {
    s->error = EFD_PE_NO_ERROR;
    efd_parse_copy_state(&back, s);
    efd_parse_scrub_state(&back);
    return efd_parse_float(s);
  } else {
    efd_parse_scrub_state(&back);
    if (to->type == EFD_RT_GLOBAL_NUM) {
      gl = efd_get_global(to->addr->name);
      if (gl == NULL) {
        s->error = EFD_PE_MALFORMED;
        s->context = "num ref (destination does not exist)";
        cleanup_efd_reference(to);
        return EFD_PARSER_NUM_ERROR;
      }
      if (gl->h.type == EFD_NT_INTEGER || gl->h.type == EFD_NT_NUMBER) {
        cleanup_efd_reference(to);
        return efd_as_n(gl);
      } else if (gl->h.type == EFD_NT_FN_INT || gl->h.type == EFD_NT_FN_NUM) {
        glv = efd_get_value(gl);
        result = efd_as_n(glv);
        cleanup_efd_node(glv);
        cleanup_efd_reference(to);
        return result;
      } else {
        s->error = EFD_PE_MALFORMED;
        s->context = "num ref (found global with incompatible type)";
        cleanup_efd_reference(to);
        return EFD_PARSER_NUM_ERROR;
      }
    } else {
      s->error = EFD_PE_MALFORMED;
      s->context = "num ref (incompatible destination type)";
      cleanup_efd_reference(to);
      return EFD_PARSER_NUM_ERROR;
    }
  }
}

string* efd_parse_str_or_ref(efd_parse_state *s) {
  string *result;
  efd_parse_state back;
  back.current_address = NULL;
  efd_reference *to;
  efd_node *gl, *glv;

  efd_parse_copy_state(s, &back);
  to = efd_parse_global_ref(s);
  if (efd_parse_failed(s)) {
    s->error = EFD_PE_NO_ERROR;
    efd_parse_copy_state(&back, s);
    efd_parse_scrub_state(&back);
    return efd_parse_str(s);
  } else {
    efd_parse_scrub_state(&back);
    if (to->type == EFD_RT_GLOBAL_STR) {
      gl = efd_get_global(to->addr->name);
      if (gl == NULL) {
        s->error = EFD_PE_MALFORMED;
        s->context = "str ref (destination does not exist)";
        cleanup_efd_reference(to);
        return NULL;
      }
      if (gl->h.type == EFD_NT_STRING) {
        cleanup_efd_reference(to);
        return copy_string(*efd__s(gl));
      } else if (gl->h.type == EFD_NT_FN_STR) {
        glv = efd_get_value(gl);
        result = copy_string(*efd__s(glv));
        cleanup_efd_node(glv);
        cleanup_efd_reference(to);
        return result;
      } else {
        s->error = EFD_PE_MALFORMED;
        s->context = "str ref (found global with incompatible type)";
        cleanup_efd_reference(to);
        return NULL;
      }
    } else {
      s->error = EFD_PE_MALFORMED;
      s->context = "str ref (incompatible destination type)";
      cleanup_efd_reference(to);
      return NULL;
    }
  }
}

void* efd_parse_obj_ref(efd_parse_state *s) {
  void *result;
  efd_copy_function copier;
  efd_reference *to;
  efd_node *gl, *glv;

  to = efd_parse_global_ref(s);
  if (efd_parse_failed(s)) {
    return NULL;
  } else {
    if (to->type == EFD_RT_GLOBAL_OBJ) {
      gl = efd_get_global(to->addr->name);
      if (gl == NULL) {
        s->error = EFD_PE_MALFORMED;
        s->context = "obj ref (destination does not exist)";
        cleanup_efd_reference(to);
        return NULL;
      }
      if (gl->h.type == EFD_NT_OBJECT) {
        copier = efd_lookup_copier(gl->b.as_object.format);
        if (copier == NULL) {
          efd_report_error(
            s_("Error finding copier during attempt get reference value from:"),
            gl
          );
        }
        cleanup_efd_reference(to);
        return copier(gl->b.as_object.value);
      } else if (gl->h.type == EFD_NT_FN_OBJ) {
        glv = efd_get_value(gl);
        result = copy_string(*efd__s(glv));
        copier = efd_lookup_copier(glv->b.as_object.format);
        if (copier == NULL) {
          efd_report_error(
            s_("Error finding copier during attempt get reference value from:"),
            glv
          );
        }
        result = copier(glv->b.as_object.value);
        cleanup_efd_node(glv);
        cleanup_efd_reference(to);
        return result;
      } else {
        s->error = EFD_PE_MALFORMED;
        s->context = "obj ref (found global with incompatible type)";
        cleanup_efd_reference(to);
        return NULL;
      }
    } else {
      s->error = EFD_PE_MALFORMED;
      s->context = "obj ref (incompatible destination type)";
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
  if (!is_opening_brace(c)) {
    s->error = EFD_PE_MALFORMED;
    s->context = e;
    return '\0';
  }
  s->pos += 1;
  if (c == EFD_PARSER_OPEN_PAREN) {
    return c;
  } // Other brace types must be doubled

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return '\0';
  }
  if (s->input[s->pos] != c) {
    s->error = EFD_PE_MALFORMED;
    s->context = e;
    return '\0';
  }
  s->pos += 1;
  return c;
}

void efd_parse_close(efd_parse_state *s) {
  static char* e = "closing brace";
  SKIP_OR_ELSE(s, e) //;

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_MISSING;
    s->context = e;
    return;
  }
  if (s->input[s->pos] != s->next_closing_brace) {
    s->error = EFD_PE_MALFORMED;
    s->context = e;
    return;
  }
  s->pos += 1;
  if (s->next_closing_brace == EFD_PARSER_CLOSE_PAREN) {
    return;
  } // other types must be doubled

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = e;
    return;
  }
  if (s->input[s->pos] != s->next_closing_brace) {
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
    case 'L':
      s->pos += 1;
      return EFD_NT_ROOT_LINK;
    case 'l':
      s->pos += 1;
      return EFD_NT_LOCAL_LINK;
    case 'V':
      s->pos += 1;
      return EFD_NT_SCOPE;
    case 'v':
      s->pos += 1;
      return EFD_NT_VARIABLE;
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
        return EFD_NT_GLOBAL;
      }
      c = s->input[s->pos];
      switch (c) {
        default:
          return EFD_NT_GLOBAL;
        case 'L':
          s->pos += 1;
          return EFD_NT_GLOBAL_LINK;
      }
      break;
    case 'f':
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

        case 'f':
          s->pos += 1;
          return EFD_NT_FUNCTION;
        case 'o':
          s->pos += 1;
          return EFD_NT_FN_OBJ;
        case 'i':
          s->pos += 1;
          return EFD_NT_FN_INT;
        case 'n':
          s->pos += 1;
          return EFD_NT_FN_NUM;
        case 's':
          s->pos += 1;
          return EFD_NT_FN_STR;
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

            case 'i':
              s->pos += 1;
              return EFD_NT_FN_AR_INT;
            case 'n':
              s->pos += 1;
              return EFD_NT_FN_AR_NUM;
            case 's':
              s->pos += 1;
              return EFD_NT_FN_AR_STR;
          }
          break;
      }
      break;
    case 'g':
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

        case 'g':
          s->pos += 1;
          return EFD_NT_GENERATOR;
        case 'o':
          s->pos += 1;
          return EFD_NT_GN_OBJ;
        case 'i':
          s->pos += 1;
          return EFD_NT_GN_INT;
        case 'n':
          s->pos += 1;
          return EFD_NT_GN_NUM;
        case 's':
          s->pos += 1;
          return EFD_NT_GN_STR;
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

            case 'i':
              s->pos += 1;
              return EFD_NT_GN_AR_INT;
            case 'n':
              s->pos += 1;
              return EFD_NT_GN_AR_NUM;
            case 's':
              s->pos += 1;
              return EFD_NT_GN_AR_STR;
          }
          break;
      }
      break;
  }
  // This shouldn't be reachable...
  s->error = EFD_PE_MALFORMED;
  s->context = e;
  return EFD_NT_INVALID;
}

string* efd_parse_name(efd_parse_state *s) {
  char c;
  char const *start;
  size_t len;

  efd_parse_skip(s);
  if (efd_parse_failed(s)) {
    s->context = "identifier";
    return NULL;
  }

  start = (s->input + s->pos);
  len = 0;

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_INCOMPLETE;
    s->context = "identifier (missing)";
    return NULL;
  }

  c = s->input[s->pos];

  if (c >= '0' && c <= '9') {
    s->error = EFD_PE_MALFORMED;
    s->context = "identifier (initial numeral not allowed)";
    return NULL;
  }

  // Scan until we hit the end of input or find a whitespace or special char:
  while (
    !efd_parse_atend(s)
 && !(
      is_whitespace(c)
   || is_special(c)
    )
  ) {
    len += 1;
    s->pos += 1;
    c = s->input[s->pos];
  }
  if (len == 0) {
    s->error = EFD_PE_MISSING;
    s->context = "identifier (missing)";
    return NULL;
  }
  return create_string_from_chars(start, len);
}

string* efd_parse_annotation(efd_parse_state *s, char sep) {
  char c;
  string *result;

  efd_parse_skip(s);
  if (efd_parse_failed(s)) {
    s->context = "annotation";
    return NULL;
  }

  if (efd_parse_atend(s)) {
    s->error = EFD_PE_MISSING;
    s->context = "annotation (missing)";
    return NULL;
  }

  c = s->input[s->pos];

  if (c != sep) {
    s->error = EFD_PE_MISSING;
    s->context = "annotation (no indicator)";
    return NULL;
  }
  s->pos += 1;

  result = efd_parse_name(s);
  if (efd_parse_failed(s)) {
    // (error from efd_parse_name)
    s->context = "annotation (invalid name)";
    return NULL;
  }
  return result;
}


efd_int_t efd_parse_int(efd_parse_state *s) {
  efd_int_t result;
  efd_int_t sign;
  efd_int_t base;
  efd_int_t digit;
  efd_int_t count;
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

efd_num_t efd_parse_float(efd_parse_state *s) {
  efd_num_t sign;
  efd_num_t characteristic;
  efd_num_t mantissa;
  efd_num_t mant_div;
  efd_num_t expsign;
  efd_num_t exponent;
  efd_num_t digit;
  intptr_t count;
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
          return EFD_PARSER_NUM_ERROR;
        }
        break;

      case '-':
        if (state == EFD_FLOAT_STATE_PRE) {
          if (sign == 1) {
            sign = -1;
          } else {
            s->error = EFD_PE_MALFORMED;
            s->context = "number (multiple signs)";
            return EFD_PARSER_NUM_ERROR;
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
          return EFD_PARSER_NUM_ERROR;
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
      return EFD_PARSER_NUM_ERROR;
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
    return EFD_PARSER_NUM_ERROR;
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
  intptr_t idx = s->current_index;
  efd_reference *result = create_efd_reference(type, s->current_address, idx);
  return result;
}

efd_reference* efd_parse_global_ref(efd_parse_state *s) {
  string *name;
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
  s->pos += 1;

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
    case EFD_RT_OBJ:
      type = EFD_RT_GLOBAL_OBJ;
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

  name = efd_parse_name(s);
  if (efd_parse_failed(s)) {
    return NULL;
  }

  addr = create_efd_address(NULL, name);
  cleanup_string(name);

  result = create_efd_reference(
    type,
    addr,
    -1
  );
  cleanup_efd_address(addr);

  return result;
}

efd_address* efd_parse_address(efd_parse_state *s) {
  string *name;
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
  c = EFD_ADDR_SEP_CHR;
  while (c == EFD_ADDR_SEP_CHR) {
    s->pos += 1;
    efd_parse_skip(s);
    if (efd_parse_failed(s)) {
      if (result != NULL) {
        cleanup_efd_address(result);
      }
      s->error = EFD_PE_MALFORMED;
      s->context = "address (pre separator)";
      return NULL;
    }
    name = efd_parse_name(s);
    if (efd_parse_failed(s)) {
      if (!efd_parse_atend(s)) {
        // A special exception for the EFD_ADDR_PARENT_CHR character, which is
        // not normally a valid name but may appear in addresses.
        c = s->input[s->pos];
        if (c == EFD_ADDR_PARENT_CHR) {
          name = copy_string(EFD_ADDR_PARENT_STR);
          s->error = EFD_PE_NO_ERROR;
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
      cleanup_string(name);
      name = NULL;
      tail = result;
    } else {
      tail->next = create_efd_address(tail, name);
      cleanup_string(name);
      name = NULL;
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
        s->context = "address (post separator)";
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
        } else if (!is_whitespace(c)) {
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
  int offset;
  int sursize;
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

    // check for manual abort:
    if (
      s->pos <= s->input_length - 2
   && s->input[s->pos] == EFD_PARSER_ABORT
   && s->input[s->pos+1] == EFD_PARSER_ABORT
    ) {
      s->error = EFD_PE_ABORT;
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
        fprintf(
          stderr,
          "Unknown parsing error while parsing %.*s.\n",
          EFD_PARSER_MAX_CONTEXT_DISPLAY,
          context
        );
        break;
      case EFD_PE_ABORT:
        fprintf(
          stderr,
          "Manual abort while parsing %.*s.\n",
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

  // If we're aborting manually, print the current node:
  if (s->error == EFD_PE_ABORT) {
    efd_report_error_full(
      s_("Current node at manual abort:"),
      s->current_node
    );
  }
}

void efd_throw_parse_error(efd_parse_state *s) {
  if (s->error != EFD_PE_NO_ERROR) {
    efd_print_parse_error(s);
    exit(s->error);
  }
}
