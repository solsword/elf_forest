#ifndef INCLUDE_ELFSCRIPT_PARSER_H
#define INCLUDE_ELFSCRIPT_PARSER_H

// elfscript_parser.h
// Parsing for ElfScript.

#include <stddef.h>

#include "elfscript.h"

/*********
 * Enums *
 *********/

// Different kinds of parsing errors:
enum elfscript_parse_error_e {
  ELFSCRIPT_PE_NO_ERROR = 0,     // carry on
  ELFSCRIPT_PE_ABORT,            // manual abort
  ELFSCRIPT_PE_UNKNOWN,          // ???
  ELFSCRIPT_PE_MISSING,          // no input
  ELFSCRIPT_PE_MALFORMED,        // wrong input
  ELFSCRIPT_PE_CLOSING_BRACE,    // wrong input (closing brace)
  ELFSCRIPT_PE_INCOMPLETE,       // partial input
  ELFSCRIPT_PE_SKIP_CHILD,       // elfscript_parse_children should skip this node
  ELFSCRIPT_PE_CHILDREN_DONE     // elfscript_parse_children should terminate
};
typedef enum elfscript_parse_error_e elfscript_parse_error;

// States for parsing integers:
enum elfscript_int_state_e {
  ELFSCRIPT_INT_STATE_PRE,
  ELFSCRIPT_INT_STATE_BASE,
  ELFSCRIPT_INT_STATE_PRE_DIGITS,
  ELFSCRIPT_INT_STATE_DIGITS,
  ELFSCRIPT_INT_STATE_DONE,
};
typedef enum elfscript_int_state_e elfscript_int_state;

// States for parsing floats:
enum elfscript_float_state_e {
  ELFSCRIPT_FLOAT_STATE_PRE,
  ELFSCRIPT_FLOAT_STATE_ZERO,
  ELFSCRIPT_FLOAT_STATE_CHAR,
  ELFSCRIPT_FLOAT_STATE_MANT,
  ELFSCRIPT_FLOAT_STATE_EXP,
  ELFSCRIPT_FLOAT_STATE_EXP_SIGN,
  ELFSCRIPT_FLOAT_STATE_EXP_DIGITS,
  ELFSCRIPT_FLOAT_STATE_DONE,
};
typedef enum elfscript_float_state_e elfscript_float_state;

// States for grabbing quoted strings:
enum elfscript_quote_state_e {
  ELFSCRIPT_QUOTE_STATE_NORMAL,
  ELFSCRIPT_QUOTE_STATE_MAYBE_DONE,
  ELFSCRIPT_QUOTE_STATE_DONE,
};
typedef enum elfscript_quote_state_e elfscript_quote_state;

// States for skipping whitespace and comments:
enum elfscript_skip_state_e {
  ELFSCRIPT_SKIP_STATE_NORMAL,
  ELFSCRIPT_SKIP_STATE_MAYBE_COMMENT,
  ELFSCRIPT_SKIP_STATE_LINE_COMMENT,
  ELFSCRIPT_SKIP_STATE_BLOCK_COMMENT,
  ELFSCRIPT_SKIP_STATE_MAYBE_BLOCK_END,
  ELFSCRIPT_SKIP_STATE_DONE,
};
typedef enum elfscript_skip_state_e elfscript_skip_state;

/**************
 * Structures *
 **************/

struct elfscript_parse_state_s;
typedef struct elfscript_parse_state_s elfscript_parse_state;

/*************
 * Constants *
 *************/

#define ELFSCRIPT_PARSER_MAX_FILENAME_DISPLAY 4096
#define ELFSCRIPT_PARSER_MAX_CONTEXT_DISPLAY 160
#define ELFSCRIPT_PARSER_ERROR_BEFORE 35
#define ELFSCRIPT_PARSER_ERROR_AFTER 35
#define ELFSCRIPT_PARSER_ERROR_LINE 80
// note: (LINE / 2) - BEFORE must be >= 3 and (LINE / 2) - AFTER must be >= 4

#define ELFSCRIPT_PARSER_MAX_DIGITS 1024
#define ELFSCRIPT_PARSER_INT_ERROR 1717
#define ELFSCRIPT_PARSER_NUM_ERROR 9995.5

#define ELFSCRIPT_PARSER_COLON ':'
#define ELFSCRIPT_PARSER_OPEN_BRACE '['
#define ELFSCRIPT_PARSER_CLOSE_BRACE ']'
#define ELFSCRIPT_PARSER_OPEN_ANGLE '<'
#define ELFSCRIPT_PARSER_CLOSE_ANGLE '>'
#define ELFSCRIPT_PARSER_OPEN_CURLY '{'
#define ELFSCRIPT_PARSER_CLOSE_CURLY '}'
#define ELFSCRIPT_PARSER_OPEN_PAREN '('
#define ELFSCRIPT_PARSER_CLOSE_PAREN ')'
#define ELFSCRIPT_PARSER_EQUALS '='
#define ELFSCRIPT_PARSER_ARRAY_SEP ','
#define ELFSCRIPT_PARSER_HASH '#'
#define ELFSCRIPT_PARSER_RENAME '@'
#define ELFSCRIPT_PARSER_ABORT '!'

/*************************
 * Structure Definitions *
 *************************/

struct elfscript_parse_state_s {
  char const * input;
  ptrdiff_t input_length;
  ptrdiff_t pos;
  char const * filename;
  ptrdiff_t lineno;
  char next_closing_brace;
  char const * context;
  elfscript_parse_error error;
  elfscript_address *current_address;
  elfscript_node *current_node;
  ptrdiff_t current_index;
};

/********************
 * Inline Functions *
 ********************/

static inline int is_whitespace(char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static inline int is_special(char c) {
  switch (c) {
    default:
      return 0;
    case ELFSCRIPT_ADDR_SEP_CHR:
    case ELFSCRIPT_ADDR_PARENT_CHR:
    case ELFSCRIPT_PARSER_COLON:
    case ELFSCRIPT_PARSER_OPEN_BRACE:
    case ELFSCRIPT_PARSER_CLOSE_BRACE:
    case ELFSCRIPT_PARSER_OPEN_ANGLE:
    case ELFSCRIPT_PARSER_CLOSE_ANGLE:
    case ELFSCRIPT_PARSER_OPEN_CURLY:
    case ELFSCRIPT_PARSER_CLOSE_CURLY:
    case ELFSCRIPT_PARSER_OPEN_PAREN:
    case ELFSCRIPT_PARSER_CLOSE_PAREN:
    case ELFSCRIPT_PARSER_EQUALS:
    case ELFSCRIPT_PARSER_ARRAY_SEP:
    case ELFSCRIPT_PARSER_HASH:
    case ELFSCRIPT_PARSER_RENAME:
      return 1;
  }
}

static inline int is_opening_brace(char c) {
  switch (c) {
    default:
      return 0;
    case ELFSCRIPT_PARSER_OPEN_BRACE:
    case ELFSCRIPT_PARSER_OPEN_ANGLE:
    case ELFSCRIPT_PARSER_OPEN_CURLY:
    case ELFSCRIPT_PARSER_OPEN_PAREN:
    case ELFSCRIPT_PARSER_HASH:
      return 1;
  }
}

static inline int is_closing_brace(char c) {
  switch (c) {
    default:
      return 0;
    case ELFSCRIPT_PARSER_CLOSE_BRACE:
    case ELFSCRIPT_PARSER_CLOSE_ANGLE:
    case ELFSCRIPT_PARSER_CLOSE_CURLY:
    case ELFSCRIPT_PARSER_CLOSE_PAREN:
    case ELFSCRIPT_PARSER_HASH:
      return 1;
  }
}

static inline char closing_brace_for(char o) {
  switch (o) {
    default:
      fprintf(stderr, "ERROR: Invalid opening brace type '%c'.", o);
      return '\0';
    case ELFSCRIPT_PARSER_OPEN_BRACE:
      return ELFSCRIPT_PARSER_CLOSE_BRACE;
    case ELFSCRIPT_PARSER_OPEN_ANGLE:
      return ELFSCRIPT_PARSER_CLOSE_ANGLE;
    case ELFSCRIPT_PARSER_OPEN_CURLY:
      return ELFSCRIPT_PARSER_CLOSE_CURLY;
    case ELFSCRIPT_PARSER_OPEN_PAREN:
      return ELFSCRIPT_PARSER_CLOSE_PAREN;
    case ELFSCRIPT_PARSER_HASH:
      return ELFSCRIPT_PARSER_HASH;
  }
}

// Checks whether the current character is NUL or beyond the end of the input
// (if input_length is non-negative) and returns 1 if so or 0 otherwise.
static inline int elfscript_parse_atend(elfscript_parse_state *s) {
  return (
    (s->input_length != 0 && s->pos >= s->input_length)
 || s->input[s->pos] == '\0'
  );
}

/*************
 * Functions *
 *************/

// State copying for backtracking (input and current node aren't copied):
void elfscript_parse_copy_state(elfscript_parse_state *from, elfscript_parse_state *to);

// Cleans up the state's current address for backtracking:
void elfscript_parse_scrub_state(elfscript_parse_state *state);

// Parses an entire file, adding node(s) encountered as children of the given
// parent node (must be a container). Nodes encountered are unpacked and global
// reference values are filled in. Returns 1 if it succeeds or 0 otherwise.
int elfscript_parse_file(
  elfscript_node *parent,
  char const * const filename
);

// Parses a string as an ELFSCRIPT address and returns a newly-allocated address.
elfscript_address* elfscript_parse_string_address(string const * const astr);

// Top level parsing function that delegates to the more specific functions:
elfscript_node* elfscript_parse_any(elfscript_parse_state *s);


// Parsing functions for the ELFSCRIPT primitive types:
//-----------------------------------------------

void elfscript_parse_children(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_link(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_function(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_proto(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_integer(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_number(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_string(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_obj_array(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_int_array(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_num_array(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_str_array(elfscript_node *result, elfscript_parse_state *s);


// Parsing functions for ELFSCRIPT globals:
//-----------------------------------

void elfscript_parse_int_global(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_num_global(elfscript_node *result, elfscript_parse_state *s);

void elfscript_parse_str_global(elfscript_node *result, elfscript_parse_state *s);


// Functions for parsing pieces that might be references:
//-------------------------------------------------------

elfscript_int_t elfscript_parse_int_or_ref(elfscript_parse_state *s);

elfscript_num_t elfscript_parse_float_or_ref(elfscript_parse_state *s);

string * elfscript_parse_str_or_ref(elfscript_parse_state *s);

void * elfscript_parse_obj_ref(elfscript_parse_state *s);


// Functions for parsing bits & pieces:
//-------------------------------------

// Parsers for node open/close brackets. The first returns a character tracking
// the type of bracket used, and the second accepts that character to ensure a
// match.
char elfscript_parse_open(elfscript_parse_state *s);
void elfscript_parse_close(elfscript_parse_state *s);

// Parses a node type off of the front of the input:
elfscript_node_type elfscript_parse_type(elfscript_parse_state *s);

// Parses a node name off of the front of the input.
string* elfscript_parse_name(elfscript_parse_state *s);

// Parses the annotation part of an 'o', 'l', or 'L' declaration, including the
// leading separator character (which must match the given 'sep' argument.
string* elfscript_parse_annotation(elfscript_parse_state *s, char sep);

// Parses an integer off of the front of the input:
elfscript_int_t elfscript_parse_int(elfscript_parse_state *s);

// Parses a floating-point number off of the front of the input:
elfscript_num_t elfscript_parse_float(elfscript_parse_state *s);

// Parses a quoted string off of the front of the input. Returns a newly
// malloc'd string if it succeeds or NULL if it fails (so if it fails, freeing
// the result is unnecessary).
string* elfscript_parse_str(elfscript_parse_state *s);

// Starts at the initial position and finds an opening quote, setting the
// starting point to point to that quote. Then it scans until it finds an
// (unescaped) closing quote that matches the type (single or double) of the
// first and sets the end to point to that closing quote.
void elfscript_grab_string_limits(
  elfscript_parse_state *s,
  ptrdiff_t *start,
  ptrdiff_t *end
);

// Takes an underlying input string and start/end markers and transfers the
// characters between those markers (exclusive) to a newly-malloc'd string.
// Along the way, replaces double-instances of the opening limiter with single
// instances. The caller should free the return value.
char * elfscript_purify_string(
  char const * const input,
  ptrdiff_t start,
  ptrdiff_t end
);


// Constructs and returns a newly-allocated reference to the node being parsed.
elfscript_reference* construct_elfscript_reference_to_here(elfscript_parse_state* s);

// Parses a reference off of the front of the input, allocating and returning a
// new elfscript_reference.
elfscript_reference* elfscript_parse_global_ref(elfscript_parse_state *s);

// Parses an address off of the front of the input, allocating and returning a
// new elfscript_address.
elfscript_address* elfscript_parse_address(elfscript_parse_state *s);

// Skips whitespace and comments (// to end of line and /* to */):
void elfscript_parse_skip(elfscript_parse_state *s);

// Parses optional whitespace followed by a separator.
void elfscript_parse_sep(elfscript_parse_state *s);

// Helper functions:
//------------------

// Checks that the most recent parsing function succeeded and returns 0 if it
// did, or 1 if it indicated an error.
int elfscript_parse_failed(elfscript_parse_state *s);

// Prints the error for the given parse state (if there is one; otherwise does
// nothing).
void elfscript_print_parse_error(elfscript_parse_state *s);

// Checks if the given parse state reports an error and if it does prints that
// error and exits.
void elfscript_throw_parse_error(elfscript_parse_state *s);

#endif // INCLUDE_ELFSCRIPT_PARSER_H
