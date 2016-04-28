#ifndef INCLUDE_EFD_H
#define INCLUDE_EFD_H

// efd.h
// Definition of the Elf Forest Data format.

#include <stdint.h>

#include "datatypes/list.h"
#include "datatypes/string.h"
#include "datatypes/dictionary.h"

#include "boilerplate.h"

/********
 * Meta *
 ********/

#define EFD_GL(x, y) y

/*********
 * Enums *
 *********/

// Raw types that a single EFD node can take on:
#define EFD_NUM_TYPES 34
enum efd_node_type_e {
  EFD_GL(i, EFD_NT_INVALID    = 0 ), //  -    marks an invalid node internally
  EFD_GL(i, EFD_NT_ANY        = 1 ), //  -    stands for any valid type
  EFD_GL(i, EFD_NT_CONTAINER  = 2 ), // 'c'   no data, just children
  EFD_GL(i, EFD_NT_LINK       = 3 ), // 'L'   global link
  EFD_GL(i, EFD_NT_LOCAL_LINK = 4 ), // 'l'   local link
  EFD_GL(i, EFD_NT_SCOPE      = 5 ), // 'V'   scope
  EFD_GL(i, EFD_NT_VARIABLE   = 6 ), // 'v'   variable
  EFD_GL(i, EFD_NT_PROTO      = 7 ), //  -    raw object data pre-assembly
  EFD_GL(i, EFD_NT_OBJECT     = 8 ), // 'o'   automatic parse-to-struct
  EFD_GL(i, EFD_NT_INTEGER    = 9 ), // 'i'   efd_int_t
  EFD_GL(i, EFD_NT_NUMBER     = 10), // 'n'   efd_num_t
  EFD_GL(i, EFD_NT_STRING     = 11), // 's'   quoted string
  EFD_GL(i, EFD_NT_ARRAY_INT  = 12), // 'ai'  array of efd_int_t
  EFD_GL(i, EFD_NT_ARRAY_NUM  = 13), // 'an'  array of efd_num_t
  EFD_GL(i, EFD_NT_ARRAY_STR  = 14), // 'as'  array of quoted strings
  EFD_GL(i, EFD_NT_GLOBAL_INT = 15), // 'Gi'  global integer
  EFD_GL(i, EFD_NT_GLOBAL_NUM = 16), // 'Gn'  global numeric
  EFD_GL(i, EFD_NT_GLOBAL_STR = 17), // 'Gs'  global string
  EFD_GL(i, EFD_NT_FUNCTION   = 18), // 'ff'  function (returns a container)
  EFD_GL(i, EFD_NT_FN_OBJ     = 19), // 'fo'  function (returns an object)
  EFD_GL(i, EFD_NT_FN_INT     = 20), // 'fi'  function (returns an integer)
  EFD_GL(i, EFD_NT_FN_NUM     = 21), // 'fn'  function (returns a number)
  EFD_GL(i, EFD_NT_FN_STR     = 22), // 'fs'  function (returns a string)
  EFD_GL(i, EFD_NT_FN_AR_INT  = 23), // 'fai' function (returns array of ints)
  EFD_GL(i, EFD_NT_FN_AR_NUM  = 24), // 'fan' function (returns array of nums)
  EFD_GL(i, EFD_NT_FN_AR_STR  = 25), // 'fas' function (returns array of strs)
  EFD_GL(i, EFD_NT_GENERATOR  = 26), // 'gg'  generator (returns containers)
  EFD_GL(i, EFD_NT_GN_OBJ     = 27), // 'go'  generator (returns objects)
  EFD_GL(i, EFD_NT_GN_INT     = 28), // 'gi'  generator (returns integers)
  EFD_GL(i, EFD_NT_GN_NUM     = 29), // 'gn'  generator (returns numbers)
  EFD_GL(i, EFD_NT_GN_STR     = 30), // 'gs'  generator (returns strings)
  EFD_GL(i, EFD_NT_GN_AR_INT  = 31), // 'gai' generator (returns arrays of ints)
  EFD_GL(i, EFD_NT_GN_AR_NUM  = 32), // 'gan' generator (returns arrays of nums)
  EFD_GL(i, EFD_NT_GN_AR_STR  = 33), // 'gas' generator (returns arrays of strs)
};
typedef enum efd_node_type_e efd_node_type;

// Types of reference endpoint.
enum efd_ref_type_e {
  EFD_RT_INVALID = 0,
  EFD_RT_GLOBAL_INT,     // (efd_int_t) global integer
  EFD_RT_GLOBAL_NUM,     // (efd_num_t) global number
  EFD_RT_GLOBAL_STR,     // (string*) global string
  EFD_RT_NODE,           // (void*) a pointer to an entire EFD node
  EFD_RT_CHAIN,          // (void*) a pointer to an EFD node which is a link
  EFD_RT_OBJ,            // (void*) contents an object node
  EFD_RT_INT,            // (efd_int_t) contents of an integer node
  EFD_RT_NUM,            // (efd_num_t) contents of a number node
  EFD_RT_STR,            // (string*) contents of a string node
  EFD_RT_INT_ARR_ENTRY,  // (efd_int_t) entry in an integer array
  EFD_RT_NUM_ARR_ENTRY,  // (efd_num_t) entry in a number array
  EFD_RT_STR_ARR_ENTRY,  // (string*) entry in a string array
};
typedef enum efd_ref_type_e efd_ref_type;

/*
 * TODO: Get rid of this!
// Types of path elements: should a path element be treated as a normal node, a
// reference, or skipped entirely during variable resolution?
enum efd_path_element_type_e {
  EFD_PET_UNKNOWN = 0,
  EFD_PET_NORMAL, // a normal path element
  EFD_PET_LINK, // a link path element; affects variable resolution
  EFD_PET_PARENT // a parent path element, skipped during variable resolution
};
typedef enum efd_path_element_type_e efd_path_element_type;
*/

enum efd_generator_type_e {
  EFD_GL(i, EFD_GT_INVALID        = 0),
  EFD_GL(i, EFD_GT_CHILDREN       = 1), // iterate over non-SCOPE children
  EFD_GL(i, EFD_GT_INDICES        = 2), // iterate through array entries
  EFD_GL(i, EFD_GT_FUNCTION       = 3), // call a generator function
  EFD_GL(i, EFD_GT_EXTEND_RESTART = 4), // extend by restarting
  EFD_GL(i, EFD_GT_EXTEND_HOLD    = 5), // extend repeating the final value
  EFD_GL(i, EFD_GT_PARALLEL       = 6), // generate parallel results
};
typedef enum efd_generator_type_e efd_generator_type;

/*********
 * Types *
 *********/

typedef intptr_t efd_int_t;
typedef float efd_num_t;

/******************
 * Function Types *
 ******************/

typedef void * (*efd_unpack_function)(efd_node *);
typedef efd_node* (*efd_pack_function)(void *);
typedef void * (*efd_copy_function)(void *);
typedef void (*efd_destroy_function)(void *);

typedef efd_node* (*efd_eval_function)(efd_node const * const);

typedef efd_node* (*efd_generator_function)(efd_generator_state *state);

typedef efd_generator_state * (*efd_generator_constructor)(
  efd_node const * const base
);

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

// An EFD node address:
struct efd_address_s;
typedef struct efd_address_s efd_address;

/*
 * TODO: Get rid of this!
// An EFD path for keeping track of lookup paths and variable resolution:
struct efd_path_s;
typedef struct efd_path_s efd_path;
*/

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

// An entry in the object format registry describes the string key, pack/unpack
// functions, and copy/destroy functions for an object format.
struct efd_object_format_s;
typedef struct efd_object_format_s efd_object_format;

// An entry in the function registry defines the string key and function for a
// EFD eval function.
struct efd_function_declaration_s;
typdef struct efd_function_declaration_s efd_function_declaration;

// An entry in the generator registry defines a string key plus a function for
// creating a generator from an EFD_NT_GN_* node.
struct efd_generator_declaration_s;
typdef struct efd_generator_declaration_s efd_generator_declaration;

// Generic generator state specifies the type of generator and information
// needed to generate the next result.
struct efd_generator_state_s;
typedef struct efd_generator_state_s efd_generator_state;

/*************
 * Constants *
 *************/

#define EFD_GLOBALS_KEY_ARITY 4
#define EFD_GLOBALS_TABLE_SIZE 2048

#define EFD_DEFAULT_DICTIONARY_SIZE 8

#define EFD_ADDR_SEP_CHR '.'
#define EFD_ADDR_PARENT_CHR '^'

extern string const * const EFD_ADDR_SEP_STR;
extern string const * const EFD_ADDR_PARENT_STR;

extern string const * const EFD_ANON_NAME;
extern string const * const EFD_ROOT_NAME;

extern char const * const EFD_NT_NAMES[];
extern char const * const EFD_NT_ABBRS[];

extern efd_node *EFD_ROOT;

extern dictionary *EFD_INT_GLOBALS;
extern dictionary *EFD_NUM_GLOBALS;
extern dictionary *EFD_STR_GLOBALS;

/*************************
 * Structure Definitions *
 *************************/

struct efd_node_header_s {
  efd_node_type type;
  string *name;
  efd_node *parent;
};

struct efd_container_s {
  dictionary *children;
};

struct efd_link_s {
  efd_address *target;
};

struct efd_function_s {
  string *function;
  dictionary *children;
};

struct efd_proto_s {
  string *format;
  efd_node *input;
};

struct efd_object_s {
  string *format;
  void *value;
};

struct efd_integer_s {
  efd_int_t value;
};

struct efd_number_s {
  efd_num_t value;
};

struct efd_string_s {
  string *value;
};

struct efd_array_int_s {
  size_t count;
  efd_int_t *values;
};

struct efd_array_num_s {
  size_t count;
  efd_num_t *values;
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
  efd_array_int as_int_array;
  efd_array_num as_num_array;
  efd_array_str as_str_array;
};

struct efd_node_s {
  efd_node_header h;
  efd_node_body b;
};

struct efd_address_s {
  string *name;
  efd_address *parent;
  efd_address *next;
};

/*
 * TODO: Get rid of this!
struct efd_path_s {
  efd_path_element_type type;
  efd_node *node;
  efd_path *parent;
};
*/

struct efd_reference_s {
  efd_ref_type type;
  efd_address *addr;
  intptr_t idx;
};

struct efd_bridge_s {
  efd_reference *from;
  efd_reference *to;
};

struct efd_index_s {
  list *unprocessed;
  list *processed;
};

struct efd_object_format_s {
  char *key;
  efd_unpack_function unpacker;
  efd_pack_function packer;
  efd_copy_function copier;
  efd_destroy_function destructor;
};

struct efd_function_declaration_s {
  char *key;
  efd_eval_function function;
};

struct efd_generator_declaration_s {
  char *key;
  efd_generator_constructor constructor;
};

struct efd_generator_state_s {
  efd_generator_type type;
  string *name;
  intptr_t index;
  void *state;
  void *stash;
};

/*******************
 * Early Functions *
 *******************/

// Asserts that a type matches and throws an error if it does not (or if it's
// NULL). Can be turned off by defining EFD_NO_TYPECHECKS, although the NULL
// check will still take place.
void efd_assert_type(efd_node const * const n, efd_node_type t);

// Asserts that the given node is a function or generator node and that its
// return type matches the given type. Throws an error if the type doesn't
// match. Defining EFD_NO_TYPECHECKS will make this a no-op. Unlike
// efd_assert_type, this does not check whether the incoming node is NULL.
void efd_assert_return_type(efd_node const * const n, efd_node_type t);

/********************
 * Inline Functions *
 ********************/

static inline size_t efd_node_depth(efd_node const * const n) {
  size_t result = 0;
  efd_node const *p = n;
  do {
    if (p->h.parent == NULL) {
      return result;
    } else {
      result += 1;
      p = p->h.parent;
    }
  } while (1);
}

static inline int efd_is_link_node(efd_node const * const n) {
  return (
    n->h.type == EFD_NT_LINK
 || n->h.type == EFD_NT_LOCAL_LINK
 || n->h.type == EFD_NT_VARIABLE
  );
}

static inline int efd_is_container_node(efd_node const * const n) {
  return (
    n->h.type == EFD_NT_CONTAINER
 || n->h.type == EFD_NT_SCOPE
 || n->h.type == EFD_NT_FUNCTION
 || n->h.type == EFD_NT_FN_OBJ
 || n->h.type == EFD_NT_FN_INT
 || n->h.type == EFD_NT_FN_NUM
 || n->h.type == EFD_NT_FN_STR
 || n->h.type == EFD_NT_FN_AR_INT
 || n->h.type == EFD_NT_FN_AR_NUM
 || n->h.type == EFD_NT_FN_AR_STR
 || n->h.type == EFD_NT_GENERATOR
 || n->h.type == EFD_NT_GN_OBJ
 || n->h.type == EFD_NT_GN_INT
 || n->h.type == EFD_NT_GN_NUM
 || n->h.type == EFD_NT_GN_STR
 || n->h.type == EFD_NT_GN_AR_INT
 || n->h.type == EFD_NT_GN_AR_NUM
 || n->h.type == EFD_NT_GN_AR_STR
  );
}

static inline string* efd__p_fmt(efd_node *n) {
  efd_assert_type(n, EFD_NT_PROTO);
  return n->b.as_proto.format;
}

static inline string* efd__o_fmt(efd_node *n) {
  efd_assert_type(n, EFD_NT_OBJECT);
  return n->b.as_object.format;
}

static inline void** efd__o(efd_node *n) {
  efd_assert_type(n, EFD_NT_OBJECT);
  return &(n->b.as_object.value);
}

static inline efd_int_t* efd__i(efd_node *n) {
  efd_assert_type(n, EFD_NT_INTEGER);
  return &(n->b.as_integer.value);
}

static inline efd_num_t* efd__n(efd_node *n) {
  efd_assert_type(n, EFD_NT_NUMBER);
  return &(n->b.as_number.value);
}

static inline string** efd__s(efd_node *n) {
  efd_assert_type(n, EFD_NT_STRING);
  return &(n->b.as_string.value);
}

static inline efd_int_t** efd__ai(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_INT);
  return &(n->b.as_int_array.values);
}

static inline size_t* efd__ai_count(efd_node *n) {
  efd_assert_type(n, EFD_NT_ARRAY_INT);
  return &(n->b.as_int_array.count);
}

static inline efd_num_t** efd__an(efd_node *n) {
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

// Type casting functions with well-defined behavior for out-of-bounds values:
static inline efd_int_t efd_cast_to_int(efd_num_t value) {
  efd_num_t rounded = roundf(value);
  if (rounded > (efd_num_t) smaxof(efd_int_t)) {
    return smaxof(efd_int_t);
  } else if (rounded < (efd_num_t) sminof(efd_int_t)) {
    return sminof(efd_int_t);
  }
  return (efd_int_t) rounded;
}

static inline efd_num_t efd_cast_to_num(efd_int_t value) {
  return (efd_num_t) value;
}

// Type coercion functions:
static inline efd_int_t efd_as_i(efd_node *n) {
  if (n->h.type == EFD_NT_INTEGER) {
    return *efd__i(n);
  } else if (n->h.type == EFD_NT_NUMBER) {
    return efd_cast_to_int(*(efd__n(n)));
  } else { // invalid
    // TODO: Context here!
    fprintf(
      stderr,
      "Error: Attempted to coerce non-numeric type to an integer.\n";
    );
    exit(EXIT_FAILURE);
  }
}

static inline efd_int_t efd_as_n(efd_node *n) {
  if (n->h.type == EFD_NT_INTEGER) {
    return efd_cast_to_num(*efd__i(n));
  } else if (n->h.type == EFD_NT_NUMBER) {
    return *(efd__n(n));
  } else { // invalid
    // TODO: Context here!
    fprintf(
      stderr,
      "Error: Attempted to coerce non-numeric type to a number.\n";
    );
    exit(EXIT_FAILURE);
  }
}

// Converts a node type to the corresponding reference type.
static inline efd_ref_type efd_nt__rt(efd_node_type nt) {
  switch (nt) {
    default:
    case EFD_NT_INVALID:
    case EFD_NT_ANY:
      return EFD_RT_INVALID;
    case EFD_NT_CONTAINER:
    case EFD_NT_SCOPE:
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
    case EFD_NT_ARRAY_INT:
      return EFD_RT_INT_ARR_ENTRY;
    case EFD_NT_ARRAY_NUM:
      return EFD_RT_NUM_ARR_ENTRY;
    case EFD_NT_ARRAY_STR:
      return EFD_RT_STR_ARR_ENTRY;
  }
}

// Takes a function node and returns the 
static inline efd_node_type efd_return_type_of(efd_node * function_node) {
  switch (nt) {
    default:
    case EFD_NT_FUNCTION:
    case EFD_NT_GENERATOR:
      return EFD_NT_ANY;
    case EFD_NT_FN_OBJ:
    case EFD_NT_GN_OBJ:
      return EFD_NT_OBJECT;
    case EFD_NT_FN_INT:
    case EFD_NT_GN_INT:
      return EFD_NT_INTEGER;
    case EFD_NT_FN_NUM:
    case EFD_NT_GN_NUM:
      return EFD_NT_NUMBER;
    case EFD_NT_FN_STR:
    case EFD_NT_GN_STR:
      return EFD_NT_STRING;
    case EFD_NT_FN_AR_INT:
    case EFD_NT_GN_AR_INT:
      return EFD_NT_ARRAY_INT;
    case EFD_NT_FN_AR_NUM:
    case EFD_NT_GN_AR_NUM:
      return EFD_NT_ARRAY_NUM;
    case EFD_NT_FN_AR_STR:
    case EFD_NT_GN_AR_STR:
      return EFD_NT_ARRAY_STR;
  }
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocate and return a new EFD node of the given type. The given string is
// copied, so the caller should clean it up if necessary.
efd_node * create_efd_node(efd_node_type t, string const * const name);

// Allocate and return a new EFD node of type EFD_NT_OBJECT containing a copy
// of the given object, which is produced according to the given format. The
// format and name are also copied, rather than held as references.
efd_node * construct_efd_obj_node(
  string const * const name,
  string const * const format,
  void * obj
);

// Allocates and returns a new EFD_NT_INTEGER node with the given value.
efd_node * construct_efd_int_node(string const * const name, efd_int_t value);

// Allocates and returns a new EFD_NT_NUMBER node with the given value.
efd_node * construct_efd_num_node(string const * const name, efd_num_t value);

// Allocates and returns a new EFD_NT_STRING node using a copy of the given
// value string.
efd_node * construct_efd_str_node(
  string const * const name,
  string const * const value
);

// Allocates and returns a deep copy of the given node, which of course
// includes deep copies of all of the node's children recursively. Note that
// any objects contained in the node or its children are also copied, as it is
// assumed that cleanup_efd_node will be sufficient for memory management.
efd_node * copy_efd_node(efd_node const * const src);

// Same as copy_efd_node, but renames the new node, using a copy of the given
// new_name string.
efd_node * copy_efd_node_as(
  efd_node const * const src,
  string const * const new_name
);

// Clean up memory from the given EFD node.
CLEANUP_DECL(efd_node);

// Allocate and creates a new EFD address. The given string is copied, so the
// caller should clean it up if necessary. The parent pointer of the new
// address is set, but the next pointer of the parent is not modified.
efd_address * create_efd_address(
  efd_address *parent,
  string const * const name
);

// Allocates and fills in a new EFD address for the given node.
efd_address * construct_efd_address_of_node(efd_node const * const node);

// Allocate an new efd_address and copy in information from the given address.
efd_address * copy_efd_address(efd_address const * const src);

// Clean up memory from the given EFD address (and recursively, all of its
// children). Parents of the given address are not affected (so for example if
// the address has a parent it's next pointer should be set to NULL before
// cleanup up its child.
CLEANUP_DECL(efd_address);

/*
 * TODO: Get rid of this!
// Allocate and create a new EFD path, with the given parent and target. If
// type_override has a value other than EFD_PET_UNKNOWN, it will be used as the
// resulting path's type. The newly created path contains a reference to the
// given parent path, which will be freed if cleanup is called on the result.
efd_path* create_efd_path(
  efd_path *parent,
  efd_node *here,
  efd_path_element_type type_override
);

// Allocates and creates a new EFD path for the given node, using its canonical
// ancestors as the path.
efd_path* construct_efd_path_for(efd_node *node);

// Allocates and returns a new path that's a copy of the given one.
efd_path* copy_efd_path(efd_path const * const src);

// Cleans up memory from the given EFD path, along with all of its ancestors
// recursively. Doesn't touch the node that it points to.
CLEANUP_DECL(efd_path);
*/

// Allocate and return a new EFD reference. The given address is copied, so it
// should be freed by the caller if it doesn't need it.
efd_reference* create_efd_reference(
  efd_ref_type type,
  efd_address *addr,
  intptr_t idx
);

// Clean up memory from the given reference, including its address.
CLEANUP_DECL(efd_reference);

// Allocate and return a new bridge between the two given references. They will
// be cleaned up when the bridge is so the caller doesn't need to track them.
// If the types of the given references aren't compatible, it doesn't allocate
// anything and returns NULL. In that case, the caller should clean up the from
// and to references.
efd_bridge * create_efd_bridge(efd_reference *from, efd_reference *to);

// Clean up memory from the given bridge, including its references.
CLEANUP_DECL(efd_bridge);

// Allocate and return a new empty EFD index.
efd_index * create_efd_index(void);

// Clean up memory for the given index, including any bridges it contains.
CLEANUP_DECL(efd_index);

// Allocates and returns a new efd_generator_state, using a copy of the given
// name string but taking the given state without copying it.
efd_generator_state * create_efd_generator_state(
  efd_generator_type type,
  string const * const name,
  void *state
);

CLEANUP_DECL(efd_generator_state);

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
// matches the given string.
int efd_format_is(efd_node *n, string const * const fmt);

// Renames the given node using a copy of the given string.
void efd_rename(efd_node * node, string const * const new_name);

// Builds a fully-qualified name for the given node and returns a pointer to a
// newly-allocated string holding this name.
string* efd_build_fqn(efd_node const * const n);

// Gets the children dictionary from a node of any container type (returns NULL
// for non-container nodes and prints a warning if DEBUG is on).
dictionary* efd_children_dict(efd_node const * const n);

// Compares the two nodes and returns 1 if they are exactly equivalent
// (including any children, recursively) or 0 if they differ. Ancestors are not
// compared, so two "equal" nodes with different ancestors may give different
// results under efd_eval (if this is a concern just call efd_equals on the
// eval results).
int efd_equals(efd_node const * const cmp, efd_node const * const agn);

// Adds the given child to the parent's dictionary of children (parent must be
// a container node).
void efd_add_child(efd_node *n, efd_node *child);

// Works like efd_add_child but the child is treated as having been defined
// first within its parent.
void efd_prepend_child(efd_node *n, efd_node *child);

// Removes the given child from this node's dictionary of children (this node
// must be a container node). If DEBUG is on and the parent doesn't contain the
// child, an error message is printed.
void efd_remove_child(efd_node *n, efd_node *child);

// Appends the given name to the given address:
void efd_append_address(efd_address *a, string const * const name);

// Extends the given address using the given extension. The extension can be
// safely forgotten, as future cleanup of the base address will deal with the
// extension as well.
void efd_extend_address(efd_address *a, efd_address *e);

// Adds the given name as the deepest level of the given address.
void efd_push_address(efd_address *a, string const * const name);

// Removes the deepest level of the given address, returning a pointer to the
// address that was popped (which should eventually be cleaned up).
efd_address* efd_pop_address(efd_address *a);

// Looks up a child node within a parent, returning NULL if no node with the
// given name exists as a child of the given node (or when the given node
// doesn't have children). Prints a warning if the given node is of a
// non-container type. Note that this function does not handle link nodes (see
// efd_lookup).
efd_node* efd_find_child(
  efd_node const * const parent,
  string const * const name
);

// Look for any scope node(s) within the given node and searches for the target
// variable within them in order, setting the given scope path to the path to
// the matching variable or NULL if there is no match.
efd_node* efd_find_variable_in(
  efd_node const * const base,
  efd_address const * const target
);

// Takes a variable node and returns the node that it refers to. If no referent
// can be found, it returns NULL. This function searches progressively upwards
// through the EFD tree, returning the first match found. This function just
// does one step of resolution, so the node it returns may still be a link or
// variable.
efd_node* efd_resolve_variable(
  efd_node const * const var
);

// Takes a node and returns the concrete node that it refers to, following any
// link(s) encountered until a non-link node is found. If a link is broken or
// if the input is NULL, it returns NULL. Note that this function doesn't
// remember where it's been, so infinite loops can occur.
// TODO: Change that?
efd_node* efd_concrete(efd_node const * const base);

// Returns the number of non-SCOPE children that the given node has. Returns -1
// if the given node is a non-container node.
intptr_t efd_normal_child_count(efd_node const * const node);

// Returns the nth child of the given node, not counting scope nodes.
efd_node* efd_nth(efd_node const * const node, size_t index);

// This function returns the child with the given key in the given node. It
// iterates over children until it hits one with a matching name, so only the
// first is used if multiple children share a name. The key argument is treated
// as a single key within the given parent node. If no match is found it
// returns NULL. Unlike efd_find_child, this function properly handles link
// nodes. This function always calls efd_concrete on its results, so the result
// is never a link node.
efd_node* efd_lookup(efd_node const * const node, string const * const key);

// The most ubiquitous EFD function 'efd' does a recursive address lookup to
// find an EFD node given some root node to start from and an address to find.
// Internally it uses efd_lookup, so when multiple children of a node share a
// name, the first one is used. If no match is found it sets the given path to
// NULL.
efd_node* efd(efd_node const * const root, efd_address const * addr);

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
efd_node* efdx(efd_node const * const root, string const * const saddr);

// Evaluates a function or generator node, returning a newly-allocated node
// (which may have newly-allocated children) representing the result. For any
// other type of node, a copy is returned except that efd_eval is called on
// each of its children. The caller should dispose of the returned node and its
// children using cleanup_efd_node. The args argument may be given as NULL, but
// if not, it should be a SCOPE node which will be copied and inserted into the
// copy of the target node as its first child before evaluation begins. If the
// target node is not a container node args should be NULL (and will be
// ignored).
efd_node* efd_eval(efd_node const * const target, efd_node const * const args);

// Adds the given bridge to the given index.
void efd_add_crossref(efd_index *cr, efd_bridge *bridge);

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

// Lookup for functions (defined in efd_setup.c).
efd_eval_function efd_lookup_function(string const * const key);

// Lookup for generator constructors (as above).
efd_generator_constructor efd_lookup_generator(string const * const key) {

// Lookups for packers, unpackers, copiers, and destructors. As above, these
// are not actually defined in efd.c but rather in  efd_setup.c.
efd_object_format * efd_lookup_format(string const * const key);
efd_unpack_function efd_lookup_unpacker(string const * const key);
efd_pack_function efd_lookup_packer(string const * const key);
efd_copy_function efd_lookup_copier(string const * const key);
efd_destroy_function efd_lookup_destructor(string const * const key);

// Functions for getting and setting global integers, numbers, and strings:
efd_int_t efd_get_global_i(string const * const key);
efd_num_t efd_get_global_n(string const * const key);
string* efd_get_global_s(string const * const key);
void efd_set_global_i(string const * const key, efd_int_t value);
void efd_set_global_n(string const * const key, efd_num_t value);
void efd_set_global_s(string const * const key, string *value);

// A function with the same signature as a normal copy function that just
// returns the original pointer (warning: may lead to a double-free if
// misused).
void* dont_copy(void *v);

// A function with the same signature as a normal cleanup function that doesn't
// do anything.
void dont_cleanup(void *v);

// Given a generator, returns the next value from it (a newly allocated EFD
// node) and advances its state. If the generator is exhausted, it will return
// NULL.
efd_node * efd_gen_next(efd_generator_state *gen);

// Resets the given generator.
void efd_gen_reset(efd_generator_state *gen);

// Calls next on the given generator until it is exhausted, collecting results
// into a newly-allocated container node and  returning that. Warning: calling
// this on an infinite generator will hang and/or crash.
efd_node * efd_gen_all(efd_generator_state *gen);

// Takes an EFD node and returns the corresponding generator object for
// iteration over that node. Works with container, generator function, and
// array nodes, but returns NULL for other node types (including link types).
efd_generator_state * efd_generator_for(efd_node *node);

#endif // INCLUDE_EFD_H
