#ifndef INCLUDE_ELFSCRIPT_H
#define INCLUDE_ELFSCRIPT_H

// elfscript.h
// Definition and implementation of ElfScript.

#include <stdint.h>
#include <math.h>

#include "datatypes/list.h"
#include "datatypes/string.h"
#include "datatypes/dictionary.h"
#include "datatypes/dict_string_keys.h"
#include "datatypes/map.h"

#include "boilerplate.h"
#include "util.h"

#include "elfscript_gl.h"

/*********
 * Enums *
 *********/

// The ELFSCRIPT instruction set:
#define ELFSCRIPT_MAX_INSTRUCTION 128
enum es_instruction_e {
  ES_INSTR_NOP        =   0, // -    a no-op
  ES_INSTR_INVALID    =   1, // -    an invalid instruction
  // control flow operations
  ES_INSTR_BRANCH     =  63, //'?'   conditionally execute the following
  ES_INSTR_ELSE       =  58, //':'   beginning of 'else' code
  ES_INSTR_DONE       =  59, //';'   end of block
  // constant operations
  ES_INSTR_LINT       = 105, //'i'   literal int
  ES_INSTR_LNUM       = 110, //'n'   literal number
  ES_INSTR_LSTR       = 115, //'s'   literal string
  // object operations
  ES_INSTR_POBJ       = 111, //'o'   package last scope as a primitive
  ES_INSTR_PSCOPE     =  47, //'O'   push last scope as an object
  ES_INSTR_GET_PROP   =  46, //'.'   get property of an object
  // scope operations
  ES_INSTR_OSC        = 123, //'{'   open scope
  ES_INSTR_CSC        = 125, //'}'   close scope
  ES_INSTR_LIDX       =  40, //'('   load variable by index
  ES_INSTR_SIDX       =  41, //')'   store anonymous value
  ES_INSTR_VAR        =  36, //'$'   reference variable by name
  ES_INSTR_SVAR       =  35, //'#'   store variable by name
  ES_INSTR_LGLB       =  91, //'['   load global
  ES_INSTR_SGLB       =  93, //']'   store global
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

ELFSCRIPT_GL(i, ES_INSTR_NOP)
ELFSCRIPT_GL(i, ES_INSTR_INVALID)

ELFSCRIPT_GL(i, ES_INSTR_BRANCH)

ELFSCRIPT_GL(i, ES_INSTR_LINT)
ELFSCRIPT_GL(i, ES_INSTR_LNUM)
ELFSCRIPT_GL(i, ES_INSTR_LSTR)

ELFSCRIPT_GL(i, ES_INSTR_POBJ)
ELFSCRIPT_GL(i, ES_INSTR_PSCOPE)
ELFSCRIPT_GL(i, ES_INSTR_GET_PROP)

ELFSCRIPT_GL(i, ES_INSTR_OSC)
ELFSCRIPT_GL(i, ES_INSTR_CSC)
ELFSCRIPT_GL(i, ES_INSTR_LIDX)
ELFSCRIPT_GL(i, ES_INSTR_SIDX)
ELFSCRIPT_GL(i, ES_INSTR_VAR)
ELFSCRIPT_GL(i, ES_INSTR_SVAR)
ELFSCRIPT_GL(i, ES_INSTR_LGLB)
ELFSCRIPT_GL(i, ES_INSTR_SGLB)

ELFSCRIPT_GL(i, ES_INSTR_ADD)
ELFSCRIPT_GL(i, ES_INSTR_SUB)
ELFSCRIPT_GL(i, ES_INSTR_NEG)
ELFSCRIPT_GL(i, ES_INSTR_ABS)
ELFSCRIPT_GL(i, ES_INSTR_MULT)
ELFSCRIPT_GL(i, ES_INSTR_FDIV)
ELFSCRIPT_GL(i, ES_INSTR_IDIV)
ELFSCRIPT_GL(i, ES_INSTR_MOD)
ELFSCRIPT_GL(i, ES_INSTR_POW)
ELFSCRIPT_GL(i, ES_INSTR_LOG)

ELFSCRIPT_GL(i, ES_INSTR_BIT_AND)
ELFSCRIPT_GL(i, ES_INSTR_BIT_OR)
ELFSCRIPT_GL(i, ES_INSTR_BIT_XOR)
ELFSCRIPT_GL(i, ES_INSTR_BIT_NOT)

ELFSCRIPT_GL(i, ES_INSTR_AND)
ELFSCRIPT_GL(i, ES_INSTR_OR)
ELFSCRIPT_GL(i, ES_INSTR_NOT)
ELFSCRIPT_GL(i, ES_INSTR_LT)
ELFSCRIPT_GL(i, ES_INSTR_LE)
ELFSCRIPT_GL(i, ES_INSTR_EQ)
ELFSCRIPT_GL(i, ES_INSTR_GE)
ELFSCRIPT_GL(i, ES_INSTR_GT)

ELFSCRIPT_GL(i, ES_INSTR_FDEF)
ELFSCRIPT_GL(i, ES_INSTR_MDEF)
ELFSCRIPT_GL(i, ES_INSTR_GDEF)
ELFSCRIPT_GL(i, ES_INSTR_GMDEF)
ELFSCRIPT_GL(i, ES_INSTR_FCALL)
ELFSCRIPT_GL(i, ES_INSTR_MCALL)
ELFSCRIPT_GL(i, ES_INSTR_BCALL)
ELFSCRIPT_GL(i, ES_INSTR_FJUMP)
ELFSCRIPT_GL(i, ES_INSTR_RET)

// Elfscript data types
enum es_type_e {
  ES_DT_INVALID     = 0, // -   invalid
  ES_DT_ANY            , //'A'  matches any type
  ES_DT_INT            , //'i'  integer
  ES_DT_NUM            , //'n'  number
  ES_DT_STR            , //'s'  string
  ES_DT_SCP            , //'S'  scope
  ES_DT_OBJ            , //'o'  object
  ES_DT_FCN            , //'f'  function
  ES_DT_MTH            , //'m'  method
  ES_DT_GEN            , //'g'  generator
  ES_DT_GNM            , //'M'  generator method
};
typedef enum es_type_e es_type;

ELFSCRIPT_GL(i, ES_DT_INVALID)
ELFSCRIPT_GL(i, ES_DT_ANY)
ELFSCRIPT_GL(i, ES_DT_INT)
ELFSCRIPT_GL(i, ES_DT_NUM)
ELFSCRIPT_GL(i, ES_DT_STR)
ELFSCRIPT_GL(i, ES_DT_SCP)
ELFSCRIPT_GL(i, ES_DT_OBJ)
ELFSCRIPT_GL(i, ES_DT_FCN)
ELFSCRIPT_GL(i, ES_DT_MTH)
ELFSCRIPT_GL(i, ES_DT_GEN)
ELFSCRIPT_GL(i, ES_DT_GNM)

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

ELFSCRIPT_GL(i, ES_GT_INVALID)
ELFSCRIPT_GL(i, ES_GT_VARIABLES)
ELFSCRIPT_GL(i, ES_GT_INDICES)
ELFSCRIPT_GL(i, ES_GT_FUNCTION)
ELFSCRIPT_GL(i, ES_GT_EXTEND_RESTART)
ELFSCRIPT_GL(i, ES_GT_EXTEND_HOLD)
ELFSCRIPT_GL(i, ES_GT_PARALLEL)

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

// An entry on the Elfscript value stack
struct es_stack_entry_s;
typedef struct es_stack_entry_s es_stack_entry;

// An Elfscript scope
struct es_scope_s;
typedef struct es_scope_s es_scope;

// An Elfscript variable (to be stored in a scope)
struct es_var_s;
typedef struct es_var_s es_var;

// A header for an object specifying its format and pointing to the object
// itself.
struct es_obj_s;
typedef struct es_obj_s es_obj;

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
typedef es_scope* (*es_probj_unpackage_function)(es_probj_t);
typedef es_probj_t (*es_probj_package_function)(es_scope *);
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
#define ELFSCRIPT_BYTECODE_LARGE_SIZE 128

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

struct es_stack_entry_s {
  es_type type;
  es_val_t value;
  // TODO: HERE?
};

struct es_scope_s {
  dictionary *variables;
  // TODO: HERE?
};

struct es_var_s {
  int refcount;
  es_type type;
  es_val_t value;
};

struct es_obj_s {
  es_object_format *format;
  es_probj_t value;
};

struct es_object_format_s {
  char *key;
  es_probj_unpackage_function unpacker;
  es_probj_package_function packer;
  es_probj_copy_function copier;
  es_probj_destroy_function destructor;
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

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new bytecode object.
es_bytecode* create_es_bytecode(void);

// Creates a new bytecode object with a specific starting capacity.
es_bytecode* create_es_bytecode_sized(size_t size);

// Copies the given bytecode object.
es_bytecode* copy_es_bytecode(es_bytecode *src);

// Clean up memory from the given bytecode.
CLEANUP_DECL(es_bytecode);

// Allocates and returns a new scope object.
es_scope* create_es_scope(void);

// Cleans up memory from the given scope object. As part of this process,
// decrements the refcounts of each variable in the scope, potentially causing
// them to be cleaned up as a result.
CLEANUP_DECL(es_scope);

// These allocate and return new variable objects of the appropriate types.
es_var* create_es_var(es_type type, es_val_t value);
es_var* create_es_int_var(es_int_t value);
es_var* create_es_num_var(es_num_t value);
es_var* create_es_str_var(string* value);
es_var* create_es_obj_var(es_obj* value);
es_var* create_es_scp_var(es_scope* value);

// Clean up memory from the given variable.
CLEANUP_DECL(es_var);

// Allocates and returns a new object.
es_obj* create_es_obj(es_object_format *format, es_probj_t obj);

// Cleans up the given object.
CLEANUP_DECL(es_obj);

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

// Clean up memory from a generator state.
CLEANUP_DECL(es_generator_state);

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
  // TODO: fulfill promise about out-of-bounds values above?
  return (es_num_t) value;
}

// Code to ensure that a bytecode object has capacity for the given number of
// extra bytes:
static inline void es_ensure_bytecode_capacity(es_bytecode *code, size_t extra){
  char *expanded;
  if (code->len + extra <= code->capacity) {
    return; // do nothing; capacity is fine
  }
  // Need to reallocate:
  while (code->len + extra > code->capacity) {
    code->capacity *= 2;
  }
  expanded = (char*) malloc(sizeof(char) * code->capacity);
  memcpy(expanded, code->bytes, code->len);
  free(code->bytes);
  code->bytes = expanded;
}

// Gets the value of a variable and converts it to an integer. Works on int-
// and num-type variables.
static inline es_int_t es_as_i(es_var *var) {
  switch (var->type) {
    case ES_DT_INT:
      return (es_int_t) var->value;
    case ES_DT_NUM:
      return es_cast_to_int((es_num_t) var->value);
    default:
#ifdef DEBUG
      fprintf(stderr, "ERROR: es_as_i called on non-numeric variable.\n");
#endif
      return 0;
  }
}

// Gets the value of a variable and converts it to a number. Works on int- and
// num-type variables.
static inline es_num_t es_as_n(es_var *var) {
  switch (var->type) {
    case ES_DT_INT:
      return es_cast_to_num((es_int_t) var->value);
    case ES_DT_NUM:
      return (es_num_t) var->value;
    default:
#ifdef DEBUG
      fprintf(stderr, "ERROR: es_as_n called on non-numeric variable.\n");
#endif
      return 0.0;
  }
}

// Gets the value of a variable and converts it to a string. Only works on
// str-type variables.
static inline string* es_as_s(es_var *var) {
  switch (var->type) {
    case ES_DT_STR:
      return (string*) var->value;
    default:
#ifdef DEBUG
      fprintf(stderr, "ERROR: es_as_s called on non-string variable.\n");
#endif
      return NULL;
  }
}

// Gets the value of a variable and converts it to an object. Only works on
// obj-type variables.
static inline es_obj* es_as_obj(es_var *var) {
  switch (var->type) {
    case ES_DT_OBJ:
      return (es_obj*) var->value;
    default:
#ifdef DEBUG
      fprintf(stderr, "ERROR: es_as_obj called on non-object variable.\n");
#endif
      return NULL;
  }
}

// Gets the value of a variable and converts it to a scope. Only works on
// scp-type variables.
static inline es_scope* es_as_scope(es_var *var) {
  switch (var->type) {
    case ES_DT_OBJ:
      return (es_scope*) var->value;
    default:
#ifdef DEBUG
      fprintf(stderr, "ERROR: es_as_scope called on non-string variable.\n");
#endif
      return NULL;
  }
}

// Gets the raw value of a variable as a void*. "Works" on any kind of variable.
static inline void* es_raw_value(es_var *var) {
  return (void*) var->value;
}

// Increments the reference counter of the given variable.
static inline void es_incref(es_var *var) {
  var->refcount += 1;
}

// Decrements the reference counter of the given variable, cleaning it up if
// the reference counter becomes <= 0.
static inline void es_decref(es_var *var) {
  var->refcount -= 1;
  if (var->refcount <= 0) {
    cleanup_es_var(var);
  }
}

// Version for use with data structures; as a real function so it can be
// pointed to.
void es_v_decref(void *v_var);

// Ensures that the value stored in the given object has the given format
// (specified by its key), and returns the value. If the format doesn't match,
// it returns NULL.
static inline es_probj_t es_obj_as_fmt(es_obj *obj, string const * const fmt) {
  if (s_check_bytes(fmt, obj->format->key)) {
    return obj->value;
#ifdef DEBUG
  } else {
    fprintf(stderr, "ERROR: es_obj_as_fmt format mismatch.\n");
  }
#endif
  return NULL;
}

/*************
 * Functions *
 *************/

// Writes the given code to the given filename, overwriting the previous
// contents if there were any. The format includes a header.
void es_write_esb(char const * const filename, es_bytecode *code);

// Reads the contents of a .esb file and returns the bytecode stored within.
es_bytecode * es_load_esb(char const * const filename);

// Returns a new bytecode object which concatenates the bytecode from the given
// two objects. Frees each of the given objects.
es_bytecode* es_join_bytecode(es_bytecode *first, es_bytecode *second);

// Adds the given instruction to the end of the given code.
void es_add_instruction(es_bytecode *code, es_instruction instr);

// Functions for adding literals to bytecode.
void es_add_int_literal(es_bytecode *code, es_int_t value);
void es_add_num_literal(es_bytecode *code, es_num_t value);
void es_add_str_literal(es_bytecode *code, string *value);

// Adds a variable reference to bytecode.
void es_add_var(es_bytecode *code, string *value);

// Returns the number of variables in the given scope.
size_t es_scope_size(es_scope *sc);

// Returns the nth variable in the given scope. Starts counting from zero.
es_var * es_read_nth(es_scope *sc, size_t n);

// Looks up the given name within the given scope and returns a pointer to the
// associated variable, or NULL if that name doesn't exist in the given scope.
es_var * es_read_var(es_scope *sc, string *name);

// Adds the given value as the last element of the given scope, giving it the
// name ELFSCRIPT_ANON_NAME (thus potentially shadowing anything that had that
// name previously; see datatypes/dictionary.h for the relevant behavior).
void es_write_last(es_scope *sc, es_var *value);

// Sets the given name within the given scope to the given value. If there was
// a value there previously, it is kicked out and its reference count is
// decreased, possibly causing it to be destroyed.
void es_write_var(es_scope *sc, string *name, es_var *value);

// Reports an error during Elfscript evaluation.
void es_report_error(string *message);

/********************
 * Lookup Functions *
 ********************/

// Note: these are defined in elfscript_setup.c, not elfscript.c.

// Lookup for functions
es_eval_function es_lookup_function(string const * const key);

// Lookup for generator constructors
es_generator_constructor es_lookup_generator(string const * const key);

// Lookups for packers, unpackers, copiers, and destructors.
es_object_format * es_lookup_format(string const * const key);
es_probj_unpackage_function es_lookup_unpacker(string const * const key);
es_probj_package_function es_lookup_packer(string const * const key);
es_probj_copy_function es_lookup_copier(string const * const key);
es_probj_destroy_function es_lookup_destructor(string const * const key);

#endif // INCLUDE_ELFSCRIPT_H
