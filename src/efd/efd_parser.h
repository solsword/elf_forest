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
#define EFD_PARSER_ERROR_CONTEXT 20
#define EFD_PARSER_MAX_CONTEXT_DISPLAY 32

#define EFD_PARSER_MAX_DIGITS 1024
#define EFD_PARSER_INT_ERROR 1717
#define EFD_PARSER_FLOAT_ERROR 9995.5

#define EFD_PARSER_OPEN_BRACE '['
#define EFD_PARSER_CLOSE_BRACE ']'
#define EFD_PARSER_ARRAY_SEP ','

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
};

/********************
 * Inline Functions *
 ********************/

static inline int is_whitespace(char *c) {
  return *c == ' ' || *c == '\n' || *c == '\t' || *c == '\r';
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
int efd_parse_file(efd_node *parent, char const * const filename);

// Top level parsing function that delegates to the more specific functions:
efd_node* efd_parse_any(efd_parse_state *s);


// Parsing functions for the EFD primitive types:
//-----------------------------------------------

void efd_parse_children(efd_node *result, efd_parse_state *s);

void efd_parse_proto(efd_node *result, efd_parse_state *s);

void efd_parse_integer(efd_node *result, efd_parse_state *s);

void efd_parse_number(efd_node *result, efd_parse_state *s);

void efd_parse_string(efd_node *result, efd_parse_state *s);

void efd_parse_obj_array(efd_node *result, efd_parse_state *s);

void efd_parse_int_array(efd_node *result, efd_parse_state *s);

void efd_parse_num_array(efd_node *result, efd_parse_state *s);

void efd_parse_str_array(efd_node *result, efd_parse_state *s);


// Functions for parsing bits & pieces:
//-------------------------------------

// Parsers for node open/close brackets:
void efd_parse_open(efd_parse_state *s);
void efd_parse_close(efd_parse_state *s);

// Parses a node type off of the front of the input:
efd_node_type efd_parse_type(efd_parse_state *s);

// Parses a node name off of the front of the input. Puts up to
// EFD_NODE_NAME_SIZE characters into r_name (including a NUL terminator),
// overwriting whatever may have been there.
void efd_parse_name(efd_parse_state *s, char *r_name);

// Parses the schema name part of a 'c' or 'o' declaration, including the
// leading ':'. Puts up to EFD_NODE_NAME_SIZE characters (including a trailing
// NUL) into r_name, overwriting any previous contents.
void efd_parse_schema(efd_parse_state *s, char *r_name);

// Parses an integer off of the front of the input:
ptrdiff_t efd_parse_int(efd_parse_state *s);

// Parses a floating-point number off of the front of the input:
float efd_parse_float(efd_parse_state *s);

// Parses an object reference off of the front of the input:
void* efd_parse_ref(efd_parse_state *s);

// Starts at the initial position and finds an opening quote, setting the
// starting point to point to the byte after that quote. Then it scans until it
// finds an (unescaped) closing quote that matches the type (single or double)
// of the first and sets the end to point to the character before that
// position.
void efd_grab_string_limits(
  efd_parse_state *s,
  ptrdiff_t *start,
  ptrdiff_t *end
);

// Skips whitespace and comments (// to end of line and /* to */):
void efd_parse_skip(efd_parse_state *s);

// Parses optional whitespace followed by a separator.
void efd_parse_sep(efd_parse_state *s);

// Helper functions:
//------------------

// Checks that the most recent parsing function succeeded and returns 0 if it
// did, or 1 if it indicated an error.
int efd_parse_failed(efd_parse_state *s);

// Checks if the given parse state reports an error and if it does prints that
// error and exits.
void efd_throw_parse_error(efd_parse_state *s);

// Checks whether the most recent parsing function succeeded and if it did,
// returns 1. If it failed, returns 0 and resets the parsing position to the
// given reset point.
int efd_test_parse_progress(efd_parse_state *s, ptrdiff_t reset_to);

#endif // INCLUDE_EFD_PARSER_H
