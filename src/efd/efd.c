// efd.c
// Definition of the Elf Forest Data format.

#include "efd.h"

#include <stdio.h>
#include <errno.h>

/*************
 * Constants *
 *************/

char const * const * const EFD_NT_NAMES = {
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
  "<invalid>"
};

char const * const * const EFD_NT_ABBRS = {
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
  "!"
};

char const * const EFD_PROTO_NAME = "-";
efd_node *EFD_ROOT = NULL;

/******************************
 * Constructors & Destructors *
 ******************************/

efd_node* create_efd_node(efd_node_type t, char* name) {
  efd_node *result = (efd_node*) malloc(sizeof(efd_node));
  result->h.type = t;
  switch (t == EFD_NT_CONTAINER) {
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
      n->b.as_obj_array.count = 0;
      n->b.as_obj_array.values = NULL;
      break;

    case EFD_NT_ARRAY_INT:
      n->b.as_int_array.count = 0;
      n->b.as_int_array.values = NULL;
      break;

    case EFD_NT_ARRAY_NUM:
      n->b.as_num_array.count = 0;
      n->b.as_num_array.values = NULL;
      break;

    case EFD_NT_ARRAY_STR:
      n->b.as_str_array.count = 0;
      n->b.as_str_array.values = NULL;
      break;
  }
  strncpy(result->h.name, name, EFD_NODE_NAME_SIZE - 1);
  result->h.name[EFD_NODE_NAME_SIZE-1] = '\0';
  result->h.parent = NULL;
  return result;
}

// Private helper for looping over children and cleaning them up:
void _iter_efd_cleanup_child(void *v_child) {
  efd_node* child = (efd_node*) v_child;
  cleanup_efd_node(child);
}

void cleanup_efd_node(efd_node *n) {
  // Special-case cleanup:
  switch (n->h.type) {
    default:
    case EFD_NT_INTEGER:
    case EFD_NT_NUMBER:
      // do nothing
      break;

    case EFD_NT_CONTAINER:
      // Clean up all of our children:
      l_foreach(n->b.as_container.children, &_iter_efd_cleanup_child);
      cleanup_list(n->b.as_container.children);
      // Note that the schema is not cleaned up.
      break;

    case EFD_NT_PROTO:
      if (n->b.as_proto.input != NULL) {
        cleanup_efd_node(n->b.as_proto.input);
      }
      break;

    case EFD_NT_OBJECT:
      efd_lookup_destructor(n->b.as_object.format)(n->b.as_object.value);
      // the destructor must call free if needed
      break;

    case EFD_NT_STRING:
      if (n->b.as_string.value != NULL) {
        cleanup_string(n->b.as_string.value);
      }
      break;

    case EFD_NT_ARRAY_OBJ:
      if (n->b.as_obj_array.values != NULL) {
        free(n->b.as_obj_array.values);
      }
      break;

    case EFD_NT_ARRAY_INT:
      if (n->b.as_int_array.values != NULL) {
        free(n->b.as_int_array.values);
      }
      break;

    case EFD_NT_ARRAY_NUM:
      if (n->b.as_num_array.values != NULL) {
        free(n->b.as_num_array.values);
      }
      break;

    case EFD_NT_ARRAY_STR:
      if (n->b.as_str_array.values != NULL) {
        free(n->b.as_str_array.values);
      }
      break;
  }
  // If this node is a child, remove it from its parent's list of children:
  if (n->h.parent != NULL) {
    efd_remove_child(n->h.parent->b.as_container.children, n);
  }
  // Finally free the memory for this node:
  free(n);
}

efd_schema* create_efd_schema(efd_node_type t, char* name) {
  efd_schema *result = (efd_schema*) malloc(sizeof(efd_schema));
  result.type = t;
  strncpy(result.name, name, EFD_NODE_NAME_SIZE - 1);
  result.name[EFD_NODE_NAME_SIZE-1] = '\0';
  result.parent = NULL;
  result.children = create_list();
  return result;
}

// Private helper for looping over children and cleaning them up:
void _iter_efd_cleanup_schema_child(void *v_child) {
  efd_schema* child = (efd_schema*) v_child;
  cleanup_efd_schema(child);
}

void cleanup_efd_schema(efd_schema *schema) {
  // Free our children:
  l_foreach(schema->children, &_iter_efd_cleanup_schema_child);
  cleanup_list(schema->children);
  // Remove ourselves from our parent's list of children:
  if (schema->parent != NULL) {
    l_remove_item(schema->parent->children, schema);
  }
  // Finally free the schema itself:
  free(schema);
}

/*************
 * Functions *
 *************/

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
        EFD_NODE_NAME_SIZE * EFD_MAX_NAME_DEPTH + 3,
        fqn,
        EFD_NT_NAMES[n->h.type],
        EFD_NT_NAMES[t]
      )
    } else {
      fprintf(
        stderr,
        "Error: EFD node '%.*s' has type '%d' rather than '%d' as required.\n",
        EFD_NODE_NAME_SIZE * EFD_MAX_NAME_DEPTH + 3,
        fqn,
        n->h.type,
        t
      )
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
    return strncmp(*efd_fmt__p(n), fmt, EFD_OBJECT_FORMAT_SIZE) == 0;
  } else if (efd_is_type(n, EFD_NT_OBJECT)) {
    return strncmp(*efd_fmt__o(n), fmt, EFD_OBJECT_FORMAT_SIZE) == 0;
  }
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
    strcat(result, "..") // separator is already there
  }
  return result;
}

void efd_add_child(efd_node *n, efd_node *child) {
  efd_assert_type(n, EFD_NT_CONTAINER);
  l_append_element(n->b.as_container.children, child);
}

void efd_remove_child(efd_node *n, efd_node *child) {
  efd_assert_type(n, EFD_NT_CONTAINER);
  l_remove_item(n->b.as_container.children, child);
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
  return (efd_node*) l_get_item(match);
}

efd_node* efdx(efd_node* root, char const * const keypath) {
  efd_assert_type(root, EFD_NT_CONTAINER);
  static char key[EFD_NODE_NAME_SIZE];
  char *c;
  ptrdiff_t i;
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
  return efd_pack_unnode((efd_node*) v_child);
}

void efd_unpack_node(efd_node *root) {
  void *obj;
  efd_node *input;
  efd_proto *p;
  efd_object *o;
  if (root->h.type == EFD_NT_CONTAINER) {
    l_apply(root->b.as_container.children, &_iter_efd_unpack_children);
  } else if (root->h.type == EFD_NT_PROTO) { // transform into an object
    root->h.type = EFD_NT_OBJECT; // change the node type
    p = &(root->b.as_proto);
    input = efd_unpack_node(p->input); // First recursively unpack the input
    obj = efd_lookup_unpacker(p->format)(input); // unpack
    cleanup_efd_node(p->input); // free now-unnecessary EFD data
    o = &(root->b.as_object);
    // format field should overlap perfectly and thus need no change
    o->value = obj; // o->value is in the same place as p->input
  } // else nothing to do
}

// Private iterator:
void _iter_efd_pack_children(void *v_child) {
  return efd_pack_node((efd_node*) v_child);
}

efd_node* efd_pack_node(efd_node *root) {
  efd_node *n;
  efd_proto *p;
  efd_object *o;
  if (root->h.type == EFD_NT_CONTAINER) { // pack children:
    l_apply(root->b.as_container.children, &_iter_efd_pack_children);
  } else if (root->h.type == EFD_NT_OBJECT) { // transform this into a proto
    root->h.type = EFD_NT_PROTO; // change the node type
    o = &(root->b.as_object);
    n = efd_lookup_packer(o->format)(o->value); // pack
    efd_lookup_destructor(o->format)(o->value); // free unpacked data
    p = &(root->b.as_proto);
    // format field should overlap perfectly and thus need no change
    p->input = n; // p->input is in the same place as o->value
  } else { // no change
    return root;
  }
}

efd_schema* efd_fetch_schema(char const * const name) {
  // TODO: HERE!
  return NULL;
}

void efd_merge_node(efd_node *base, efd_node *victim) {
  // TODO: HERE!
}
