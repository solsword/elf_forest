// efd.c
// Definition of the Elf Forest Data format.

#include "efd.h"
#include "efd_parser.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

/*************
 * Constants *
 *************/

char const * const EFD_NT_NAMES[] = {
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
  "array_obj",
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
  "function_array_object",
  "function_array_integer",
  "function_array_number",
  "function_array_string",
  "generator",
  "generator_object",
  "generator_integer",
  "generator_number",
  "generator_string",
  "generator_array_object",
  "generator_array_integer",
  "generator_array_number",
  "generator_array_string",
  "<invalid>"
};

char const * const EFD_NT_ABBRS[] = {
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
  "ao",
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
  "fao",
  "fai",
  "fan",
  "fas",
  "gg",
  "go",
  "gi",
  "gn",
  "gs",
  "gao",
  "gai",
  "gan",
  "gas",
  "!"
};

char const * const EFD_ANON_NAME = "-";

char const * const EFD_ROOT_NAME = "_";

efd_node *EFD_ROOT = NULL;
efd_scope *EFD_ROOT_SCOPE = NULL;

map *EFD_INT_GLOBALS = NULL;
map *EFD_NUM_GLOBALS = NULL;
map *EFD_STR_GLOBALS = NULL;

/******************************
 * Constructors & Destructors *
 ******************************/

efd_node* create_efd_node(efd_node_type t, char const * const name) {
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
      result->b.as_container.children = create_list();
      break;
    case EFD_NT_FUNCTION:
    case EFD_NT_FN_VOID:
    case EFD_NT_FN_OBJ:
    case EFD_NT_FN_INT:
    case EFD_NT_FN_NUM:
    case EFD_NT_FN_STR:
    case EFD_NT_FN_AR_OBJ:
    case EFD_NT_FN_AR_INT:
    case EFD_NT_FN_AR_NUM:
    case EFD_NT_FN_AR_STR:
    case EFD_NT_GENERATOR:
    case EFD_NT_GN_VOID:
    case EFD_NT_GN_OBJ:
    case EFD_NT_GN_INT:
    case EFD_NT_GN_NUM:
    case EFD_NT_GN_STR:
    case EFD_NT_GN_AR_OBJ:
    case EFD_NT_GN_AR_INT:
    case EFD_NT_GN_AR_NUM:
    case EFD_NT_GN_AR_STR:
      result->b.as_function.children = create_list();
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      result->b.as_link.target = NULL;
      result->b.as_link.children = create_list();
      break;

    case EFD_NT_PROTO:
      result->b.as_proto.format[0] = '\0';
      result->b.as_proto.input = NULL;
      break;

    case EFD_NT_OBJECT:
      result->b.as_object.format[0] = '\0';
      result->b.as_object.value = NULL;
      break;

    case EFD_NT_STRING:
      result->b.as_string.value = NULL;
      break;

    case EFD_NT_ARRAY_OBJ:
      result->b.as_obj_array.count = 0;
      result->b.as_obj_array.values = NULL;
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
  strncpy(result->h.name, name, EFD_NODE_NAME_SIZE - 1);
  result->h.name[EFD_NODE_NAME_SIZE-1] = '\0';
  result->h.parent = NULL;
  return result;
}

CLEANUP_IMPL(efd_node) {
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
      // Clean up all of our children:
      l_foreach(doomed->b.as_container.children, &cleanup_v_efd_node);
      cleanup_list(doomed->b.as_container.children);
      break;

    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
      // Clean up all of our children:
      l_foreach(doomed->b.as_link.children, &cleanup_v_efd_node);
      cleanup_list(doomed->b.as_link.children);
      // Clean up our target address:
      if (doomed->b.as_link.target != NULL) {
        cleanup_efd_address(doomed->b.as_link.target);
      }
      break;

    case EFD_NT_PROTO:
      if (doomed->b.as_proto.input != NULL) {
        cleanup_efd_node(doomed->b.as_proto.input);
      }
      break;

    case EFD_NT_OBJECT:
      df = efd_lookup_destructor(doomed->b.as_object.format);
      if (df != NULL) {
        df(doomed->b.as_object.value);
      }
      // the destructor must call free if needed
      break;

    case EFD_NT_STRING:
      if (doomed->b.as_string.value != NULL) {
        cleanup_string(doomed->b.as_string.value);
      }
      break;

    case EFD_NT_ARRAY_OBJ:
      if (doomed->b.as_obj_array.values != NULL) {
        free(doomed->b.as_obj_array.values);
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
        free(doomed->b.as_str_array.values);
      }
      break;
  }
  // If this node is a child, remove it from its parent's list of children:
  if (doomed->h.parent != NULL) {
    efd_remove_child(doomed->h.parent, doomed);
  }
  // Finally free the memory for this node:
  free(doomed);
}

efd_address* create_efd_address(efd_address* parent, char const * const name) {
  efd_address *result = (efd_address*) malloc(sizeof(efd_address));
  strncpy(result->name, name, EFD_NODE_NAME_SIZE);
  result->name[EFD_NODE_NAME_SIZE] = '\0';
  result->parent = parent;
  result->next = NULL;
  return result;
}

// Private helper which returns an address tail:
efd_address* _construct_efd_node_direct_address(efd_node *node) {
  efd_address *result = (efd_address*) malloc(sizeof(efd_address));
  strncmp(result->name, node->h.name, EFD_NODE_NAME_SIZE);
  result->name[EFD_NODE_NAME_SIZE] = '\0';
  result->parent = NULL;
  result->next = NULL;
  if (node->h.parent != NULL) {
    result->parent = _construct_efd_node_direct_address(node->h.parent);
    result->parent->next = result;
  }
  return result;
}

efd_address* construct_efd_address_of_node(efd_node *node) {
  efd_address *result = _construct_efd_node_direct_address(node);
  while (result->parent != NULL) {
    result = result->parent;
  }
  return result;
}

efd_address* copy_efd_address(efd_address *src) {
  efd_address *n, *rn;
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
  efd_address *n;
  efd_address *nn;
  while (doomed->next != NULL) {
    n = doomed,
    nn = doomed->next;
    while (nn->next != NULL) {
      n = nn;
      nn = nn->next;
    }
    n->next = NULL;
    free(nn);
  }
  free(doomed);
}

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
  efd_path *head, *tail, *next;
  head = NULL;
  tail = NULL;
  while (original != NULL) {
    next = create_efd_path(NULL, original->node, EFD_PET_UNKNOWN);
    if (tail == NULL) { tail = next; }
    if (head != NULL) { head->parent = next; }
    head = next;
    original = original->parent;
  }
  return tail;
}

CLEANUP_IMPL(efd_path) {
  efd_path *parent = doomed->parent;
  free(doomed);
  cleanup_efd_path(parent);
}

efd_reference* create_efd_reference(
  efd_ref_type type,
  efd_address *addr,
  ptrdiff_t idx
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

/*************
 * Functions *
 *************/

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
    case EFD_RT_OBJ_ARR_ENTRY:
      return (
        to == EFD_RT_NODE
     || to == EFD_RT_OBJ
     || to == EFD_RT_OBJ_ARR_ENTRY
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

void efd_assert_type(efd_node *n, efd_node_type t) {
  if (n == NULL) {
    fprintf(stderr, "Error: Missing EFD node in efd_assert_type!\n");
    exit(1);
  }
#ifndef EFD_NO_TYPECHECKS
  if (n->h.type != t) {
    char *fqn = efd_build_fqn(n);
    if (n->h.type >= 0
     && n->h.type < EFD_NUM_TYPES
     && t >= 0
     && t < EFD_NUM_TYPES
    ) {
      fprintf(
        stderr,
        "Error: EFD node '%.*s' has type '%s' rather than '%s' as required.\n",
        (int) (EFD_NODE_NAME_SIZE * EFD_MAX_NAME_DEPTH + 3),
        fqn,
        EFD_NT_NAMES[n->h.type],
        EFD_NT_NAMES[t]
      );
    } else {
      fprintf(
        stderr,
        "Error: EFD node '%.*s' has type '%d' rather than '%d' as required.\n",
        (int) (EFD_NODE_NAME_SIZE * EFD_MAX_NAME_DEPTH + 3),
        fqn,
        n->h.type,
        t
      );
    }
    free(fqn);
    exit(1);
  }
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

int efd_format_is(efd_node *n, char const * const fmt) {
  if (efd_is_type(n, EFD_NT_PROTO)) {
    return strncmp(efd__p_fmt(n), fmt, EFD_ANNOTATION_SIZE) == 0;
  } else if (efd_is_type(n, EFD_NT_OBJECT)) {
    return strncmp(efd__o_fmt(n), fmt, EFD_ANNOTATION_SIZE) == 0;
  }
  // failsafe
  return 0;
}

char* efd_build_fqn(efd_node *n) {
  size_t nd = efd_node_depth(n);
  char * result = (char*) malloc(
    sizeof(char) * EFD_NODE_NAME_SIZE * EFD_MAX_NAME_DEPTH
  + 3 // '...' at the end if needed
  );
  if (result == NULL) {
    perror("Error: ran out of memory while building fully qualified name.\n");
    exit(errno);
  }
  size_t depth = 0;
  for (depth = 0; depth < nd && depth < EFD_MAX_NAME_DEPTH; ++depth) {
    strncat(result, n->h.name, EFD_NODE_NAME_SIZE);
    if (depth < nd - 1) {
      strcat(result, ".");
    }
  }
  if (nd > EFD_MAX_NAME_DEPTH) {
    strcat(result, ".."); // separator is already there
  }
  return result;
}

void efd_add_child(efd_node *n, efd_node *child) {
  efd_assert_type(n, EFD_NT_CONTAINER);
  l_append_element(n->b.as_container.children, child);
}

void efd_remove_child(efd_node *n, efd_node *child) {
  efd_assert_type(n, EFD_NT_CONTAINER);
  l_remove_element(n->b.as_container.children, (void*) child);
}

void efd_append_address(efd_address *a, char const * const name) {
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
    exit(1);
  }
#endif
  e->parent = n;
  n->next = e;
}

void efd_push_address(efd_address *a, char const * const name) {
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
      (int) EFD_NODE_NAME_SIZE, 
      a->name
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
int _match_efd_name(void* v_child, void* v_key) {
  efd_node *child = (efd_node*) v_child;
  char *key = (char*) v_key;
  return strncmp(child->h.name, key, EFD_NODE_NAME_SIZE) == 0;
}

efd_node* efd_find_child(efd_node* parent, char* name) {
  list *children;
  size_t match;
  switch (node->h.type) {
    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
      children = node->b.as_container.children;
      break;
    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
      children = node->b.as_link.children;
      break;
    default:
#ifdef DEBUG
      fprintf(
        stderr,
        "Warning: efd_find_child called with key '%.*s' "
        "on non-container node '%.*s'.\n",
        (int) EFD_NODE_NAME_SIZE,
        name,
        (int) EFD_NODE_NAME_SIZE,
        node->h.name
      );
#endif
      return NULL;
  }
  match = l_scan_indices(
    children,
    (void*) name,
    &_match_efd_name
  );
  if (match < 0) {
    return NULL;
  }
  return (efd_node*) l_get_item(children, match);
}

efd_node* efd_find_variable_in(efd_node* base, efd_address* target) {
  list *children;
  efd_node *child, *result;
  size_t i;
  switch (node->h.type) {
    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
      children = node->b.as_container.children;
      break;
    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
      children = node->b.as_link.children;
      break;
    default:
#ifdef DEBUG
      fprintf(
        stderr,
        "Warning: efd_find_variable_in called with address starting '%.*s' "
        "on non-container node '%.*s'.\n",
        (int) EFD_NODE_NAME_SIZE,
        target->name,
        (int) EFD_NODE_NAME_SIZE,
        base->h.name
      );
#endif
      return NULL;
  }
  result = NULL;
  for (i = 0; i < l_get_length(children); ++i) {
    child = (efd_node*) l_get_item(children, i);
    if (child->h.type == EFD_NT_SCOPE) {
      result = efd(child, target);
      if (result != NULL) {
        break;
      }
    }
  }
  return result;
}

efd_node* efd_resolve_variable(efd_scope* path) {
  efd_address *target;
  efd_node *result, *test;

  target = var->b.as_link.target;

  result = efd_find_variable_in(scope->node, target);
  while (scope != NULL && result == NULL) {
    result = efd_find_variable_in(scope->node, target);
    scope = scope->parent;
  }
  // Keep searching any links leading directly to the place we got a hit, using
  // the *earliest* of them that also hits if one does.
  while (scope != NULL && efd_is_link_node(scope->node)) {
    test = efd_find_variable_in(scope->node, target);
    if (test != NULL) {
      result = test;
    }
    scope = scope->parent;
  }
  return result;
}

efd_scope* efd_concrete(efd_scope* path) {
  efd_node *here = path->node;
  efd_scope *result;
  switch (node->h.type) {
    default:
      result = path;
    case EFD_NT_LINK:
      result = efd_concrete(efd(EFD_ROOT_SCOPE, root->b.as_link.target));
    case EFD_NT_LOCAL_LINK:
      result = efd_concrete(
        efd(construct_efd_scope_for(here->h.parent), node->b.as_link.target, inner),
        inner
      );
    case EFD_NT_VARIABLE:
      result = efd_concrete(efd_resolve_variable(node, scope));
  }
}

efd_path* efd_lookup(efd_path const * const p_base, char const * const key) {
  efd_node *here = p_base->node;
  efd_node *linked_node;
  efd_node *result;
  if (key[0] == EFD_ADDR_PARENT && key[1] == '\0') {
    // Special handling for 'parent' path entries:
    return create_efd_path(
      copy_efd_path(p_base),
      here->h.parent,
      EFD_PET_PARENT
    );
  } else {
    // Normal lookups:
    switch (here->h.type) {
      case EFD_NT_CONTAINER:
        return efd_find_child(here, key);
      case EFD_NT_LINK:
      case EFD_NT_LOCAL_LINK:
        // First search within the linking node, so keys there overwrite keys
        // at the link destination.
        result = efd_find_child(here, key);
        if (result == NULL) {
          // Look up in the linked node if a key wasn't overwritten.
          if (here->h.type == EFD_NT_LOCAL_LINK) { // local link
            linked_node = efd(here, here->b.as_link.target);
          } else { // global link
            linked_node = efd(EFD_ROOT, here->b.as_link.target);
          }
          return efd_lookup(linked_node, key);
        } else {
          return result;
        }
      default:
#ifdef DEBUG
        fprintf(
          stderr,
          "ERROR: Attempt to find '%.*s' in node '%.*s' "
          "which isn't a container.\n",
          (int) EFD_NODE_NAME_SIZE,
          key,
          (int) EFD_NODE_NAME_SIZE,
          here->h.name
        );
#endif
        return NULL;
    }
  }
}

efd_path* efd(efd_path const * const p_base, efd_address* addr) {
  efd_path *here = copy_efd_path(p_base);
  efd_path *next = here;
  while (addr != NULL) {
    // TODO: Better here!
    next = efd_lookup(here, addr->name);
    cleanup_efd_path(here);
    here = next;
    addr = addr->next;
  }
  return here;
}

efd_path* efdx(efd_path const * const p_base, char const * const saddr) {
  return efd(p_base, efd_parse_string_address(saddr));
}

efd_node* efd_eval(efd_path const * const target, efd_node const * const args) {
  size_t i;
  efd_node *result;
  efd_node *child, *new_child;
  switch (target->h.type) {
    default:
    case EFD_NT_CONTAINER:
      result = create_efd_node(EFD_NT_CONTAINER, target->h.name);
      for (i = 0; i < l_get_length(target->b.as_container.children); ++i) {
        child = (efd_node*) l_get_item(target->b.as_container.children, i);
        new_child = efd_eval(child, args);
      }
  }
  return NULL;
  // TODO: HERE
}

void efd_add_crossref(efd_index *cr, efd_bridge* bridge) {
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
  if (root->h.type == EFD_NT_CONTAINER) {
    l_witheach(
      root->b.as_container.children,
      (void*) cr,
      &_iter_efd_unpack_children
    );
  } else if (root->h.type == EFD_NT_PROTO) { // transform into an object
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
  if (root->h.type == EFD_NT_CONTAINER) { // pack children:
    l_witheach(
      root->b.as_container.children,
      (void*) cr,
      &_iter_efd_pack_children
    );
  } else if (root->h.type == EFD_NT_OBJECT) { // transform this into a proto
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

// Helper for transforming character keys into map x/y/z keys. r_key should be
// a ptrdiff_t[EFD_GLOBALS_KEY_ARITY].
void _get_global_key(
  char const * const key,
  uintptr_t *r_key
) {
  memset(
    (char*) r_key,
    0,
    EFD_GLOBALS_KEY_ARITY * sizeof(uintptr_t) / sizeof(char)
  );
  strncpy(
    (char*) r_key,
    key,
    EFD_GLOBALS_KEY_ARITY * sizeof(uintptr_t) / sizeof(char)
  );
}

void * _efd_get_global(map *m, char const * const key) {
  uintptr_t mkey[EFD_GLOBALS_KEY_ARITY];
  _get_global_key(key, mkey);
  switch (EFD_GLOBALS_KEY_ARITY) {
    case 1:
      return m1_get_value(
        m,
        (void*) mkey[0]
      );
    case 2:
      return m2_get_value(
        m,
        (void*) mkey[0],
        (void*) mkey[1]
      );
    case 3:
      return m3_get_value(
        m,
        (void*) mkey[0],
        (void*) mkey[1],
        (void*) mkey[2]
      );
    case 4:
      return m_get_value(
        m,
        (void*) mkey[0],
        (void*) mkey[1],
        (void*) mkey[2],
        (void*) mkey[3]
      );
    case 5:
      return m_get_value(
        m,
        (void*) mkey[0],
        (void*) mkey[1],
        (void*) mkey[2],
        (void*) mkey[3],
        (void*) mkey[4]
      );
    default:
      fprintf(
        stderr,
        "ERROR: Invalid EFD global key arity %d!\n",
        EFD_GLOBALS_KEY_ARITY
      );
      exit(1);
  }
}

void * _efd_set_global(map *m, char const * const key, void *value) {
  uintptr_t mkey[EFD_GLOBALS_KEY_ARITY];
  _get_global_key(key, mkey);
  switch (EFD_GLOBALS_KEY_ARITY) {
    case 1:
      return m1_put_value(
        m,
        (void*) value,
        (void*) mkey[0]
      );
    case 2:
      return m2_put_value(
        m,
        (void*) value,
        (void*) mkey[0],
        (void*) mkey[1]
      );
    case 3:
      return m3_put_value(
        m,
        (void*) value,
        (void*) mkey[0],
        (void*) mkey[1],
        (void*) mkey[2]
      );
    case 4:
      return m_put_value(
        m,
        (void*) value,
        (void*) mkey[0],
        (void*) mkey[1],
        (void*) mkey[2],
        (void*) mkey[3]
      );
    case 5:
      return m_put_value(
        m,
        (void*) value,
        (void*) mkey[0],
        (void*) mkey[1],
        (void*) mkey[2],
        (void*) mkey[3],
        (void*) mkey[4]
      );
    default:
      fprintf(
        stderr,
        "ERROR: Invalid EFD global key arity %d!\n",
        EFD_GLOBALS_KEY_ARITY
      );
      exit(1);
  }
}

ptrdiff_t efd_get_global_i(char const * const key) {
  return (ptrdiff_t) _efd_get_global(EFD_INT_GLOBALS, key);
}

float efd_get_global_n(char const * const key) {
  void *r = _efd_get_global(EFD_NUM_GLOBALS, key);
  return *((float*) &r);
}

string* efd_get_global_s(char const * const key) {
  return (string*) _efd_get_global(EFD_STR_GLOBALS, key);
}

void efd_set_global_i(char const * const key, ptrdiff_t value) {
  _efd_set_global(EFD_INT_GLOBALS, key, (void*) value);
}

void efd_set_global_n(char const * const key, float value) {
  uintptr_t v = 0;
  v = *((uintptr_t*) &value); // TODO: Safer float <-> void* conversion?
  _efd_set_global(EFD_NUM_GLOBALS, key, (void*) v);
}

void efd_set_global_s(char const * const key, string* value) {
  string *tmp = (string*) _efd_set_global(EFD_STR_GLOBALS, key, (void*) value);
  if (tmp != NULL) {
    cleanup_string(tmp);
  }
}

void dont_cleanup(void* v) {
  return;
}
