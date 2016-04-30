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

/*************
 * Constants *
 *************/

CSTR(EFD_ADDR_SEP_STR, ".", 1);
CSTR(EFD_ADDR_PARENT_STR, "^", 1);

CSTR(EFD_ANON_NAME, "-", 1);
CSTR(EFD_ROOT_NAME, "_", 1);

char const * const EFD_NT_NAMES[] = {
  "<invalid>",
  "<any>",
  "container",
  "global_link",
  "local_link",
  "scope",
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
  "L",
  "l",
  "V",
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

dictionary *EFD_INT_GLOBALS = NULL;
dictionary *EFD_NUM_GLOBALS = NULL;
dictionary *EFD_STR_GLOBALS = NULL;

/******************************
 * Constructors & Destructors *
 ******************************/

efd_node* create_efd_node(efd_node_type t, string const * const name) {
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

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      result->b.as_link.target = NULL;
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
  result->b.as_object.format = copy_string(format);
  result->b.as_object.value =  efd_lookup_copier(format)(obj);
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

efd_node* copy_efd_node(efd_node const * const src) {
  size_t i;
  dictionary *children;
  efd_node *result;
  
  result = create_efd_node(src->h.type, src->h.name);
  switch (src->h.type) {
    default:
    case EFD_NT_INVALID:
#ifdef DEBUG
      fprintf(
        stderr,
        "Warning: copy_efd_node called on INVALID node '%.*s'\n.",
        (int) s_get_length(src->h.name),
        s_raw(src->h.name)
      );
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

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      // target
      result->b.as_link.target = copy_efd_address(src->b.as_link.target);
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
      result->b.as_object.value = efd_lookup_copier(src->b.as_object.format)(
        src->b.as_object.value
      );
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
      d_foreach(doomed->b.as_container.children, &cleanup_v_efd_node);
      cleanup_dictionary(doomed->b.as_container.children);
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
      // Clean up the function name:
      cleanup_string(doomed->b.as_function.function);
      // Clean up any children:
      d_foreach(doomed->b.as_function.children, &cleanup_v_efd_node);
      cleanup_dictionary(doomed->b.as_function.children);
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      // Clean up our target address:
      if (doomed->b.as_link.target != NULL) {
        cleanup_efd_address(doomed->b.as_link.target);
      }
      break;

    case EFD_NT_PROTO:
      // Clean up the format string:
      cleanup_string(doomed->b.as_proto.format);
      // Clean up the input node:
      if (doomed->b.as_proto.input != NULL) {
        cleanup_efd_node(doomed->b.as_proto.input);
      }
      break;

    case EFD_NT_OBJECT:
      // Clean up the object:
      df = efd_lookup_destructor(doomed->b.as_object.format);
      if (df != NULL) {
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
  // If this node is a child, remove it from its parent's children:
  if (doomed->h.parent != NULL) {
    efd_remove_child(doomed->h.parent, doomed);
  }
  // Clean up the node's name:
  cleanup_string(doomed->h.name);
  // Finally free the memory for this node:
  free(doomed);
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
  result->name = copy_string(node->h.name);
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

/*
 * TODO: Get rid of this!
efd_path* create_efd_path(
  efd_path *parent,
  efd_node *here,
  efd_path_element_type type_override
) {
  efd_path *result = (efd_path*) malloc(sizeof(efd_path));
  result->node = here;
  result->parent = parent;
  if (type_override == EFD_PET_UNKNOWN) {
    if (efd_is_link_node(here)) {
      result->type = EFD_PET_LINK;
    } else {
      result->type = EFD_PET_NORMAL;
    }
  } else {
    result->type = type_override;
  }
  return result;
}

efd_path* construct_efd_path_for(efd_node *node) {
  efd_path *head = NULL;
  efd_path *next;
  efd_path *tail = NULL;
  while (node != NULL) {
    next = create_efd_path(NULL, node, EFD_PET_UNKNOWN);
    if (tail == NULL) { tail = next; }
    if (head != NULL) { head->parent = next; }
    head = next;
    node = node->h.parent;
  }
  return tail;
}

efd_path* copy_efd_path(efd_path const * const original) {
  efd_path const *head;
  efd_path *tail, *next, *last;
  head = original;
  tail = NULL;
  last = NULL;
  while (head != NULL) {
    next = create_efd_path(NULL, head->node, EFD_PET_UNKNOWN);
    if (tail == NULL) { tail = next; }
    if (last != NULL) { last->parent = next; }
    last = next;
    head = head->parent;
  }
  return tail;
}

CLEANUP_IMPL(efd_path) {
  efd_path *parent = doomed->parent;
  free(doomed);
  cleanup_efd_path(parent);
}
*/

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
  if (!efd_ref_types_are_compatible(from->type, to->type)) {
#ifdef DEBUG
    fprintf(
      stderr,
      "ERROR: Incompatible EFD reference types %d and %d cannot be a bridge.\n",
      from->type,
      to->type
    );
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

efd_index* create_efd_index(void) {
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
        cleanup_v_efd_node(doomed->stash);
      }
      break;

    case EFD_GT_EXTEND_RESTART:
      cleanup_v_efd_generator_state(doomed->state);
      break;

    case EFD_GT_EXTEND_HOLD:
      cleanup_v_efd_generator_state(doomed->state);
      if (doomed->stash != NULL) {
        cleanup_v_efd_node(doomed->stash);
      }
      break;

    case EFD_GT_PARALLEL:
      l_foreach((list*) doomed->state, &cleanup_v_efd_generator_state);
      cleanup_list((list*) doomed->state);
      break;
  }
}

/*************
 * Functions *
 *************/

void * v_efd__v_i(void *v_node) {
  efd_node *n = (efd_node*) v_node;
  return (void*) efd_as_i(n);
}

void * v_efd__v_n(void *v_node) {
  efd_node *n = (efd_node*) v_node;
  efd_num_t num = efd_as_n(n);
  return *f_as_p(&num);
}

void * v_efd__v_s(void *v_node) {
  efd_node *n = (efd_node*) v_node;
  return (void*) (*efd__s(n));
}

int efd_ref_types_are_compatible(efd_ref_type from, efd_ref_type to) {
  switch (from) {
    default:
    case EFD_RT_INVALID:
    case EFD_RT_NODE:
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
     || to == EFD_RT_OBJ
     || to == EFD_RT_INT
     || to == EFD_RT_INT_ARR_ENTRY
     || to == EFD_RT_NUM
     || to == EFD_RT_NUM_ARR_ENTRY
     || to == EFD_RT_STR
     || to == EFD_RT_STR_ARR_ENTRY
      );
  }
  // failsafe
  return 0;
}

void efd_assert_type(efd_node const * const n, efd_node_type t) {
  if (n == NULL) {
    fprintf(stderr, "ERROR: Missing EFD node in efd_assert_type!\n");
    exit(EXIT_FAILURE);
  }
#ifndef EFD_NO_TYPECHECKS
  if (n->h.type != t) {
    string *fqn = efd_build_fqn(n);
    if (n->h.type >= 0
     && n->h.type < EFD_NUM_TYPES
     && t >= 0
     && t < EFD_NUM_TYPES
    ) {
      fprintf(
        stderr,
        "ERROR: EFD node '%.*s' has type '%s' rather than '%s' as required.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        EFD_NT_NAMES[n->h.type],
        EFD_NT_NAMES[t]
      );
    } else {
      fprintf(
        stderr,
        "ERROR: EFD node '%.*s' has type '%d' rather than '%d' as required.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        n->h.type,
        t
      );
    }
    free(fqn);
    exit(EXIT_FAILURE);
  }
#endif // EFD_NO_TYPECHECKS
}

void efd_assert_return_type(efd_node const * const n, efd_node_type t) {
#ifndef EFD_NO_TYPECHECKS
  efd_node_type rt;
  rt = efd_return_type_of(n);
  if (rt != t) {
    string *fqn = efd_build_fqn(n);
    if (rt >= 0
     && rt < EFD_NUM_TYPES
     && t >= 0
     && t < EFD_NUM_TYPES
    ) {
      fprintf(
        stderr,
        "ERROR: EFD node '%.*s' has return type '%s' "
        "rather than '%s' as required.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        EFD_NT_NAMES[rt],
        EFD_NT_NAMES[t]
      );
    } else {
      fprintf(
        stderr,
        "ERROR: EFD node '%.*s' has return type '%d' "
        "rather than '%d' as required.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        rt,
        t
      );
    }
    free(fqn);
    exit(EXIT_FAILURE);
  }
#else
  return;
#endif // EFD_NO_TYPECHECKS
}

int efd_is_type(efd_node *n, efd_node_type t) {
  if (n == NULL) {
    return 0;
  }
#ifndef EFD_NO_TYPECHECKS
  return (n->h.type == t);
#else // EFD_NO_TYPECHECKS
  return 1;
#endif // EFD_NO_TYPECHECKS
}

int efd_format_is(efd_node *n, string const * const fmt) {
  if (efd_is_type(n, EFD_NT_PROTO)) {
    return s_equals(efd__p_fmt(n), fmt);
  } else if (efd_is_type(n, EFD_NT_OBJECT)) {
    return s_equals(efd__o_fmt(n), fmt);
  }
  // failsafe
  return 0;
}

void efd_rename(efd_node * node, string const * const new_name) {
  cleanup_string(node->h.name);
  node->h.name = copy_string(new_name);
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

  SSTR(s_space, " ", 1);
  SSTR(s_lsb, "[[", 2);
  SSTR(s_rsb, "]]", 2);
  SSTR(s_slsb, "[", 1);
  SSTR(s_srsb, "]", 1);
  SSTR(s_lab, "<<", 2);
  SSTR(s_rab, ">>", 2);
  SSTR(s_rcb, "{", 1);
  SSTR(s_lcb, "}", 1);
  SSTR(s_dots, "...", 3);
  SSTR(s_colon, ":", 1);
  SSTR(s_tilde, "~", 1);
  SSTR(s_pct, "%", 1);
  SSTR(s_caret, "->", 2);
  SSTR(s_quote, "\"", 1);

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
      s_append(scopes, s_space);
    }
  } else {
    s_append(scopes, s_space);
  } // scopes always has a trailing space
  

  contents = create_empty_string();
  for (i = 0; i < ncount && i < 3; ++i) {
    child = efd_nth(n, i);
    if (efd_is_link_node(child)) {
      s_append(contents, s_lab);
      s_devour(
        contents,
        create_string_from_ntchars(EFD_NT_ABBRS[child->h.type])
      );
      s_append(contents, s_space);
      s_append(contents, child->h.name);
      s_append(contents, s_rab);
    } else {
      s_append(contents, s_lsb);
      s_devour(
        contents,
        create_string_from_ntchars(EFD_NT_ABBRS[child->h.type])
      );
      s_append(contents, s_space);
      s_append(contents, child->h.name);
      s_append(contents, s_rsb);
    }
    if (i < ncount - 1) {
      s_append(contents, s_space);
    }
  }
  if (ncount >= 0 && i < ncount) {
    s_append(contents, s_dots);
  }

  value = create_empty_string();
  switch (n->h.type) {
    default:
      // leave 'value' empty
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
      s_append(value, s_pct);
      s_append(value, n->b.as_function.function);
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      s_append(value, s_caret);
      s_devour(value, efd_addr_string(n->b.as_link.target));
      break;

    case EFD_NT_PROTO:
      s_append(value, s_colon);
      s_append(value, s_tilde);
      s_append(value, n->b.as_proto.format);
      break;

    case EFD_NT_OBJECT:
      s_append(value, s_colon);
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
          s_append(value, s_space);
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
          s_append(value, s_space);
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
        s_devour(value, n->b.as_str_array.values[i]);
        s_append(value, s_quote);
        if (i < count - 1) {
          s_append(value, s_space);
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

  s_devour(result, create_string_from_ntchars(EFD_NT_ABBRS[n->h.type]));
  s_append(result, s_space);

  s_devour(result, fqn);
  s_append(result, s_space);

  s_devour(result, scopes);
  // scopes comes with a trailing space built-in

  s_devour(result, contents);
  s_append(result, s_space);

  s_devour(result, value);

  if (efd_is_link_node(n)) {
    s_append(result, s_rab);
  } else {
    s_append(result, s_rsb);
  }

  return result;
}

dictionary * efd_children_dict(efd_node const * const n) {
#ifdef DEBUG
  string *fqn;
#endif
  switch (n->h.type) {
    default:
#ifdef DEBUG
      fqn = efd_build_fqn(n);
      if (n->h.type >= 0 && n->h.type < EFD_NUM_TYPES) {
        fprintf(
          stderr,
          "Warning: Attempt to get children of non-container EFD node '%.*s' "
          "of type '%s'.\n",
          (int) s_get_length(fqn),
          s_raw(fqn),
          EFD_NT_NAMES[n->h.type]
        );
      } else {
        fprintf(
          stderr,
          "Warning: Attempt to get children of non-container EFD node '%.*s' "
          "of type '%d'.\n",
          (int) s_get_length(fqn),
          s_raw(fqn),
          n->h.type
        );
      }
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
  int cmp_names
) {
  size_t i;
  dictionary *cmp_ch, *agn_ch;
  efd_node *cmp_t, *agn_t;
  efd_index *empty_index;
  if (
    cmp->h.type != agn->h.type
 || (cmp_names && !s_equals(cmp->h.name, agn->h.name))
  ) {
    return 0;
  }
  switch (cmp->h.type) {
    default:
#ifdef DEBUG
      fprintf(
        stderr,
        "ERROR: Invalid node type %d in efd comparison.\n",
        cmp->h.type
      );
#endif
      return 0;

    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
      cmp_ch = cmp->b.as_container.children;
      agn_ch = agn->b.as_container.children;
      if (d_get_count(cmp_ch) != d_get_count(agn_ch)) {
        return 0;
      }
      for (i = 0; i < d_get_count(cmp_ch); ++i) {
        if (!_efd_cmp(d_get_item(cmp_ch, i), d_get_item(agn_ch, i), cmp_names)){
          return 0;
        }
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
        return 0;
      }
      cmp_ch = cmp->b.as_function.children;
      agn_ch = agn->b.as_function.children;
      if (d_get_count(cmp_ch) != d_get_count(agn_ch)) {
        return 0;
      }
      for (i = 0; i < d_get_count(cmp_ch); ++i) {
        if (!_efd_cmp(d_get_item(cmp_ch, i), d_get_item(agn_ch, i), cmp_names)){
          return 0;
        }
      }
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      if (!_efd_cmp(efd_concrete(cmp), efd_concrete(agn), cmp_names)) {
        return 0;
      }
      break;

    case EFD_NT_PROTO:
      if (
        !s_equals(cmp->b.as_proto.format, agn->b.as_proto.format)
     || !_efd_cmp(cmp->b.as_proto.input, agn->b.as_proto.input, cmp_names)
      ) {
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
      if (!_efd_cmp(cmp_t, agn_t, cmp_names)) {
        cleanup_efd_node(cmp_t);
        cleanup_efd_node(agn_t);
        return 0;
      }
      cleanup_efd_index(empty_index);
      cleanup_efd_node(cmp_t);
      cleanup_efd_node(agn_t);
      break;

    case EFD_NT_INTEGER:
      if (cmp->b.as_integer.value != agn->b.as_integer.value) {
        return 0;
      }
      break;

    case EFD_NT_NUMBER:
      if (cmp->b.as_number.value != agn->b.as_number.value) {
        return 0;
      }
      break;

    case EFD_NT_STRING:
      if (!s_equals(cmp->b.as_string.value, agn->b.as_string.value)) {
        return 0;
      }
      break;

    case EFD_NT_ARRAY_INT:
      if (cmp->b.as_int_array.count != agn->b.as_int_array.count) {
        return 0;
      }
      for (i = 0; i < cmp->b.as_int_array.count; ++i) {
        if (cmp->b.as_int_array.values[i] != agn->b.as_int_array.values[i]) {
          return 0;
        }
      }
      break;

    case EFD_NT_ARRAY_NUM:
      if (cmp->b.as_num_array.count != agn->b.as_num_array.count) {
        return 0;
      }
      for (i = 0; i < cmp->b.as_num_array.count; ++i) {
        if (cmp->b.as_num_array.values[i] != agn->b.as_num_array.values[i]) {
          return 0;
        }
      }
      break;

    case EFD_NT_ARRAY_STR:
      if (cmp->b.as_str_array.count != agn->b.as_str_array.count) {
        return 0;
      }
      for (i = 0; i < cmp->b.as_str_array.count; ++i) {
        if (
          !s_equals(
            cmp->b.as_str_array.values[i],
            agn->b.as_str_array.values[i]
          )
        ) {
          return 0;
        }
      }
      break;
  }
  return 1;
}

int efd_equals(efd_node const * const cmp, efd_node const * const agn) {
  return _efd_cmp(cmp, agn, 1);
}

int efd_equivalent(efd_node const * const cmp, efd_node const * const agn) {
  return _efd_cmp(cmp, agn, 0);
}

void efd_add_child(efd_node *n, efd_node *child) {
#ifdef DEBUG
  string *fqn, *cfqn;
  if (!efd_is_container_node(n)) {
    fqn = efd_build_fqn(n);
    if (n->h.type >= 0 && n->h.type < EFD_NUM_TYPES) {
      fprintf(
        stderr,
        "ERROR: Can't add child to non-container EFD node '%.*s' "
        "of type '%s'.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        EFD_NT_NAMES[n->h.type]
      );
    } else {
      fprintf(
        stderr,
        "ERROR: Can't add child to non-container EFD node '%.*s' "
        "of type '%d'.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        n->h.type
      );
    }
    exit(EXIT_FAILURE);
  }
  if (child->h.parent != NULL) {
    fqn = efd_build_fqn(n);
    cfqn = efd_build_fqn(child);
    fprintf(
      stderr,
      "Warning: Adding child node '%.*s' to parent '%.*s' "
      "but the child already has a parent.\n",
      (int) s_get_length(cfqn),
      s_raw(cfqn),
      (int) s_get_length(fqn),
      s_raw(fqn)
    );
  }
#endif
  d_add_value_s(efd_children_dict(n), child->h.name, (void*) child);
  child->h.parent = n;
}

void efd_prepend_child(efd_node *n, efd_node *child) {
#ifdef DEBUG
  string *fqn, *cfqn;
  if (!efd_is_container_node(n)) {
    fqn = efd_build_fqn(n);
    if (n->h.type >= 0 && n->h.type < EFD_NUM_TYPES) {
      fprintf(
        stderr,
        "ERROR: Can't prepend child to non-container EFD node '%.*s' "
        "of type '%s'.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        EFD_NT_NAMES[n->h.type]
      );
    } else {
      fprintf(
        stderr,
        "ERROR: Can't prepend child to non-container EFD node '%.*s' "
        "of type '%d'.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        n->h.type
      );
    }
    exit(EXIT_FAILURE);
  }
  if (child->h.parent != NULL) {
    fqn = efd_build_fqn(n);
    cfqn = efd_build_fqn(child);
    fprintf(
      stderr,
      "Warning: Prepending child node '%.*s' to parent '%.*s' "
      "but the child already has a parent.\n",
      (int) s_get_length(cfqn),
      s_raw(cfqn),
      (int) s_get_length(fqn),
      s_raw(fqn)
    );
  }
#endif
  d_prepend_value_s(efd_children_dict(n), child->h.name, (void*) child);
  child->h.parent = n;
}

void efd_remove_child(efd_node *n, efd_node *child) {
#ifdef DEBUG
  string *fqn, *cfqn;
  if (!efd_is_container_node(n)) {
    fqn = efd_build_fqn(n);
    if (n->h.type >= 0 && n->h.type < EFD_NUM_TYPES) {
      fprintf(
        stderr,
        "ERROR: Can't remove child from non-container EFD node '%.*s' "
        "of type '%s'.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        EFD_NT_NAMES[n->h.type]
      );
    } else {
      fprintf(
        stderr,
        "ERROR: Can't remove child from non-container EFD node '%.*s' "
        "of type '%d'.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        n->h.type
      );
    }
    exit(EXIT_FAILURE);
  }
#endif
#ifdef DEBUG
  d_remove_value(efd_children_dict(n), (void*) child);
  // if d_remove_value returns NULL that's fine (shadow children)
  if (child->h.parent != n) {
    fqn = efd_build_fqn(n);
    cfqn = efd_build_fqn(child);
    fprintf(
      stderr,
      "Warning: Attempt to remove child '%.*s' from node '%.*s' "
      "but the child wasn't descended from the parent.\n",
      (int) s_get_length(cfqn),
      s_raw(cfqn),
      (int) s_get_length(fqn),
      s_raw(fqn)
    );
  }
#else
  d_remove_value(efd_children_dict(n), (void*) child);
#endif
  child->h.parent = NULL;
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
    fprintf(
      stderr,
      "Warning: Tried to pop topmost address '%.*s'!\n",
      (int) s_get_length(a->name), 
      s_raw(a->name)
    );
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

// Private helper:
int _match_efd_name(void *v_child, void *v_key) {
  efd_node *child = (efd_node*) v_child;
  string *key = (string*) v_key;
  return s_equals(child->h.name, key);
}

efd_node* efd_find_child(
  efd_node const * const parent,
  string const * const name
) {
  if (!efd_is_container_node(parent)) {
#ifdef DEBUG
    fprintf(
      stderr,
      "Warning: efd_find_child called with key '%.*s' "
      "on non-container node '%.*s'.\n",
      (int) s_get_length(name),
      s_raw(name),
      (int) s_get_length(parent->h.name),
      s_raw(parent->h.name)
    );
#endif
    return NULL;
  }
  return (efd_node*) d_get_value_s(efd_children_dict(parent), name);
}

efd_node* efd_find_variable_in(
  efd_node const * const node,
  efd_address const * const target
) {
  dictionary *children;
  efd_node *child, *result;
  size_t i;
  if (!efd_is_container_node(node)) {
#ifdef DEBUG
    fprintf(
      stderr,
      "Warning: efd_find_variable_in called with address starting '%.*s' "
      "on non-container node '%.*s'.\n",
      (int) s_get_length(target->name),
      s_raw(target->name),
      (int) s_get_length(node->h.name),
      s_raw(node->h.name)
    );
#endif
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
  return result;
}

efd_node* efd_resolve_variable(efd_node const * const var) {
  efd_address *target;
  efd_node *result, *scope;

  efd_assert_type(var, EFD_NT_VARIABLE);
  target = var->b.as_link.target;

  scope = var->h.parent;

  result = NULL;
  while (scope != NULL && result == NULL && result != var) {
    result = efd_find_variable_in(scope, target);
    scope = scope->h.parent;
  }
  return result;
}

efd_node* efd_concrete(efd_node const * const base) {
  if (base == NULL) {
    return NULL;
  }
  efd_node *linked;
  switch (base->h.type) {
    default:
      return (efd_node*) base;
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

  if (!efd_is_container_node(node)) {
#ifdef DEBUG
    fprintf(
      stderr,
      "Warning: attempt to get indexed child in non-container node.\n"
    );
#endif
    return NULL;
  }

  children = efd_children_dict(node);
  for (i = 0; i < d_get_count(children); ++i) {
    child = (efd_node*) d_get_item(children, i);
    if (child->h.type != EFD_NT_SCOPE) {
      if (index == 0) {
        return child;
      }
      index -= 1;
    }
  }
#ifdef DEBUG
  fprintf(
    stderr,
    "Warning: efd_index called with out-of-range index %lu.\n",
    original
  );
#endif
  return NULL;
}

efd_node* efd_lookup(efd_node const * const node, string const * const key) {
  if (s_equals(key, EFD_ADDR_PARENT_STR)) {
    // Special handling for 'parent' path entries:
    return node->h.parent;
  } else {
    // Normal lookups:
    switch (node->h.type) {
      case EFD_NT_LINK:
      case EFD_NT_LOCAL_LINK:
      case EFD_NT_VARIABLE:
        return efd_lookup(efd_concrete(node), key);
      default:
        return efd_find_child(node, key);
    }
  }
}

efd_node* efd(efd_node const * const root, efd_address const * addr) {
  efd_node const * result = root;
  while (addr != NULL && result != NULL) {
    result = efd_lookup(result, addr->name);
    addr = addr->next;
  }
  return (efd_node*) result;
}

efd_node* efdx(efd_node const * const root, string const * const saddr) {
  return efd(root, efd_parse_string_address(saddr));
}

efd_node* efd_eval_in_context(
  efd_node const * const target,
  efd_node const * const args,
  efd_node const * const set_parent
) {
  size_t i;
  string *fqn, *taddr;
  efd_node *result, *transformed;
  efd_node *child, *new_child;
  dictionary *children;

  // If the target is a link, recurse on its target node
  if (efd_is_link_node(target)) {
    result = copy_efd_node(target);
    if (set_parent != NULL) {
      result->h.parent = (efd_node*) set_parent;
    }
    transformed = efd_concrete(result);
    if (transformed == NULL) {
      fqn = efd_build_fqn(target);
      taddr = efd_addr_string(result->b.as_link.target);
      fprintf(
        stderr,
        "Broken link during evaluation at node '%.*s' "
        "(target '%.*s' doesn't exist).\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        (int) s_get_length(taddr),
        s_raw(taddr)
      );
      exit(EXIT_FAILURE);
    }
    transformed = efd_eval_in_context(transformed, args, NULL);
    cleanup_efd_node(result);
    return transformed;
  }

  // Otherwise do real work:
  result = create_efd_node(target->h.type, target->h.name);

#ifdef DEBUG
  if (!efd_is_container_node(result) && args != NULL) {
    fqn = efd_build_fqn(target);
    fprintf(
      stderr,
      "Warning: non-NULL args to evaluation of non-container node '%.*s'.\n",
      (int) s_get_length(fqn),
      s_raw(fqn)
    );
    cleanup_string(fqn);
  }
#endif

  // Set up this node for link resolution via the parent of the original node:
  if (set_parent != NULL) {
    result->h.parent = (efd_node*) set_parent;
  } else {
    result->h.parent = target->h.parent;
  }

  // Handle children:
  if (efd_is_container_node(result)) {
    if (args != NULL) {
      efd_add_child(result, copy_efd_node(args));
    }
    children = efd_children_dict(target);
    for (i = 0; i < d_get_count(children); ++i) {
      child = (efd_node*) d_get_item(children, i);
      new_child = efd_eval_in_context(child, NULL, result);
      efd_add_child(result, new_child);
    }
  }

  // Handle node contents:
  switch (result->h.type) {
    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
#ifdef DEBUG // should be impossible
      fqn = efd_build_fqn(target);
      fprintf(
        stderr,
        "ERROR: efd_concrete resulted in a link node '%.*s' "
        "during evaluation.\n",
        (int) s_get_length(fqn),
        s_raw(fqn)
      );
      exit(EXIT_FAILURE);
#endif // else fall through:

    default:
    case EFD_NT_INVALID:
      fqn = efd_build_fqn(target);
      fprintf(
        stderr,
        "ERROR: encountered invalid node '%.*s' (type %d) during evaluation.\n",
        (int) s_get_length(fqn),
        s_raw(fqn),
        result->h.type
      );
      exit(EXIT_FAILURE);

    case EFD_NT_PROTO:
      fqn = efd_build_fqn(target);
      fprintf(
        stderr,
        "ERROR: encountered packed node '%.*s' during evaluation.\n",
        (int) s_get_length(fqn),
        s_raw(fqn)
      );
      exit(EXIT_FAILURE);

    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
      // Nothing to do (children were already copied above)
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
      // children have already been evaluated
      result->b.as_function.function = copy_string(
        target->b.as_function.function
      );
      // Call the specified function and use its result in place of the default
      // result (after cleaning up the default result, of course):
      transformed = efd_lookup_function(result->b.as_function.function)(result);
      cleanup_efd_node(result);
      result = transformed;
      break;

    case EFD_NT_OBJECT:
      result->b.as_object.format = copy_string(target->b.as_object.format);
      result->b.as_object.value = efd_lookup_copier(result->b.as_object.format)(
        target->b.as_object.value
      );
      break;

    case EFD_NT_INTEGER:
      result->b.as_integer.value = target->b.as_integer.value;
      break;

    case EFD_NT_NUMBER:
      result->b.as_number.value = target->b.as_number.value;
      break;

    case EFD_NT_STRING:
      result->b.as_string.value = copy_string(target->b.as_string.value);
      break;

    case EFD_NT_ARRAY_INT:
      result->b.as_int_array.count = target->b.as_int_array.count;
      result->b.as_int_array.values = (efd_int_t*) malloc(
        result->b.as_int_array.count * sizeof(efd_int_t)
      );
      for (i = 0; i < result->b.as_int_array.count; ++i) {
        result->b.as_int_array.values[i] = target->b.as_int_array.values[i];
      }
      break;

    case EFD_NT_ARRAY_NUM:
      result->b.as_num_array.count = target->b.as_num_array.count;
      result->b.as_num_array.values = (efd_num_t*) malloc(
        result->b.as_num_array.count * sizeof(efd_num_t)
      );
      for (i = 0; i < result->b.as_num_array.count; ++i) {
        result->b.as_num_array.values[i] = target->b.as_num_array.values[i];
      }
      break;

    case EFD_NT_ARRAY_STR:
      result->b.as_str_array.count = target->b.as_str_array.count;
      result->b.as_str_array.values = (string**) malloc(
        result->b.as_str_array.count * sizeof(string*)
      );
      for (i = 0; i < result->b.as_str_array.count; ++i) {
        result->b.as_str_array.values[i] = copy_string(
          target->b.as_str_array.values[i]
        );
      }
      break;
  }
  return result;
}

void efd_add_crossref(efd_index *cr, efd_bridge *bridge) {
  l_append_element(cr->unprocessed, bridge);
}

// Private iterator:
void _iter_efd_unpack_children(void *v_child, void *v_cr) {
  efd_unpack_node((efd_node*) v_child, (efd_index*) v_cr);
}

void efd_unpack_node(efd_node *root, efd_index *cr) {
  void *obj;
  efd_proto *p;
  efd_object *o;

  // First unpack any children recursively:
  if (efd_is_container_node(root)) {
    d_witheach(
      efd_children_dict(root),
      (void*) cr,
      &_iter_efd_unpack_children
    );
  }

  // Transform this node into an OBJECT if necessary:
  if (root->h.type == EFD_NT_PROTO) {
    root->h.type = EFD_NT_OBJECT; // change the node type
    p = &(root->b.as_proto);
    efd_unpack_node(p->input, cr); // First recursively unpack the input
    obj = efd_lookup_unpacker(p->format)(p->input); // unpack
    cleanup_efd_node(p->input); // free now-unnecessary EFD data
    o = &(root->b.as_object);
    // format field should overlap perfectly and thus need no change
    o->value = obj; // o->value is in the same place as p->input
  } // else just need to process globals
  // TODO: Process global links!
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
    n = efd_lookup_packer(o->format)(o->value); // pack
    efd_lookup_destructor(o->format)(o->value); // free unpacked data
    efd_pack_node(n, cr); // recursively pack the results
    p = &(root->b.as_proto);
    // format field should overlap perfectly and thus need no change
    p->input = n; // p->input is in the same place as o->value
  } // else do nothing
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
  void **v;
  v = f_as_p(&value);
  d_set_value_s(EFD_NUM_GLOBALS, key, *v);
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
  efd_node *node, *scope;
  efd_generator_state *sub;
  list *children;

  switch (gen->type) {
    default:
    case EFD_GT_INVALID:
      fprintf(
        stderr,
        "Warning: Attempt to get next value from INVALID generator.\n"
      );
      return NULL;

    case EFD_GT_CHILDREN:
      node = (efd_node*) gen->state;
      if (gen->index < efd_normal_child_count(node)) {
        return copy_efd_node(efd_nth(node, gen->index++));
      } else {
        return NULL;
      } // all paths return

    case EFD_GT_INDICES:
      node = (efd_node*) gen->state;
      switch (node->h.type) {
        default:
          fprintf(
            stderr,
            "Warning: Array index generator has invalid non-array node state.\n"
          );
          return NULL;
        case EFD_NT_ARRAY_INT:
          return construct_efd_int_node(
            gen->name,
            (*efd__ai(node))[gen->index++]
          );
        case EFD_NT_ARRAY_NUM:
          return construct_efd_num_node(
            gen->name,
            (*efd__an(node))[gen->index++]
          );
        case EFD_NT_ARRAY_STR:
          return construct_efd_str_node(
            gen->name,
            (*efd__as(node))[gen->index++]
          );
      } // all paths return

    case EFD_GT_FUNCTION:
      return ((efd_generator_function) gen->state)(gen);

    case EFD_GT_EXTEND_RESTART:
      sub = (efd_generator_state*) gen->state;
      node = efd_gen_next(sub);
      if (node == NULL) {
        efd_gen_reset(sub);
        node = efd_gen_next(sub);
      }
      efd_rename(node, gen->name);
      return node;

    case EFD_GT_EXTEND_HOLD:
      sub = (efd_generator_state*) gen->state;
      node = efd_gen_next(sub);
      if (node == NULL) {
        return copy_efd_node((efd_node*) gen->stash);
      }
      if (gen->stash != NULL) {
        cleanup_v_efd_node(gen->stash);
      }
      gen->stash = (void*) copy_efd_node_as(node, gen->name);
      efd_rename(node, gen->name);
      return node;

    case EFD_GT_PARALLEL:
      children = (list*) gen->state;
      scope = create_efd_node(EFD_NT_SCOPE, gen->name);
      for (i = 0; i < l_get_length(children); ++i) {
        sub = (efd_generator_state*) l_get_item(children, i);
        node = efd_gen_next(sub);
        if (node == NULL) {
          cleanup_efd_node(scope);
          return NULL;
        } else {
          efd_add_child(scope, node);
        }
      }
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
      //reset each sub-generator
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
  }
  return result;
}

efd_generator_state * efd_generator_for(efd_node *node) {
  switch (node->h.type) {
    default:
    case EFD_NT_INVALID:
      return NULL;

    case EFD_NT_CONTAINER:
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
      return efd_lookup_generator(node->b.as_function.function)(node);
  }
}
