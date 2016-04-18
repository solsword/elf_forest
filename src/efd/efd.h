#ifndef INCLUDE_EFD_H
#define INCLUDE_EFD_H

// efd.h
// Definition of the Elf Forest Data format.

#include <stdint.h>

#include "datatypes/list.h"
#include "datatypes/string.h"
#include "datatypes/map.h"

#include "boilerplate.h"

/********
 * Meta *
 ********/

#define EFD_GL(x, y) y

/*********
 * Enums *
 *********/

// Raw types that a single EFD node can take on:
#define EFD_NUM_TYPES 36
enum efd_node_type_e {
  EFD_NT_INVALID    = 0,   //  -    marks an invalid node internally
  EFD_NT_CONTAINER  = 1,   // 'c'   no data, just children
  EFD_NT_LINK       = 2,   // 'L'   global link
  EFD_NT_LOCAL_LINK = 3,   // 'l'   local link
  EFD_NT_SCOPE      = 4,   // 'V'   scope
  EFD_NT_VARIABLE   = 5,   // 'v'   variable
  EFD_NT_PROTO      = 6,   //  -    raw object data pre-assembly
  EFD_NT_OBJECT     = 7,   // 'o'   automatic parse-to-struct
  EFD_NT_INTEGER    = 8,   // 'i'   ptrdiff_t
  EFD_NT_NUMBER     = 9,   // 'n'   float
  EFD_NT_STRING     = 10,   // 's'   quoted string
  EFD_NT_ARRAY_OBJ  = 11,  // 'ao'  array of void*
  EFD_NT_ARRAY_INT  = 12,  // 'ai'  array of ptrdiff_t
  EFD_NT_ARRAY_NUM  = 13,  // 'an'  array of float
  EFD_NT_ARRAY_STR  = 14,  // 'as'  array of quoted strings
  EFD_NT_GLOBAL_INT = 15,  // 'Gi'  global integer
  EFD_NT_GLOBAL_NUM = 16,  // 'Gn'  global numeric
  EFD_NT_GLOBAL_STR = 17,  // 'Gs'  global string
  EFD_NT_FUNCTION   = 18,  // 'ff'  function (returns a container)
  EFD_NT_FN_VOID    = 19,  // 'fv'  void function (returns NULL)
  EFD_NT_FN_OBJ     = 20,  // 'fo'  function (returns an object)
  EFD_NT_FN_INT     = 21,  // 'fi'  function (returns an integer)
  EFD_NT_FN_NUM     = 22,  // 'fn'  function (returns a number)
  EFD_NT_FN_STR     = 23,  // 'fs'  function (returns a string)
  EFD_NT_FN_AR_OBJ  = 24,  // 'fao' function (returns an array of objects)
  EFD_NT_FN_AR_INT  = 25,  // 'fai' function (returns an array of integers)
  EFD_NT_FN_AR_NUM  = 26,  // 'fan' function (returns an array of numbers)
  EFD_NT_FN_AR_STR  = 27,  // 'fas' function (returns an array of strings)
  EFD_NT_GENERATOR  = 28,  // 'gg'  generator (returns containers)
  EFD_NT_GN_VOID    = 29,  // 'gv'  void generator (returns NULLs)
  EFD_NT_GN_OBJ     = 30,  // 'go'  generator (returns objects)
  EFD_NT_GN_INT     = 31,  // 'gi'  generator (returns integers)
  EFD_NT_GN_NUM     = 32,  // 'gn'  generator (returns numbers)
  EFD_NT_GN_STR     = 33,  // 'gs'  generator (returns strings)
  EFD_NT_GN_AR_OBJ  = 34,  // 'gao' generator (returns arrays of objects)
  EFD_NT_GN_AR_INT  = 35,  // 'gai' generator (returns arrays of integers)
  EFD_NT_GN_AR_NUM  = 36,  // 'gan' generator (returns arrays of numbers)
  EFD_NT_GN_AR_STR  = 37   // 'gas' generator (returns arrays of strings)
};
typedef enum efd_node_type_e efd_node_type;

// Types of reference endpoint.
enum efd_ref_type_e {
  EFD_RT_INVALID = 0,
  EFD_RT_GLOBAL_INT, // (ptrdiff_t) global integer
  EFD_RT_GLOBAL_NUM, // (float) global number
  EFD_RT_GLOBAL_STR, // (string*) global string
  EFD_RT_NODE, // (void*) a void* pointer to an entire EFD node
  EFD_RT_CHAIN, // (void*) a void* pointer to an EFD node which is a link
  EFD_RT_OBJ, // (void*) contents an object node
  EFD_RT_INT, // (ptrdiff_t) contents of an integer node
  EFD_RT_NUM, // (float) contents of a number node
  EFD_RT_STR, // (string*) contents of a string node
  EFD_RT_OBJ_ARR_ENTRY, // (void*) entry in an object array
  EFD_RT_INT_ARR_ENTRY, // (ptrdiff_t) entry in an integer array
  EFD_RT_NUM_ARR_ENTRY, // (float) entry in a number array
  EFD_RT_STR_ARR_ENTRY // (string*) entry in a string array
};
typedef enum efd_ref_type_e efd_ref_type;

/**************
 * Structures *
 **************/

// A common node header:
struct efd_node_header_s;
typedef struct efd_node_header_s efd_node_header;

// Various specific node bodies:
struct efd_container_s;
typedef struct efd_container_s efd_container;

struct efd_link_s;
typedef struct efd_link_s efd_link;

struct efd_function_s;
typedef struct efd_function_s efd_function;

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
typedef struct efd_node_s efd_node;

// A global efd node address:
struct efd_address_s;
typedef struct efd_address_s efd_address;

// A reference specifies the location of a particular value:
struct efd_reference_s;
typedef struct efd_reference_s efd_reference;

// A bridge contains two compatible references:
struct efd_bridge_s;
typedef struct efd_bridge_s efd_bridge;

// A comprehensive set of cross-references that consists of a list of bridges
// each of which is either processed or unprocessed.
struct efd_index_s;
typedef struct efd_index_s efd_index;

/******************
 * Function types *
 ******************/

typedef void* (*efd_unpack_function)(efd_node *);
typedef efd_node* (*efd_pack_function)(void *);
typedef void (*efd_destroy_function)(void *);

/*************
 * Constants *
 *************/

#define EFD_GLOBALS_KEY_ARITY 4
#define EFD_GLOBALS_TABLE_SIZE 2048
#define EFD_NODE_NAME_SIZE ( \
    EFD_GLOBALS_KEY_ARITY*(sizeof(void*) / sizeof(char)) \
  ) // should be 32
#define EFD_GLOBAL_NAME_SIZE ( \
    EFD_GLOBALS_KEY_ARITY*(sizeof(void*) / sizeof(char)) \
  ) // should be 32
#define EFD_MAX_NAME_DEPTH 1024

#define EFD_NODE_SEP '.'
#define EFD_ADDR_PARENT '^'

#define EFD_ANNOTATION_SIZE (sizeof(uint64_t) / sizeof(char)) // should be 8

extern char const * const EFD_NT_NAMES[];
extern char const * const EFD_NT_ABBRS[];

extern char const * const EFD_ANON_NAME;

extern char const * const EFD_ROOT_NAME;

extern efd_node *EFD_ROOT;

extern map *EFD_INT_GLOBALS;
extern map *EFD_NUM_GLOBALS;
extern map *EFD_STR_GLOBALS;

/*************************
 * Structure Definitions *
 *************************/

struct efd_node_header_s {
  efd_node_type type;
  char name[EFD_NODE_NAME_SIZE + 1];
  efd_node *parent;
};

struct efd_container_s {
  list *children;
};

struct efd_link_s {
  efd_address *target;
  list *children;
};

struct efd_function_s {
  char function[EFD_ANNOTATION_SIZE];
  list *children;
};

struct efd_proto_s {
  char format[EFD_ANNOTATION_SIZE];
  efd_node *input;
};

struct efd_object_s {
  char format[EFD_ANNOTATION_SIZE];
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
  void **values;
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
  string **values;
};

union efd_node_body_u {
  efd_container as_container;
  efd_link as_link;
  efd_function as_function;
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

struct efd_address_s {
  char name[EFD_NODE_NAME_SIZE + 1];
  efd_address *parent;
  efd_address *next;
};

struct efd_reference_s {
  efd_ref_type type;
  efd_address *addr;
  ptrdiff_t idx;
};

struct efd_bridge_s {
  efd_reference *from;
  efd_reference *to;
};

struct efd_index_s {
  list *unprocessed;
  list *processed;
};


/*******************
 * Early Functions *
 *******************/

// Asserts that a type matches and throws an error if it does not (or if it's
// NULL). Can be turned off by defining EFD_NO_TYPECHECKS, although the NULL
// check will still take place.
void efd_assert_type(efd_node *n, efd_node_type t);

/********************
 * Inline Functions *
 ********************/

static inline size_t efd_node_depth(efd_node *n) {
  size_t result = 0;
  do {
    if (n->h.parent == NULL) {
      return result;
    } else {
      result += 1;
      n = n->h.parent;
    }
  } while (1);
}

static inline char* efd__p_fmt(efd_node *n) {
  efd_assert_type(n, EFD_NT_PROTO);
  return n->b.as_proto.format;
}

static inline char* efd__o_fmt(efd_node *n) {
  efd_assert_type(n, EFD_NT_OBJECT);
  return n->b.as_object.format;
}

static inline void** efd__o(efd_node *n) {
  efd_assert_type(n, EFD_NT_OBJECT);
  return &(n->b.as_object.value);
}

static inline ptrdiff_t* efd__i(efd_node *n) {
  efd_assert_type(n, EFD_NT_INTEGER);
  return &(n->b.as_integer.value);
}

static inline float* efd__n(efd_node *n) {
  efd_assert_type(n, EFD_NT_NUMBER);
  return &(n->b.as_number.value);
}

static inline string** efd__s(efd_node *n) {
  efd_assert_type(n, EFD_NT_STRING);
  return &(n->b.as_string.value);
}

static inline void*** efd__ao(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_OBJ);
  return &(n->b.as_obj_array.values);
}

static inline size_t* efd__ao_count(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_OBJ);
  return &(n->b.as_obj_array.count);
}

static inline ptrdiff_t** efd__ai(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_INT);
  return &(n->b.as_int_array.values);
}

static inline size_t* efd__ai_count(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_INT);
  return &(n->b.as_int_array.count);
}

static inline float** efd__an(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_NUM);
  return &(n->b.as_num_array.values);
}

static inline size_t* efd__an_count(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_NUM);
  return &(n->b.as_num_array.count);
}

static inline string*** efd__as(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_STR);
  return &(n->b.as_str_array.values);
}

static inline size_t* efd__as_count(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_STR);
  return &(n->b.as_str_array.count);
}

static inline efd_ref_type efd_nt__rt(efd_node_type nt) {
  switch (nt) {
    default:
    case EFD_NT_INVALID:
      return EFD_RT_INVALID;
    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
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
      return EFD_RT_NODE;
    case EFD_NT_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      return EFD_RT_CHAIN;
    case EFD_NT_PROTO:
    case EFD_NT_GLOBAL_INT:
      return EFD_RT_GLOBAL_INT;
    case EFD_NT_GLOBAL_NUM:
      return EFD_RT_GLOBAL_NUM;
    case EFD_NT_GLOBAL_STR:
      return EFD_RT_GLOBAL_STR;
    case EFD_NT_OBJECT:
      return EFD_RT_OBJ;
    case EFD_NT_INTEGER:
      return EFD_RT_INT;
    case EFD_NT_NUMBER:
      return EFD_RT_NUM;
    case EFD_NT_STRING:
      return EFD_RT_STR;
    case EFD_NT_ARRAY_OBJ:
      return EFD_RT_OBJ_ARR_ENTRY;
    case EFD_NT_ARRAY_INT:
      return EFD_RT_INT_ARR_ENTRY;
    case EFD_NT_ARRAY_NUM:
      return EFD_RT_NUM_ARR_ENTRY;
    case EFD_NT_ARRAY_STR:
      return EFD_RT_STR_ARR_ENTRY;
  }
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocate and return a new EFD node of the given type. Up to
// EFD_NODE_NAME_SIZE characters are copied from the given string, but a
// reference to it is not maintained.
efd_node* create_efd_node(efd_node_type t, char const * const name);

// Clean up memory from the given EFD node.
CLEANUP_DECL(efd_node);

// Allocate and creates a new EFD address. Up to EFD_NODE_NAME_SIZE characters
// are copied from the given string, but a reference to it is not maintained.
efd_address* create_efd_address(efd_address* parent, char const * const name);

// Allocates and fills in a new EFD address for the given node.
efd_address* construct_efd_address_of_node(efd_node *node);

// Allocate an new efd_address and copy in information from the given address.
efd_address* copy_efd_address(efd_address *src);

// Clean up memory from the given EFD address (and recursively, all of its
// children). Parents of the given address are not affected (so for example if
// the address has a parent it's next pointer should be set to NULL before
// cleanup up its child.
CLEANUP_DECL(efd_address);

// Allocate and return a new EFD reference. The given address is copied, so it
// should be freed by the caller if it doesn't need it.
efd_reference* create_efd_reference(
  efd_ref_type type,
  efd_address *addr,
  ptrdiff_t idx
);

// Clean up memory from the given reference, including its address.
CLEANUP_DECL(efd_reference);

// Allocate and return a new bridge between the two given references. They will
// be cleaned up when the bridge is so the caller doesn't need to track them.
// If the types of the given references aren't compatible, it doesn't allocate
// anything and returns NULL. In that case, the caller should clean up the from
// and to references.
efd_bridge* create_efd_bridge(efd_reference *from, efd_reference *to);

// Clean up memory from the given bridge, including its references.
CLEANUP_DECL(efd_bridge);

// Allocate and return a new empty EFD index.
efd_index* create_efd_index(void);

// Clean up memory for the given index, including any bridges it contains.
CLEANUP_DECL(efd_index);

/*************
 * Functions *
 *************/

// Returns 1 if the given reference types are compatible and 0 otherwise.
int efd_ref_types_are_compatible(efd_ref_type from, efd_ref_type to);

// Checks that a type matches and returns 1 if it does or 0 if it does not (or
// if the given node is NULL). Setting EFD_NO_TYPECHECKS will turn off the type
// check but retain the NULL check.
int efd_is_type(efd_node *n, efd_node_type t);

// Given an EFD_NT_PROTO or EFD_NT_OBJECT type node, checks that the format
// matches the given string (up to the max length).
int efd_format_is(efd_node *n, char const * const fmt);

// Builds a fully-qualified name for the given node and returns a pointer to
// newly-allocated memory holding this name. Note that this will only
// accommodate names up to a maximum depth of EFD_MAX_NAME_DEPTH nestings.
char* efd_build_fqn(efd_node *n);

// Adds the given child to the parent's list of children (parent must be a
// container node).
void efd_add_child(efd_node *n, efd_node *child);

// Removes the given child from this node's list of children (this node must be
// a container node).
void efd_remove_child(efd_node *n, efd_node *child);

// Appends the given name to the given address:
void efd_append_address(efd_address *a, char const * const name);

// Extends the given address using the given extension. The extension can be
// safely forgotten, as future cleanup of the base address will deal with the
// extension as well.
void efd_extend_address(efd_address *a, efd_address *e);

// Adds the given name as the deepest level of the given address.
void efd_push_address(efd_address *a, char const * const name);

// Removes the deepest level of the given address, returning a pointer to the
// address that was popped (which should eventually be cleaned up).
efd_address* efd_pop_address(efd_address *a);

// This function simply looks up the first child with the given key within the
// given node. It iterates over children until it hits one with a matching
// name, so only the first is returned if multiple children share a name. The
// key argument is treated as a single key within the given parent node. If no
// match is found it returns NULL.
efd_node* efd_lookup(efd_node* root, char const * const key);

// The most ubiquitous EFD function 'efd' does a recursive address lookup to
// find an EFD node given some root node to start from. Internally it uses
// efd_lookup, so when multiple children of a node share a name, the first one
// is used. If no match is found it returns NULL.
efd_node* efd(efd_node* root, efd_address* addr);

// Works like efd, but instead of taking an address it takes a key which is
// parsed into an address. So
//
//   efdx(r, "foo.bar.baz");
//
// is equivalent to
//
//  efd(r, efd_parse_address("foo.bar.baz"))
//
// which is further equivalent to
//
//   efd_lookup(efd_lookup(efd_lookup(r, "foo"), "bar"), "baz");
//
// The use of sprintf and passing a dynamic key to efdx is probably better
// solved by a mix of calls to efd and efdx to avoid buffer-length problems.
// Note that efdx's use of efd_parse_address means that it is limited by
// EFD_MAX_NAME_DEPTH, although multiple calls to efd/efdx can overcome this.
efd_node* efdx(efd_node* root, char const * const saddr);

// Evaluates a function or generator node, returning a newly-allocated node
// (which may have newly-allocated children) representing the result. For any
// other type of node, an identical node is returned except that efd_eval is
// called on each of its children. The caller should dispose of the returned
// node and its children using cleanup_efd_node.
efd_node* efd_eval(efd_node* target, efd_node* args);

// Adds the given bridge to the given index.
void efd_add_crossref(efd_index *cr, efd_bridge* bridge);

// Unpacks this node and all of its children recursively, turning PROTO nodes
// containing raw EFD into OBJECT nodes containing unpacked structs. At this
// point global references are replaced by the corresponding global values
// wherever possible.
void efd_unpack_node(efd_node *root, efd_index *cr);

// Packs this node and all of its children recursively, turning OBJECT nodes
// containing unpacked structs into PROTO nodes containing EFD COLLECTIONs.
// Unlike efd_unpack_node, it does not deal with global references; those must
// be handled at serialization time.
void efd_pack_node(efd_node *root, efd_index *cr);

// TODO: Serialization (separate files)...

// Lookups for packers, unpackers, and destructors. Note that these are not
// actually defined in efd.c but rather in  efd_setup.c.
efd_unpack_function efd_lookup_unpacker(char const * const key);
efd_pack_function efd_lookup_packer(char const * const key);
efd_destroy_function efd_lookup_destructor(char const * const key);

// Functions for getting and setting global integers, numbers, and strings:
ptrdiff_t efd_get_global_i(char const * const key);
float efd_get_global_n(char const * const key);
string* efd_get_global_s(char const * const key);
void efd_set_global_i(char const * const key, ptrdiff_t value);
void efd_set_global_n(char const * const key, float value);
void efd_set_global_s(char const * const key, string* value);

// A function with the same signature as a normal cleanup function that doesn't
// do anything.
void dont_cleanup(void* v);

#endif // INCLUDE_EFD_H
