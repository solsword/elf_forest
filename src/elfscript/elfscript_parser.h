#ifndef INCLUDE_ELFSCRIPT_PARSER_H
#define INCLUDE_ELFSCRIPT_PARSER_H

// elfscript_parser.h
// Parsing for ElfScript.

#include <stddef.h>

#include "elfscript.h"

/*********
 * Enums *
 *********/

// Parse errors
enum es_parse_error_e {
  ES_PE_NO_ERROR = 0,
  ES_PE_MALFORMED,
  ES_PE_INCOMPLETE,
  ES_PE_MISSING,
  ES_PE_ABORT,
};
typedef enum es_parse_error_e es_parse_error;

// States for parsing integers:
enum es_int_state_e {
  ES_INT_STATE_PRE,
  ES_INT_STATE_BASE,
  ES_INT_STATE_PRE_DIGITS,
  ES_INT_STATE_DIGITS,
  ES_INT_STATE_DONE,
};
typedef enum es_int_state_e es_int_state;

// States for parsing floats:
enum es_float_state_e {
  ES_FLOAT_STATE_PRE,
  ES_FLOAT_STATE_ZERO,
  ES_FLOAT_STATE_CHAR,
  ES_FLOAT_STATE_MANT,
  ES_FLOAT_STATE_EXP,
  ES_FLOAT_STATE_EXP_SIGN,
  ES_FLOAT_STATE_EXP_DIGITS,
  ES_FLOAT_STATE_DONE,
};
typedef enum es_float_state_e es_float_state;

// States for grabbing quoted strings:
enum es_quote_state_e {
  ES_QUOTE_STATE_NORMAL,
  ES_QUOTE_STATE_MAYBE_DONE,
  ES_QUOTE_STATE_DONE,
};
typedef enum es_quote_state_e es_quote_state;

// States for skipping whitespace and comments:
enum es_skip_state_e {
  ES_SKIP_STATE_NORMAL,
  ES_SKIP_STATE_MAYBE_COMMENT,
  ES_SKIP_STATE_LINE_COMMENT,
  ES_SKIP_STATE_BLOCK_COMMENT,
  ES_SKIP_STATE_MAYBE_BLOCK_END,
  ES_SKIP_STATE_DONE,
};
typedef enum es_skip_state_e es_skip_state;

/**************
 * Structures *
 **************/

struct es_parse_state_s;
typedef struct es_parse_state_s es_parse_state;

struct es_expr_fragment_s;
typedef struct es_expr_fragment_s es_expr_fragment;

struct es_expr_tree_s;
typedef struct es_expr_tree_s es_expr_tree;

/*************
 * Constants *
 *************/

// Constants regulating error reporting:
#define ES_PARSER_MAX_FILENAME_DISPLAY 4096
#define ES_PARSER_MAX_CONTEXT_DISPLAY 160
#define ES_PARSER_ERROR_BEFORE 35
#define ES_PARSER_ERROR_AFTER 35
#define ES_PARSER_ERROR_LINE 80
// note: (LINE / 2) - BEFORE must be >= 3 and (LINE / 2) - AFTER must be >= 4

// Maximum number of digits in a numerical literal
#define ES_PARSER_MAX_DIGITS 1000

#define ES_BAD_INTEGER 1002003004
#define ES_BAD_FLOAT -12e-34

#define ES_CH_STATEMENT_END ';'
#define ES_CH_STATEMENTS_SEP ':'
#define ES_CH_OPEN_PAREN '('
#define ES_CH_CLOSE_PAREN ')'
#define ES_CH_OPEN_SLICE '['
#define ES_CH_CLOSE_SLICE ']'
#define ES_CH_OPEN_SCOPE '{'
#define ES_CH_CLOSE_SCOPE '}'
#define ES_CH_ACCESS '.'
#define ES_CH_GLOBAL '$'
#define ES_CH_ADD '+'
#define ES_CH_SUBTRACT '-'
#define ES_CH_MULTIPLY '*'
#define ES_CH_DIVIDE '/'
#define ES_CH_MOD '%'
#define ES_CH_EXPONENTIATE '^'
#define ES_CH_AND '&'
#define ES_CH_OR '|'
#define ES_CH_XOR '!'
#define ES_CH_NOT '~'
#define ES_CH_LESS '<'
#define ES_CH_GREATER '>'
#define ES_CH_EQUALS '='
#define ES_CH_COMMENT '`'
#define ES_CH_BLOCK_COMMENT '*'
#define ES_CH_NEWLINE '\n'
#define ES_CH_ABORT_PARSING '#'

/*************************
 * Structure Definitions *
 *************************/

struct es_parse_state_s {
  char const * input;
  ptrdiff_t input_length;
  ptrdiff_t pos;
  char const * filename;
  ptrdiff_t lineno;
  char next_closing_brace;
  char const * context;
  es_parse_error error;
};

struct es_expr_fragment_s {
  int pdepth;
  es_bytecode *valcode;
  es_instruction op;
};

struct es_expr_tree_s {
  es_expr_fragment *here;
  es_expr_tree_s *parent;
  es_expr_tree_s *left;
  es_expr_tree_s *right;
};

/********************
 * Inline Functions *
 ********************/

// Checks whether the current character is NUL or beyond the end of the input
// (if input_length is non-negative) and returns 1 if so or 0 otherwise.
static inline int es_parse_atend(es_parse_state *s) {
  return (
    (s->input_length != 0 && s->pos >= s->input_length)
 || s->input[s->pos] == '\0'
  );
}

// Checks whether an error has occurred.
static inline int es_parse_failed(es_parse_state *s) {
  return s->error != ES_PE_NO_ERROR;
}

static inline int es_is_whitespace(char c) {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

static inline int es_is_special(char c) {
  switch (c) {
    default:
      return 0;
    case ES_CH_STATEMENT_END:
    case ES_CH_STATEMENTS_SEP:
    case ES_CH_OPEN_PAREN:
    case ES_CH_CLOSE_PAREN:
    case ES_CH_OPEN_SLICE:
    case ES_CH_CLOSE_SLICE:
    case ES_CH_OPEN_SCOPE:
    case ES_CH_CLOSE_SCOPE:
    case ES_CH_ACCESS:
    case ES_CH_GLOBAL:
    case ES_CH_ADD:
    case ES_CH_SUBTRACT:
    case ES_CH_MULTIPLY:
    case ES_CH_DIVIDE:
    case ES_CH_MOD:
    case ES_CH_EXPONENTIATE:
    case ES_CH_AND:
    case ES_CH_OR:
    case ES_CH_XOR:
    case ES_CH_NOT:
    case ES_CH_LESS:
    case ES_CH_GREATER:
    case ES_CH_EQUALS:
    case ES_CH_COMMENT:
    // case ES_CH_BLOCK_COMMENT: (same as ES_CH_MULTIPLY)
      return 1;
  }
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Creates a new expression fragment. The given bytecode is consumed, and
// becomes the responsibility of the expression fragment.
es_expr_fragment *create_es_expr_fragment(
  int pdepth,
  es_bytecode *valcode,
  es_instruction op
);

// Cleanup for expression fragments.
CLEANUP_DECL(es_expr_fragment);

// Creates an empty expression tree node.
es_expr_tree *create_es_expr_tree(void);

// Cleanup for expression trees.
CLEANUP_DECL(es_expr_tree);

/*************
 * Functions *
 *************/

// Uses es_parse_statements to parse the given file and returns the resulting
// bytecode. This function compiles elfscript to elfscript bytecode.
es_bytecode* es_parse_file(char const * const filename);

// Creates a new parse state using the given filename/line number and parses
// the contents of the given string as a sequence of statements, returning the
// corresponding bytecode.
es_bytecode* es_parse_statements(
  char const * const fname,
  ptrdiff_t first_lineno,
  char const * const src,
  size_t slen
);

// State copying for backtracking (input isn't copied):
void es_parse_copy_state(es_parse_state *from, es_parse_state *to);

// Parse a statement
es_bytecode* es_parse_statement(es_parse_state *s);

// Parses an expression, returning bytecode that leaves that expression's value
// alone on top of the value stack.
es_bytecode* es_parse_expression(es_parse_state *s);

// Takes a list of es_expr_fragments which ends with some at the given paren
// depth, and collapses those into a single fragment, paying attention to the
// order of operations.
void es_parse_aggregate_fragments(list *fragments, int pdepth);

// Propagates bytecode from the leaves upwards to the top of the given
// expression tree. As it goes the lower bytecode is destroyed.
void es_parse_collect_bytecode(tree);

// Parses a unary operator, returning the corresponding instruction. Note that
// some instructions may be refined later based on datatypes.
es_instruction es_parse_unop(es_parse_state *s);

// Parses a binary operator, returning the corresponding instruction. Note that
// some instructions will be refined later based on datatypes.
es_instruction es_parse_binop(es_parse_state *s);

/***********************
 * Operator Precedence *
 ***********************/

// Returns the precedence of the given operator (unary or binary).
int es_op_precedence(es_instruction op);

/*************************
 * Primitive Value Types *
 *************************/

// Parses a value (which might be a variable) and generates code for putting
// that value on the stack.
es_bytecode* es_parse_value(es_parse_state *s);

// Parses an identifier and returns it as a string.
string* es_parse_identifier(es_parse_state *s);

// Parses and returns an integer literal:
es_int_t es_parse_int(es_parse_state *s);

// Parses and returns a floating-point literal:
es_num_t es_parse_float(es_parse_state *s);

// Parses and returns the value of a string literal:
string* es_parse_string(es_parse_state *s);

// Finds the limits of a string literal, setting the start and end return
// parameters to the indices of those positions within the parse state's input.
void es_find_string_limits(
  es_parse_state *s,
  ptrdiff_t *r_start,
  ptrdiff_t *r_end
);

// Given the start and end of a string, handles escaping characters in the
// string to produce a modified character sequence.
char * es_purify_string(
  char const * const input,
  ptrdiff_t start,
  ptrdiff_t end
);

// Skip whitespace and/or comments
void es_parse_skip(es_parse_state *s);

/********************
 * Helper Functions *
 ********************/
void es_print_parse_error(es_parse_state *s);

#endif // INCLUDE_ELFSCRIPT_PARSER_H
