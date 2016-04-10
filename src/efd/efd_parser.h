#ifndef INCLUDE_EFD_PARSER_H
#define INCLUDE_EFD_PARSER_H

// efd_parser.h
// Parsing for the Elf Forest Data format.

#include <stddef.h>

#include "efd.h"

/*********
 * Enums *
 *********/

// Different kinds of parsing errors:
enum efd_parse_error_e {
  EFD_PE_NO_ERROR = 0,
  EFD_PE_MISSING,
  EFD_PE_UNKNOWN,
  EFD_PE_MALFORMED,
  EFD_PE_INCOMPLETE
};
typedef enum efd_parse_error_e efd_parse_error;

// States for parsing integers:
enum efd_int_state_e {
  EFD_INT_STATE_PRE,
  EFD_INT_STATE_BASE,
  EFD_INT_STATE_PRE_DIGITS,
  EFD_INT_STATE_DIGITS,
  EFD_INT_STATE_DONE,
};
typedef enum efd_int_state_e efd_int_state;

// States for parsing floats:
enum efd_float_state_e {
  EFD_FLOAT_STATE_PRE,
  EFD_FLOAT_STATE_ZERO,
  EFD_FLOAT_STATE_CHAR,
  EFD_FLOAT_STATE_MANT,
  EFD_FLOAT_STATE_EXP,
  EFD_FLOAT_STATE_EXP_SIGN,
  EFD_FLOAT_STATE_EXP_DIGITS,
  EFD_FLOAT_STATE_DONE,
};
typedef enum efd_float_state_e efd_float_state;

// States for grabbing quoted strings:
enum efd_quote_state_e {
  EFD_QUOTE_STATE_NORMAL,
  EFD_QUOTE_STATE_MAYBE_DONE,
  EFD_QUOTE_STATE_DONE
};
typedef enum efd_quote_state_e efd_quote_state;

// States for skipping whitespace and comments:
enum efd_skip_state_e {
  EFD_SKIP_STATE_NORMAL,
  EFD_SKIP_STATE_MAYBE_COMMENT,
  EFD_SKIP_STATE_LINE_COMMENT,
  EFD_SKIP_STATE_BLOCK_COMMENT,
  EFD_SKIP_STATE_MAYBE_BLOCK_END,
  EFD_SKIP_STATE_DONE
};
typedef enum efd_skip_state_e efd_skip_state;

/**************
 * Structures *
 **************/

struct efd_parse_state_s;
typedef struct efd_parse_state_s efd_parse_state;

/*************
 * Constants *
 *************/

#define EFD_PARSER_MAX_FILENAME_DISPLAY 4096
#define EFD_PARSER_MAX_CONTEXT_DISPLAY 32
#define EFD_PARSER_ERROR_BEFORE 35
#define EFD_PARSER_ERROR_AFTER 35
#define EFD_PARSER_ERROR_LINE 80
// note: (LINE / 2) - BEFORE must be >= 3 and (LINE / 2) - AFTER must be >= 4

#define EFD_PARSER_MAX_DIGITS 1024
#define EFD_PARSER_INT_ERROR 1717
#define EFD_PARSER_INT_REFVAL 1718
#define EFD_PARSER_FLOAT_ERROR 9995.5
#define EFD_PARSER_FLOAT_REFVAL 9995.6

#define EFD_PARSER_COLON ':'
#define EFD_PARSER_OPEN_BRACE '['
#define EFD_PARSER_CLOSE_BRACE ']'
#define EFD_PARSER_REF_OPEN '<'
#define EFD_PARSER_REF_CLOSE '>'
#define EFD_PARSER_EQUALS '='
#define EFD_PARSER_ARRAY_SEP ','
#define EFD_PARSER_HASH '#'
#define EFD_PARSER_RENAME '@'

/*************************
 * Structure Definitions *
 *************************/

struct efd_parse_state_s {
  char const * input;
  ptrdiff_t input_length;
  ptrdiff_t pos;
  char const * filename;
  ptrdiff_t lineno;
  char const * context;
  efd_parse_error error;
  efd_address *current_address;
};

/********************
 * Inline Functions *
 ********************/

static inline int is_whitespace(char *c) {
  return *c == ' ' || *c == '\n' || *c == '\t' || *c == '\r';
}

static inline int is_special(char *c) {
  switch (*c) {
    default:
      return 0;
    case EFD_NODE_SEP:
    case EFD_ADDR_PARENT:
    case EFD_PARSER_COLON:
    case EFD_PARSER_OPEN_BRACE:
    case EFD_PARSER_CLOSE_BRACE:
    case EFD_PARSER_REF_OPEN:
    case EFD_PARSER_REF_CLOSE:
    case EFD_PARSER_EQUALS:
    case EFD_PARSER_ARRAY_SEP:
    case EFD_PARSER_HASH:
    case EFD_PARSER_RENAME:
      return 1;
  }
}

// Checks whether the current character is NUL or beyond the end of the input
// (if input_length is non-negative) and returns 1 if so or 0 otherwise.
static inline int efd_parse_atend(efd_parse_state *s) {
  return (
    (s->input_length != 0 && s->pos >= s->input_length)
 || s->input[s->pos] == '\0'
  );
}

/*************
 * Functions *
 *************/

// State copying for backtracking (input isn't copied):
void efd_parse_copy_state(efd_parse_state *from, efd_parse_state *to);

// Parses an entire file, adding node(s) encountered as children of the given
// parent node (must be a container). Returns 1 if it succeeds or 0 otherwise.
int efd_parse_file(
  efd_node *parent,
  efd_index *cr,
  char const * const filename
);

// Parses a string as an EFD address. The string should be at most
// EFD_MAX_NAME_DEPTH * (EFD_NODE_NAME_SIZE + 1) characters long; further
// characters will be ignored.
efd_address* efd_parse_string_address(char const * const astr);

// Top level parsing function that delegates to the more specific functions:
efd_node* efd_parse_any(efd_parse_state *s, efd_index *cr);


// Parsing functions for the EFD primitive types:
//-----------------------------------------------

void efd_parse_children(efd_node *result, efd_parse_state *s, efd_index *cr);

void efd_parse_proto(efd_node *result, efd_parse_state *s, efd_index *cr);

void efd_parse_integer(efd_node *result, efd_parse_state *s, efd_index *cr);

void efd_parse_number(efd_node *result, efd_parse_state *s, efd_index *cr);

void efd_parse_string(efd_node *result, efd_parse_state *s, efd_index *cr);

void efd_parse_obj_array(efd_node *result, efd_parse_state *s, efd_index *cr);

void efd_parse_int_array(efd_node *result, efd_parse_state *s, efd_index *cr);

void efd_parse_num_array(efd_node *result, efd_parse_state *s, efd_index *cr);

void efd_parse_str_array(efd_node *result, efd_parse_state *s, efd_index *cr);


// Parsing functions for EFD globals:
//-----------------------------------

void efd_parse_int_global(efd_node *result, efd_parse_state *s, efd_index *cr);

void efd_parse_num_global(efd_node *result, efd_parse_state *s, efd_index *cr);

void efd_parse_str_global(efd_node *result, efd_parse_state *s, efd_index *cr);


// Functions for parsing pieces that might be references:
//-------------------------------------------------------

ptrdiff_t efd_parse_int_or_ref(efd_parse_state *s, efd_index *cr);

float efd_parse_float_or_ref(efd_parse_state *s, efd_index *cr);

string* efd_parse_str_or_ref(efd_parse_state *s, efd_index *cr);

void* efd_parse_obj_ref(efd_parse_state *s, efd_index *cr);


// Functions for parsing bits & pieces:
//-------------------------------------

// Parsers for node open/close brackets:
void efd_parse_open(efd_parse_state *s);
void efd_parse_close(efd_parse_state *s);

// Parses a node type off of the front of the input:
efd_node_type efd_parse_type(efd_parse_state *s);

// Parses a node name off of the front of the input. Puts up to
// EFD_NODE_NAME_SIZE characters into r_name (plus one for a NUL terminator),
// overwriting whatever may have been there.
void efd_parse_name(efd_parse_state *s, char *r_name);

// Parses the annotation part of a 'c' or 'o' declaration, including the
// leading ':'. Puts up to EFD_NODE_NAME_SIZE characters (plus one for a
// trailing NUL) into r_name, overwriting any previous contents.
void efd_parse_annotation(efd_parse_state *s, char *r_name);

// Parses an integer off of the front of the input:
ptrdiff_t efd_parse_int(efd_parse_state *s);

// Parses a floating-point number off of the front of the input:
float efd_parse_float(efd_parse_state *s);

// Parses a quoted string off of the front of the input. Returns a newly
// malloc'd string if it succeeds or NULL if it fails (so if it fails, freeing
// the result is unnecessary).
string* efd_parse_str(efd_parse_state *s);

// Starts at the initial position and finds an opening quote, setting the
// starting point to point to that quote. Then it scans until it finds an
// (unescaped) closing quote that matches the type (single or double) of the
// first and sets the end to point to that closing quote.
void efd_grab_string_limits(
  efd_parse_state *s,
  ptrdiff_t *start,
  ptrdiff_t *end
);

// Takes an underlying input string and start/end markers and transfers the
// characters between those markers (exclusive) to a newly-malloc'd string.
// Along the way, replaces double-instances of the opening limiter with single
// instances. The caller should free the return value.
char * efd_purify_string(
  char const * const input,
  ptrdiff_t start,
  ptrdiff_t end
);

// Parses a reference off of the front of the input, allocating and returning a
// new efd_reference.
efd_reference* efd_parse_global_ref(efd_parse_state *s);

// Parses an address off of the front of the input, allocating and returning a
// new efd_address.
efd_address* efd_parse_address(efd_parse_state *s);

// Skips whitespace and comments (// to end of line and /* to */):
void efd_parse_skip(efd_parse_state *s);

// Parses optional whitespace followed by a separator.
void efd_parse_sep(efd_parse_state *s);

// Helper functions:
//------------------

// Checks that the most recent parsing function succeeded and returns 0 if it
// did, or 1 if it indicated an error.
int efd_parse_failed(efd_parse_state *s);

// Prints the error for the given parse state (if there is one; otherwise does
// nothing).
void efd_print_parse_error(efd_parse_state *s);

// Checks if the given parse state reports an error and if it does prints that
// error and exits.
void efd_throw_parse_error(efd_parse_state *s);

#endif // INCLUDE_EFD_PARSER_H
