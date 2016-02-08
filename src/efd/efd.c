// efd.c
// Definition of the Elf Forest Data format.

#include "efd.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

/*************
 * Constants *
 *************/

char const * const EFD_NT_NAMES[] = {
  "container",
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
  "<invalid>"
};

char const * const EFD_NT_ABBRS[] = {
  "c",
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
  "!"
};

char const * const EFD_PROTO_NAME = "-";

char const * const EFD_ROOT_NAME = "_";

efd_node *EFD_ROOT = NULL;

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
      result->b.as_container.children = create_list();
      result->b.as_container.schema = NULL;
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
      // Note that the schema is not cleaned up.
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

efd_schema* create_efd_schema(efd_node_type t, char* name) {
  efd_schema *result = (efd_schema*) malloc(sizeof(efd_schema));
  result->type = t;
  strncpy(result->name, name, EFD_NODE_NAME_SIZE - 1);
  result->name[EFD_NODE_NAME_SIZE-1] = '\0';
  result->parent = NULL;
  result->children = create_list();
  return result;
}

CLEANUP_IMPL(efd_schema) {
  // Free our children:
  l_foreach(doomed->children, &cleanup_v_efd_schema);
  cleanup_list(doomed->children);
  // Remove ourselves from our parent's list of children:
  if (doomed->parent != NULL) {
    l_remove_element(doomed->parent->children, (void*) doomed);
  }
  // Finally free the doomed itself:
  free(doomed);
}

efd_address* create_efd_address(char const * const name) {
  efd_address *result = (efd_address*) malloc(sizeof(efd_address));
  strncpy(result->name, name, EFD_NODE_NAME_SIZE);
  result->name[EFD_NODE_NAME_SIZE] = '\0';
  return result;
}

efd_address* copy_efd_address(efd_address *src) {
  efd_address *n, *rn;
  efd_address *result;
  n = src;
  result = create_efd_address(n->name);
  rn = result;
  while (n->next != NULL) {
    rn->next = create_efd_address(n->next->name);
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
    return strncmp(efd_fmt__p(n), fmt, EFD_OBJECT_FORMAT_SIZE) == 0;
  } else if (efd_is_type(n, EFD_NT_OBJECT)) {
    return strncmp(efd_fmt__o(n), fmt, EFD_OBJECT_FORMAT_SIZE) == 0;
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
  efd_address *new = create_efd_address(name);
  efd_extend_address(a, new);
}

void efd_extend_address(efd_address *a, efd_address *e) {
  efd_address *n = a;
  while (n->next != NULL) {
    n = n->next;
  }
  n->next = e;
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

efd_node* efd(efd_node* root, char const * const key) {
  efd_assert_type(root, EFD_NT_CONTAINER);
  ptrdiff_t match = l_scan_indices(
    root->b.as_container.children,
    (void*) key,
    &_match_efd_name
  );
  if (match < 0) { return NULL; }
  return (efd_node*) l_get_item(root->b.as_container.children, match);
}

efd_node* efdx(efd_node* root, char const * const keypath) {
  efd_assert_type(root, EFD_NT_CONTAINER);
  static char key[EFD_NODE_NAME_SIZE];
  char const *c;
  ptrdiff_t i = 0;
  for (
    c = keypath;
    (
      *c != '\0'
   && (c - keypath) / sizeof(char) < EFD_NODE_NAME_SIZE * EFD_MAX_NAME_DEPTH
    );
    ++c
  ) {
    key[i] = *c;
    if (*c == EFD_NODE_SEP) {
      key[i] = '\0'; // cut it off here
      root = efd(root, key); // update root
      if (root == NULL) {
        // TODO: Throw error here since we have good context?
        return NULL;
      }
      i = -1; // reset iterator (it's about to be incremented back to 0)
    } else if (i > EFD_NODE_NAME_SIZE - 2) { // leave room for the '\0'
      // TODO: Throw error here as well! (make sure to change cond to -1)
      i -= 1; // effectively just truncate the key...
    }
    i += 1;
  }
  key[i] = '\0'; // match the terminator of keypath
  root = efd(root, key); // final lookup
  return root;
}

efd_node* efdr(char const * const keypath) {
  return efdx(EFD_ROOT, keypath);
}

// Private iterator:
void _iter_efd_unpack_children(void *v_child) {
  efd_unpack_node((efd_node*) v_child);
}

void efd_unpack_node(efd_node *root) {
  void *obj;
  efd_proto *p;
  efd_object *o;
  if (root->h.type == EFD_NT_CONTAINER) {
    l_foreach(root->b.as_container.children, &_iter_efd_unpack_children);
  } else if (root->h.type == EFD_NT_PROTO) { // transform into an object
    root->h.type = EFD_NT_OBJECT; // change the node type
    p = &(root->b.as_proto);
    efd_unpack_node(p->input); // First recursively unpack the input
    obj = efd_lookup_unpacker(p->format)(p->input); // unpack
    cleanup_efd_node(p->input); // free now-unnecessary EFD data
    o = &(root->b.as_object);
    // format field should overlap perfectly and thus need no change
    o->value = obj; // o->value is in the same place as p->input
  } // else nothing to do
}

// Private iterator:
void _iter_efd_pack_children(void *v_child) {
  efd_pack_node((efd_node*) v_child);
}

void efd_pack_node(efd_node *root) {
  efd_node *n;
  efd_proto *p;
  efd_object *o;
  if (root->h.type == EFD_NT_CONTAINER) { // pack children:
    l_foreach(root->b.as_container.children, &_iter_efd_pack_children);
  } else if (root->h.type == EFD_NT_OBJECT) { // transform this into a proto
    root->h.type = EFD_NT_PROTO; // change the node type
    o = &(root->b.as_object);
    n = efd_lookup_packer(o->format)(o->value); // pack
    efd_lookup_destructor(o->format)(o->value); // free unpacked data
    p = &(root->b.as_proto);
    // format field should overlap perfectly and thus need no change
    p->input = n; // p->input is in the same place as o->value
  } // else do nothing
}

efd_schema* efd_fetch_schema(char const * const name) {
  // TODO: HERE!
  return NULL;
}

void efd_merge_node(efd_node *base, efd_node *victim) {
  // TODO: HERE!
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
