// efd.c
// Definition of the Elf Forest Data format.

#include "efd.h"
#include "efd_parser.h"

#include "datatypes/string.h"
#include "datatypes/list.h"
#include "datatypes/dictionary.h"
#include "datatypes/dict_string_keys.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

/***********
 * Globals *
 ***********/

int EFD_TRACK_ERROR_CONTEXTS;
list *EFD_ERROR_CONTEXT;

CSTR(EFD_FILE_EXTENSION, "efd", 3);

CSTR(EFD_COMMON_DIR_NAME, "data", 4);
string * EFD_COMMON_DIR; // assigned in efd_setup.h based on PS_RES_DIRECTORY

CSTR(EFD_ADDR_SEP_STR, ".", 1);
CSTR(EFD_ADDR_PARENT_STR, "^", 1);

CSTR(EFD_ANON_NAME, "_ANON_", 6);
CSTR(EFD_ROOT_NAME, "_ROOT_", 6);

char const * const EFD_NT_NAMES[] = {
  "<invalid>",
  "<any>",
  "container",
  "scope",
  "reroute",
  "global_link",
  "local_link",
  "variable",
  "prototype",
  "object",
  "integer",
  "number",
  "string",
  "array_int",
  "array_num",
  "array_str",
  "global_int",
  "global_num",
  "global_str",
  "function",
  "function_object",
  "function_integer",
  "function_number",
  "function_string",
  "function_array_integer",
  "function_array_number",
  "function_array_string",
  "generator",
  "generator_object",
  "generator_integer",
  "generator_number",
  "generator_string",
  "generator_array_integer",
  "generator_array_number",
  "generator_array_string"
};

char const * const EFD_NT_ABBRS[] = {
  "!",
  "*",
  "c",
  "V",
  "R",
  "L",
  "l",
  "v",
  "p",
  "o",
  "i",
  "n",
  "s",
  "ai",
  "an",
  "as",
  "Gi",
  "Gn",
  "Gs",
  "ff",
  "fo",
  "fi",
  "fn",
  "fs",
  "fai",
  "fan",
  "fas",
  "gg",
  "go",
  "gi",
  "gn",
  "gs",
  "gai",
  "gan",
  "gas"
};

efd_node *EFD_ROOT = NULL;
efd_index *EFD_COMMON_INDEX = NULL;

dictionary *EFD_INT_GLOBALS = NULL;
dictionary *EFD_NUM_GLOBALS = NULL;
dictionary *EFD_STR_GLOBALS = NULL;

/******************************
 * Constructors & Destructors *
 ******************************/

efd_node * create_efd_node(efd_node_type t, string const * const name) {
  efd_node *result = (efd_node*) malloc(sizeof(efd_node));
  result->h.type = t;
  switch (t) {
    default:
    case EFD_NT_INTEGER:
    case EFD_NT_NUMBER:
      // do nothing
      break;

    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
      result->b.as_container.children = create_dictionary(
        EFD_DEFAULT_DICTIONARY_SIZE
      );
      break;

    case EFD_NT_REROUTE:
      result->b.as_reroute.child = NULL;
      result->b.as_reroute.target = NULL;
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      result->b.as_link.target = NULL;
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
      result->b.as_function.children = create_dictionary(
        EFD_DEFAULT_DICTIONARY_SIZE
      );
      break;

    case EFD_NT_PROTO:
      result->b.as_proto.format = NULL;
      result->b.as_proto.input = NULL;
      break;

    case EFD_NT_OBJECT:
      result->b.as_object.format = NULL;
      result->b.as_object.value = NULL;
      break;

    case EFD_NT_STRING:
      result->b.as_string.value = NULL;
      break;

    case EFD_NT_ARRAY_INT:
      result->b.as_int_array.count = 0;
      result->b.as_int_array.values = NULL;
      break;

    case EFD_NT_ARRAY_NUM:
      result->b.as_num_array.count = 0;
      result->b.as_num_array.values = NULL;
      break;

    case EFD_NT_ARRAY_STR:
      result->b.as_str_array.count = 0;
      result->b.as_str_array.values = NULL;
      break;
  }
  result->h.name = copy_string(name);
  result->h.parent = NULL;
  return result;
}

efd_node * construct_efd_obj_node(
  string const * const name,
  string const * const format,
  void * obj
) {
  efd_node *result = create_efd_node(EFD_NT_OBJECT, name);
  efd_push_error_context_with_node(
    s_("...during the creation of object node:"),
    result
  );
  result->b.as_object.format = copy_string(format);
  efd_copy_function copier = efd_lookup_copier(format);
  if (copier == NULL) {
    efd_report_error(
      s_("Error finding copier during attempt to construct object node:"),
      result
    );
  }
  result->b.as_object.value =  copier(obj);
  efd_pop_error_context();
  return result;
}

efd_node * construct_efd_int_node(string const * const name, efd_int_t value) {
  efd_node *result = create_efd_node(EFD_NT_INTEGER, name);
  result->b.as_integer.value = value;
  return result;
}

efd_node * construct_efd_num_node(string const * const name, efd_num_t value) {
  efd_node *result = create_efd_node(EFD_NT_NUMBER, name);
  result->b.as_number.value = value;
  return result;
}

efd_node * construct_efd_str_node(
  string const * const name,
  string const * const value
) {
  efd_node *result = create_efd_node(EFD_NT_STRING, name);
  result->b.as_string.value = copy_string(value);
  return result;
}

efd_node * construct_efd_link_node_to(
  string const * const name,
  efd_node const * const target
) {
  efd_node *result = create_efd_node(EFD_NT_LINK, name);
  result->b.as_link.target = construct_efd_address_of_node(target);
  return result;
}

efd_node * construct_efd_function_node(
  string const * const name,
  efd_node_type returns,
  string const * const function
) {
  efd_node_type type = efd_function_type_that_returns(returns);
  efd_node *result = create_efd_node(type, name);
  result->b.as_function.function = copy_string(function);
  return result;
}

efd_node * copy_efd_node(efd_node const * const src) {
  size_t i;
  dictionary *children;
  efd_node *result;

  efd_push_error_context_with_node(
    s_("...during attempt to copy node:"),
    src
  );
  
  result = create_efd_node(src->h.type, src->h.name);
  switch (src->h.type) {
    default:
    case EFD_NT_INVALID:
#ifdef DEBUG
      efd_report_error(
        s_("Warning: copy_efd_node called on INVALID node:"),
        src
      );
      fprintf(stderr, "\n");
#endif
      break;
    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
      // children
      children = src->b.as_container.children;
      for (i = 0; i < d_get_count(children); ++i) {
        efd_add_child(
          result,
          copy_efd_node((efd_node*) d_get_item(children, i))
        );
      }
      break;

    case EFD_NT_REROUTE:
      // child & target
      result->b.as_reroute.child = copy_efd_node(src->b.as_reroute.child);
      result->b.as_reroute.target = src->b.as_reroute.target;
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      // target
      result->b.as_link.target = copy_efd_address(src->b.as_link.target);
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
      // function
      result->b.as_function.function = copy_string(src->b.as_function.function);
      // children
      children = src->b.as_function.children;
      for (i = 0; i < d_get_count(children); ++i) {
        efd_add_child(
          result,
          copy_efd_node((efd_node*) d_get_item(children, i))
        );
      }
      break;

    case EFD_NT_PROTO:
      // format
      result->b.as_proto.format = copy_string(src->b.as_proto.format);
      // input
      result->b.as_proto.input = copy_efd_node(src->b.as_proto.input);
      break;

    case EFD_NT_OBJECT:
      // format
      result->b.as_object.format = copy_string(src->b.as_object.format);
      // value
      efd_copy_function copier = efd_lookup_copier(src->b.as_object.format);
      if (copier == NULL) {
        efd_report_error(
          s_("Error finding copier during attempt to copy object node:"),
          src
        );
      }
      result->b.as_object.value = copier(src->b.as_object.value);
      break;

    case EFD_NT_INTEGER:
      result->b.as_integer.value = src->b.as_integer.value;
      break;

    case EFD_NT_NUMBER:
      result->b.as_number.value = src->b.as_number.value;
      break;

    case EFD_NT_STRING:
      result->b.as_string.value = copy_string(src->b.as_string.value);
      break;

    case EFD_NT_ARRAY_INT:
      result->b.as_int_array.count = src->b.as_int_array.count;
      result->b.as_int_array.values = (efd_int_t*) malloc(
        result->b.as_int_array.count * sizeof(efd_int_t)
      );
      for (i = 0; i < result->b.as_int_array.count; ++i) {
        result->b.as_int_array.values[i] = src->b.as_int_array.values[i];
      }
      break;

    case EFD_NT_ARRAY_NUM:
      result->b.as_num_array.count = src->b.as_num_array.count;
      result->b.as_num_array.values = (efd_num_t*) malloc(
        result->b.as_num_array.count * sizeof(efd_num_t)
      );
      for (i = 0; i < result->b.as_num_array.count; ++i) {
        result->b.as_num_array.values[i] = src->b.as_num_array.values[i];
      }
      break;

    case EFD_NT_ARRAY_STR:
      result->b.as_str_array.count = src->b.as_str_array.count;
      result->b.as_str_array.values = (string**) malloc(
        result->b.as_str_array.count * sizeof(string*)
      );
      for (i = 0; i < result->b.as_str_array.count; ++i) {
        result->b.as_str_array.values[i] = copy_string(
          src->b.as_str_array.values[i]
        );
      }
      break;
  }
  efd_pop_error_context();
  return result;
}

efd_node * copy_efd_node_as(
  efd_node const * const src,
  string const * const new_name
) {
  efd_node *result = copy_efd_node(src);
  cleanup_string(result->h.name);
  result->h.name = copy_string(new_name);
  return result;
}

CLEANUP_IMPL(efd_node) {
  size_t i;
  efd_destroy_function df;
  efd_push_error_context_with_node(
    s_("...during cleanup of node:"),
    doomed
  );
#ifdef DEBUG
  // recognize double-cleanups:
  if (doomed->h.type >= EFD_NUM_TYPES + 1 || doomed->h.name == NULL) {
    efd_report_error(
      s_("Cleanup targeting already-cleaned-up node:"),
      doomed
    );
    efd_pop_error_context();
    return;
#ifdef DEBUG_TRACE_EFD_CLEANUP
  } else {
    efd_report_error(
      s_("Cleanup of fresh node:"),
      doomed
    );
#endif
  }
#endif
  // Special-case cleanup:
  switch (doomed->h.type) {
    default:
    case EFD_NT_INVALID:
    case EFD_NT_INTEGER:
    case EFD_NT_NUMBER:
    case EFD_NT_GLOBAL_INT:
    case EFD_NT_GLOBAL_NUM:
    case EFD_NT_GLOBAL_STR: // string should be in globals
      // do nothing
      break;

    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
      // Clean up all of our children:
#ifdef DEBUG_TRACE_EFD_CLEANUP
      fprintf(stderr, "Doomed container children cleanup.\n");
#endif
      while (d_get_count(doomed->b.as_container.children) > 0) {
        cleanup_v_efd_node(d_get_item(doomed->b.as_container.children, 0));
      }
      cleanup_dictionary(doomed->b.as_container.children);
#ifdef DEBUG_TRACE_EFD_CLEANUP
      fprintf(stderr, "Doomed container children done.\n");
#endif
      break;

    case EFD_NT_REROUTE:
      // Clean up our child:
#ifdef DEBUG_TRACE_EFD_CLEANUP
      fprintf(stderr, "Doomed reroute child cleanup.\n");
#endif
      cleanup_efd_node(doomed->b.as_reroute.child);
      // don't clean up the target
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      // Clean up our target address:
      if (doomed->b.as_link.target != NULL) {
        cleanup_efd_address(doomed->b.as_link.target);
      }
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
      // Clean up any children:
#ifdef DEBUG_TRACE_EFD_CLEANUP
      fprintf(stderr, "Doomed function children cleanup.\n");
#endif
      while (d_get_count(doomed->b.as_function.children) > 0) {
        cleanup_v_efd_node(d_get_item(doomed->b.as_function.children, 0));
      }
      cleanup_dictionary(doomed->b.as_function.children);
      // Clean up the function name:
      cleanup_string(doomed->b.as_function.function);
      break;

    case EFD_NT_PROTO:
      // Clean up the input node:
      if (doomed->b.as_proto.input != NULL) {
#ifdef DEBUG_TRACE_EFD_CLEANUP
        fprintf(stderr, "Doomed proto input cleanup.\n");
#endif
        cleanup_efd_node(doomed->b.as_proto.input);
      }
      // Clean up the format string *afterwards*:
      cleanup_string(doomed->b.as_proto.format);
      break;

    case EFD_NT_OBJECT:
      // Clean up the object:
      df = efd_lookup_destructor(doomed->b.as_object.format);
      if (df == NULL) {
        efd_report_error(
          s_("Error finding destructor during cleanup of object node:"),
          doomed
        );
      } else {
        df(doomed->b.as_object.value);
      }
      // the destructor must call free if needed
      // Clean up the format string *afterwards*:
      cleanup_string(doomed->b.as_object.format);
      break;

    case EFD_NT_STRING:
      if (doomed->b.as_string.value != NULL) {
        cleanup_string(doomed->b.as_string.value);
      }
      break;

    case EFD_NT_ARRAY_INT:
      if (doomed->b.as_int_array.values != NULL) {
        free(doomed->b.as_int_array.values);
      }
      break;

    case EFD_NT_ARRAY_NUM:
      if (doomed->b.as_num_array.values != NULL) {
        free(doomed->b.as_num_array.values);
      }
      break;

    case EFD_NT_ARRAY_STR:
      if (doomed->b.as_str_array.values != NULL) {
        for (i = 0; i < doomed->b.as_str_array.count; ++i) {
          cleanup_string(doomed->b.as_str_array.values[i]);
        }
        free(doomed->b.as_str_array.values);
      }
      break;
  }
  // Mark the node as corrupt:
  doomed->h.type = EFD_NUM_TYPES + 1;
  // If this node is a child, remove it from its parent's children:
  if (doomed->h.parent != NULL) {
    efd_remove_child(doomed->h.parent, doomed);
  }
  // Clean up the node's name:
  cleanup_string(doomed->h.name);
  doomed->h.name = NULL;
  // Finally free the memory for this node:
  free(doomed);
  efd_pop_error_context();
}

efd_address * create_efd_address(
  efd_address *parent,
  string const * const name
){
  efd_address *result = (efd_address*) malloc(sizeof(efd_address));
  result->name = copy_string(name);
  result->parent = parent;
  result->next = NULL;
  return result;
}

// Private helper which returns an address tail:
efd_address * _construct_efd_node_direct_address(efd_node const * const node) {
  efd_address *result = (efd_address*) malloc(sizeof(efd_address));
  if (node->h.type >= EFD_NUM_TYPES || node->h.type < 0) { // a corrupted node
    result->name = s_("<corrupt>");
    result->parent = NULL;
    return result;
  } else if (node->h.type == EFD_NT_INVALID) {
    result->name = s_("<deleted>");
    result->parent = NULL;
    return result;
  } else if (node->h.name != NULL) {
    result->name = copy_string(node->h.name);
  } else {
    fprintf(
      stderr,
      "ERROR: Attempt to get address of node with no name.\n"
    );
  }
  result->parent = NULL;
  result->next = NULL;
  if (node->h.parent != NULL) {
    result->parent = _construct_efd_node_direct_address(node->h.parent);
    result->parent->next = result;
  }
  return result;
}

efd_address * construct_efd_address_of_node(efd_node const * const node) {
  efd_address *result = _construct_efd_node_direct_address(node);
  while (result->parent != NULL) {
    result = result->parent;
  }
  return result;
}

efd_address * copy_efd_address(efd_address const * const src) {
  efd_address const *n;
  efd_address *rn;
  efd_address *result;
  n = src;
  result = create_efd_address(n->parent, n->name);
  rn = result;
  while (n->next != NULL) {
    rn->next = create_efd_address(rn, n->next->name);
    rn = rn->next;
    n = n->next;
  }
  return result;
}

CLEANUP_IMPL(efd_address) {
  if (doomed->next != NULL) {
    cleanup_efd_address(doomed->next);
  }
  cleanup_string(doomed->name);
  free(doomed);
}

efd_reference* create_efd_reference(
  efd_ref_type type,
  efd_address *addr,
  intptr_t idx
) {
  efd_reference *result = (efd_reference*) malloc(sizeof(efd_reference));
  result->type = type;
  result->addr = copy_efd_address(addr);
  result->idx = idx;
  return result;
}

CLEANUP_IMPL(efd_reference) {
  cleanup_efd_address(doomed->addr);
  free(doomed);
}

efd_bridge* create_efd_bridge(efd_reference *from, efd_reference *to) {
  string *tn;
  if (!efd_ref_types_are_compatible(from->type, to->type)) {
#ifdef DEBUG
    fprintf(
      stderr,
      "ERROR: Incompatible EFD reference types cannot form a bridge:"
    );
    tn = efd_type_name(from->type);
    s_fprintln(stderr, tn);
    cleanup_string(tn);
    tn = efd_type_name(to->type);
    s_fprintln(stderr, tn);
    cleanup_string(tn);
#endif
    return NULL;
  }
  efd_bridge *result = (efd_bridge*) malloc(sizeof(efd_bridge));
  result->from = from;
  result->to = to;
  return result;
}

CLEANUP_IMPL(efd_bridge) {
  cleanup_efd_reference(doomed->from);
  cleanup_efd_reference(doomed->to);
  free(doomed);
}

efd_index * create_efd_index(void) {
  efd_index *result = (efd_index*) malloc(sizeof(efd_index));
  result->unprocessed = create_list();
  result->processed = create_list();
  return result;
}

CLEANUP_IMPL(efd_index) {
  l_foreach(doomed->unprocessed, &cleanup_v_efd_bridge);
  l_foreach(doomed->processed, &cleanup_v_efd_bridge);
  cleanup_list(doomed->unprocessed);
  cleanup_list(doomed->processed);
  free(doomed);
}

efd_value_cache * create_efd_value_cache(void) {
  efd_value_cache *result = (efd_value_cache*) malloc(sizeof(efd_value_cache));
  result->values = create_map(1, EFD_EVAL_MAP_TABLE_SIZE);
  result->active = NULL;
  result->stack = create_map(1, EFD_EVAL_DEP_TABLE_SIZE);
  return result;
}

CLEANUP_IMPL(efd_value_cache) {
  // TODO: Are we sure these nodes aren't children of each other?
#ifdef DEBUG_TRACE_EFD_CLEANUP
  fprintf(stderr, "Doomed value cache cleanup.\n");
#endif
  m_foreach(doomed->values, &cleanup_v_efd_node);
  cleanup_map(doomed->values);
  cleanup_map(doomed->stack);
  free(doomed);
}

efd_generator_state * create_efd_generator_state(
  efd_generator_type type,
  string const * const name,
  void *state
) {
  efd_generator_state *result = (efd_generator_state*) malloc(
    sizeof(efd_generator_state)
  );
  result->type = type;
  result->name = copy_string(name);
  result->index = 0;
  result->state = state;
  result->stash = NULL;
  return result;
}

CLEANUP_IMPL(efd_generator_state) {
  cleanup_string(doomed->name);
  switch(doomed->type) {
    case EFD_GT_INVALID:
    case EFD_GT_CHILDREN: // doesn't copy the parent node
    case EFD_GT_INDICES: // doesn't copy the array node
      // nothing to do
      break;

    case EFD_GT_FUNCTION:
      // functions may use an efd_node as their stash if they want
      if (doomed->stash != NULL) {
#ifdef DEBUG_TRACE_EFD_CLEANUP
        fprintf(stderr, "Doomed FUNCTION generator state stash cleanup.\n");
#endif
        cleanup_v_efd_node(doomed->stash);
      }
      break;

    case EFD_GT_EXTEND_RESTART:
      cleanup_v_efd_generator_state(doomed->state);
      break;

    case EFD_GT_EXTEND_HOLD:
      cleanup_v_efd_generator_state(doomed->state);
      if (doomed->stash != NULL) {
#ifdef DEBUG_TRACE_EFD_CLEANUP
        fprintf(stderr, "Doomed EXT_HOLD generator state stash cleanup.\n");
#endif
        cleanup_v_efd_node(doomed->stash);
      }
      break;

    case EFD_GT_PARALLEL:
      l_foreach((list*) doomed->state, &cleanup_v_efd_generator_state);
      cleanup_list((list*) doomed->state);
      break;
  }
  free(doomed);
}

/*************
 * Functions *
 *************/

void * v_efd__v_i(void *v_node) {
  efd_node *n = (efd_node*) v_node;
  return i_as_p(efd_as_i(n));
}

void * v_efd__v_n(void *v_node) {
  efd_node *n = (efd_node*) v_node;
  return f_as_p(efd_as_n(n));
}

void * v_efd__v_s(void *v_node) {
  efd_node *n = (efd_node*) v_node;
  return (void*) (*efd__s(n));
}

void * v_efd__o(void *v_node) {
  efd_node *n = (efd_node*) v_node;
  return *efd__o(n);
}

void efd_push_error_context(string *context) {
  if (EFD_TRACK_ERROR_CONTEXTS) {
    l_append_element(EFD_ERROR_CONTEXT, (void*) context);
  }
}

void efd_push_error_context_with_node(
  string *message,
  efd_node const * const node
) {
  efd_address *a;
  if (EFD_TRACK_ERROR_CONTEXTS) {
    if (node == NULL) {
      s_append(message, S_NL);
      s_devour(message, s_("<NULL node>"));
    } else {
      a = construct_efd_address_of_node(node);
      s_append(message, S_NL);
      s_devour(message, efd_addr_string(a));
      cleanup_efd_address(a);
    }
    efd_push_error_context(message);
  }
}

void efd_pop_error_context(void) {
  if (EFD_TRACK_ERROR_CONTEXTS) {
    cleanup_string((string*) l_pop_element(EFD_ERROR_CONTEXT));
  }
}

void efd_print_error_context(void) {
  size_t i;
  string *message;
  for (i = 0; i < l_get_length(EFD_ERROR_CONTEXT); ++i) {
    message = (string*) l_get_item(EFD_ERROR_CONTEXT, i);
    s_fprintln(stderr, message);
  }
  fprintf(stderr, "\n");
}

int efd_ref_types_are_compatible(efd_ref_type from, efd_ref_type to) {
  switch (from) {
    default:
    case EFD_RT_INVALID:
    case EFD_RT_NODE:
    case EFD_RT_CHAIN:
      return 0;
    case EFD_RT_GLOBAL_INT:
    case EFD_RT_INT:
    case EFD_RT_INT_ARR_ENTRY:
      return (
        to == EFD_RT_GLOBAL_INT
     || to == EFD_RT_INT
     || to == EFD_RT_INT_ARR_ENTRY
      );
    case EFD_RT_GLOBAL_NUM:
    case EFD_RT_NUM:
    case EFD_RT_NUM_ARR_ENTRY:
      return (
        to == EFD_RT_GLOBAL_NUM
     || to == EFD_RT_NUM
     || to == EFD_RT_NUM_ARR_ENTRY
      );
    case EFD_RT_GLOBAL_STR:
    case EFD_RT_STR:
    case EFD_RT_STR_ARR_ENTRY:
      return (
        to == EFD_RT_GLOBAL_STR
     || to == EFD_RT_STR
     || to == EFD_RT_STR_ARR_ENTRY
      );
    case EFD_RT_OBJ:
      return (
        to == EFD_RT_NODE
     || to == EFD_RT_CHAIN
     || to == EFD_RT_OBJ
     || to == EFD_RT_GLOBAL_INT
     || to == EFD_RT_INT
     || to == EFD_RT_INT_ARR_ENTRY
     || to == EFD_RT_GLOBAL_NUM
     || to == EFD_RT_NUM
     || to == EFD_RT_NUM_ARR_ENTRY
     || to == EFD_RT_GLOBAL_STR
     || to == EFD_RT_STR
     || to == EFD_RT_STR_ARR_ENTRY
      );
  }
  // failsafe
  return 0;
}

void efd_assert_type(efd_node const * const n, efd_node_type t) {
  string *tn;
  efd_push_error_context_with_node(
    s_("...in efd_assert_type for node:"),
    n
  );
  if (n == NULL) {
    fprintf(stderr, "ERROR: Missing EFD node in efd_assert_type!\n");
    exit(EXIT_FAILURE);
  }
#ifndef EFD_NO_TYPECHECKS
  if (n->h.type != t) {
    efd_report_error(
      s_("ERROR: type of node doesn't match required type:"),
      n
    );
    fprintf(stderr, "\n");
    tn = create_string_from_ntchars(" Expected: ");
    s_devour(tn, efd_type_name(t));
    s_devour(tn, create_string_from_ntchars("\n Got: "));
    s_devour(tn, efd_type_name(n->h.type));
    s_fprintln(stderr, tn);
    cleanup_string(tn);
    exit(EXIT_FAILURE);
  }
#endif // EFD_NO_TYPECHECKS
  efd_pop_error_context();
}

void efd_assert_return_type(efd_node const * const n, efd_node_type t) {
#ifndef EFD_NO_TYPECHECKS
  efd_node_type rt;
  efd_push_error_context_with_node(
    s_("...in efd_assert_return_type for node:"),
    n
  );
  rt = efd_return_type_of(n);
  if (rt != t) {
    if (rt >= 0
     && rt < EFD_NUM_TYPES
     && t >= 0
     && t < EFD_NUM_TYPES
    ) {
      efd_report_error(
        s_sprintf(
          "ERROR: node has return type '%s' rather than the required '%s':",
          EFD_NT_NAMES[rt],
          EFD_NT_NAMES[t]
        ),
        n
      );
    } else {
      efd_report_error(
        s_sprintf(
          "ERROR: node has return type (%d) rather than the required (%d):",
          rt,
          t
        ),
        n
      );
    }
    exit(EXIT_FAILURE);
  }
  efd_pop_error_context();
#else
  return;
#endif // EFD_NO_TYPECHECKS
}

void efd_assert_object_format(
  efd_node const * const n,
  string const * const fmt
) {
#ifndef EFD_NO_TYPECHECKS
  efd_push_error_context_with_node(
    s_("...in efd_assert_object_format for node:"),
    n
  );
  efd_assert_type(n, EFD_NT_OBJECT);
  if (!efd_format_is(n, fmt)) {
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    efd_report_error(
      s_sprintf(
        "ERROR: object node has format '%U' rather than '%U' as required:",
        s_raw(*efd__o_fmt(n)),
        s_raw(fmt)
      ),
      n
    );
#pragma GCC diagnostic warning "-Wdiscarded-qualifiers"
    exit(EXIT_FAILURE);
  }
  efd_pop_error_context();
#else
  return;
#endif // EFD_NO_TYPECHECKS
}

void efd_v_assert_object_format(void *v_node, void *v_fmt) {
  efd_assert_object_format((efd_node*) v_node, (string*) v_fmt);
}

int efd_is_type(efd_node const * const n, efd_node_type t) {
  if (n == NULL) {
    return 0;
  }
#ifndef EFD_NO_TYPECHECKS
  return (n->h.type == t);
#else // EFD_NO_TYPECHECKS
  return 1;
#endif // EFD_NO_TYPECHECKS
}

int efd_format_is(efd_node const * const n, string const * const fmt) {
  if (efd_is_type(n, EFD_NT_PROTO)) {
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    return s_equals(*efd__p_fmt(n), fmt);
#pragma GCC diagnostic warning "-Wdiscarded-qualifiers"
  } else if (efd_is_type(n, EFD_NT_OBJECT)) {
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    return s_equals(*efd__o_fmt(n), fmt);
#pragma GCC diagnostic warning "-Wdiscarded-qualifiers"
  }
  // failsafe
  return 0;
}

void efd_rename(efd_node * node, string const * const new_name) {
  cleanup_string(node->h.name);
  node->h.name = copy_string(new_name);
}

string * efd_type_name(efd_node_type t) {
  if (t >= 0 && t < EFD_NUM_TYPES) {
    return create_string_from_ntchars(EFD_NT_NAMES[t]);
  } else {
    return s_sprintf("<type %d>", t);
  }
}

string * efd_type_abbr(efd_node_type t) {
  if (t >= 0 && t < EFD_NUM_TYPES) {
    return create_string_from_ntchars(EFD_NT_ABBRS[t]);
  } else {
    return s_sprintf("{%d}", t);
  }
}

string * efd_addr_string(efd_address *a) {
  string *result = create_empty_string();
  while (a->next != NULL) {
    s_append(result, a->name);
    s_append(result, EFD_ADDR_SEP_STR);
    a = a->next;
  }
  s_append(result, a->name);
  return result;
}

string * efd_build_fqn(efd_node const * const n) {
  efd_address *a = construct_efd_address_of_node(n);
  string *result = efd_addr_string(a);
  cleanup_efd_address(a);
  return result;
}

string * efd_repr(efd_node const * const n) {
  dictionary *children;
  efd_node *child;
  string *fqn, *scopes, *contents, *value;
  string *result;
  intptr_t ncount, i;
  size_t count;

  SSTR(s_lsb, "[[", 2);
  SSTR(s_rsb, "]]", 2);
  SSTR(s_slsb, "[", 1);
  SSTR(s_srsb, "]", 1);
  SSTR(s_lab, "<<", 2);
  SSTR(s_rab, ">>", 2);
  SSTR(s_rcb, "{", 1);
  SSTR(s_lcb, "}", 1);
  SSTR(s_dots, "...", 3);
  SSTR(S_PERCENT, "%", 1);
  SSTR(s_caret, "->", 2);
  SSTR(s_reroute, "*>", 2);
  SSTR(s_quote, "\"", 1);

  if (n->h.type >= EFD_NUM_TYPES || n->h.type < 0) { // a corrupted node
    return s_sprintf("<corrupt node [%d]>", n->h.type);
  }

  fqn = efd_build_fqn(n);

  ncount = efd_normal_child_count(n);

  if (ncount >= 0) {
    children = efd_children_dict(n);
  } else {
    children = NULL;
  }

  scopes = create_empty_string();
  if (ncount >= 0 && ncount != d_get_count(children)) {
    // must be a scope in there
    for (i = 0; i < d_get_count(children); ++i) {
      child = d_get_item(children, i);
      if (child->h.type == EFD_NT_SCOPE) {
        s_append(scopes, s_rcb);
        s_append(scopes, child->h.name);
        s_append(scopes, s_lcb);
      }
      s_append(scopes, S_SPACE);
    }
  } // otherwise it remains empty
  

  contents = create_empty_string();
  for (i = 0; i < ncount && i < 3; ++i) {
    child = efd_nth(n, i);
    if (efd_is_link_node(child)) {
      s_append(contents, s_lab);
      s_devour(
        contents,
        efd_type_abbr(child->h.type)
      );
      s_append(contents, S_SPACE);
      s_append(contents, child->h.name);
      s_append(contents, s_rab);
    } else {
      s_append(contents, s_lsb);
      s_devour(
        contents,
        efd_type_abbr(child->h.type)
      );
      s_append(contents, S_SPACE);
      s_append(contents, child->h.name);
      s_append(contents, s_rsb);
    }
    if (i < ncount - 1) {
      s_append(contents, S_SPACE);
    }
  }
  if (ncount >= 0 && i < ncount) {
    s_append(contents, s_dots);
  }
  if (ncount > 0) {
    // trailing space if non-empty
    s_append(contents, S_SPACE);
  }

  value = create_empty_string();
  switch (n->h.type) {
    default:
      // leave 'value' empty
      break;

    case EFD_NT_REROUTE:
      s_devour(value, efd_repr(n->b.as_reroute.child));
      s_append(value, S_SPACE);
      s_append(value, s_reroute);
      s_devour(value, efd_repr(n->b.as_reroute.target));
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      s_append(value, s_caret);
      s_devour(value, efd_addr_string(n->b.as_link.target));
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
      s_append(value, S_PERCENT);
      s_append(value, n->b.as_function.function);
      break;

    case EFD_NT_PROTO:
      s_append(value, S_COLON);
      s_append(value, S_TILDE);
      s_append(value, n->b.as_proto.format);
      break;

    case EFD_NT_OBJECT:
      s_append(value, S_COLON);
      s_append(value, n->b.as_proto.format);
      break;

    case EFD_NT_INTEGER:
      s_devour(value, s_sprintf("%ld", n->b.as_integer.value));
      break;

    case EFD_NT_NUMBER:
      s_devour(value, s_sprintf("%0.3f", n->b.as_number.value));
      break;

    case EFD_NT_STRING:
      s_append(value, s_quote);
      s_append(value, n->b.as_string.value);
      s_append(value, s_quote);
      break;

    case EFD_NT_ARRAY_INT:
      s_append(value, s_slsb);
      count = n->b.as_int_array.count;
      for (i = 0; i < count && i < 3; ++i) {
        s_devour(value, s_sprintf("%ld", n->b.as_int_array.values[i]));
        if (i < count - 1) {
          s_append(value, S_COMMA);
          s_append(value, S_SPACE);
        }
      }
      if (i < count) {
        s_append(value, s_dots);
      }
      s_append(value, s_srsb);
      break;

    case EFD_NT_ARRAY_NUM:
      s_append(value, s_slsb);
      count = n->b.as_num_array.count;
      for (i = 0; i < count && i < 3; ++i) {
        s_devour(value, s_sprintf("%0.3f", n->b.as_num_array.values[i]));
        if (i < count - 1) {
          s_append(value, S_COMMA);
          s_append(value, S_SPACE);
        }
      }
      if (i < count) {
        s_append(value, s_dots);
      }
      s_append(value, s_srsb);
      break;

    case EFD_NT_ARRAY_STR:
      s_append(value, s_slsb);
      count = n->b.as_str_array.count;
      for (i = 0; i < count && i < 3; ++i) {
        s_append(value, s_quote);
        s_append(value, n->b.as_str_array.values[i]);
        s_append(value, s_quote);
        if (i < count - 1) {
          s_append(value, S_COMMA);
          s_append(value, S_SPACE);
        }
      }
      if (i < count) {
        s_append(value, s_dots);
      }
      s_append(value, s_srsb);
      break;
  }

  // Finally, assembly all of our strings into one big one:
  result = create_empty_string();
  if (efd_is_link_node(n)) {
    s_append(result, s_lab);
  } else {
    s_append(result, s_lsb);
  }

  s_devour(result, efd_type_abbr(n->h.type));
  s_append(result, S_SPACE);

  s_devour(result, fqn);
  s_append(result, S_SPACE);

  s_devour(result, scopes);
  // scopes comes with a trailing space when needed

  s_devour(result, value);

  s_devour(result, contents);
  // contents also contains a trailing space when needed

  if (efd_is_link_node(n)) {
    s_append(result, s_rab);
  } else {
    s_append(result, s_rsb);
  }

  return result;
}

string * _efd_full_repr(efd_node const * const n, size_t indent) {
  dictionary *children;
  efd_node *child;
  string *fqn, *contents, *value;
  string *result;
  size_t i, count;

  SSTR(s_lsb, "[[", 2);
  SSTR(s_rsb, "]]", 2);
  SSTR(s_slsb, "[", 1);
  SSTR(s_srsb, "]", 1);
  SSTR(s_lab, "<<", 2);
  SSTR(s_rab, ">>", 2);
  SSTR(S_PERCENT, "%", 1);
  SSTR(s_caret, "->", 2);
  SSTR(s_reroute, "*>", 2);
  SSTR(s_quote, "\"", 1);

  string *s_nl = s_("\n");
  string *s_outer_nl = s_("\n");
  string *ind = create_empty_string();
  for (i = 0; i < indent; ++i) {
    s_append(ind, S_SPACE);
    if (i < indent - 2) {
      s_append(s_outer_nl, S_SPACE);
    }
  }
  s_append(s_nl, ind);

  if (n->h.type >= EFD_NUM_TYPES || n->h.type < 0) { // a corrupted node
    return s_sprintf("<corrupt node [%d]>", n->h.type);
  }

  fqn = efd_build_fqn(n);

  contents = create_empty_string();
  if (efd_is_container_node(n)) {
    children = efd_children_dict(n);
    s_append(contents, s_nl);
    for (i = 0; i < d_get_count(children); ++i) {
      child = d_get_item(children, i);
      s_devour(contents, _efd_full_repr(child, indent + 2));
      if (i < d_get_count(children) - 1) {
        s_append(contents, s_nl);
      } else {
        s_append(contents, s_outer_nl);
      }
    }
  }

  value = create_empty_string();
  switch (n->h.type) {
    default:
      // leave 'value' empty
      break;

    case EFD_NT_REROUTE:
      s_append(value, efd_full_repr(n->b.as_reroute.child));
      s_append(value, S_SPACE);
      s_append(value, s_reroute);
      s_devour(value, efd_full_repr(n->b.as_reroute.target));
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      s_append(value, s_caret);
      s_devour(value, efd_addr_string(n->b.as_link.target));
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
      s_append(value, S_PERCENT);
      s_append(value, n->b.as_function.function);
      break;

    case EFD_NT_PROTO:
      s_append(value, S_COLON);
      s_append(value, S_TILDE);
      s_append(value, n->b.as_proto.format);
      s_append(value, s_nl);
      s_devour(value, efd_full_repr(n->b.as_proto.input));
      s_append(value, s_nl);
      break;

    case EFD_NT_OBJECT:
      s_append(value, S_COLON);
      s_append(value, n->b.as_proto.format);
      break;

    case EFD_NT_INTEGER:
      s_devour(value, s_sprintf("%ld", n->b.as_integer.value));
      break;

    case EFD_NT_NUMBER:
      s_devour(value, s_sprintf("%0.3f", n->b.as_number.value));
      break;

    case EFD_NT_STRING:
      s_append(value, s_quote);
      s_append(value, n->b.as_string.value);
      s_append(value, s_quote);
      break;

    case EFD_NT_ARRAY_INT:
      s_append(value, s_slsb);
      count = n->b.as_int_array.count;
      for (i = 0; i < count; ++i) {
        s_devour(value, s_sprintf("%ld", n->b.as_int_array.values[i]));
        if (i < count - 1) {
          s_append(value, S_COMMA);
          s_append(value, S_SPACE);
        }
      }
      s_append(value, s_srsb);
      break;

    case EFD_NT_ARRAY_NUM:
      s_append(value, s_slsb);
      count = n->b.as_num_array.count;
      for (i = 0; i < count; ++i) {
        s_devour(value, s_sprintf("%0.3f", n->b.as_num_array.values[i]));
        if (i < count - 1) {
          s_append(value, S_COMMA);
          s_append(value, S_SPACE);
        }
      }
      s_append(value, s_srsb);
      break;

    case EFD_NT_ARRAY_STR:
      s_append(value, s_slsb);
      count = n->b.as_str_array.count;
      for (i = 0; i < count; ++i) {
        s_append(value, s_quote);
        s_append(value, n->b.as_str_array.values[i]);
        s_append(value, s_quote);
        if (i < count - 1) {
          s_append(value, S_COMMA);
          s_append(value, S_SPACE);
        }
      }
      s_append(value, s_srsb);
      break;
  }

  // Finally, assembly all of our strings into one big one:
  result = create_empty_string();
  if (efd_is_link_node(n)) {
    s_append(result, s_lab);
  } else {
    s_append(result, s_lsb);
  }

  s_devour(result, efd_type_abbr(n->h.type));
  s_append(result, S_SPACE);

  s_devour(result, fqn);
  s_append(result, S_SPACE);

  s_devour(result, value);

  s_devour(result, contents);
  // contents contains a trailing space when needed

  if (efd_is_link_node(n)) {
    s_append(result, s_rab);
  } else {
    s_append(result, s_rsb);
  }

  return result;
}

string * efd_full_repr(efd_node const * const n) {
  return _efd_full_repr(n, 2);
}

void efd_report_error(string *message, efd_node const * const n) {
  efd_print_error_context();
  string *repr = efd_repr(n);
  s_fprintln(stderr, message);
  fprintf(stderr, "\n");
  s_fprintln(stderr, repr);
  fprintf(stderr, "\n");
  cleanup_string(message);
}

void efd_report_error_full(string *message, efd_node const * const n) {
  efd_print_error_context();
  string *repr = efd_full_repr(n);
  s_fprintln(stderr, message);
  s_fprintln(stderr, repr);
  cleanup_string(message);
}

string *_efd_trace_link(efd_node const * const n, string * sofar) {
  string *fqn, *ltype;
  efd_node *linked;
  SSTR(s_arrow, "\n  -> ", 6);
  SSTR(s_broken, "\n  -X> ", 7);
  SSTR(s_lab, "<", 1);
  SSTR(s_rab, ">", 1);

  if (n == NULL) {
    // We should never hit this case while recursing---only from an initial
    // NULL argument.
    return create_string_from_ntchars("<NULL>");
  }

  fqn = efd_build_fqn(n);

  ltype = create_empty_string();
  s_append(ltype, s_lab);
  s_devour(ltype, efd_type_abbr(n->h.type));
  s_append(ltype, s_rab);

  s_append(sofar, s_arrow);
  s_devour(sofar, ltype);
  s_append(sofar, S_SPACE);
  s_devour(sofar, fqn);

  switch (n->h.type) {
    default:
      return sofar;

    case EFD_NT_REROUTE:
      linked = n->b.as_reroute.child;
      break;

    case EFD_NT_LINK:
      linked = efd(EFD_ROOT, n->b.as_link.target);
      break;

    case EFD_NT_LOCAL_LINK:
      linked = efd(n->h.parent, n->b.as_link.target);
      break;

    case EFD_NT_VARIABLE:
      linked = efd_resolve_variable(n);
      break;
  }
  if (linked == NULL) {
    s_append(sofar, s_broken);
    s_devour(sofar, efd_addr_string(n->b.as_link.target));
    if (n->h.type == EFD_NT_VARIABLE) {
      s_append(sofar, S_NL);
      s_devour(sofar, efd_variable_search_trace(n));
    }
    return sofar;
  } else {
    return _efd_trace_link(linked, sofar);
  }
}

string *efd_trace_link(efd_node const * const n) {
  return _efd_trace_link(n, s_("Trace:"));
}

void efd_report_broken_link(string *message, efd_node const * const n) {
  string *repr = efd_repr(n);
  string *trace = efd_trace_link(n);
  fprintf(stderr, "EFD link node has an invalid target:\n");
  s_fprintln(stderr, repr);
  s_fprintln(stderr, trace);
  s_fprintln(stderr, message);
  cleanup_string(message);
}

void efd_report_eval_error(
  efd_node const * const orig,
  efd_node const * const evald,
  string *message
) {
  string *orepr = efd_repr(orig);
  string *erepr = efd_full_repr(evald);
  SSTR(s_broken, "  -X>", 5);
  fprintf(stderr, "Error during EFD node evaluation:\n");
  s_fprintln(stderr, orepr);
  s_fprintln(stderr, s_broken);
  s_fprintln(stderr, erepr);
  s_fprintln(stderr, message);
  cleanup_string(message);
}

dictionary * efd_children_dict(efd_node const * const n) {
  switch (n->h.type) {
    default:
#ifdef DEBUG
      efd_report_error(
        s_("Warning: Attempt to get children of non-container node:"),
        n
      );
      fprintf(stderr, "\n");
#endif
      return NULL;
    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
      return n->b.as_container.children;
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
      return n->b.as_function.children;
  }
}

// private helper for both efd_equals and efd_equivalent
int _efd_cmp(
  efd_node const * const cmp,
  efd_node const * const agn,
  int strict
) {
  intptr_t i, count;
  dictionary *cmp_ch, *agn_ch;
  efd_node *cmp_t, *agn_t;
  efd_index *empty_index;

  string *ctx = s_("...in efd_cmp for nodes:\n");
  s_devour(ctx, efd_repr(cmp));
  s_devour(ctx, s_("\n ...and:\n"));
  s_devour(ctx, efd_repr(agn));
  efd_push_error_context(ctx);

  if (cmp == NULL && agn == NULL) {
    efd_pop_error_context();
    return 1;
  } else if (cmp == NULL || agn == NULL) {
    efd_pop_error_context();
    return 0;
  }
  if (
    cmp->h.type != agn->h.type
 || (strict && !s_equals(cmp->h.name, agn->h.name))
  ) {
    efd_pop_error_context();
    return 0;
  }
  switch (cmp->h.type) {
    default:
#ifdef DEBUG
      efd_report_error(
        s_("ERROR: Invalid node type in efd comparison:"),
        cmp
      );
      fprintf(stderr, "\n");
#endif
      efd_pop_error_context();
      return 0;

    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
      // children are compared below
      break;

    case EFD_NT_REROUTE:
      if (
         !_efd_cmp(cmp->b.as_reroute.child, agn->b.as_reroute.child, strict)
      || (cmp->b.as_reroute.target != agn->b.as_reroute.target)
      ) {
        efd_pop_error_context();
        return 0;
      }
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      if (!_efd_cmp(efd_concrete(cmp), efd_concrete(agn), strict)) {
        efd_pop_error_context();
        return 0;
      }
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
      if (!s_equals(cmp->b.as_function.function, agn->b.as_function.function)) {
        efd_pop_error_context();
        return 0;
      }
      // children are compared below
      break;

    case EFD_NT_PROTO:
      if (
        !s_equals(cmp->b.as_proto.format, agn->b.as_proto.format)
     || !_efd_cmp(cmp->b.as_proto.input, agn->b.as_proto.input, strict)
      ) {
        efd_pop_error_context();
        return 0;
      }
      break;

    case EFD_NT_OBJECT:
      // pack copies of each node and compare them as protos:
      cmp_t = copy_efd_node(cmp);
      agn_t = copy_efd_node(agn);
      empty_index = create_efd_index();
      efd_pack_node(cmp_t, empty_index);
      efd_pack_node(agn_t, empty_index);
      if (!_efd_cmp(cmp_t, agn_t, strict)) {
#ifdef DEBUG_TRACE_EFD_CLEANUP
        fprintf(stderr, "Object CMP fail cleanup.\n");
#endif
        cleanup_efd_node(cmp_t);
        cleanup_efd_node(agn_t);
        efd_pop_error_context();
        return 0;
      }
      cleanup_efd_index(empty_index);
#ifdef DEBUG_TRACE_EFD_CLEANUP
      fprintf(stderr, "Object CMP match cleanup.\n");
#endif
      cleanup_efd_node(cmp_t);
      cleanup_efd_node(agn_t);
      break;

    case EFD_NT_INTEGER:
      if (cmp->b.as_integer.value != agn->b.as_integer.value) {
        efd_pop_error_context();
        return 0;
      }
      break;

    case EFD_NT_NUMBER:
      if (cmp->b.as_number.value != agn->b.as_number.value) {
        efd_pop_error_context();
        return 0;
      }
      break;

    case EFD_NT_STRING:
      if (!s_equals(cmp->b.as_string.value, agn->b.as_string.value)) {
        efd_pop_error_context();
        return 0;
      }
      break;

    case EFD_NT_ARRAY_INT:
      if (cmp->b.as_int_array.count != agn->b.as_int_array.count) {
        efd_pop_error_context();
        return 0;
      }
      for (i = 0; i < cmp->b.as_int_array.count; ++i) {
        if (cmp->b.as_int_array.values[i] != agn->b.as_int_array.values[i]) {
          efd_pop_error_context();
          return 0;
        }
      }
      break;

    case EFD_NT_ARRAY_NUM:
      if (cmp->b.as_num_array.count != agn->b.as_num_array.count) {
        efd_pop_error_context();
        return 0;
      }
      for (i = 0; i < cmp->b.as_num_array.count; ++i) {
        if (cmp->b.as_num_array.values[i] != agn->b.as_num_array.values[i]) {
          efd_pop_error_context();
          return 0;
        }
      }
      break;

    case EFD_NT_ARRAY_STR:
      if (cmp->b.as_str_array.count != agn->b.as_str_array.count) {
        efd_pop_error_context();
        return 0;
      }
      for (i = 0; i < cmp->b.as_str_array.count; ++i) {
        if (
          !s_equals(
            cmp->b.as_str_array.values[i],
            agn->b.as_str_array.values[i]
          )
        ) {
          efd_pop_error_context();
          return 0;
        }
      }
      break;
  }
  // if the nodes are containers, compare children:
  if (efd_is_container_node(cmp)) {
    if (strict) {
      cmp_ch = cmp->b.as_container.children;
      agn_ch = agn->b.as_container.children;
      if (d_get_count(cmp_ch) != d_get_count(agn_ch)) {
        efd_pop_error_context();
        return 0;
      }
      for (i = 0; i < d_get_count(cmp_ch); ++i) {
        if (!_efd_cmp(d_get_item(cmp_ch, i), d_get_item(agn_ch, i), strict)) {
          efd_pop_error_context();
          return 0;
        }
      }
    } else {
      if (efd_normal_child_count(cmp) != efd_normal_child_count(agn)) {
        efd_pop_error_context();
        return 0;
      }
      count = efd_normal_child_count(cmp);
      for (i = 0; i < count; ++i) {
        if (!_efd_cmp(efd_nth(cmp, i), efd_nth(agn, i), strict)) {
          efd_pop_error_context();
          return 0;
        }
      }
    }
  }
  efd_pop_error_context();
  return 1;
}

int efd_equals(efd_node const * const cmp, efd_node const * const agn) {
  return _efd_cmp(cmp, agn, 1);
}

int efd_equivalent(efd_node const * const cmp, efd_node const * const agn) {
  return _efd_cmp(cmp, agn, 0);
}

void efd_add_child(efd_node *n, efd_node *child) {
  efd_push_error_context_with_node(s_("...while adding child to node:"), n);
#ifdef DEBUG
  if (!efd_is_container_node(n) && n->h.type != EFD_NT_REROUTE) {
    efd_report_error(
      s_("ERROR: Can't add child to non-container node."),
      n
    );
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
  }
  if (child->h.parent != NULL && child->h.parent != n) {
    efd_report_error(
      s_("(while adding to parent:)"),
      n
    );
    efd_report_error(
      s_("Warning: efd_add_child: child already had a parent."),
      child
    );
    fprintf(stderr, "\n");
  }
#endif
  if (n->h.type == EFD_NT_REROUTE) {
    if (n->b.as_reroute.child != NULL) {
      efd_report_error(
        s_("ERROR: Can't add child to REROUTE node which already has one:"),
        n
      );
      exit(EXIT_FAILURE);
    }
    n->b.as_reroute.child = child;
    child->h.parent = n;
  } else {
    d_add_value_s(efd_children_dict(n), child->h.name, (void*) child);
    child->h.parent = n;
  }
  efd_pop_error_context();
}

void efd_prepend_child(efd_node *n, efd_node *child) {
  efd_push_error_context_with_node(s_("...while prepending child to node:"), n);
#ifdef DEBUG
  if (!efd_is_container_node(n) && n->h.type != EFD_NT_REROUTE) {
    efd_report_error(
      s_("ERROR: Can't add child to non-container node:"),
      n
    );
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
  }
  if (child->h.parent != NULL) {
    efd_report_error(
      s_("(while adding to parent:)"),
      n
    );
    efd_report_error(
      s_("Warning: efd_prepend_child: child already had a parent:"),
      child
    );
    fprintf(stderr, "\n");
  }
#endif
  if (n->h.type == EFD_NT_REROUTE) {
    if (n->b.as_reroute.child != NULL) {
      efd_report_error(
        s_("ERROR: Can't add child to REROUTE node which already has one:"),
        n
      );
      exit(EXIT_FAILURE);
    }
    n->b.as_reroute.child = child;
    child->h.parent = n;
  } else {
    d_prepend_value_s(efd_children_dict(n), child->h.name, (void*) child);
    child->h.parent = n;
  }
  efd_pop_error_context();
}

void efd_remove_child(efd_node *n, efd_node *child) {
  efd_push_error_context_with_node(s_("...while removing child from node:"), n);
#ifdef DEBUG
  efd_node *r;
  if (!efd_is_container_node(n) && n->h.type != EFD_NT_REROUTE) {
    efd_report_error(
      s_("ERROR: Can't remove child from non-container node:"),
      n
    );
    exit(EXIT_FAILURE);
  }
  if (child->h.parent != n) {
    efd_report_error(
      s_("(while removing from parent:)"),
      n
    );
    efd_report_error(
      s_("Warning: child not descended from parent."),
      child
    );
  }
  if (n->h.type == EFD_NT_REROUTE) {
    r = n->b.as_reroute.child;
    n->b.as_reroute.child = NULL;
  } else {
    r = (efd_node*) d_remove_value(efd_children_dict(n), (void*) child);
  }
  if (r == NULL || r != child) {
    efd_report_error(
      s_("(while removing from parent:)"),
      n
    );
    efd_report_error(
      s_("Warning: child wasn't present among parent's children."),
      child
    );
    exit(EXIT_FAILURE);
  }
#else
  if (n->h.type == EFD_NT_REROUTE) {
    n->b.as_reroute.child = NULL;
  } else {
    d_remove_value(efd_children_dict(n), (void*) child);
  }
#endif
  child->h.parent = NULL;
  efd_pop_error_context();
}

void efd_append_address(efd_address *a, string const * const name) {
  efd_address *new = create_efd_address(NULL, name);
  efd_extend_address(a, new);
}

void efd_extend_address(efd_address *a, efd_address *e) {
  efd_address *n = a;
  while (n->next != NULL) {
    n = n->next;
  }
#ifdef DEBUG
  if (e->parent != NULL) {
    fprintf(
      stderr,
      "ERROR: Tried to extend using an address that already had a parent!"
    );
    exit(EXIT_FAILURE);
  }
#endif
  e->parent = n;
  n->next = e;
}

void efd_push_address(efd_address *a, string const * const name) {
  efd_address *tail;
  tail = a;
  while (tail->next != NULL) {
    tail = tail->next;
  }
  tail->next = create_efd_address(tail, name);
}

efd_address* efd_pop_address(efd_address *a) {
  efd_address *n, *result;
  if (a->next == NULL) {
#ifdef DEBUG
    char *enc = s_encode_nt(a->name);
    fprintf(stderr, "Warning: Tried to pop topmost address '%s'!\n", enc);
    free(enc);
#endif
    return a;
  }
  n = a;
  while (n->next->next != NULL) {
    n = n->next;
  }
  result = n->next;
  n->next = NULL;
  return result;
}

efd_node * efd_create_shadow_clone(efd_node const * const original) {
  efd_node *result = create_efd_node(EFD_NT_REROUTE, original->h.name);
  result->b.as_reroute.child = copy_efd_node(original);
  result->b.as_reroute.child->h.parent = result;
  result->b.as_reroute.target = original->h.parent;
  return result;
}

efd_node * efd_create_reroute(
  efd_node const * const shadow_parent,
  efd_node const * const original
) {
  efd_node *result = create_efd_node(EFD_NT_REROUTE, original->h.name);
  result->b.as_reroute.child = copy_efd_node(original);
  result->b.as_reroute.child->h.parent = result;
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
  result->b.as_reroute.target = shadow_parent;
#pragma GCC diagnostic warning "-Wdiscarded-qualifiers"
  return result;
}

// Private helper:
int _match_efd_name(void *v_child, void *v_key) {
  efd_node *child = (efd_node*) v_child;
  string *key = (string*) v_key;
  return s_equals(child->h.name, key);
}

efd_node * efd_find_child(
  efd_node const * const parent,
  string const * const name
) {
  efd_node *result;
  efd_push_error_context_with_node(
    s_("...during efd_find_child in parent:"),
    parent
  );
  if (!efd_is_container_node(parent)) {
#ifdef DEBUG
    efd_report_error(
      s_("Warning: efd_find_child on non-container node:"),
      parent
    );
    fprintf(stderr, "Key was:\n  ");
    s_fprintln(stderr, name);
    fprintf(stderr, "\n");
#endif
    efd_pop_error_context();
    return NULL;
  }
  result = (efd_node*) d_get_value_s(efd_children_dict(parent), name);
  efd_pop_error_context();
  return result;
}

list * efd_find_all_children(
  efd_node const * const parent,
  string const * const name
) {
  list *result;
  efd_push_error_context_with_node(
    s_("...during efd_find_all_children in parent:"),
    parent
  );
  if (!efd_is_container_node(parent)) {
#ifdef DEBUG
    efd_report_error(
      s_("Warning: efd_find_all_children on non-container node:"),
      parent
    );
    fprintf(stderr, "Key was:\n  ");
    s_fprintln(stderr, name);
    fprintf(stderr, "\n");
#endif
    efd_pop_error_context();
    return NULL;
  }
  result = d_get_all_s(efd_children_dict(parent), name);
  efd_pop_error_context();
  return result;
}

efd_node * efd_find_variable_in(
  efd_node const * const node,
  efd_address const * const target
) {
  dictionary *children;
  efd_node *child, *result;
  size_t i;

  efd_push_error_context_with_node(
    s_("...during efd_find_variable_in node:"),
    node
  );

  if (!efd_is_container_node(node)) {
#ifdef DEBUG
    efd_report_error(
      s_("Warning: efd_find_variable_in on non-container node:"),
      node
    );
    fprintf(stderr, "Key was:\n  ");
    s_fprintln(stderr, target->name);
    fprintf(stderr, "\n");
#endif
    efd_pop_error_context();
    return NULL;
  }
  children = efd_children_dict(node);
  result = NULL;
  for (i = 0; i < d_get_count(children); ++i) {
    child = (efd_node*) d_get_item(children, i);
    if (child->h.type == EFD_NT_SCOPE) {
      result = efd(child, target);
      if (result != NULL) {
        break;
      }
    }
  }
  if (result == NULL && node->h.type == EFD_NT_SCOPE && node->h.parent == NULL){
    result = efd(node, target);
  }
  efd_pop_error_context();
  return result;
}

efd_node * efd_resolve_variable(efd_node const * const var) {
  efd_address *target;
  efd_node *result, *scope;

  efd_push_error_context_with_node(
    s_("...during efd_resolve_variable for variable:"),
    var
  );

  efd_assert_type(var, EFD_NT_VARIABLE);
  target = var->b.as_link.target;

  scope = var->h.parent;

  result = NULL;
  while (scope != NULL && (result == NULL || result == var)) {
    if (scope->h.type == EFD_NT_REROUTE) {
      scope = scope->b.as_reroute.target;
    } else {
      result = efd_find_variable_in(scope, target);
      scope = scope->h.parent;
    }
  }
  efd_pop_error_context();
  return result;
}

string * efd_variable_search_trace(efd_node const * const var) {
  efd_address *target;
  efd_node *result, *scope;
  string *trace;
  SSTR(s_nl, "\n?-", 3);

  efd_assert_type(var, EFD_NT_VARIABLE);
  target = var->b.as_link.target;
  trace = s_("Var trace:");

  scope = var->h.parent;

  result = NULL;
  while (scope != NULL && (result == NULL || result == var)) {
    s_append(trace, s_nl);
    s_devour(trace, efd_repr(scope));
    if (scope->h.type == EFD_NT_REROUTE) {
      scope = scope->b.as_reroute.target;
    } else {
      result = efd_find_variable_in(scope, target);
      scope = scope->h.parent;
    }
  }
  if (result != NULL) {
    s_append(trace, s_nl);
    s_devour(trace, s_("*success*"));
  } else {
    s_append(trace, s_nl);
    s_devour(trace, s_("*failure*"));
  }
  return trace;
}

efd_node * efd_concrete(efd_node const * const base) {
  efd_push_error_context_with_node(
    s_("...during efd_concrete for node:"),
    base
  );
  if (base == NULL) {
    efd_pop_error_context();
    return NULL;
  }
  efd_node *linked;
  switch (base->h.type) {
    default:
      efd_pop_error_context();
      return (efd_node*) base;
    case EFD_NT_REROUTE:
      linked = base->b.as_reroute.child;
      break;
    case EFD_NT_LINK:
      linked = efd(EFD_ROOT, base->b.as_link.target);
      break;
    case EFD_NT_LOCAL_LINK:
      linked = efd(base->h.parent, base->b.as_link.target);
      break;
    case EFD_NT_VARIABLE:
      linked = efd_resolve_variable(base);
      break;
  }
#ifdef DEBUG
  if (linked == base) {
    efd_report_error(
      s_("ERROR: Link which refers to itself!"),
      base
    );
    exit(EXIT_FAILURE);
  }
#endif
  efd_pop_error_context();
  return efd_concrete(linked);
}

intptr_t efd_normal_child_count(efd_node const * const node) {
  size_t i;
  intptr_t result;
  dictionary *children;
  efd_node *child;

  if (!efd_is_container_node(node)) {
    return -1;
  }

  result = 0;
  children = efd_children_dict(node);
  for (i = 0; i < d_get_count(children); ++i) {
    child = d_get_item(children, i);
    if (child->h.type != EFD_NT_SCOPE) {
      result += 1;
    }
  }
  return result;
}

// TODO: Separate scopes to make this much more efficient?
efd_node *efd_nth(efd_node const * const node, size_t index) {
  size_t i;
#ifdef DEBUG
  size_t original = index;
#endif
  dictionary *children;
  efd_node *child;

  efd_push_error_context_with_node(
    s_sprintf("...while getting nth child of node (n=%zu):", index),
    node
  );

  if (!efd_is_container_node(node)) {
#ifdef DEBUG
    efd_report_error(
      s_("Warning: attempt to get indexed child in non-container node:"),
      node
    );
    fprintf(stderr, "\n");
#endif
    efd_pop_error_context();
    return NULL;
  }

  children = efd_children_dict(node);
  for (i = 0; i < d_get_count(children); ++i) {
    child = (efd_node*) d_get_item(children, i);
    if (child->h.type != EFD_NT_SCOPE) {
      if (index == 0) {
        efd_pop_error_context();
        return child;
      }
      index -= 1;
    }
  }
#ifdef DEBUG
  efd_report_error(
    s_("Warning: efd_index called with out-of-range index."),
    node
  );
  fprintf(stderr, "Index was: %lu\n", original);
  fprintf(stderr, "\n");
#endif
  efd_pop_error_context();
  return NULL;
}

efd_node * efd_lookup(efd_node const * const node, string const * const key) {
  if (node == NULL) {
    return NULL;
  }
  if (s_equals(key, EFD_ADDR_PARENT_STR)) {
    // Special handling for 'parent' path entries:
    return node->h.parent;
  } else {
    // Normal lookups:
    switch (node->h.type) {
      case EFD_NT_REROUTE:
      case EFD_NT_LINK:
      case EFD_NT_LOCAL_LINK:
      case EFD_NT_VARIABLE:
        return efd_lookup(efd_concrete(node), key);
      default:
        return efd_find_child(node, key);
    }
  }
}

list * efd_lookup_all(
  efd_node const * const node,
  string const * const key
) {
  efd_push_error_context_with_node(
    s_("...during efd_lookup_all in node:"),
    node
  );
  if (node == NULL) {
    efd_pop_error_context();
    return NULL;
  }
  if (s_equals(key, EFD_ADDR_PARENT_STR)) {
    efd_report_error(
      s_("Warning: efd_lookup_all asked for 'parent' of node:"),
      node
    );
    efd_pop_error_context();
    return NULL;
  } else {
    // Normal lookups:
    switch (node->h.type) {
      case EFD_NT_REROUTE:
      case EFD_NT_LINK:
      case EFD_NT_LOCAL_LINK:
      case EFD_NT_VARIABLE:
        efd_pop_error_context();
        return efd_lookup_all(efd_concrete(node), key);
      default:
        efd_pop_error_context();
        return efd_find_all_children(node, key);
    }
  }
}

efd_node * efd(efd_node const * const root, efd_address const * addr) {
  efd_node const * result = root;
  if (root == EFD_ROOT && s_equals(addr->name, EFD_ROOT_NAME)) {
    addr = addr->next;
  }
  while (addr != NULL && result != NULL) {
    result = efd_lookup(result, addr->name);
    addr = addr->next;
  }
  return (efd_node*) result;
}

efd_node * efdx(efd_node const * const root, string const * const saddr) {
  efd_address *addr = efd_parse_string_address(saddr);
  efd_node *result = efd(root, addr);
  cleanup_efd_address(addr);
  return result;
}

efd_node * efd_eval(efd_node const * const target, efd_value_cache *cache) {
  efd_node *result, *trace;
  efd_node const *last_active;
  efd_eval_function feval;

  efd_push_error_context_with_node(s_("...during evaluation of node:"), target);

  if (target == NULL) {
    efd_report_error(
      s_("ERROR: NULL target for evaluation."),
      target
    );
    efd_pop_error_context();
    return NULL;
  }

  // If the target is a function node, actually evaluate it:
  if (efd_is_function_node(target)) {
    // Check for evaluation cycles:
    if (m1_contains_key(cache->stack, (map_key_t) target)) {
      efd_report_error(
        s_("ERROR: Evaluation target depends on its own value."),
        target
      );
      trace = (efd_node*) m1_get_value(cache->stack, (map_key_t) target);
      while (trace != NULL) {
        efd_report_error(
          s_("   ...during evaluation of..."),
          trace
        );
        trace = (efd_node*) m1_get_value(cache->stack, (map_key_t) trace);
      }
      fprintf(stderr, "   ...trace finished.\n");
    } else {
      m1_put_value(
        cache->stack,
        (void*) cache->active,
        (map_key_t) target
      );
    }

    // Do the evaluation:
    last_active = cache->active;
    cache->active = target;
    feval = efd_lookup_function(target->b.as_function.function);
    if (feval == NULL) {
      efd_report_error(
        s_("ERROR: function node with invalid function:"),
        target
      );
    }
    efd_push_error_context(
      s_sprintf(
        "...during call to function '%U':",
        s_raw(target->b.as_function.function)
      )
    );
    result = feval(target, cache);
    efd_pop_error_context();
    cache->active = last_active;

    // Cache the result:
    m1_put_value(cache->values, (void*) result, (map_key_t) target);
    /* don't care = */ m1_pop_value(cache->stack, (map_key_t) target);
  } else {
    // If the target isn't a function node, just return the target pointer:
    result = (efd_node*) target;
  }
  efd_pop_error_context();
  return result;
}

efd_node * efd_get_value(
  efd_node const * const target,
  efd_value_cache * cache
) {
  efd_node *result, *ct;
  efd_push_error_context_with_node(
    s_("...while getting the value of node:"),
    target
  );
  if (target == NULL) {
    efd_pop_error_context();
    return NULL;
  }

  ct = efd_concrete(target);
  if (ct == NULL) {
    efd_report_broken_link(
      s_("ERROR: Broken link passed to efd_get_value:"),
      target
    );
  }
  result = m1_get_value(cache->values, (map_key_t) ct);

  if (result == NULL) { // cache miss
    result = efd_eval(ct, cache);
#ifdef DEBUG
    if (target == NULL) {
      fprintf(
        stderr,
        "ERROR: NULL target for efd_get_value.\n"
      );
      exit(EXIT_FAILURE);
    } else if (result == NULL) {
      efd_report_error(
        s_("Warning: evaluation result is NULL in efd_get_value:"),
        target
      );
    }
#endif // ifdef DEBUG
  }

  efd_pop_error_context();
  return result;
}

efd_node * efd_fresh_value(efd_node const * const target) {
  efd_value_cache *tmp = create_efd_value_cache();
  efd_node *result = efd_get_value(target, tmp);
  result = copy_efd_node(result); // old result is in the value cache
  cleanup_efd_value_cache(tmp);
  return result;
}

// Private implementation w/ cache argument:
efd_node * _efd_flatten(efd_node const * const target, efd_value_cache *cache) {
  size_t i;
  dictionary *children;
  efd_node *sub, *result;
  efd_push_error_context_with_node(
    s_("...while flattening node:"),
    target
  );
  if (efd_is_link_node(target)) {
    sub = efd_concrete(target);
    if (sub == NULL) {
      efd_report_broken_link(
        s_("ERROR: Broken link during efd_flatten."),
        target
      );
    }
    result = _efd_flatten(sub, cache);
    efd_rename(result, target->h.name);
    efd_pop_error_context();
    return result;
  } else if (efd_is_function_node(target)) {
    sub = efd_get_value(target, cache);
    if (sub == NULL) {
      efd_report_error(
        s_("ERROR: Target with NULL value during efd_flatten:"),
        target
      );
    }
    efd_pop_error_context();
    return _efd_flatten(sub, cache);
  } else {
    if (efd_is_container_node(target)) {
      result = create_efd_node(target->h.type, target->h.name);
      children = efd_children_dict(target);
      for (i = 0; i < d_get_count(children); ++i) {
        sub = (efd_node*) d_get_item(children, i);
        if (sub == NULL) {
          efd_report_error(
            s_sprintf("ERROR: NULL child (%lu) during efd_flatten:", i),
            target
          );
        }
        efd_add_child(
          result,
          _efd_flatten(sub, cache)
        );
      }
      // If it's a generator instead of just a container or scope we need to
      // copy the function value:
      if (efd_is_generator_node(target) /* function node is impossible */) {
        result->b.as_function.function = copy_string(
          target->b.as_function.function
        );
      }
      efd_pop_error_context();
      return result;
    } else {
      efd_pop_error_context();
      return copy_efd_node(target);
    }
  }
}

efd_node * efd_flatten(efd_node const * const target) {
  efd_value_cache *tmp = create_efd_value_cache();
  efd_node *result = _efd_flatten(target, tmp);
  cleanup_efd_value_cache(tmp);
  return result;
}

// private helper for efd_compute_values:
void _efd_compute_values(efd_node const * const root, efd_value_cache * cache) {
  size_t i;
  dictionary *children;
  efd_node *child;
  if (efd_is_function_node(root)) {
    /* don't care = */ efd_eval(root, cache);
  } else if (efd_is_container_node(root)) {
    children = efd_children_dict(root);
    for (i = 0; i < d_get_count(children); ++i) {
      child = d_get_item(children, i);
      if (efd_is_link_node(child)) {
        child = efd_concrete(child);
      }
      _efd_compute_values(child, cache);
    }
  }
}

efd_node * efd_call_function(
  efd_node const * const function,
  efd_node *args
) {
  SSTR(s_ftype, "call", 4);

  efd_node *call_node;
  efd_node *result;

  call_node = construct_efd_function_node(
    EFD_ANON_NAME,
    EFD_NT_CONTAINER,
    s_ftype
  );
  efd_add_child(
    call_node,
    construct_efd_link_node_to(EFD_ANON_NAME, function)
  );
  efd_add_child(call_node, args);

  result = efd_fresh_value(call_node);
  cleanup_efd_node(call_node);

  return result;
}

efd_value_cache * efd_compute_values(efd_node const * const root) {
  efd_value_cache *result = create_efd_value_cache();
  _efd_compute_values(root, result);
  return result;
}

void efd_add_crossref(efd_index *cr, efd_bridge *bridge) {
  l_append_element(cr->unprocessed, bridge);
}

// Private iteration helper:
void _iter_efd_process_child_references(void *v_child, void *v_cr) {
  efd_process_references((efd_node*) v_child, (efd_index*) v_cr);
}

// Private search function:
int _find_matching_reference(void *element, void *ref) {
  efd_bridge *b = (efd_bridge*) element;
  efd_node *target = (efd_node*) ref;
  efd_node *cmp = efd(EFD_ROOT, b->from->addr);
  return cmp == target;
}

void efd_process_references(efd_node *root, efd_index *cr) {
  efd_push_error_context_with_node(
    s_("...while processing references in node:"),
    root
  );
  // First process references in any children recursively:
  if (efd_is_container_node(root)) {
    d_witheach(
      efd_children_dict(root),
      (void*) cr,
      &_iter_efd_process_child_references
    );
  } else if (root->h.type == EFD_NT_PROTO) {
    // Recurse into the inputs for prototypes as well as normal children:
    efd_process_references(root->b.as_proto.input, cr);
  }

  // Then if this node is the target of a global reference change it's value:
  // TODO: more efficiency here!
  efd_bridge *match = (efd_bridge*) l_scan_elements(
    cr->unprocessed,
    (void*) root,
    &_find_matching_reference
  );

  if (match != NULL && efd_process_bridge(match)) {
    l_remove_element(cr->unprocessed, (void*) match);
    l_append_element(cr->processed, (void*) match);
  }
  efd_pop_error_context();
}

int efd_process_bridge(efd_bridge *b) {
  efd_node *src = efd(EFD_ROOT, b->to->addr);
  efd_node *dst = efd(EFD_ROOT, b->from->addr);

  string *err_addr;
  efd_node *node;
  size_t acount;

  efd_int_t int_value = 0;
  efd_num_t num_value = 0;
  string * str_value = NULL;
  void * obj_value = NULL;

  if (src == NULL) {
    return 0; // this bridge is still missing it's target
  }
  if (dst == NULL) {
    // This bridge has an invalid target!
    fprintf(
      stderr,
      "ERROR: efd_process_bridge called with an invalid destination:\n"
    );
    err_addr = efd_addr_string(b->to->addr);
    s_fprintln(stderr, err_addr);
    cleanup_string(err_addr);
    exit(EXIT_FAILURE);
  }
  if (!efd_ref_types_are_compatible(b->from->type, b->to->type)) {
    // This bridge is broken as its endpoints are incompatible
    fprintf(
      stderr,
      "ERROR: efd_process_bridge called with incompatible endpoints from:\n"
    );
    err_addr = efd_addr_string(b->from->addr);
    s_fprintln(stderr, err_addr);
    cleanup_string(err_addr);
    fprintf(stderr, "to:\n");
    err_addr = efd_addr_string(b->to->addr);
    s_fprintln(stderr, err_addr);
    cleanup_string(err_addr);
    exit(EXIT_FAILURE);
  }
  switch (b->to->type) {
    default:
    case EFD_RT_INVALID:
      fprintf(
        stderr,
        "ERROR: efd_process_bridge called with invalid source type.\n"
      );
      exit(EXIT_FAILURE);
    case EFD_RT_GLOBAL_INT:
      int_value = efd_get_global_i(b->to->addr->name);
      obj_value = i_as_p(int_value);
      break;
    case EFD_RT_GLOBAL_NUM:
      num_value = efd_get_global_n(b->to->addr->name);
      obj_value = f_as_p(num_value);
      break;
    case EFD_RT_GLOBAL_STR:
      str_value = efd_get_global_s(b->to->addr->name);
      obj_value = (void*) str_value;
      break;
    case EFD_RT_NODE:
      obj_value = efd(EFD_ROOT, b->to->addr);
      break;
    case EFD_RT_CHAIN:
      obj_value = efd(EFD_ROOT, b->to->addr);
      break;
    case EFD_RT_OBJ:
      obj_value = *efd__o(efd(EFD_ROOT, b->to->addr));
      break;
    case EFD_RT_INT:
      int_value = *efd__i(efd(EFD_ROOT, b->to->addr));
      obj_value = i_as_p(int_value);
      break;
    case EFD_RT_NUM:
      num_value = *efd__n(efd(EFD_ROOT, b->to->addr));
      obj_value = f_as_p(num_value);
      break;
    case EFD_RT_STR:
      str_value = *efd__s(efd(EFD_ROOT, b->to->addr));
      obj_value = (void*) str_value;
      break;
    case EFD_RT_INT_ARR_ENTRY:
      node = efd(EFD_ROOT, b->to->addr);
      acount = *efd__ai_count(node);
      if (b->to->idx < 0 || b->to->idx >= acount) {
        fprintf(
          stderr,
          "ERROR: efd_process_bridge called with bad array index %ld into:\n",
          b->to->idx
        );
        err_addr = efd_addr_string(b->to->addr);
        s_fprintln(stderr, err_addr);
        cleanup_string(err_addr);
        exit(EXIT_FAILURE);
      }
      int_value = (*efd__ai(node))[b->to->idx];
      obj_value = i_as_p(int_value);
      break;
    case EFD_RT_NUM_ARR_ENTRY:
      node = efd(EFD_ROOT, b->to->addr);
      acount = *efd__an_count(node);
      if (b->to->idx < 0 || b->to->idx >= acount) {
        fprintf(
          stderr,
          "ERROR: efd_process_bridge called with bad array index %ld into:\n",
          b->to->idx
        );
        err_addr = efd_addr_string(b->to->addr);
        s_fprintln(stderr, err_addr);
        cleanup_string(err_addr);
        exit(EXIT_FAILURE);
      }
      num_value = (*efd__an(node))[b->to->idx];
      obj_value = f_as_p(num_value);
      break;
    case EFD_RT_STR_ARR_ENTRY:
      node = efd(EFD_ROOT, b->to->addr);
      acount = *efd__as_count(node);
      if (b->to->idx < 0 || b->to->idx >= acount) {
        fprintf(
          stderr,
          "ERROR: efd_process_bridge called with bad array index %ld into:\n",
          b->to->idx
        );
        err_addr = efd_addr_string(b->to->addr);
        s_fprintln(stderr, err_addr);
        cleanup_string(err_addr);
        exit(EXIT_FAILURE);
      }
      str_value = (*efd__as(node))[b->to->idx];
      obj_value = (void*) str_value;
      break;
  }
  // Now that we have a value, put it in the appropriate spot (the
  // compatibility check earlier ensures that the switch statement above always
  // produces the value we need here.
  switch (b->from->type) {
    default:
    case EFD_RT_INVALID:
    case EFD_RT_NODE:
    case EFD_RT_CHAIN:
      fprintf(
        stderr,
        "ERROR: efd_process_bridge called with invalid destination type.\n"
      );
      exit(EXIT_FAILURE);
    case EFD_RT_GLOBAL_INT:
      efd_set_global_i(b->from->addr->name, int_value);
      break;
    case EFD_RT_GLOBAL_NUM:
      efd_set_global_n(b->from->addr->name, num_value);
      break;
    case EFD_RT_GLOBAL_STR:
      efd_set_global_s(b->from->addr->name, copy_string(str_value));
      break;
    case EFD_RT_OBJ:
      // TODO: Handle the old value more gently?
      node = efd(EFD_ROOT, b->from->addr);
      if (node->b.as_object.value != NULL) {
        fprintf(
          stderr,
          "ERROR: efd_process_bridge called to replace non-NULL obj value at:\n"
        );
        err_addr = efd_addr_string(b->from->addr);
        s_fprintln(stderr, err_addr);
        cleanup_string(err_addr);
        exit(EXIT_FAILURE);
      }
      node->b.as_object.value = obj_value;
      break;
    case EFD_RT_INT:
      efd(EFD_ROOT, b->from->addr)->b.as_integer.value = int_value;
      break;
    case EFD_RT_NUM:
      efd(EFD_ROOT, b->from->addr)->b.as_number.value = num_value;
      break;
    case EFD_RT_STR:
      efd(EFD_ROOT, b->from->addr)->b.as_string.value = str_value;
      break;
    case EFD_RT_INT_ARR_ENTRY:
      node = efd(EFD_ROOT, b->from->addr);
      acount = *efd__ai_count(node);
      if (b->from->idx < 0 || b->from->idx >= acount) {
        fprintf(
          stderr,
          "ERROR: efd_process_bridge called with bad array index %ld into:\n",
          b->from->idx
        );
        err_addr = efd_addr_string(b->from->addr);
        s_fprintln(stderr, err_addr);
        cleanup_string(err_addr);
      }
      (*efd__ai(node))[b->from->idx] = int_value;
      break;
    case EFD_RT_NUM_ARR_ENTRY:
      node = efd(EFD_ROOT, b->from->addr);
      acount = *efd__an_count(node);
      if (b->from->idx < 0 || b->from->idx >= acount) {
        fprintf(
          stderr,
          "ERROR: efd_process_bridge called with bad array index %ld into:\n",
          b->from->idx
        );
        err_addr = efd_addr_string(b->from->addr);
        s_fprintln(stderr, err_addr);
        cleanup_string(err_addr);
      }
      (*efd__an(node))[b->from->idx] = num_value;
      break;
    case EFD_RT_STR_ARR_ENTRY:
      node = efd(EFD_ROOT, b->from->addr);
      acount = *efd__as_count(node);
      if (b->from->idx < 0 || b->from->idx >= acount) {
        fprintf(
          stderr,
          "ERROR: efd_process_bridge called with bad array index %ld into:\n",
          b->from->idx
        );
        err_addr = efd_addr_string(b->from->addr);
        s_fprintln(stderr, err_addr);
        cleanup_string(err_addr);
      }
      (*efd__as(node))[b->from->idx] = str_value;
      break;
  }
  return 1;
}

// Private iterator:
void _iter_efd_unpack_children(void *v_child) {
  efd_unpack_node((efd_node*) v_child);
}

void efd_unpack_node(efd_node *root) {
  void *obj;
  efd_proto *p;
  efd_object *o;

  efd_push_error_context_with_node(
    s_("...while unpacking node:"),
    root
  );

  // First unpack any children recursively:
  if (efd_is_container_node(root)) {
    d_foreach(
      efd_children_dict(root),
      &_iter_efd_unpack_children
    );
  }

  // Then transform this node into an OBJECT if necessary:
  if (root->h.type == EFD_NT_PROTO) {
    root->h.type = EFD_NT_OBJECT; // change the node type
    p = &(root->b.as_proto);
    efd_unpack_node(p->input); // First recursively unpack the input
    efd_unpack_function unpacker = efd_lookup_unpacker(p->format);
    if (unpacker == NULL) {
      efd_report_error(
        s_("Error during attempt to unpack node:"),
        root
      );
    }
    obj = unpacker(p->input); // unpack
#ifdef DEBUG_TRACE_EFD_CLEANUP
    fprintf(stderr, "Object input cleanup.\n");
#endif
    cleanup_efd_node(p->input); // free now-unnecessary EFD data
    o = &(root->b.as_object);
    // format field should overlap perfectly and thus need no change
    o->value = obj; // o->value is in the same place as p->input
  }
  efd_pop_error_context();
}

// Private iterator:
void _iter_efd_pack_children(void *v_child, void *v_cr) {
  efd_pack_node((efd_node*) v_child, (efd_index*) v_cr);
}

void efd_pack_node(efd_node *root, efd_index *cr) {
  efd_node *n;
  efd_proto *p;
  efd_object *o;
  dictionary *children;

  efd_push_error_context_with_node(s_("...while packing node:"), root);

  // First pack any children recursively:
  if (efd_is_container_node(root)) {
    children = efd_children_dict(root);
    d_witheach(
      children,
      (void*) cr,
      &_iter_efd_pack_children
    );
  }

  // Transform this node into a PROTO if necessary:
  if (root->h.type == EFD_NT_OBJECT) {
    root->h.type = EFD_NT_PROTO; // change the node type
    o = &(root->b.as_object);
    efd_pack_function packer = efd_lookup_packer(o->format);
    if (packer == NULL) {
      efd_report_error(
        s_("Error finding packer during attempt to pack node:"),
        root
      );
    }
    efd_destroy_function destructor = efd_lookup_destructor(o->format);
    if (destructor == NULL) {
      efd_report_error(
        s_("Error finding destructor during attempt to pack node:"),
        root
      );
    }
    n = packer(o->value); // pack
    destructor(o->value); // free unpacked data
    efd_pack_node(n, cr); // recursively pack the results
    p = &(root->b.as_proto);
    // format field should overlap perfectly and thus need no change
    p->input = n; // p->input is in the same place as o->value
  } // else do nothing
  efd_pop_error_context();
}

efd_int_t efd_get_global_i(string const * const key) {
  return (efd_int_t) d_get_value_s(EFD_INT_GLOBALS, key);
}

efd_num_t efd_get_global_n(string const * const key) {
  void *r = d_get_value_s(EFD_NUM_GLOBALS, key);
  return *((efd_num_t*) &r);
}

string* efd_get_global_s(string const * const key) {
  return (string*) d_get_value_s(EFD_STR_GLOBALS, key);
}

void efd_set_global_i(string const * const key, efd_int_t value) {
  d_set_value_s(EFD_INT_GLOBALS, key, (void*) value);
}

void efd_set_global_n(string const * const key, efd_num_t value) {
  d_set_value_s(EFD_NUM_GLOBALS, key, f_as_p(value));
}

void efd_set_global_s(string const * const key, string *value) {
  string *tmp;
  tmp = (string*) d_pop_value_s(EFD_STR_GLOBALS, key);
  while (tmp != NULL) {
    cleanup_string(tmp);
    tmp = (string*) d_pop_value_s(EFD_STR_GLOBALS, key);
  }
  d_add_value_s(EFD_STR_GLOBALS, key, (void*) value);
}

void* dont_copy(void *v) {
  return v;
}

void dont_cleanup(void *v) {
  return;
}

efd_node * efd_gen_next(efd_generator_state *gen) {
  size_t i;
  efd_node *node, *scope, *result;
  efd_generator_state *sub;
  list *children;

  efd_push_error_context(s_("...during efd_gen_next:"));

  switch (gen->type) {
    default:
    case EFD_GT_INVALID:
      fprintf(
        stderr,
        "Warning: Attempt to get next value from INVALID generator.\n"
      );
      efd_pop_error_context();
      return NULL;

    case EFD_GT_CHILDREN:
      node = (efd_node*) gen->state;
      if (gen->index < efd_normal_child_count(node)) {
        result = efd_fresh_value(efd_nth(node, gen->index));
        gen->index += 1;
        efd_pop_error_context();
        return result;
      } else {
        efd_pop_error_context();
        return NULL;
      } // all paths return

    case EFD_GT_INDICES:
      node = (efd_node*) gen->state;
      switch (node->h.type) {
        default:
          efd_report_error(
            s_("Warning: Array index generator has non-array node state:"),
            node
          );
          fprintf(stderr, "\n");
          efd_pop_error_context();
          return NULL;
        case EFD_NT_ARRAY_INT:
          efd_pop_error_context();
          return construct_efd_int_node(
            gen->name,
            (*efd__ai(node))[gen->index++]
          );
        case EFD_NT_ARRAY_NUM:
          efd_pop_error_context();
          return construct_efd_num_node(
            gen->name,
            (*efd__an(node))[gen->index++]
          );
        case EFD_NT_ARRAY_STR:
          efd_pop_error_context();
          return construct_efd_str_node(
            gen->name,
            (*efd__as(node))[gen->index++]
          );
      } // all paths return

    case EFD_GT_FUNCTION:
      efd_pop_error_context();
      return ((efd_generator_function) gen->state)(gen);

    case EFD_GT_EXTEND_RESTART:
      sub = (efd_generator_state*) gen->state;
      node = efd_gen_next(sub);
      if (node == NULL) {
        efd_gen_reset(sub);
        node = efd_gen_next(sub);
      }
      efd_rename(node, gen->name);
      efd_pop_error_context();
      return node;

    case EFD_GT_EXTEND_HOLD:
      sub = (efd_generator_state*) gen->state;
      node = efd_gen_next(sub);
      if (node == NULL) {
        efd_pop_error_context();
        return copy_efd_node((efd_node*) gen->stash);
      }
      if (gen->stash != NULL) {
        fprintf(stderr, "EXT_HOLD generator stash turnover cleanup.\n");
        cleanup_v_efd_node(gen->stash);
      }
      gen->stash = (void*) copy_efd_node_as(node, gen->name);
      efd_rename(node, gen->name);
      efd_pop_error_context();
      return node;

    case EFD_GT_PARALLEL:
      children = (list*) gen->state;
      scope = create_efd_node(EFD_NT_SCOPE, gen->name);
      for (i = 0; i < l_get_length(children); ++i) {
        sub = (efd_generator_state*) l_get_item(children, i);
        node = efd_gen_next(sub);
        if (node == NULL) {
          fprintf(stderr, "Finished generator scope cleanup.\n");
          cleanup_efd_node(scope);
          efd_pop_error_context();
          return NULL;
        } else {
          efd_add_child(scope, node);
        }
      }
      efd_pop_error_context();
      return scope;
  }
}

void efd_gen_reset(efd_generator_state *gen) {
  size_t i;
  list *children;
  efd_generator_state *sub;
  // reset our index:
  gen->index = 0;
  // any additional reset functionality:
  switch (gen->type) {
    default:
    case EFD_GT_INVALID:
      fprintf(
        stderr,
        "Warning: Attempt to reset an INVALID generator.\n"
      );
      break;

    case EFD_GT_CHILDREN:
    case EFD_GT_INDICES:
    case EFD_GT_FUNCTION:
      // nothing to do
      break;

    case EFD_GT_EXTEND_RESTART:
    case EFD_GT_EXTEND_HOLD:
      // reset the sub-generator as well
      efd_gen_reset((efd_generator_state*) gen->state);
      break;

    case EFD_GT_PARALLEL:
      // reset each sub-generator
      children = (list*) gen->state;
      for (i = 0; i < l_get_length(children); ++i) {
        sub = (efd_generator_state*) l_get_item(children, i);
        efd_gen_reset(sub);
      }
      break;
  }
}

efd_node * efd_gen_all(efd_generator_state *gen) {
  efd_node *result = create_efd_node(EFD_NT_CONTAINER, gen->name);
  efd_node *next = efd_gen_next(gen);
  while (next != NULL) {
    efd_add_child(result, next);
    next = efd_gen_next(gen);
  }
  return result;
}

efd_generator_state * efd_generator_for(
  efd_node *node,
  efd_value_cache *cache
) {
  switch (node->h.type) {
    default:
    case EFD_NT_INVALID:
      return NULL;

    case EFD_NT_CONTAINER:
    case EFD_NT_REROUTE:
      return create_efd_generator_state(
        EFD_GT_CHILDREN,
        node->h.name,
        (void*) node
      );

    case EFD_NT_ARRAY_INT:
    case EFD_NT_ARRAY_NUM:
    case EFD_NT_ARRAY_STR:
      return create_efd_generator_state(
        EFD_GT_INDICES,
        node->h.name,
        (void*) node
      );

    case EFD_NT_GENERATOR:
    case EFD_NT_GN_OBJ:
    case EFD_NT_GN_INT:
    case EFD_NT_GN_NUM:
    case EFD_NT_GN_STR:
    case EFD_NT_GN_AR_INT:
    case EFD_NT_GN_AR_NUM:
    case EFD_NT_GN_AR_STR:
      return efd_lookup_generator(node->b.as_function.function)(node, cache);
  }
}
