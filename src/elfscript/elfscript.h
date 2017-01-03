#ifndef INCLUDE_ELFSCRIPT_H
#define INCLUDE_ELFSCRIPT_H

// elfscript.h
// Definition of ElfScript.

#include <stdint.h>
#include <math.h>

#include "datatypes/list.h"
#include "datatypes/string.h"
#include "datatypes/dictionary.h"
#include "datatypes/map.h"

#include "boilerplate.h"
#include "util.h"

#include "es_gl.h"

/*********
 * Enums *
 *********/

// The ELFSCRIPT instruction set:
#define ELFSCRIPT_MAX_INSTRUCTION 128
enum es_instruction_e {
  ELFSCRIPT_INSTR_NOP        =   0, // -    a no-op
  // constant operations
  ELFSCRIPT_INSTR_CINT       = 105, //'i'   constant int
  ELFSCRIPT_INSTR_CNUM       = 110, //'n'   constant number
  ELFSCRIPT_INSTR_CSTR       = 115, //'s'   constant string
  // object operations
  ELFSCRIPT_INSTR_POBJ       = 111, //'o'   package last scope as a primitive
  ELFSCRIPT_INSTR_OPROP      =  46, //'.'   get property of a primitive object
  ELFSCRIPT_INSTR_PSCOPE     =  47, //'O'   push last scope as an object
  ELFSCRIPT_INSTR_SCPROP     =  58, //':'   get property of a scope object
  // scope operations
  ELFSCRIPT_INSTR_OSC        = 123, //'{'   open scope
  ELFSCRIPT_INSTR_CSC        = 125, //'}'   close scope
  ELFSCRIPT_INSTR_LIDX       =  40, //'('   load variable by index
  ELFSCRIPT_INSTR_SIDX       =  41, //')'   store anonymous value
  ELFSCRIPT_INSTR_LVAR       =  91, //'['   load variable by name
  ELFSCRIPT_INSTR_SVAR       =  93, //']'   store variable by name
  ELFSCRIPT_INSTR_LGLB       =  36, //'$'   load global
  ELFSCRIPT_INSTR_SGLB       =  35, //'#'   store global
  // math operations
  ELFSCRIPT_INSTR_ADD        =  43, //'+'   addition
  ELFSCRIPT_INSTR_SUB        =  45, //'-'   subtraction
  ELFSCRIPT_INSTR_MULT       =  42, //'*'   multiplication
  ELFSCRIPT_INSTR_FDIV       =  47, //'/'   floating point division
  ELFSCRIPT_INSTR_IDIV       =  95, //'_'   integer division
  ELFSCRIPT_INSTR_MOD        =  37, //'%'   modulus
  ELFSCRIPT_INSTR_POW        =  94, //'^'   exponentiation
  ELFSCRIPT_INSTR_EXP        = 101, //'e'   e^x
  ELFSCRIPT_INSTR_LN         = 108, //'l'   ln(x)
  ELFSCRIPT_INSTR_LOG        =  76, //'L'   log_x(y)
  // bitwise operations
  ELFSCRIPT_INSTR_BIT_AND    =  38, //'&'   bitwise AND
  ELFSCRIPT_INSTR_BIT_OR     = 124, //'|'   bitwise OR
  ELFSCRIPT_INSTR_BIT_XOR    =  88, //'X'   bitwise XOR
  ELFSCRIPT_INSTR_BIT_NOT    = 126, //'~'   bitwise NOT
  // logical operations
  ELFSCRIPT_INSTR_AND        =  65, //'A'   logical AND
  ELFSCRIPT_INSTR_OR         =  79, //'O'   logical OR
  ELFSCRIPT_INSTR_NOT        =  33, //'!'   logical NOT
  ELFSCRIPT_INSTR_LT         =  60, //'<'   logical GREATER_THAN
  ELFSCRIPT_INSTR_EQ         =  61, //'='   logical EQUALS
  ELFSCRIPT_INSTR_GT         =  62, //'>'   logical GREATER_THAN
  // function operations
  ELFSCRIPT_INSTR_FDEF       =  92, //'\'   define function
  ELFSCRIPT_INSTR_MDEF       =  39, //'''   define method
  ELFSCRIPT_INSTR_GDEF       =  96, //'`'   define generator
  ELFSCRIPT_INSTR_GMDEF      =  34, //'"'   define generator method
  ELFSCRIPT_INSTR_FCALL      = 102, //'f'   function call (begin args code)
  ELFSCRIPT_INSTR_MCALL      = 109, //'m'   method call (begin args code)
  ELFSCRIPT_INSTR_BCALL      =  98, //'b'   builtin call (begin args code)
  ELFSCRIPT_INSTR_FJUMP      = 106, //'j'   function call (jump to function)
  ELFSCRIPT_INSTR_RET        = 114, //'r'   return from function
};
typedef enum es_instruction_e es_instruction;

ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_NOP)

ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_CINT)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_CNUM)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_CSTR)

ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_POBJ)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_OPROP)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_PSCOPE)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_SCPROP)

ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_OSC)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_CSC)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_LIDX)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_SIDX)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_LVAR)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_SVAR)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_LGLB)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_SGLB)

ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_ADD)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_SUB)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_MULT)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_FDIV)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_IDIV)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_MOD)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_POW)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_EXP)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_LN)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_LOG)

ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_BIT_AND)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_BIT_OR)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_BIT_XOR)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_BIT_NOT)

ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_AND)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_OR)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_NOT)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_LT)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_EQ)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_GT)

ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_FDEF)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_MDEF)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_GDEF)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_GMDEF)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_FCALL)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_MCALL)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_BCALL)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_FJUMP)
ELFSCRIPT_GL(i, ELFSCRIPT_INSTR_RET)

// Elfscript data types
enum es_type_e {
  ELFSCRIPT_DT_INVALID     = 0, // -   invalid
  ELFSCRIPT_DT_ANY            , //'A'  matches any type
  ELFSCRIPT_DT_INT            , //'i'  integer
  ELFSCRIPT_DT_NUM            , //'n'  number
  ELFSCRIPT_DT_STR            , //'s'  string
  ELFSCRIPT_DT_OBJ            , //'o'  object
  ELFSCRIPT_DT_FCN            , //'f'  function
  ELFSCRIPT_DT_MTH            , //'m'  method
  ELFSCRIPT_DT_GEN            , //'g'  generator
  ELFSCRIPT_DT_GNM            , //'M'  generator method
};
typedef enum es_type_e es_type;

ELFSCRIPT_GL(i, ELFSCRIPT_DT_INVALID)
ELFSCRIPT_GL(i, ELFSCRIPT_DT_ANY)
ELFSCRIPT_GL(i, ELFSCRIPT_DT_INT)
ELFSCRIPT_GL(i, ELFSCRIPT_DT_NUM)
ELFSCRIPT_GL(i, ELFSCRIPT_DT_STR)
ELFSCRIPT_GL(i, ELFSCRIPT_DT_OBJ)
ELFSCRIPT_GL(i, ELFSCRIPT_DT_FCN)
ELFSCRIPT_GL(i, ELFSCRIPT_DT_MTH)
ELFSCRIPT_GL(i, ELFSCRIPT_DT_GEN)
ELFSCRIPT_GL(i, ELFSCRIPT_DT_GNM)

enum es_generator_type_e {
  ELFSCRIPT_GT_INVALID        = 0,
  ELFSCRIPT_GT_VARIABLES         , // iterate through variables in a scope
  ELFSCRIPT_GT_INDICES           , // iterate through array entries
  ELFSCRIPT_GT_FUNCTION          , // call a generator function
  ELFSCRIPT_GT_EXTEND_RESTART    , // extend by restarting
  ELFSCRIPT_GT_EXTEND_HOLD       , // extend repeating the final value
  ELFSCRIPT_GT_PARALLEL          , // generate parallel results
};
typedef enum es_generator_type_e es_generator_type;

ELFSCRIPT_GL(i, ELFSCRIPT_GT_INVALID)
ELFSCRIPT_GL(i, ELFSCRIPT_GT_VARIABLES)
ELFSCRIPT_GL(i, ELFSCRIPT_GT_INDICES)
ELFSCRIPT_GL(i, ELFSCRIPT_GT_FUNCTION)
ELFSCRIPT_GL(i, ELFSCRIPT_GT_EXTEND_RESTART)
ELFSCRIPT_GL(i, ELFSCRIPT_GT_EXTEND_HOLD)
ELFSCRIPT_GL(i, ELFSCRIPT_GT_PARALLEL)

/*********
 * Types *
 *********/

typedef int64_t es_int_t;
typedef float es_num_t;
typedef string* es_string_t;
typedef void* es_probj_t;
typedef intptr_t es_val_t;

/**************
 * Structures *
 **************/

// The Elfscript value stack
struct es_stack_s;
typedef struct es_stack_s es_stack;

// An entry on the value stack
struct es_stack_entry_s;
typedef struct es_stack_entry_s es_stack_entry;

// An Elfscript scope
struct es_scope_s;
typedef struct es_scope_s es_scope;

// An Elfscript variable (to be stored in a scope)
struct es_var_s;
typedef sruct es_var_s es_var;

// For storing expressions as they're parsed
struct es_expression_s;
typedef struct es_expression_s es_expression;

// An entry in the object format registry describes the string key, pack/unpack
// functions, and copy/destroy functions for an object format.
struct es_object_format_s;
typedef struct es_object_format_s es_object_format;

// An entry in the function registry defines the string key and function for a
// Elfscript builtin function.
struct es_function_declaration_s;
typedef struct es_function_declaration_s es_function_declaration;

// An entry in the generator registry defines a string key plus a function for
// a built-in Elfscript generator.
struct es_generator_declaration_s;
typedef struct es_generator_declaration_s es_generator_declaration;

// Generic generator state specifies the type of generator and information
// needed to generate the next result.
struct es_generator_state_s;
typedef struct es_generator_state_s es_generator_state;

/******************
 * Function Types *
 ******************/

// Functions for working with primitive objects
typedef es_probj_t (*es_probj_package_function)(es_scope *);
typedef es_scope* (*es_probj_unpackage_function)(es_probj_t);
typedef es_val_t (*es_probj_access_function)(es_probj_t);
typedef es_probj_t (*es_probj_copy_function)(es_probj_t);
typedef void (*es_probj_destroy_function)(es_probj_t);

typedef es_val_t (*es_eval_function)(es_scope *);

typedef es_val_t (*es_generator_function)(es_generator_state *state);

typedef es_generator_state * (*es_generator_constructor)(
  es_scope const * const base
);

/*************
 * Constants *
 *************/

#define ELFSCRIPT_GLOBALS_TABLE_SIZE 2048

#define ELFSCRIPT_DEFAULT_DICTIONARY_SIZE 16

#define ELFSCRIPT_ADDR_SEP_CHR '.'
#define ELFSCRIPT_ADDR_PARENT_CHR '`'

/***********
 * Globals *
 ***********/

// A switch to control tracing and a list of error contexts for tracing:
extern int ELFSCRIPT_TRACK_ERROR_CONTEXTS;
extern list *ELFSCRIPT_ERROR_CONTEXT;

extern string const * const ELFSCRIPT_FILE_EXTENSION;

extern string const * const ELFSCRIPT_DATA_DIR_NAME;
extern string const * const ELFSCRIPT_COMMON_DIR_NAME;
extern string const * const ELFSCRIPT_GLOBALS_DIR_NAME;

extern string * ELFSCRIPT_DATA_DIR;
extern string * ELFSCRIPT_GLOBALS_DIR;
extern string * ELFSCRIPT_COMMON_DIR;

extern string const * const ELFSCRIPT_ADDR_SEP_STR;
extern string const * const ELFSCRIPT_ADDR_PARENT_STR;

extern string const * const ELFSCRIPT_ANON_NAME;
extern string const * const ELFSCRIPT_ROOT_NAME;

extern char const * const ELFSCRIPT_DT_NAMES[];
extern char const * const ELFSCRIPT_DT_ABBRS[];
extern char const * const ELFSCRIPT_GT_NAMES[];

extern es_scope *ELFSCRIPT_GLOBAL_SCOPE;

extern dictionary *ELFSCRIPT_GLOBAL_VALS;

/*************************
 * Structure Definitions *
 *************************/

// HERE

struct es_object_format_s {
  char *key;
  es_unpack_function unpacker;
  es_pack_function packer;
  es_copy_function copier;
  es_destroy_function destructor;
};

struct es_function_declaration_s {
  char *key;
  es_eval_function function;
};

struct es_generator_declaration_s {
  char *key;
  es_generator_constructor constructor;
};

struct es_generator_state_s {
  es_generator_type type;
  string *name;
  intptr_t index;
  void *state;
  void *stash;
};

/*******************
 * Early Functions *
 *******************/

// Asserts that a type matches and throws an error if it does not (or if it's
// NULL). Can be turned off by defining ELFSCRIPT_NO_TYPECHECKS, although the NULL
// check will still take place.
void es_assert_type(es_node const * const n, es_node_type t);

// Asserts that the given node is a function or generator node and that its
// return type matches the given type. Throws an error if the type doesn't
// match. Defining ELFSCRIPT_NO_TYPECHECKS will make this a no-op. Unlike
// es_assert_type, this does not check whether the incoming node is NULL.
void es_assert_return_type(es_node const * const n, es_node_type t);

// Asserts that the given node is an object node with the given format. Throws
// an error if either of those assertions are false. Defining ELFSCRIPT_NO_TYPECHECKS
// will make this a no-op. Calls es_assert_type which performs a NULL check.
void es_assert_object_format(
  es_node const * const n,
  string const * const fmt
);

// For use with e.g. l_witheach to verify that multiple objects have a given
// format. The first argument must be an es_node while the second must be a
// string.
void es_v_assert_object_format(void *v_node, void *v_fmt);

// Asserts that the normal child count of the given node is between min and max
// inclusive. Throws an error otherwise. Negative constraints indicate "no
// constraint."
// TODO: Use this in all function/unpack implementations.
void es_assert_child_count(
  es_node const * const n,
  intptr_t min, intptr_t max
);

/*****************************
 * Error-Reporting Functions *
 *****************************/

// Reports an error with the given node, displaying the given message on stderr
// along with a representation of the given node. Devours the given message, so
// the caller doesn't need to free it. The _full version prints a full
// representation of the given node instead of an abbreviation, while the
// _light version prints just the node's address.
void es_report_error_light(string *message, es_node const * const n);
void es_report_error(string *message, es_node const * const n);
void es_report_error_full(string *message, es_node const * const n);

// Works like es_report_error but doesn't require (or print info about) a node.
void es_report_free_error(string *message);

// Returns a string containing a trace of resolution of the given link.
string *es_trace_link(es_node const * const n);

// Reports an error with a link, printing the given message on stderr as well
// as an analysis of where the given node (which should be a link node) fails
// to resolve. Devours the given message.
void es_report_broken_link(string *message, es_node const * const n);

// Report an error with evaluation, displaying the given message along with a
// summary of the original node and a full report of the (partial) evaluation
// result. Devours the given message.
void es_report_eval_error(
  es_node const * const orig,
  es_node const * const evald,
  string *message
);


/********************
 * Inline Functions *
 ********************/

static inline size_t es_node_depth(es_node const * const n) {
  size_t result = 0;
  es_node const *p = n;
  do {
    if (p->h.parent == NULL) {
      return result;
    } else {
      result += 1;
      p = p->h.parent;
    }
  } while (1);
  return 0;
}

static inline int es_is_link_type(es_node_type t) {
  return (
    t == ELFSCRIPT_NT_ROOT_LINK
 || t == ELFSCRIPT_NT_GLOBAL_LINK
 || t == ELFSCRIPT_NT_LOCAL_LINK
 || t == ELFSCRIPT_NT_VARIABLE
  );
}

static inline int es_is_link_node(es_node const * const n) {
  return es_is_link_type(n->h.type);
}

static inline int es_is_container_type(es_node_type t) {
  return (
    t == ELFSCRIPT_NT_CONTAINER
 || t == ELFSCRIPT_NT_SCOPE
 || t == ELFSCRIPT_NT_GLOBAL
 || t == ELFSCRIPT_NT_FUNCTION
 || t == ELFSCRIPT_NT_FN_OBJ
 || t == ELFSCRIPT_NT_FN_INT
 || t == ELFSCRIPT_NT_FN_NUM
 || t == ELFSCRIPT_NT_FN_STR
 || t == ELFSCRIPT_NT_FN_AR_INT
 || t == ELFSCRIPT_NT_FN_AR_NUM
 || t == ELFSCRIPT_NT_FN_AR_STR
 || t == ELFSCRIPT_NT_GENERATOR
 || t == ELFSCRIPT_NT_GN_OBJ
 || t == ELFSCRIPT_NT_GN_INT
 || t == ELFSCRIPT_NT_GN_NUM
 || t == ELFSCRIPT_NT_GN_STR
 || t == ELFSCRIPT_NT_GN_AR_INT
 || t == ELFSCRIPT_NT_GN_AR_NUM
 || t == ELFSCRIPT_NT_GN_AR_STR
  );
}

static inline int es_is_container_node(es_node const * const n) {
  return es_is_container_type(n->h.type);
}

static inline int es_is_function_type(es_node_type t) {
  return (
    t == ELFSCRIPT_NT_FUNCTION
 || t == ELFSCRIPT_NT_FN_OBJ
 || t == ELFSCRIPT_NT_FN_INT
 || t == ELFSCRIPT_NT_FN_NUM
 || t == ELFSCRIPT_NT_FN_STR
 || t == ELFSCRIPT_NT_FN_AR_INT
 || t == ELFSCRIPT_NT_FN_AR_NUM
 || t == ELFSCRIPT_NT_FN_AR_STR
  );
}

static inline int es_is_function_node(es_node const * const n) {
  return es_is_function_type(n->h.type);
}

static inline int es_is_generator_type(es_node_type t) {
  return (
    t == ELFSCRIPT_NT_GENERATOR
 || t == ELFSCRIPT_NT_GN_OBJ
 || t == ELFSCRIPT_NT_GN_INT
 || t == ELFSCRIPT_NT_GN_NUM
 || t == ELFSCRIPT_NT_GN_STR
 || t == ELFSCRIPT_NT_GN_AR_INT
 || t == ELFSCRIPT_NT_GN_AR_NUM
 || t == ELFSCRIPT_NT_GN_AR_STR
  );
}

static inline int es_is_generator_node(es_node const * const n) {
  return es_is_generator_type(n->h.type);
}

static inline string** es__p_fmt(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_PROTO);
  return &(n->b.as_proto.format);
}

static inline string** es__o_fmt(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_OBJECT);
  return &(n->b.as_object.format);
}

static inline void** es__o(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_OBJECT);
  return &(n->b.as_object.value);
}

static inline es_int_t* es__i(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_INTEGER);
  return &(n->b.as_integer.value);
}

static inline es_num_t* es__n(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_NUMBER);
  return &(n->b.as_number.value);
}

static inline string** es__s(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_STRING);
  return &(n->b.as_string.value);
}

static inline es_int_t** es__ai(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_ARRAY_INT);
  return &(n->b.as_int_array.values);
}

static inline size_t* es__ai_count(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_ARRAY_INT);
  return &(n->b.as_int_array.count);
}

static inline es_num_t** es__an(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_ARRAY_NUM);
  return &(n->b.as_num_array.values);
}

static inline size_t* es__an_count(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_ARRAY_NUM);
  return &(n->b.as_num_array.count);
}

static inline string*** es__as(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_ARRAY_STR);
  return &(n->b.as_str_array.values);
}

static inline size_t* es__as_count(es_node *n) {
  es_assert_type(n, ELFSCRIPT_NT_ARRAY_STR);
  return &(n->b.as_str_array.count);
}

// Type casting functions with well-defined behavior for out-of-bounds values:
static inline es_int_t es_cast_to_int(es_num_t value) {
  es_num_t rounded = roundf(value);
  if (rounded > (es_num_t) smaxof(es_int_t)) {
    return smaxof(es_int_t);
  } else if (rounded < (es_num_t) sminof(es_int_t)) {
    return sminof(es_int_t);
  }
  return (es_int_t) rounded;
}

static inline es_num_t es_cast_to_num(es_int_t value) {
  return (es_num_t) value;
}

// Converts a node type to the corresponding reference type.
static inline es_ref_type es_nt__rt(es_node_type nt) {
  switch (nt) {
    default:
    case ELFSCRIPT_NT_INVALID:
    case ELFSCRIPT_NT_ANY:
    case ELFSCRIPT_NT_GLOBAL:
      return ELFSCRIPT_RT_INVALID;
    case ELFSCRIPT_NT_CONTAINER:
    case ELFSCRIPT_NT_SCOPE:
    case ELFSCRIPT_NT_FUNCTION:
    case ELFSCRIPT_NT_FN_OBJ:
    case ELFSCRIPT_NT_FN_INT:
    case ELFSCRIPT_NT_FN_NUM:
    case ELFSCRIPT_NT_FN_STR:
    case ELFSCRIPT_NT_FN_AR_INT:
    case ELFSCRIPT_NT_FN_AR_NUM:
    case ELFSCRIPT_NT_FN_AR_STR:
    case ELFSCRIPT_NT_GENERATOR:
    case ELFSCRIPT_NT_GN_OBJ:
    case ELFSCRIPT_NT_GN_INT:
    case ELFSCRIPT_NT_GN_NUM:
    case ELFSCRIPT_NT_GN_STR:
    case ELFSCRIPT_NT_GN_AR_INT:
    case ELFSCRIPT_NT_GN_AR_NUM:
    case ELFSCRIPT_NT_GN_AR_STR:
      return ELFSCRIPT_RT_NODE;
    case ELFSCRIPT_NT_ROOT_LINK:
    case ELFSCRIPT_NT_GLOBAL_LINK:
    case ELFSCRIPT_NT_LOCAL_LINK:
    case ELFSCRIPT_NT_VARIABLE:
      return ELFSCRIPT_RT_CHAIN;
    case ELFSCRIPT_NT_PROTO:
    case ELFSCRIPT_NT_OBJECT:
      return ELFSCRIPT_RT_OBJ;
    case ELFSCRIPT_NT_INTEGER:
      return ELFSCRIPT_RT_INT;
    case ELFSCRIPT_NT_NUMBER:
      return ELFSCRIPT_RT_NUM;
    case ELFSCRIPT_NT_STRING:
      return ELFSCRIPT_RT_STR;
    case ELFSCRIPT_NT_ARRAY_INT:
      return ELFSCRIPT_RT_INT_ARR_ENTRY;
    case ELFSCRIPT_NT_ARRAY_NUM:
      return ELFSCRIPT_RT_NUM_ARR_ENTRY;
    case ELFSCRIPT_NT_ARRAY_STR:
      return ELFSCRIPT_RT_STR_ARR_ENTRY;
  }
}

// Takes a function node and returns the kind of node it will return:
static inline es_node_type es_return_type_of(
  es_node const * const function_node
) {
  switch (function_node->h.type) {
    default:
    case ELFSCRIPT_NT_FUNCTION:
    case ELFSCRIPT_NT_GENERATOR:
      return ELFSCRIPT_NT_ANY;
    case ELFSCRIPT_NT_FN_OBJ:
    case ELFSCRIPT_NT_GN_OBJ:
      return ELFSCRIPT_NT_OBJECT;
    case ELFSCRIPT_NT_FN_INT:
    case ELFSCRIPT_NT_GN_INT:
      return ELFSCRIPT_NT_INTEGER;
    case ELFSCRIPT_NT_FN_NUM:
    case ELFSCRIPT_NT_GN_NUM:
      return ELFSCRIPT_NT_NUMBER;
    case ELFSCRIPT_NT_FN_STR:
    case ELFSCRIPT_NT_GN_STR:
      return ELFSCRIPT_NT_STRING;
    case ELFSCRIPT_NT_FN_AR_INT:
    case ELFSCRIPT_NT_GN_AR_INT:
      return ELFSCRIPT_NT_ARRAY_INT;
    case ELFSCRIPT_NT_FN_AR_NUM:
    case ELFSCRIPT_NT_GN_AR_NUM:
      return ELFSCRIPT_NT_ARRAY_NUM;
    case ELFSCRIPT_NT_FN_AR_STR:
    case ELFSCRIPT_NT_GN_AR_STR:
      return ELFSCRIPT_NT_ARRAY_STR;
  }
}

// Takes a node type and returns the type of function node that returns that
// node type, or INVALID if there is no such type.
static inline es_node_type es_function_type_that_returns(
  es_node_type return_type
) {
  switch (return_type) {
    default:
      return ELFSCRIPT_NT_INVALID;
    case ELFSCRIPT_NT_ANY:
    case ELFSCRIPT_NT_CONTAINER:
      return ELFSCRIPT_NT_FUNCTION;
    case ELFSCRIPT_NT_OBJECT:
      return ELFSCRIPT_NT_FN_OBJ;
    case ELFSCRIPT_NT_INTEGER:
      return ELFSCRIPT_NT_FN_INT;
    case ELFSCRIPT_NT_NUMBER:
      return ELFSCRIPT_NT_FN_NUM;
    case ELFSCRIPT_NT_STRING:
      return ELFSCRIPT_NT_FN_STR;
    case ELFSCRIPT_NT_ARRAY_INT:
      return ELFSCRIPT_NT_FN_AR_INT;
    case ELFSCRIPT_NT_ARRAY_NUM:
      return ELFSCRIPT_NT_FN_AR_NUM;
    case ELFSCRIPT_NT_ARRAY_STR:
      return ELFSCRIPT_NT_FN_AR_STR;
  }
}

// Returns the type of node that the value of the given node will be: uses
// es_return_type_of if it's a function node, or just returns the base type if
// it's not a function node.
static inline es_node_type es_value_type_of(es_node const * const base) {
  if (es_is_function_node(base)) {
    return es_return_type_of(base);
  } else {
    return base->h.type;
  }
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocate and return a new ELFSCRIPT node of the given type. The given string is
// copied, so the caller should clean it up if necessary. The context argument
// is used for local links and variable lookup, and may be NULL.
es_node * create_es_node(
  es_node_type t,
  string const * const name,
  es_node const * const context
);

// Allocate and return a new ELFSCRIPT node of type ELFSCRIPT_NT_OBJECT containing a copy
// of the given object, which is produced according to the given format. The
// format and name are also copied, rather than held as references. The
// "SINGLETON" format can be used for separately-managed objects which can't be
// copied, although the resulting node will not be packable.
es_node * construct_es_obj_node(
  string const * const name,
  es_node const * const context,
  string const * const format,
  void * obj
);

// Allocates and returns a new ELFSCRIPT_NT_INTEGER node with the given value.
es_node * construct_es_int_node(
  string const * const name,
  es_node const * const context,
  es_int_t value
);

// Allocates and returns a new ELFSCRIPT_NT_NUMBER node with the given value.
es_node * construct_es_num_node(
  string const * const name,
  es_node const * const context,
  es_num_t value
);

// Allocates and returns a new ELFSCRIPT_NT_STRING node using a copy of the given
// value string.
es_node * construct_es_str_node(
  string const * const name,
  es_node const * const context,
  string const * const value
);

// Allocates and returns a new ELFSCRIPT_NT_ROOT_LINK node that points to the given
// node. Note that because of the use of addresses, if the given node has
// siblings with identical names the link may not be accurate.
es_node * construct_es_link_node_to(
  string const * const name,
  es_node const * const context,
  es_node const * const target
);

// Allocates and returns a new function node whose type corresponds to the
// given return type. The given function name is copied.
es_node * construct_es_function_node(
  string const * const name,
  es_node const * const context,
  es_node_type returns,
  string const * const function
);

// Allocates and returns a deep copy of the given node, which of course
// includes deep copies of all of the node's children recursively. Note that
// any objects contained in the node or its children are also copied, as it is
// assumed that cleanup_es_node will be sufficient for memory management. The
// copied node has its parent set to NULL, but shares the same context pointer
// as the original.
es_node * copy_es_node(es_node const * const src);

// Same as copy_es_node, but renames the new node, using a copy of the given
// new_name string.
es_node * copy_es_node_as(
  es_node const * const src,
  string const * const new_name
);

// Clean up memory from the given ELFSCRIPT node.
CLEANUP_DECL(es_node);

// Allocate and creates a new ELFSCRIPT address. The given string is copied, so the
// caller should clean it up if necessary. The parent pointer of the new
// address is set, but the next pointer of the parent is not modified.
es_address * create_es_address(
  es_address *parent,
  string const * const name
);

// Allocates and fills in a new ELFSCRIPT address for the given node.
es_address * construct_es_address_of_node(es_node const * const node);

// Allocate an new es_address and copy in information from the given address.
es_address * copy_es_address(es_address const * const src);

// Clean up memory from the given ELFSCRIPT address (and recursively, all of its
// children). Parents of the given address are not affected (so for example if
// the address has a parent it's next pointer should be set to NULL before
// cleanup up its child.
CLEANUP_DECL(es_address);

// Allocate and return a new ELFSCRIPT reference. The given address is copied, so it
// should be freed by the caller if it doesn't need it.
es_reference* create_es_reference(
  es_ref_type type,
  es_address *addr,
  intptr_t idx
);

// Clean up memory from the given reference, including its address.
CLEANUP_DECL(es_reference);

// Allocates and returns a new es_generator_state, using a copy of the given
// name string but taking the given state without copying it.
es_generator_state * create_es_generator_state(
  es_generator_type type,
  string const * const name,
  void *state
);

CLEANUP_DECL(es_generator_state);

/*************
 * Functions *
 *************/

// Functions for using with e.g. l_map to convert elfscript nodes to
// corresponding raw types:
void * v_es__v_i(void *v_node);
void * v_es__v_n(void *v_node);
void * v_es__v_s(void *v_node);
void * v_es__o(void *v_node);

// TODO: Make this stuff thread-safe!
// Sets the ELFSCRIPT error context. The given string is copied, so the caller
// retains responsibility for it.
void es_push_error_context(string const * const context);

// Sets the ELFSCRIPT error context including details of the given node. The given
// message is copied.
void es_push_error_context_with_node(
  string const * const message,
  es_node const * const node
);

// Removes the most-specific element of the current ELFSCRIPT error context.
void es_pop_error_context(void);

// Prints the ELFSCRIPT error context.
void es_print_error_context(void);

// Returns 1 if the given reference types are compatible and 0 otherwise.
int es_ref_types_are_compatible(es_ref_type from, es_ref_type to);

// Checks that a type matches and returns 1 if it does or 0 if it does not (or
// if the given node is NULL). Setting ELFSCRIPT_NO_TYPECHECKS will turn off the type
// check but retain the NULL check.
int es_is_type(es_node const * const n, es_node_type t);

// Given an ELFSCRIPT_NT_PROTO or ELFSCRIPT_NT_OBJECT type node, checks that the format
// matches the given string.
int es_format_is(es_node  const * const n, string const * const fmt);

// Renames the given node using a copy of the given string.
void es_rename(es_node * node, string const * const new_name);

// Creates and returns a string holding the name of the given type:
string * es_type_name(es_node_type t);

// Creates and returns a string holding the abbreviation of the given type:
string * es_type_abbr(es_node_type t);

// Creates a string from an ELFSCRIPT address.
string * es_addr_string(es_address const * a);

// Builds a fully-qualified name for the given node and returns a pointer to a
// newly-allocated string holding this name.
string * es_build_fqn(es_node const * const n);

// Creates a new string containing an abbreviated representation of the given
// ELFSCRIPT node.
string * es_repr(es_node const * const n);

// Creates a new string containing a full representation of the given ELFSCRIPT node.
// This doesn't handle things like global references, so it's not suitable for
// persistence to a file, but it can be used for debugging and diagnostics.
string * es_full_repr(es_node const * const n);

// Gets the children dictionary from a node of any container type (returns NULL
// for non-container nodes and prints a warning if DEBUG is on).
dictionary * es_children_dict(es_node const * const n);

// Compares the two nodes and returns 1 if they are exactly equivalent
// (including any children, recursively) or 0 if they differ. Ancestors are not
// compared, so two "equal" nodes with different ancestors may give different
// results under es_eval (if this is a concern just call es_equals on the
// eval results). NULL arguments are accepted.
int es_equals(es_node const * const cmp, es_node const * const agn);

// Compares two nodes as with es_equals, but only looks at values and ignores
// node names (including for children). Also ignores scopes entirely, testing
// just normal nodes. NULL arguments are accepted.
int es_equivalent(es_node const * const cmp, es_node const * const agn);

// Adds the given child to the parent's dictionary of children (parent must be
// a container node). If the child's context is NULL, sets it to point to the
// parent, but otherwise leaves it alone.
void es_add_child(es_node *n, es_node *child);

// Works like es_add_child but the child is treated as having been defined
// first within its parent.
void es_prepend_child(es_node *n, es_node *child);

// Removes the given child from this node's dictionary of children (this node
// must be a container node). Does not affect the child's context pointer. If
// DEBUG is on and the parent doesn't contain the child, an error message is
// printed.
void es_remove_child(es_node *n, es_node *child);

// Appends the given name to the given address:
void es_append_address(es_address *a, string const * const name);

// Extends the given address using the given extension. The extension can be
// safely forgotten, as future cleanup of the base address will deal with the
// extension as well.
void es_extend_address(es_address *a, es_address *e);

// Adds the given name as the deepest level of the given address.
void es_push_address(es_address *a, string const * const name);

// Removes the deepest level of the given address, returning a pointer to the
// address that was popped (which should eventually be cleaned up).
es_address* es_pop_address(es_address *a);

// Looks up a child node within a parent, returning NULL if no node with the
// given name exists as a child of the given node (or when the given node
// doesn't have children). Prints a warning if the given node is of a
// non-container type. Note that this function does not handle link nodes (see
// es_lookup).
es_node * es_find_child(
  es_node const * const parent,
  string const * const name
);

// Works like es_find_child, but returns a newly-allocated list containing
// *all* children of the given node with the given name. The list's values are
// not copies, so the list should be cleaned up, not destroyed.
list * es_find_all_children(
  es_node const * const parent,
  string const * const name
);

// Look for any scope node(s) within the given node and searches for the target
// variable within them in order, setting the given scope path to the path to
// the matching variable or NULL if there is no match.
es_node * es_find_variable_in(
  es_node const * const base,
  es_address const * const target
);

// Returns a trace of the search for a variable within a node.
string * es_trace_variable_in(
  es_node const * const base,
  es_address const * const target
);

// Takes a variable node and returns the node that it refers to. If no referent
// can be found, it returns NULL. This function searches progressively upwards
// through the ELFSCRIPT tree, returning the first match found. This function just
// does one step of resolution, so the node it returns may still be a link or
// variable.
es_node * es_resolve_variable(es_node const * const var);

// Returns a string showing the trace of search locations checked for the given
// variable.
string * es_variable_search_trace(es_node const * const var);

// Takes a node and returns the concrete node that it refers to, following any
// link(s) encountered until a non-link node is found. If a link is broken or
// if the input is NULL, it returns NULL. Note that this function doesn't
// remember where it's been, so infinite loops can occur.
// TODO: Change that?
es_node * es_concrete(es_node const * const base);

// Returns the number of non-SCOPE children that the given node has. Returns -1
// if the given node is a non-container node.
intptr_t es_normal_child_count(es_node const * const node);

// Returns the nth child of the given node, not counting scope nodes.
es_node * es_nth(es_node const * const node, size_t index);

// This function returns the child with the given key in the given node. It
// iterates over children until it hits one with a matching name, so only the
// first is used if multiple children share a name. The key argument is treated
// as a single key within the given parent node. If no match is found or if the
// input node is NULL it returns NULL. Unlike es_find_child, this function
// properly handles link nodes. This function always calls es_concrete on its
// results, so the result is never a link node.
es_node * es_lookup(es_node const * const node, string const * const key);

// Just like es_lookup but throws an error if it misses.
es_node * es_lookup_expected(
  es_node const * const node,
  string const * const key
);

// Works like es_lookup, returns a list of all matching children with the
// given key, instead of just the first. The list's values are not copies, but
// the list itself should be cleaned up by the caller. If the 'parent' path
// string is passed to es_lookup_all it returns NULL and a warning is printed.
list * es_lookup_all(
  es_node const * const node,
  string const * const key
);

// The most ubiquitous ELFSCRIPT function 'elfscript' does a recursive address
// lookup to find an ELFSCRIPT node given some root node to start from and an
// address to find. Internally it uses es_lookup, so when multiple
// children of a node share a name, the first one is used. If no match is found
// it returns NULL.
es_node * elfscript(es_node const * const root, es_address const * addr);

// Works like elfscript, but instead of taking an address it takes a key which is
// parsed into an address. So
//
//   elfscriptx(r, "foo.bar.baz");
//
// is equivalent to
//
//  elfscript(r, es_parse_address("foo.bar.baz"))
//
// which is further equivalent to
//
//   es_lookup(es_lookup(es_lookup(r, "foo"), "bar"), "baz");
//
// The use of sprintf and passing a dynamic key to elfscriptx is probably better
// solved by a mix of calls to elfscript and elfscriptx to avoid buffer-length problems.
// Note that elfscriptx's use of es_parse_address means that it is limited by
// ELFSCRIPT_MAX_NAME_DEPTH, although multiple calls to elfscript/elfscriptx can overcome this.
es_node * elfscriptx(es_node const * const root, string const * const saddr);

// Evaluates a node, returning a newly-allocated node (which may have newly-
// allocated children) representing the result. External code should almost
// always call es_get_value instead, which returns cached values when
// available and caches new values as they're computed.
es_node * _es_eval(es_node const * const target);

// Gets the value of the given node, either cached in target->h.value, or via
// es_eval (in which case it caches the value). In either case the caller
// doesn't need to worry about cleanup for the node as it will be present in
// the target's cache. This function calls es_concrete on its input before
// trying to fetch a value, so the result won't be the same as just looking up
// target->h.value when the target is a link node. If the input is NULL this
// will return NULL immediately. The returned node will have a NULL parent, but
// its context will be set to the target's context.
es_node * es_get_value(es_node *target);

// Takes a pointer to an ELFSCRIPT node and creates a temporary function node with
// function type "call" that calls that node using the given scope node as
// arguments. Proceeds to evaluate the temporary function node and return its
// value. The newly-returned node and the first argument are the responsibility
// of the caller, but the args node is cleaned up during the evaluation
// process.
es_node * es_call_function(
  es_node const * const function,
  es_node * args
);

// Takes an OBJECT node and returns a new PROTO node whose value is the
// original OBJECT.
es_node * es_pack_object(es_node const * const object);

// TODO: Serialization (separate files)...

// Lookup for functions (defined in es_setup.c).
es_eval_function es_lookup_function(string const * const key);

// Lookup for generator constructors (as above).
es_generator_constructor es_lookup_generator(string const * const key);

// Lookups for packers, unpackers, copiers, and destructors. As above, these
// are not actually defined in elfscript.c but rather in  es_setup.c.
es_object_format * es_lookup_format(string const * const key);
es_unpack_function es_lookup_unpacker(string const * const key);
es_pack_function es_lookup_packer(string const * const key);
es_copy_function es_lookup_copier(string const * const key);
es_destroy_function es_lookup_destructor(string const * const key);

// Functions for getting and setting globals. Note that es_get_global calls
// es_concrete on its result.
es_node* es_get_global(string const * const key);
void es_set_global(string const * const key, es_node *value);

// A function with the same signature as a normal copy function that just
// returns the original pointer (warning: may lead to a double-free if
// misused).
void* dont_copy(void *v);

// A function with the same signature as a normal cleanup function that doesn't
// do anything.
void dont_cleanup(void *v);

// Given a generator, returns the next value from it (a newly allocated ELFSCRIPT
// node) and advances its state. If the generator is exhausted, it will return
// NULL.
es_node * es_gen_next(es_generator_state *gen);

// Resets the given generator.
void es_gen_reset(es_generator_state *gen);

// Calls next on the given generator until it is exhausted, collecting results
// into a newly-allocated container node and  returning that. Warning: calling
// this on an infinite generator will hang and/or crash.
es_node * es_gen_all(es_generator_state *gen);

// Takes an ELFSCRIPT node and returns the corresponding generator object for
// iteration over that node. Works with container, generator function, and
// array nodes, but returns NULL for other node types. If it is given a link
// node it calls es_concrete before attempting to process the result.
es_generator_state * es_generator_for(es_node *node);

// Function for testing ELFSCRIPT conditions, which take the form of a container node
// that has two or more children: first a condition type specified by an
// integer node, and then some arguments. This function implements checking
// whether the condition described by that structure holds for node 'arg'.
int es_condition_holds(es_node *cond, es_node *arg);

// Type coercion functions.
// These functions each call es_concrete and check for NULL inputs, before
// coercing the given node to an appropriate type and extracting its value.
// They throw errors if the input node can't be coerced. They accept functions
// with an appropriate return type as well as plain nodes, in which case they
// use es_get_value to get a result.

es_int_t es_as_i(es_node *n);

es_num_t es_as_n(es_node *n);

string* es_as_s(es_node *n);

void* es_as_o(es_node *n);

void* es_as_o_fmt(es_node *n, string const * const fmt);

size_t es_array_count(es_node *n);

es_int_t* es_as_ai(es_node *n);

es_num_t* es_as_an(es_node *n);

string** es_as_as(es_node *n);

#endif // INCLUDE_ELFSCRIPT_H
