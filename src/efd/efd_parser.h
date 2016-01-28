#ifndef INCLUDE_EFD_PARSER_H
#define INCLUDE_EFD_PARSER_H

// efd_parser.h
// Parsing for the Elf Forest Data format.

/*************
 * Constants *
 *************/

#define EFD_OPEN_NODE "[["
#define EFD_CLOSE_NODE "]]"

/********************
 * Inline Functions *
 ********************/

static inline int is_whitespace(char *c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

/*************
 * Functions *
 *************/

// Top level parsing function that delegates to the more specific functions:
efd_node* efd_parse_any(char const * const raw, ptrdiff_t *pos);


// Parsing functions for the EFD primitive types:
//-----------------------------------------------

efd_node* efd_parse_collection(char const * const raw, ptrdiff_t *pos);

efd_node* efd_parse_proto(char const * const raw, ptrdiff_t *pos);

efd_node* efd_parse_integer(char const * const raw, ptrdiff_t *pos);

efd_node* efd_parse_number(char const * const raw, ptrdiff_t *pos);

efd_node* efd_parse_string(char const * const raw, ptrdiff_t *pos);

efd_node* efd_parse_obj_array(char const * const raw, ptrdiff_t *pos);

efd_node* efd_parse_int_array(char const * const raw, ptrdiff_t *pos);

efd_node* efd_parse_num_array(char const * const raw, ptrdiff_t *pos);

efd_node* efd_parse_str_array(char const * const raw, ptrdiff_t *pos);


// Functions for parsing bits & pieces:
//-------------------------------------

// Parsers for node open/close brackets:
void efd_parse_open(char const * const raw, ptrdiff_t *pos);
void efd_parse_close(char const * const raw, ptrdiff_t *pos);

// Parses a node type off of the front of the input:
efd_node_type efd_parse_type(char const * const raw, ptrdiff_t *pos);

// Parses a node name off of the front of the input. Puts up to
// EFD_NODE_NAME_SIZE characters into r_name (including a NUL terminator),
// overwriting whatever may have been there.
void efd_parse_name(char const * const raw, ptrdiff_t *pos, char *r_name);

// Parses the schema name part of a 'c' or 'o' declaration, including the
// leading ':'. Puts up to EFD_NODE_NAME_SIZE characters (including a trailing
// NUL) into r_name, overwriting any previous contents.
void efd_parse_schema(char const * const raw, ptrdiff_t *pos, char *r_name);

// Parses an integer off of the front of the input:
ptrdiff_t efd_parse_int(char const * const raw, ptrdiff_t *pos);

// Parses a floating-point number off of the front of the input:
float efd_parse_float(char const * const raw, ptrdiff_t *pos);

// Starts at the given starting point and finds an opening quote, setting the
// starting point to point to the character after that quote. Then it scans
// until it finds an (unescaped) closing quote that matches the type (single or
// double) of the first and sets the end to point to the character before that
// position.
void efd_grab_string_limits(
  char const * const raw,
  ptrdiff_t *start,
  ptrdiff_t *end
);

// Skips whitespace and comments:
void efd_parse_skip(char const * const raw, ptrdiff_t *pos);


// Helper functions:
//------------------

void efd_check_parse_progress(ptrdiff_t *pos);

#endif // INCLUDE_EFD_PARSER_H
