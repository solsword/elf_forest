#ifndef INCLUDE_ELFSCRIPT_H
#define INCLUDE_ELFSCRIPT_H

// elfscript.h
// Definition and implementation of ElfScript.

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
  ES_INSTR_NOP        =   0, // -    a no-op
  ES_INSTR_INVALID    =   1, // -    an invalid instruction
  // constant operations
  ES_INSTR_CINT       = 105, //'i'   constant int
  ES_INSTR_CNUM       = 110, //'n'   constant number
  ES_INSTR_CSTR       = 115, //'s'   constant string
  // object operations
  ES_INSTR_POBJ       = 111, //'o'   package last scope as a primitive
  ES_INSTR_OPROP      =  46, //'.'   get property of a primitive object
  ES_INSTR_PSCOPE     =  47, //'O'   push last scope as an object
  ES_INSTR_SCPROP     =  58, //':'   get property of a scope object
  // scope operations
  ES_INSTR_OSC        = 123, //'{'   open scope
  ES_INSTR_CSC        = 125, //'}'   close scope
  ES_INSTR_LIDX       =  40, //'('   load variable by index
  ES_INSTR_SIDX       =  41, //')'   store anonymous value
  ES_INSTR_LVAR       =  91, //'['   load variable by name
  ES_INSTR_SVAR       =  93, //']'   store variable by name
  ES_INSTR_LGLB       =  36, //'$'   load global
  ES_INSTR_SGLB       =  35, //'#'   store global
  // math operations
  ES_INSTR_ADD        =  43, //'+'   addition
  ES_INSTR_SUB        =  45, //'-'   subtraction
  ES_INSTR_NEG        =  33, //'!'   negation
  ES_INSTR_ABS        =  97, //'a'   absolute value
  ES_INSTR_MULT       =  42, //'*'   multiplication
  ES_INSTR_FDIV       =  47, //'/'   floating point division
  ES_INSTR_IDIV       =  95, //'_'   integer division
  ES_INSTR_MOD        =  37, //'%'   modulus
  ES_INSTR_POW        =  94, //'^'   exponentiation
  ES_INSTR_LOG        =  76, //'L'   log_x(y)
  // bitwise operations
  ES_INSTR_BIT_AND    =  38, //'&'   bitwise AND
  ES_INSTR_BIT_OR     = 124, //'|'   bitwise OR
  ES_INSTR_BIT_XOR    =  88, //'X'   bitwise XOR
  ES_INSTR_BIT_NOT    = 126, //'~'   bitwise NOT
  // logical operations
  ES_INSTR_AND        =  65, //'A'   logical AND
  ES_INSTR_OR         =  79, //'O'   logical OR
  ES_INSTR_NOT        =  78, //'N'   logical NOT
  ES_INSTR_LT         =  60, //'<'   logical LESS_THAN
  ES_INSTR_LE         = 108, //'l'   logical LESS_THAN_OR_EQUAL_TO
  ES_INSTR_EQ         =  61, //'='   logical EQUALS
  ES_INSTR_GE         = 103, //'g'   logical GREATER_THAN_OR_EQUAL_TO
  ES_INSTR_GT         =  62, //'>'   logical GREATER_THAN
  // function operations
  ES_INSTR_FDEF       =  92, //'\'   define function
  ES_INSTR_MDEF       =  39, //'''   define method
  ES_INSTR_GDEF       =  96, //'`'   define generator
  ES_INSTR_GMDEF      =  34, //'"'   define generator method
  ES_INSTR_DEFEND     =  64, //'@'   end of definition
  ES_INSTR_FCALL      = 102, //'f'   function call (begin args code)
  ES_INSTR_MCALL      = 109, //'m'   method call (begin args code)
  ES_INSTR_BCALL      =  98, //'b'   builtin call (begin args code)
  ES_INSTR_FJUMP      = 106, //'j'   function call (jump to function)
  ES_INSTR_RET        = 114, //'r'   return from function
};
typedef enum es_instruction_e es_instruction;

ES_GL(i, ES_INSTR_NOP)
ES_GL(i, ES_INSTR_INVALID)

ES_GL(i, ES_INSTR_CINT)
ES_GL(i, ES_INSTR_CNUM)
ES_GL(i, ES_INSTR_CSTR)

ES_GL(i, ES_INSTR_POBJ)
ES_GL(i, ES_INSTR_OPROP)
ES_GL(i, ES_INSTR_PSCOPE)
ES_GL(i, ES_INSTR_SCPROP)

ES_GL(i, ES_INSTR_OSC)
ES_GL(i, ES_INSTR_CSC)
ES_GL(i, ES_INSTR_LIDX)
ES_GL(i, ES_INSTR_SIDX)
ES_GL(i, ES_INSTR_LVAR)
ES_GL(i, ES_INSTR_SVAR)
ES_GL(i, ES_INSTR_LGLB)
ES_GL(i, ES_INSTR_SGLB)

ES_GL(i, ES_INSTR_ADD)
ES_GL(i, ES_INSTR_SUB)
ES_GL(i, ES_INSTR_NEG)
ES_GL(i, ES_INSTR_ABS)
ES_GL(i, ES_INSTR_MULT)
ES_GL(i, ES_INSTR_FDIV)
ES_GL(i, ES_INSTR_IDIV)
ES_GL(i, ES_INSTR_MOD)
ES_GL(i, ES_INSTR_POW)
ES_GL(i, ES_INSTR_LOG)

ES_GL(i, ES_INSTR_BIT_AND)
ES_GL(i, ES_INSTR_BIT_OR)
ES_GL(i, ES_INSTR_BIT_XOR)
ES_GL(i, ES_INSTR_BIT_NOT)

ES_GL(i, ES_INSTR_AND)
ES_GL(i, ES_INSTR_OR)
ES_GL(i, ES_INSTR_NOT)
ES_GL(i, ES_INSTR_LT)
ES_GL(i, ES_INSTR_LE)
ES_GL(i, ES_INSTR_EQ)
ES_GL(i, ES_INSTR_GE)
ES_GL(i, ES_INSTR_GT)

ES_GL(i, ES_INSTR_FDEF)
ES_GL(i, ES_INSTR_MDEF)
ES_GL(i, ES_INSTR_GDEF)
ES_GL(i, ES_INSTR_GMDEF)
ES_GL(i, ES_INSTR_FCALL)
ES_GL(i, ES_INSTR_MCALL)
ES_GL(i, ES_INSTR_BCALL)
ES_GL(i, ES_INSTR_FJUMP)
ES_GL(i, ES_INSTR_RET)

// Elfscript data types
enum es_type_e {
  ES_DT_INVALID     = 0, // -   invalid
  ES_DT_ANY            , //'A'  matches any type
  ES_DT_INT            , //'i'  integer
  ES_DT_NUM            , //'n'  number
  ES_DT_STR            , //'s'  string
  ES_DT_OBJ            , //'o'  object
  ES_DT_FCN            , //'f'  function
  ES_DT_MTH            , //'m'  method
  ES_DT_GEN            , //'g'  generator
  ES_DT_GNM            , //'M'  generator method
};
typedef enum es_type_e es_type;

ES_GL(i, ES_DT_INVALID)
ES_GL(i, ES_DT_ANY)
ES_GL(i, ES_DT_INT)
ES_GL(i, ES_DT_NUM)
ES_GL(i, ES_DT_STR)
ES_GL(i, ES_DT_OBJ)
ES_GL(i, ES_DT_FCN)
ES_GL(i, ES_DT_MTH)
ES_GL(i, ES_DT_GEN)
ES_GL(i, ES_DT_GNM)

// Elfscript generator types
enum es_generator_type_e {
  ES_GT_INVALID        = 0,
  ES_GT_VARIABLES         , // iterate through variables in a scope
  ELFSCRIPT_GT_INDICES           , // iterate through array entries
  ES_GT_FUNCTION          , // call a generator function
  ES_GT_EXTEND_RESTART    , // extend by restarting
  ES_GT_EXTEND_HOLD       , // extend repeating the final value
  ES_GT_PARALLEL          , // generate parallel results
};
typedef enum es_generator_type_e es_generator_type;

ES_GL(i, ES_GT_INVALID)
ES_GL(i, ES_GT_VARIABLES)
ES_GL(i, ES_GT_INDICES)
ES_GL(i, ES_GT_FUNCTION)
ES_GL(i, ES_GT_EXTEND_RESTART)
ES_GL(i, ES_GT_EXTEND_HOLD)
ES_GL(i, ES_GT_PARALLEL)

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

// A chunk of Elfscript bytecode
struct es_bytecode_s;
typedef struct es_bytecode_s es_bytecode;

// A slice within a variable for access or assignment
struct es_slice_s;
typedef struct es_slice_s es_slice;

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

#define ELFSCRIPT_BYTECODE_STARTING_SIZE 16

#define ELFSCRIPT_ADDR_SEP_CHR '.'

/***********
 * Globals *
 ***********/

// A switch to control tracing and a list of error contexts for tracing:
extern int ELFSCRIPT_TRACK_ERROR_CONTEXTS;
extern list *ELFSCRIPT_ERROR_CONTEXT;

extern string const * const ELFSCRIPT_FILE_EXTENSION;

extern string const * const ELFSCRIPT_SCRIPT_DIR_NAME;

extern string const * const ELFSCRIPT_ADDR_SEP_STR;

extern string const * const ELFSCRIPT_ANON_NAME;
extern string const * const ELFSCRIPT_GLOBAL_NAME;

extern char const * const ELFSCRIPT_DT_NAMES[];
extern char const * const ELFSCRIPT_DT_ABBRS[];
extern char const * const ELFSCRIPT_GT_NAMES[];

extern es_scope *ELFSCRIPT_GLOBAL_SCOPE;

extern dictionary *ELFSCRIPT_GLOBAL_VALS;

/*************************
 * Structure Definitions *
 *************************/

struct es_bytecode_s {
  size_t capacity;
  size_t len;
  char *bytes;
};

struct es_slice_s {
  ptrdiff_t start;
  ptrdiff_t end;
  ptrdiff_t step;
};

struct es_stack_s {
  list *values;
};

// HERE

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
// NULL). Can be turned off by defining ELFSCRIPT_NO_TYPECHECKS, although the
// NULL check will still take place.
void es_assert_type(es_node const * const n, es_node_type t);

// Asserts that the given node is a function or generator node and that its
// return type matches the given type. Throws an error if the type doesn't
// match. Defining ELFSCRIPT_NO_TYPECHECKS will make this a no-op. Unlike
// es_assert_type, this does not check whether the incoming node is NULL.
void es_assert_return_type(es_node const * const n, es_node_type t);

// Asserts that the given node is an object node with the given format. Throws
// an error if either of those assertions are false. Defining
// ELFSCRIPT_NO_TYPECHECKS will make this a no-op. Calls es_assert_type which
// performs a NULL check.
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

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new bytecode object.
es_bytecode* create_es_bytecode(void);

// Creates a new bytecode object with a specific starting capacity.
es_bytecode* create_es_bytecode_sized(size_t size);

// Copies the given bytecode object.
es_bytecode* copy_es_bytecode(es_bytecode *src);

// Clean up memory from the given object.
CLEANUP_DECL(es_bytecode);

// Allocates and returns a new slice object.
es_slice* create_es_slice(void);

// Clean up memory from a slice.
CLEANUP_DECL(es_slice);

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

// Returns a new bytecode object which concatenates the bytecode from the given
// two objects. Frees each of the given objects.
es_bytecode* es_join_bytecode(es_bytecode *first, es_bytecode *second);

// Adds the given instruction to the end of the given code.
void es_add_instruction(es_bytecode *code, es_instruction instr);
