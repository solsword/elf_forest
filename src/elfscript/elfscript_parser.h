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
};
typedef enum es_parse_error_e es_parse_error;

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
  return s->error != EFD_PE_NO_ERROR;
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

// State copying for backtracking (input isn't copied):
void es_parse_copy_state(es_parse_state *from, es_parse_state *to);

// Parse a statement
bytecode es_parse_statement(es_parse_state *s);

// Parses the left-hand-side of an assignment and returns bytecode that
// implements the assignment, to be run after the expression code for the
// right-hand side.
bytecode es_parse_lhs(es_parse_state *s);

// Parses an expression, returning bytecode that leaves that expression's value
// alone on top of the value stack.
bytecode es_parse_expression(es_parse_state *s);

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

#endif // INCLUDE_ELFSCRIPT_PARSER_H
