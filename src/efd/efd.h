#ifndef INCLUDE_EFD_H
#define INCLUDE_EFD_H

// efd.h
// Definition of the Elf Forest Data format.

#include <stdint.h>

#include "datatypes/list.h"
#include "datatypes/string.h"

/**************************
 * Enums & Function Types *
 **************************/

// Raw types that a single EFD node can take on (different from schemas, which
// involve the structure of multiple nodes).
#define EFD_NUM_TYPES 10
enum efd_node_type_e {
  EFD_NT_CONTAINER  = 0,   // 'c'   no data, just children
  EFD_NT_PROTO      = 1,   //  -    raw object data pre-assembly
  EFD_NT_OBJECT     = 2,   // 'o'   automatic parse-to-struct
  EFD_NT_INTEGER    = 3,   // 'i'   ptrdiff_t
  EFD_NT_NUMBER     = 4,   // 'n'   float
  EFD_NT_STRING     = 5,   // 's'   quoted string
  EFD_NT_ARRAY_OBJ  = 6,   // 'ao'  array of void*
  EFD_NT_ARRAY_INT  = 7,   // 'ai'  array of ptrdiff_t
  EFD_NT_ARRAY_NUM  = 8,   // 'an'  array of float
  EFD_NT_ARRAY_STR  = 9    // 'as'  array of quoted strings
};
typedef enum efd_node_type_e efd_node_type;

typedef void* (*efd_unpack_function)(efd_node *);
typedef efd_node* (*efd_pack_function)(void *);
typedef void (*efd_destroy_function)(void *);

/**************
 * Structures *
 **************/

// A common node header:
struct efd_node_header_s;
typedef struct efd_node_header_s efd_node_header;

// Various specific node bodies:
struct efd_container_s;
typedef struct efd_container_s efd_container;

struct efd_proto_s;
typedef struct efd_proto_s efd_proto;

struct efd_object_s;
typedef struct efd_object_s efd_object;

struct efd_integer_s;
typedef struct efd_integer_s efd_integer;

struct efd_number_s;
typedef struct efd_number_s efd_number;

struct efd_string_s;
typedef struct efd_string_s efd_string;

struct efd_array_obj_s;
typedef struct efd_array_obj_s efd_array_obj;

struct efd_array_int_s;
typedef struct efd_array_int_s efd_array_int;

struct efd_array_num_s;
typedef struct efd_array_num_s efd_array_num;

struct efd_array_str_s;
typedef struct efd_array_str_s efd_array_str;

// A union of the above:
union efd_node_body_u;
typedef union efd_node_body_u efd_node_body;

// A generic node struct w/ header and labeled body type:
struct efd_node_s;
typedef union efd_node_s efd_node;

/*************
 * Constants *
 *************/

#define EFD_NODE_NAME_SIZE 32
#define EFD_MAX_NAME_DEPTH 32

#define EFD_OBJECT_FORMAT_SIZE 8

extern char const * const * const EFD_NT_NAMES;
extern char const * const * const EFD_NT_ABBRS;

extern efd_node *EFD_ROOT;

/*************************
 * Structure Definitions *
 *************************/

struct efd_node_header_s {
  efd_node_type type;
  char name[EFD_NODE_NAME_SIZE];
  efd_node *parent;
};

struct efd_container_s {
  list *children;
  efd_schema *schema; // optional, NULL if not present
};

struct efd_proto_s {
  char format[EFD_OBJECT_FORMAT_SIZE];
  efd_node *input;
};

struct efd_object_s {
  char format[EFD_OBJECT_FORMAT_SIZE];
  void *value;
};

struct efd_integer_s {
  ptrdiff_t value;
};

struct efd_number_s {
  float value;
};

struct efd_string_s {
  string *value;
};

struct efd_array_obj_s {
  size_t count;
  void *values;
};

struct efd_array_int_s {
  size_t count;
  ptrdiff_t *values;
};

struct efd_array_num_s {
  size_t count;
  float *values;
};

struct efd_array_str_s {
  size_t count;
  string *values;
};

union efd_node_body_u {
  efd_container as_container;
  efd_proto as_proto;
  efd_object as_object;
  efd_integer as_integer;
  efd_number as_number;
  efd_string as_string;
  efd_array_obj as_obj_array;
  efd_array_int as_int_array;
  efd_array_num as_num_array;
  efd_array_str as_str_array;
};

struct efd_node_s {
  efd_node_header h;
  efd_node_body b;
};

struct efd_schema_s {
  efd_node_type type;
  char name[EFD_NODE_NAME_SIZE];
  efd_schema *parent;
  list *children;
};

/********************
 * Inline Functions *
 ********************/

static inline size_t efd_node_depth(efd_node *n) {
  size_t result = 0;
  do {
    if (n.h.parent == NULL) {
      return result;
    } else {
      result += 1;
      n = n.h.parent;
    }
  } while (1)
}

static inline void* efd__o(efd_node *n) {
  efd_assert_type(n, EFD_NT_OBJECT);
  return n->b.as_object.value;
}

static inline ptrdiff_t efd__i(efd_node *n) {
  efd_assert_type(n, EFD_NT_INTEGER);
  return n->b.as_integer.value;
}

static inline float efd__n(efd_node *n) {
  efd_assert_type(n, EFD_NT_NUMBER);
  return n->b.as_number.value;
}

static inline string* efd__s(efd_node *n) {
  efd_assert_type(n, EFD_NT_STRING);
  return n->b.as_string.value;
}

static inline void* efd__ao(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_OBJ);
  return n->b.as_obj_array.values;
}

static inline size_t efd_count__ao(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_OBJ);
  return n->b.as_obj_array.count;
}

static inline ptrdiff_t* efd__ai(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_INT);
  return n->b.as_int_array.values;
}

static inline size_t efd_count__ai(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_INT);
  return n->b.as_int_array.count;
}

static inline float* efd__an(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_NUM);
  return n->b.as_num_array.value;
}

static inline size_t efd_count__an(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_NUM);
  return n->b.as_num_array.count;
}

static inline string* efd__as(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_STR);
  return n->b.as_str_array.value;
}

static inline size_t efd_count__as(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_STR);
  return n->b.as_str_array.count;
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocate and return a new EFD node of the given type:
efd_node* create_efd_node(efd_node_type t, char* name);

// Clean up memory from the given EFD node.
void cleanup_efd_node(efd_node *node);

// Allocate and return a new EFD schema of the given type:
efd_schema* create_efd_schema(efd_node_type t, char* name);

// Clean up memory from the given EFD schema.
void cleanup_efd_schema(efd_schema *schema);

/*************
 * Functions *
 *************/

// Asserts that a type matches and throws an error if not. Can be turned off by
// defining EDF_NO_TYPECHECKS.
void efd_assert_type(efd_node *n, efd_node_type t);

// Builds a fully-qualified name for the given node and returns a pointer to
// newly-allocated memory holding this name. Note that this will only
// accommodate names up to a maximum depth of EFD_MAX_NAME_DEPTH nestings.
char* efd_build_fqn(efd_node *n);

// Removes the given child from this node's list of children (this node must be
// a container node).
void efd_remove_child(efd_node *n, efd_node *child);

// The most ubiquitous EFD function 'efd' just gets a child node out of its
// parent by name. It iterates over children until it hits one with a matching
// name, so only the first is returned if multiple children share a name. The
// key argument is treated as a single key within the given parent node.
efd_node* efd(efd_node* root, char const * const key);

// Works like 'efd,' except that the keypath argument may contain multiple keys
// separated by '.' characters, representing multiple lookups within a nested
// structure. So
//
//   efdx(r, "foo.bar.baz");
//
// is equivalent to
//
//   efd(edf(efd(r, "foo"), "bar"), "baz");
//
// but obviously much nicer-looking. Note that the use of sprintf and passing a
// dynamic key to efdx is probably better solved by a mix of calls to efd and
// efdx to avoid buffer-length problems. Note that efdx is limited by
// EFD_MAX_NAME_DEPTH, although multiple calls to efd/efdx can overcome this.
efd_node* efdx(efd_node* root, char const * const keypath);

// Like the 'efdx' function, except that it uses EFD_ROOT as the root instead
// of taking a node as an argument.
efd_node* efdr(char const * const keypath);

// Unpacks this node and all of its children recursively, turning PROTO nodes
// contianing raw EFD into OBJECT nodes containing unpacked structs.
// TODO: Other adjustment steps:
//   1. schema lookup
//   2. reference resolution
efd_node* efd_unpack_node(efd_node *root);

// Packs this node and all of its children recursively, turning OBJECT nodes containing unpacked structs into PROTO nodes containing EFD COLLECTIONs.
efd_node* efd_pack_node(efd_node *root);

// Lookups for packers, unpackers, and destructors. Note that these are not
// actually defined in efd.c but rather in  efd_setup.c.
efd_unpack_function efd_lookup_unpacker(char const * const key);
efd_pack_function efd_lookup_packer(char const * const key);
efd_destroy_function efd_lookup_destructor(char const * const key);

#endif // INCLUDE_EFD_H
