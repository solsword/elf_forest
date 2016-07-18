#ifndef INCLUDE_EFD_H
#define INCLUDE_EFD_H

// efd.h
// Definition of the Elf Forest Data format.

#include <stdint.h>
#include <math.h>

#include "datatypes/list.h"
#include "datatypes/string.h"
#include "datatypes/dictionary.h"
#include "datatypes/map.h"

#include "boilerplate.h"
#include "util.h"

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
  EFD_GL(i, EFD_NT_INVALID     = 0 ), // -    marks an invalid node internally
  EFD_GL(i, EFD_NT_ANY         = 1 ), // -    stands for any valid type
  EFD_GL(i, EFD_NT_CONTAINER   = 2 ), //'c'   no data, just children
  EFD_GL(i, EFD_NT_SCOPE       = 3 ), //'V'   scope
  EFD_GL(i, EFD_NT_GLOBAL      = 4 ), //'G'   globals declaration
  EFD_GL(i, EFD_NT_REROUTE     = 5 ), //'R'   a link for routing var resolution
  EFD_GL(i, EFD_NT_ROOT_LINK   = 6 ), //'L'   root link
  EFD_GL(i, EFD_NT_GLOBAL_LINK = 7 ), //'GL'  global link
  EFD_GL(i, EFD_NT_LOCAL_LINK  = 8 ), //'l'   local link
  EFD_GL(i, EFD_NT_VARIABLE    = 9 ), //'v'   variable
  EFD_GL(i, EFD_NT_PROTO       = 10), // -    raw object data pre-assembly
  EFD_GL(i, EFD_NT_OBJECT      = 11), //'o'   automatic parse-to-struct
  EFD_GL(i, EFD_NT_INTEGER     = 12), //'i'   efd_int_t
  EFD_GL(i, EFD_NT_NUMBER      = 13), //'n'   efd_num_t
  EFD_GL(i, EFD_NT_STRING      = 14), //'s'   quoted string
  EFD_GL(i, EFD_NT_ARRAY_INT   = 15), //'ai'  array of efd_int_t
  EFD_GL(i, EFD_NT_ARRAY_NUM   = 16), //'an'  array of efd_num_t
  EFD_GL(i, EFD_NT_ARRAY_STR   = 17), //'as'  array of quoted strings
  EFD_GL(i, EFD_NT_FUNCTION    = 18), //'ff'  function (returns a container)
  EFD_GL(i, EFD_NT_FN_OBJ      = 19), //'fo'  function (returns an object)
  EFD_GL(i, EFD_NT_FN_INT      = 20), //'fi'  function (returns an integer)
  EFD_GL(i, EFD_NT_FN_NUM      = 21), //'fn'  function (returns a number)
  EFD_GL(i, EFD_NT_FN_STR      = 22), //'fs'  function (returns a string)
  EFD_GL(i, EFD_NT_FN_AR_INT   = 23), //'fai' function (returns array of ints)
  EFD_GL(i, EFD_NT_FN_AR_NUM   = 24), //'fan' function (returns array of nums)
  EFD_GL(i, EFD_NT_FN_AR_STR   = 25), //'fas' function (returns array of strs)
  EFD_GL(i, EFD_NT_GENERATOR   = 26), //'gg'  generator (returns containers)
  EFD_GL(i, EFD_NT_GN_OBJ      = 27), //'go'  generator (returns objects)
  EFD_GL(i, EFD_NT_GN_INT      = 28), //'gi'  generator (returns integers)
  EFD_GL(i, EFD_NT_GN_NUM      = 29), //'gn'  generator (returns numbers)
  EFD_GL(i, EFD_NT_GN_STR      = 30), //'gs'  generator (returns strings)
  EFD_GL(i, EFD_NT_GN_AR_INT   = 31), //'gai' generator (returns arrays of ints)
  EFD_GL(i, EFD_NT_GN_AR_NUM   = 32), //'gan' generator (returns arrays of nums)
  EFD_GL(i, EFD_NT_GN_AR_STR   = 33), //'gas' generator (returns arrays of strs)
};
typedef enum efd_node_type_e efd_node_type;

// Types of reference endpoint.
enum efd_ref_type_e {
  EFD_RT_INVALID = 0,
  EFD_RT_GLOBAL_INT,     // (efd_int_t) global integer
  EFD_RT_GLOBAL_NUM,     // (efd_num_t) global number
  EFD_RT_GLOBAL_STR,     // (string*) global string
  EFD_RT_GLOBAL_OBJ,     // (void*) global object
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

/**************
 * Structures *
 **************/

// A common node header:
struct efd_node_header_s;
typedef struct efd_node_header_s efd_node_header;

// Various specific node bodies:
struct efd_container_s;
typedef struct efd_container_s efd_container;

struct efd_reroute_s;
typedef struct efd_reroute_s efd_reroute;

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

// A reference specifies the location of a particular value:
struct efd_reference_s;
typedef struct efd_reference_s efd_reference;

// A cache of function node eval results, including tracking for which node(s)
// are currently being evaluated to detect circular dependencies.
struct efd_value_cache_s;
typedef struct efd_value_cache_s efd_value_cache;

// An entry in the object format registry describes the string key, pack/unpack
// functions, and copy/destroy functions for an object format.
struct efd_object_format_s;
typedef struct efd_object_format_s efd_object_format;

// An entry in the function registry defines the string key and function for a
// EFD eval function.
struct efd_function_declaration_s;
typedef struct efd_function_declaration_s efd_function_declaration;

// An entry in the generator registry defines a string key plus a function for
// creating a generator from an EFD_NT_GN_* node.
struct efd_generator_declaration_s;
typedef struct efd_generator_declaration_s efd_generator_declaration;

// Generic generator state specifies the type of generator and information
// needed to generate the next result.
struct efd_generator_state_s;
typedef struct efd_generator_state_s efd_generator_state;

/******************
 * Function Types *
 ******************/

typedef void * (*efd_unpack_function)(efd_node *);
typedef efd_node* (*efd_pack_function)(void *);
typedef void * (*efd_copy_function)(void *);
typedef void (*efd_destroy_function)(void *);

typedef efd_node* (*efd_eval_function)(
  efd_node const * const,
  efd_value_cache *
);

typedef efd_node* (*efd_generator_function)(efd_generator_state *state);

typedef efd_generator_state * (*efd_generator_constructor)(
  efd_node const * const base,
  efd_value_cache *
);

/*************
 * Constants *
 *************/

#define EFD_GLOBALS_KEY_ARITY 4
#define EFD_GLOBALS_TABLE_SIZE 2048

#define EFD_DEFAULT_DICTIONARY_SIZE 8

#define EFD_EVAL_MAP_TABLE_SIZE 1024
#define EFD_EVAL_DEP_TABLE_SIZE 128

#define EFD_ADDR_SEP_CHR '.'
#define EFD_ADDR_PARENT_CHR '^'

/***********
 * Globals *
 ***********/

// A switch to control tracing and a list of error contexts for tracing:
extern int EFD_TRACK_ERROR_CONTEXTS;
extern list *EFD_ERROR_CONTEXT;

extern string const * const EFD_FILE_EXTENSION;

extern string const * const EFD_COMMON_DIR_NAME;

extern string * EFD_COMMON_DIR;

extern string const * const EFD_ADDR_SEP_STR;
extern string const * const EFD_ADDR_PARENT_STR;

extern string const * const EFD_ANON_NAME;
extern string const * const EFD_ROOT_NAME;

extern char const * const EFD_NT_NAMES[];
extern char const * const EFD_NT_ABBRS[];

extern efd_node *EFD_ROOT;

extern dictionary *EFD_GLOBALS;

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

struct efd_reroute_s {
  efd_node *child;
  efd_node *target;
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
  efd_reroute as_reroute;
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

struct efd_reference_s {
  efd_ref_type type;
  efd_address *addr;
  intptr_t idx;
};

struct efd_value_cache_s {
  map *values;
  efd_node const * active;
  map *stack;
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

// Asserts that the given node is an object node with the given format. Throws
// an error if either of those assertions are false. Defining EFD_NO_TYPECHECKS
// will make this a no-op. Calls efd_assert_type which performs a NULL check.
void efd_assert_object_format(
  efd_node const * const n,
  string const * const fmt
);

// For use with e.g. l_witheach to verify that multiple objects have a given
// format. The first argument must be an efd_node while the second must be a
// string.
void efd_v_assert_object_format(void *v_node, void *v_fmt);

/*****************************
 * Error-Reporting Functions *
 *****************************/

// Reports an error with the given node, displaying the given message on stderr
// along with a representation of the given node. Devours the given message, so
// the caller doesn't need to free it. The _full version prints a full
// representation of the given node instead of an abbreviation, while the
// _light version prints just the node's address.
void efd_report_error_light(string *message, efd_node const * const n);
void efd_report_error(string *message, efd_node const * const n);
void efd_report_error_full(string *message, efd_node const * const n);

// Returns a string containing a trace of resolution of the given link.
string *efd_trace_link(efd_node const * const n);

// Reports an error with a link, printing the given message on stderr as well
// as an analysis of where the given node (which should be a link node) fails
// to resolve. Devours the given message.
void efd_report_broken_link(string *message, efd_node const * const n);

// Report an error with evaluation, displaying the given message along with a
// summary of the original node and a full report of the (partial) evaluation
// result. Devours the given message.
void efd_report_eval_error(
  efd_node const * const orig,
  efd_node const * const evald,
  string *message
);


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

static inline int efd_is_link_type(efd_node_type t) {
  return (
    t == EFD_NT_REROUTE
 || t == EFD_NT_ROOT_LINK
 || t == EFD_NT_GLOBAL_LINK
 || t == EFD_NT_LOCAL_LINK
 || t == EFD_NT_VARIABLE
  );
}

static inline int efd_is_link_node(efd_node const * const n) {
  return efd_is_link_type(n->h.type);
}

static inline int efd_is_container_type(efd_node_type t) {
  return (
    t == EFD_NT_CONTAINER
 || t == EFD_NT_SCOPE
 || t == EFD_NT_GLOBAL
 || t == EFD_NT_FUNCTION
 || t == EFD_NT_FN_OBJ
 || t == EFD_NT_FN_INT
 || t == EFD_NT_FN_NUM
 || t == EFD_NT_FN_STR
 || t == EFD_NT_FN_AR_INT
 || t == EFD_NT_FN_AR_NUM
 || t == EFD_NT_FN_AR_STR
 || t == EFD_NT_GENERATOR
 || t == EFD_NT_GN_OBJ
 || t == EFD_NT_GN_INT
 || t == EFD_NT_GN_NUM
 || t == EFD_NT_GN_STR
 || t == EFD_NT_GN_AR_INT
 || t == EFD_NT_GN_AR_NUM
 || t == EFD_NT_GN_AR_STR
  );
}

static inline int efd_is_container_node(efd_node const * const n) {
  return efd_is_container_type(n->h.type);
}

static inline int efd_is_function_type(efd_node_type t) {
  return (
    t == EFD_NT_FUNCTION
 || t == EFD_NT_FN_OBJ
 || t == EFD_NT_FN_INT
 || t == EFD_NT_FN_NUM
 || t == EFD_NT_FN_STR
 || t == EFD_NT_FN_AR_INT
 || t == EFD_NT_FN_AR_NUM
 || t == EFD_NT_FN_AR_STR
  );
}

static inline int efd_is_function_node(efd_node const * const n) {
  return efd_is_function_type(n->h.type);
}

static inline int efd_is_generator_type(efd_node_type t) {
  return (
    t == EFD_NT_GENERATOR
 || t == EFD_NT_GN_OBJ
 || t == EFD_NT_GN_INT
 || t == EFD_NT_GN_NUM
 || t == EFD_NT_GN_STR
 || t == EFD_NT_GN_AR_INT
 || t == EFD_NT_GN_AR_NUM
 || t == EFD_NT_GN_AR_STR
  );
}

static inline int efd_is_generator_node(efd_node const * const n) {
  return efd_is_generator_type(n->h.type);
}

static inline string** efd__p_fmt(efd_node *n) {
  efd_assert_type(n, EFD_NT_PROTO);
  return &(n->b.as_proto.format);
}

static inline string** efd__o_fmt(efd_node *n) {
  efd_assert_type(n, EFD_NT_OBJECT);
  return &(n->b.as_object.format);
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
    efd_report_error(
      s_("Error: Attempted to coerce non-numeric type to an integer:"),
      n
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
    efd_report_error(
      s_("Error: Attempted to coerce non-numeric type to a number:"),
      n
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
    case EFD_NT_GLOBAL:
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
    case EFD_NT_REROUTE:
    case EFD_NT_ROOT_LINK:
    case EFD_NT_GLOBAL_LINK:
    case EFD_NT_LOCAL_LINK:
    case EFD_NT_VARIABLE:
      return EFD_RT_CHAIN;
    case EFD_NT_PROTO:
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

// Takes a function node and returns the kind of node it will return:
static inline efd_node_type efd_return_type_of(
  efd_node const * const function_node
) {
  switch (function_node->h.type) {
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

// Takes a node type and returns the type of function node that returns that
// node type, or INVALID if there is no such type.
static inline efd_node_type efd_function_type_that_returns(
  efd_node_type return_type
) {
  switch (return_type) {
    default:
      return EFD_NT_INVALID;
    case EFD_NT_ANY:
    case EFD_NT_CONTAINER:
      return EFD_NT_FUNCTION;
    case EFD_NT_OBJECT:
      return EFD_NT_FN_OBJ;
    case EFD_NT_INTEGER:
      return EFD_NT_FN_INT;
    case EFD_NT_NUMBER:
      return EFD_NT_FN_NUM;
    case EFD_NT_STRING:
      return EFD_NT_FN_STR;
    case EFD_NT_ARRAY_INT:
      return EFD_NT_FN_AR_INT;
    case EFD_NT_ARRAY_NUM:
      return EFD_NT_FN_AR_NUM;
    case EFD_NT_ARRAY_STR:
      return EFD_NT_FN_AR_STR;
  }
}

// Returns the type of node that the value of the given node will be: uses
// efd_return_type_of if it's a function node, or just returns the base type if
// it's not a function node.
static inline efd_node_type efd_value_type_of(efd_node const * const base) {
  if (efd_is_function_node(base)) {
    return efd_return_type_of(base);
  } else {
    return base->h.type;
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
// format and name are also copied, rather than held as references. The
// "SINGLETON" format can be used for separately-managed objects which can't be
// copied, although the resulting node will not be packable.
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

// Allocates and returns a new EFD_NT_ROOT_LINK node that points to the given
// node. Note that because of the use of addresses, if the given node has
// siblings with identical names the link may not be accurate.
efd_node * construct_efd_link_node_to(
  string const * const name,
  efd_node const * const target
);

// Allocates and returns a new function node whose type corresponds to the
// given return type. The given function name is copied.
efd_node * construct_efd_function_node(
  string const * const name,
  efd_node_type returns,
  string const * const function
);

// Allocates and returns a deep copy of the given node, which of course
// includes deep copies of all of the node's children recursively. Note that
// any objects contained in the node or its children are also copied, as it is
// assumed that cleanup_efd_node will be sufficient for memory management. The
// copied node has its parent set to NULL.
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

// Allocate and return a new EFD reference. The given address is copied, so it
// should be freed by the caller if it doesn't need it.
efd_reference* create_efd_reference(
  efd_ref_type type,
  efd_address *addr,
  intptr_t idx
);

// Clean up memory from the given reference, including its address.
CLEANUP_DECL(efd_reference);

// Allocate and return a new empty EFD value cache.
efd_value_cache * create_efd_value_cache(void);

// Clean up memory for the given value cache, including all values that it
// contains. The base nodes those values were computed from are unaffected.
CLEANUP_DECL(efd_value_cache);

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

// Functions for using with e.g. l_map to convert efd nodes to corresponding
// raw types:
void * v_efd__v_i(void *v_node);
void * v_efd__v_n(void *v_node);
void * v_efd__v_s(void *v_node);
void * v_efd__o(void *v_node);

// TODO: Make this stuff thread-safe!
// Sets the EFD error context. The given string is devoured, so the caller
// should abdicate responsibility for it.
void efd_push_error_context(string *context);

// Sets the EFD error context including details of the given node. The given
// message is devoured so the caller should abdicate responsibility for it.
void efd_push_error_context_with_node(
  string *message,
  efd_node const * const node
);

// Removes the most-specific element of the current EFD error context.
void efd_pop_error_context(void);

// Prints the EFD error context.
void efd_print_error_context(void);

// Returns 1 if the given reference types are compatible and 0 otherwise.
int efd_ref_types_are_compatible(efd_ref_type from, efd_ref_type to);

// Checks that a type matches and returns 1 if it does or 0 if it does not (or
// if the given node is NULL). Setting EFD_NO_TYPECHECKS will turn off the type
// check but retain the NULL check.
int efd_is_type(efd_node const * const n, efd_node_type t);

// Given an EFD_NT_PROTO or EFD_NT_OBJECT type node, checks that the format
// matches the given string.
int efd_format_is(efd_node  const * const n, string const * const fmt);

// Renames the given node using a copy of the given string.
void efd_rename(efd_node * node, string const * const new_name);

// Creates and returns a string holding the name of the given type:
string * efd_type_name(efd_node_type t);

// Creates and returns a string holding the abbreviation of the given type:
string * efd_type_abbr(efd_node_type t);

// Creates a string from an EFD address.
string * efd_addr_string(efd_address *a);

// Builds a fully-qualified name for the given node and returns a pointer to a
// newly-allocated string holding this name.
string * efd_build_fqn(efd_node const * const n);

// Creates a new string containing an abbreviated representation of the given
// EFD node.
string * efd_repr(efd_node const * const n);

// Creates a new string containing a full representation of the given EFD node.
// This doesn't handle things like global references, so it's not suitable for
// persistence to a file, but it can be used for debugging and diagnostics.
string * efd_full_repr(efd_node const * const n);

// Gets the children dictionary from a node of any container type (returns NULL
// for non-container nodes and prints a warning if DEBUG is on).
dictionary * efd_children_dict(efd_node const * const n);

// Compares the two nodes and returns 1 if they are exactly equivalent
// (including any children, recursively) or 0 if they differ. Ancestors are not
// compared, so two "equal" nodes with different ancestors may give different
// results under efd_eval (if this is a concern just call efd_equals on the
// eval results). NULL arguments are accepted.
int efd_equals(efd_node const * const cmp, efd_node const * const agn);

// Compares two nodes as with efd_equals, but only looks at values and ignores
// node names (including for children). Also ignores scopes entirely, testing
// just normal nodes. Finally, equivalent ignores reroute nodes entirely,
// comparing their children when one or more are given. NULL arguments are
// accepted.
int efd_equivalent(efd_node const * const cmp, efd_node const * const agn);

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

// Creates a new REROUTE node which targets the given node's parent and
// contains a copy of the given node.
efd_node * efd_create_shadow_clone(efd_node const * const original);

// Creates a new REROUTE node which targets the given shadow parent and
// contains a copy of the given node.
efd_node * efd_create_reroute(
  efd_node *shadow_parent,
  efd_node const * const original
);

// Looks up a child node within a parent, returning NULL if no node with the
// given name exists as a child of the given node (or when the given node
// doesn't have children). Prints a warning if the given node is of a
// non-container type. Note that this function does not handle link nodes (see
// efd_lookup).
efd_node * efd_find_child(
  efd_node const * const parent,
  string const * const name
);

// Works like efd_find_child, but returns a newly-allocated list containing
// *all* children of the given node with the given name. The list's values are
// not copies, so the list should be cleaned up, not destroyed.
list * efd_find_all_children(
  efd_node const * const parent,
  string const * const name
);

// Look for any scope node(s) within the given node and searches for the target
// variable within them in order, setting the given scope path to the path to
// the matching variable or NULL if there is no match.
efd_node * efd_find_variable_in(
  efd_node const * const base,
  efd_address const * const target
);

// Takes a variable node and returns the node that it refers to. If no referent
// can be found, it returns NULL. This function searches progressively upwards
// through the EFD tree, returning the first match found. This function just
// does one step of resolution, so the node it returns may still be a link or
// variable.
efd_node * efd_resolve_variable(efd_node const * const var);

// Returns a string showing the trace of search locations checked for the given
// variable.
string * efd_variable_search_trace(efd_node const * const var);

// Takes a node and returns the concrete node that it refers to, following any
// link(s) encountered until a non-link node is found. If a link is broken or
// if the input is NULL, it returns NULL. Note that this function doesn't
// remember where it's been, so infinite loops can occur.
// TODO: Change that?
efd_node * efd_concrete(efd_node const * const base);

// Returns the number of non-SCOPE children that the given node has. Returns -1
// if the given node is a non-container node.
intptr_t efd_normal_child_count(efd_node const * const node);

// Returns the nth child of the given node, not counting scope nodes.
efd_node * efd_nth(efd_node const * const node, size_t index);

// This function returns the child with the given key in the given node. It
// iterates over children until it hits one with a matching name, so only the
// first is used if multiple children share a name. The key argument is treated
// as a single key within the given parent node. If no match is found or if the
// input node is NULL it returns NULL. Unlike efd_find_child, this function
// properly handles link nodes. This function always calls efd_concrete on its
// results, so the result is never a link node.
efd_node * efd_lookup(efd_node const * const node, string const * const key);

// Works like efd_lookup, returns a list of all matching children with the
// given key, instead of just the first. The list's values are not copies, but
// the list itself should be cleaned up by the caller. If the 'parent' path
// string is passed to efd_lookup_all it returns NULL and a warning is printed.
list * efd_lookup_all(
  efd_node const * const node,
  string const * const key
);

// The most ubiquitous EFD function 'efd' does a recursive address lookup to
// find an EFD node given some root node to start from and an address to find.
// Internally it uses efd_lookup, so when multiple children of a node share a
// name, the first one is used. If no match is found it returns NULL.
efd_node * efd(efd_node const * const root, efd_address const * addr);

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
efd_node * efdx(efd_node const * const root, string const * const saddr);

// Evaluates a function or generator node, returning a newly-allocated node
// (which may have newly-allocated children) representing the result. For any
// other type of node, the target pointer is returned (the caller can check for
// this). If the returned node is a new node, it will be registered in the
// given value cache with the target node pointer as a key.
efd_node * efd_eval(efd_node const * const target, efd_value_cache * cache);

// Gets the value of the given node, either cached within the given value
// cache, or via efd_eval_in_context (thus adding the returned value to the
// given cache). In either case the caller doesn't need to worry about cleanup
// for the node as it will be present in the value cache. This function calls
// efd_concrete on its input before trying to fetch a value, so the result
// won't be the same as just looking up the target node in the value cache when
// the target is a link node. If the input is NULL this will return NULL
// immediately. The returned node will have a NULL parent.
efd_node * efd_get_value(
  efd_node const * const target,
  efd_value_cache * cache
);

// Gets the value of the given node via efd_get_value using a temporary cache
// which it cleans up afterwards. A copy of the value from the temporary cache
// is returned, so the caller is always responsible for cleaning up the
// returned node.
efd_node * efd_fresh_value(efd_node const * const target);

// Returns a copy of the given node, but with all links replaced by flattened
// versions of their destinations and all functions replaced by flattened
// versions of their values, recursively. The parent of the resulting node is
// set to NULL.
efd_node * efd_flatten(efd_node const * const target);

// Evaluates all nodes under the given root node, returning a newly-allocated
// efd_value_cache that maps node pointers to their values that covers all
// function- and generator-type nodes found, including nodes linked from the
// given subtree.
efd_value_cache * efd_compute_values(efd_node const * const root);

// Takes a pointer to an EFD node and creates a temporary function node with
// function type "call" that calls that node using the given scope node as
// arguments. Proceeds to evaluate the temporary function node and return its
// value using efd_fresh_value. The newly-returned node and the first argument
// are the responsibility of the caller, but the args node is cleaned up during
// the evaluation process.
efd_node * efd_call_function(
  efd_node const * const function,
  efd_node * args
);

// Unpacks this node and all of its children recursively, turning PROTO nodes
// containing raw EFD into OBJECT nodes containing unpacked structs.
void efd_unpack_node(efd_node *root);

// Void version of efd_unpack_node.
void efd_unpack_v_node(void *v_node);

// Packs this node and all of its children recursively, turning OBJECT nodes
// containing unpacked structs into PROTO nodes containing EFD COLLECTIONs.
void efd_pack_node(efd_node *root);

// Void version of efd_pack_node.
void efd_pack_v_node(void *v_node);

// TODO: Serialization (separate files)...

// Lookup for functions (defined in efd_setup.c).
efd_eval_function efd_lookup_function(string const * const key);

// Lookup for generator constructors (as above).
efd_generator_constructor efd_lookup_generator(string const * const key);

// Lookups for packers, unpackers, copiers, and destructors. As above, these
// are not actually defined in efd.c but rather in  efd_setup.c.
efd_object_format * efd_lookup_format(string const * const key);
efd_unpack_function efd_lookup_unpacker(string const * const key);
efd_pack_function efd_lookup_packer(string const * const key);
efd_copy_function efd_lookup_copier(string const * const key);
efd_destroy_function efd_lookup_destructor(string const * const key);

// Functions for getting and setting globals. Note that efd_get_global calls
// efd_concrete on its result.
efd_node* efd_get_global(string const * const key);
void efd_set_global(string const * const key, efd_node *value);

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
// iteration over that node, getting values from and/or adding them to the
// given cache when needed. Works with container, generator function, and array
// nodes, but returns NULL for other node types (including link types).
efd_generator_state * efd_generator_for(
  efd_node *node,
  efd_value_cache *cache
);

#endif // INCLUDE_EFD_H
