// elfscript_parser.c
// Parsing for ElfScript.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include <string.h>

#include "util.h"

#include "filesys/filesys.h"

#include "elfscript_parser.h"
#include "elfscript.h"

/**********************
 * Boilerplate Macros *
 **********************/

#define SKIP_OR_ELSE(S, X) \
  es_parse_skip(S); \
  if (es_parse_failed(S)) { \
    if (s->context == NULL) { \
      s->context = (X); \
    } \
    return; \
  }

#define SKIP_OR_RETURN(S, X, Y) \
  es_parse_skip(S); \
  if (es_parse_failed(S)) { \
    if (s->context == NULL) { \
      s->context = (X); \
    } \
    return (Y); \
  }

/******************************
 * Constructors & Destructors *
 ******************************/

es_expr_fragment *create_es_expr_fragment(
  int pdepth,
  es_bytecode *valcode,
  es_instruction op
) {
  es_expr_fragment *result = (es_expr_fragment*) malloc(
    sizeof(es_expr_fragment)
  );
  result->pdepth = pdepth;
  result->valcode = valcode;
  result->op = op;
}

CLEANUP_IMPL(es_expr_fragment) {
  if (doomed->valcode != NULL) {
    cleanup_es_bytecode(doomed->valcode);
  }
  free(doomed);
}

es_expr_tree *create_es_expr_tree(void) {
  es_expr_tree *result = (es_expr_tree*) malloc(sizeof(es_expr_tree));
  result->parent = NULL;
  result->here = NULL;
  result->left = NULL;
  result->right = NULL;
}

CLEANUP_IMPL(es_expr_tree) {
  if (doomed->here != NULL) {
    cleanup_es_expr_fragment(doomed->here);
  }
  if (doomed->left != NULL) {
    cleanup_es_expr_tree(doomed->left);
  }
  if (doomed->right != NULL) {
    cleanup_es_expr_tree(doomed->right);
  }
  free(doomed);
}

/*************
 * Functions *
 *************/

es_bytecode* es_parse_file(char const * const filename) {
  size_t flen;
  contents = load_file(filename, &flen);
  return es_parse_statements(filename, 0, contents, flen);
}

es_bytecode* es_parse_statements(
  char const * const fname,
  ptrdiff_t first_lineno,
  char const * const src,
  size_t slen
) {
  es_parse_state s;
  char *contents;

  s.input = NULL;
  s.input_length = 0;
  s.pos = 0;
  s.filename = fname;
  s.lineno = first_lineno;
  s.next_closing_brace = '\0';
  s.context = NULL;
  s.error = ES_PE_NO_ERROR;
  s.input_length = slen;
  s.input = src;

  es_bytecode *result = create_es_bytecode();
  es_bytecode *next = es_parse_statement(s);
  while (!es_parse_failed(&s) && !es_parse_atend(&s)) {
    result = es_join_bytecode(result, next);
    next = es_parse_statement(&s);
  }
  if (s->error != ES_PE_MISSING) {
    es_print_parse_error(&s);
    cleanup_es_bytecode(result);
    return NULL;
  }
  return result;
}


void es_parse_copy_state(es_parse_state *from, es_parse_state *to) {
  to->pos = from->pos;
  to->filename = from->filename;
  to->lineno = from->lineno;
  to->next_closing_brace = from->next_closing_brace;
  to->context = from->context;
  to->error = from->error;
}

es_bytecode* es_parse_statement(es_parse_state *s) {
  es_bytecode *code;

  code = es_parse_expression(s);
  if (es_parse_failed(s) || es_parse_atend(s)) {
    return NULL;
  }
  if (
    s->input[s->pos] != ES_CH_STATEMENT_END
 && s->input[s->pos] != ES_CH_STATEMENTS_SEP
  ) {
    cleanup_es_bytecode(code);
    s->error = ES_PE_MALFORMED;
    s->context = "statement (must end with ';' or ',')";
    return NULL;
  }
  s->pos += 1;
  return code;
}

es_bytecode* es_parse_expression(es_parse_state *s) {
  // A stack of es_expr_fragments
  list *fragments = create_list();
  int pdepth = 0;
  int done = 0;

  es_bytecode *valcode;
  es_expr_fragment *fragment;
  es_expr_fragment *prev_fragment;

  SKIP_OR_RETURN(s, "expression (beginning)", NULL);
  if (es_parse_atend(s)) {
    s->error = ES_PE_MISSING;
    s->context = "expression (missing)";
  }
  while (!done) {
    switch (s->input[s->pos]) {
      case ES_CH_OPEN_PAREN:
        // open paren
        pdepth += 1;
        s->pos += 1;
        break;
      case ES_CH_CLOSE_PAREN:
        // close paren
        es_parse_aggregate_fragments(fragments, pdepth);
        pdepth -= 1;
        if (pdepth < 0) {
          s->error = ES_PE_MALFORMED;
          s->context = "expression (extra closing paren)";
          l_foreach(fragments, &cleanup_v_es_expr_fragment);
          cleanup_list(fragments);
          return NULL;
        }
        s->pos += 1;
        break;
      case ES_CH_STATEMENT_END:
      case ES_CH_STATEMENTS_SEP:
        // end-of-statement
        // (don't advance parser so that es_parse_statement can see it)
        done = 1;
        break;
      default:
        // might be a unary operation in front:
        fragment = create_es_expr_fragment(pdetph, NULL, ES_INSTR_NOP);
        fragment->op = es_parse_unop(s);
        if (!es_parse_failed(s)) {
          // transfers ownership of fragment:
          l_append_element(fragments, (void*) fragment);
          // go back to top of loop in case next is paren or another unop:
          continue;
        } else {
          // ignore failure
          cleanup_es_expr_fragment(fragment);
          s->error = ES_PE_NO_ERROR;
          s->context = NULL;
        }
        fragment = NULL;

        // some kind of value
        valcode = es_parse_value(s);
        if (es_parse_failed(s)) {
          l_foreach(fragments, &cleanup_v_es_expr_fragment);
          cleanup_list(fragments);
          return NULL;
        }
        es_parse_skip(s);
        if (es_parse_failed(s)) {
          l_foreach(fragments, &cleanup_v_es_expr_fragment);
          cleanup_list(fragments);
          cleanup_es_bytecode(valcode);
          return NULL;
        }
        // transfers ownership of valcode to a *new* fragment:
        fragment = create_es_expr_fragment(pdepth, valcode, ES_INSTR_NOP)
        // transfers ownership of fragment:
        l_append_element(fragments, (void*) fragment);

        // next is an operator (closing paren or semicolon will be no-op):
        fragment->op = es_parse_binop(s);
        if (es_parse_failed(s)) {
          l_foreach(fragments, &cleanup_v_es_expr_fragment);
          cleanup_list(fragments);
          return NULL;
        }
        es_parse_skip(s);
        if (es_parse_failed(s)) {
          l_foreach(fragments, &cleanup_v_es_expr_fragment);
          cleanup_list(fragments);
          return NULL;
        }
        break;
        // next time through the loop we'll parse the coming value
    }
    if (es_parse_atend(s)) {
      break;
    }
  }
  if (pdepth != 0) {
    l_foreach(fragments, &cleanup_v_es_expr_fragment);
    cleanup_list(fragments);
    s->error = ES_PE_MALFORMED;
    s->context = "expression (missing closing paren)";
    return NULL;
  }
  // Handle order-of-operations:
  es_parse_aggregate_fragments(fragments, 0);

  // Get remaining fragment:
  fragment = (es_expr_fragment*) l_get_item(fragments, 0);
  if (
    fragment == NULL
 || fragment->pdepth != -1
 || fragment->op != ES_INSTR_NOP
 || l_get_length(fragments) != 1
  ) {
    l_foreach(fragments, &cleanup_v_es_expr_fragment);
    cleanup_list(fragments);
    s->error = ES_PE_MALFORMED;
    s->context = "expression (unable to aggregate)";
    return NULL;
  }

  // The result
  es_bytecode *result = copy_es_bytecode(fragment->valcode);

  // Cleanup
  l_foreach(fragments, &cleanup_v_es_expr_fragment);
  cleanup_list(fragments);

  // Return
  return result;
}

void es_parse_aggregate_fragments(list *fragments, int pdepth) {
  es_expr_fragment *fragment;
  es_expr_tree *tree = create_es_expr_tree();
  es_expr_tree *finger, *temp;

  // Construct an expression tree according to operator precedence
  while (((es_expr_fragment*) l_get_last(fragments))->pdepth == pdepth) {
    fragment = (es_expr_fragment*) l_pop_element(fragments);
    if (tree->here == NULL) {
      if (fragment->valcode == NULL) {
        s->error = ES_PE_MALFORMED;
        s->context = "aggregation (unary operator at end of expression)";
        cleanup_es_expr_tree(tree);
        cleanup_es_expr_fragment(fragment);
        return;
      }
      tree->here = fragment;
    } else if (tree->here->left == NULL) {
      if (fragment->valcode == NULL) { // a unary operation
        tree->left = create_es_expr_tree();
        tree->left->parent = tree;
        tree->left->here = tree->here;
        // right remains NULL
        tree->here = create_es_expr_fragment(-1, NULL, fragment->op);
      } else {
        tree->right = create_es_expr_tree();
        tree->right->parent = tree;
        tree->right->here = tree->here;
        tree->left = create_es_expr_tree();
        tree->left->parent = tree;
        tree->left->here = fragment;
        tree->here = create_es_expr_fragment(-1, NULL, tree->left->here->op);
        tree->left->here->op = ES_INSTR_NOP;
      }
    } else {
      finger = tree;
      // scan down left branches to find correct position:
      while (
        es_op_precedence(finger->here->op) < es_op_precedence(fragment->op)
     && finger->left != NULL
      ) {
        finger = finger->left;
      }
      // insert as new leftmost or by rotating existing to the right:
      if (es_op_precedence(finger->here->op) < es_op_precedence(fragment->op)) {
        // finger->left == NULL
        // insert as new leftmost node
        // will never happen for unary ops (their precedence is maximal)
        finger->right = create_es_expr_tree();
        finger->right->parent = finger;
        finger->right->here = finger->here;
        finger->left = create_es_expr_tree();
        finger->left->parent = finger;
        finger->left->here = fragment;
        finger->here = create_es_expr_fragment(-1, NULL, fragment->op);
        finger->left->here->op = ES_INSTR_NOP;
      } else {
        // finger->left != NULL
        if (fragment->valcode == NULL) { // a unary operation
          temp = create_es_expr_tree();
          temp->parent = finger->parent;
          finger->parent->left = temp;
          temp->left = finger;
          // right remains NULL
          finger->parent = temp;
          temp->here = create_es_expr_fragment(-1, NULL, fragment->op);
          // update the root if we were the root
          if (finger == tree) {
            tree = temp;
          }
        } else {
          // insert as new node with old node as right child
          temp = create_es_expr_tree();
          temp->parent = finger->parent;
          finger->parent->left = temp;
          temp->right = finger;
          finger->parent = temp;
          temp->here = create_es_expr_fragment(-1, NULL, fragment->op);
          temp->left = create_es_expr_tree();
          temp->left->parent = temp;
          temp->left->here = fragment;
          temp->left->here->op = ES_INSTR_NOP;
          // update the root if we were the root
          if (finger == tree) {
            tree = temp;
          }
        }
      }
    }
  }

  // propagate bytecode upwards:
  es_parse_collect_bytecode(tree);

  // copy out the combined code:
  es_bytecode *combined = copy_es_bytecode(tree->here->valcode);

  // clean up everything (except the combined code)
  cleanup_es_expr_tree(tree);

  // add a new fragment to the end of the list at a lower pdepth:
  fragment = create_es_expr_fragment(pdepth - 1, combined, ES_INSTR_NOP);
  l_append_element(fragments, (void*) fragment);
}

void es_parse_collect_bytecode(tree) {
  if (tree->here->valcode != NULL) {
    // leaf node (has a value)
    return;
  }

  // recursive calls first:
  if (tree->left != NULL) {
    es_parse_collect_bytecode(tree->left);
  }
  if (tree->right != NULL) {
    es_parse_collect_bytecode(tree->right);
  }

  // collect children's values:
  if (tree->left == NULL) {
#ifdef DEBUG
    if (tree->right != NULL) {
#endif
      // single child on the right
      // grab the right code for ourselves
      tree->here->valcode = tree->right->valcode;
      tree->right->valcode = NULL;
#ifdef DEBUG
    } else {
      fprintf(
        stderr,
        "Error: childless, valueless node in es_parse_collect_bytecode\n"
      );
#endif
    }
  } else if (tree->right == NULL) {
      // single child on the left
      // grab the left code for ourselves
      tree->here->valcode = tree->left->valcode;
      tree->left->valcode = NULL;
  } else {
    // combine code to put lhs on stack and then rhs on top of it:
    tree->here->valcode = es_join_bytecode(
      tree->left->here->valcode,
      tree->right->here->valcode
    );
    // NULL-out now-freed code slots:
    tree->left->here->valcode = NULL;
    tree->right->here->valcode = NULL;
  }

  // add op code
  if (tree->here->op != ES_INSTR_NOP) {
    es_add_instruction(tree->here->valcode, tree->here->op);
  }
  tree->here->op = ES_INSTR_NOP;
}

es_instruction es_parse_unop(es_parse_state *s) {
  switch (s->input[s->pos]) {
    case ES_CH_ADD:
      s->pos += 1;
      return ES_INSTR_ABS;
    case ES_CH_SUBTRACT:
      s->pos += 1;
      return ES_INSTR_NEG;
    case ES_CH_NOT:
      s->pos += 1;
      return ES_INSTR_BIT_NOT;
    case ES_CH_XOR:
      s->pos += 1;
      return ES_INSTR_NOT;
    default:
      s->error = ES_PE_MALFORMED;
      s->context = "unary operator (unknown operator)";
      return ES_INSTR_INVALID;
  }
}

es_instruction es_parse_binop(es_parse_state *s) {
  switch (s->input[s->pos]) {
    case ES_CH_STATEMENT_END:
    case ES_CH_STATEMENTS_SEP:
      // don't advance s->pos
      return ES_INSTR_NOP;
    /*
     * TODO: Implement slicing
    case ES_CH_OPEN_SLICE:
      sub = es_parse_subscript(s);
      if (es_parse_failed(s)) {
        return ES_INSTR_INVALID;
      }
      break;
    */
    case ES_CH_ACCESS:
      s->pos += 1;
      return ES_INSTR_GET_PROP;
    case ES_CH_ADD:
      s->pos += 1;
      return ES_INSTR_ADD;
    case ES_CH_SUBTRACT:
      s->pos += 1;
      return ES_INSTR_SUB;
    case ES_CH_MULTIPLY:
      s->pos += 1;
      return ES_INSTR_MULT;
    case ES_CH_DIVIDE:
      s->pos += 1;
      if (!es_parse_atend(s) && s->input[s->pos] == ES_CH_DIVIDE) {
        // '//' -> integer division
        s->pos += 1;
        return ES_INSTR_IDIV;
      }
      return ES_INSTR_FDIV;
    case ES_CH_MOD:
      s->pos += 1;
      return ES_INSTR_MOD;
    case ES_CH_EXPONENTIATE:
      s->pos += 1;
      return ES_INSTR_POW;
    case ES_CH_AND:
      s->pos += 1;
      if (!es_parse_atend(s) && s->input[s->pos] == ES_CH_AND) {
        s->pos += 1;
        return ES_INSTR_AND;
      }
      return ES_INSTR_BIT_AND;
    case ES_CH_OR:
      s->pos += 1;
      if (!es_parse_atend(s) && s->input[s->pos] == ES_CH_OR) {
        s->pos += 1;
        return ES_INSTR_OR;
      }
      return ES_INSTR_BIT_OR;
    case ES_CH_XOR:
      s->pos += 1;
      return ES_INSTR_BIT_XOR;
    case ES_CH_LESS:
      s->pos += 1;
      if (!es_parse_atend(s) && s->input[s->pos] == ES_CH_EQUALS) {
        s->pos += 1;
        return ES_INSTR_LE;
      }
      return ES_INSTR_LT;
    case ES_CH_GREATER:
      s->pos += 1;
      if (!es_parse_atend(s) && s->input[s->pos] == ES_CH_EQUALS) {
        s->pos += 1;
        return ES_INSTR_GE;
      }
      return ES_INSTR_GT;
    case ES_CH_EQUALS:
      s->pos += 1;
      if (!es_parse_atend(s) && s->input[s->pos] == ES_CH_EQUALS) {
        s->pos += 1;
        return ES_INSTR_EQ;
      }
      return ES_INSTR_SVAR;
    default:
      s->error = ES_PE_MALFORMED;
      s->context = "binary operator (unknown operator)";
      return ES_INSTR_INVALID;
  }
}

/***********************
 * Operator Precedence *
 ***********************/

int es_op_precedence(es_instruction op) {
  switch (op) {
    default:
#ifdef DEBUG
      fprintf(stderr, "Request for precedence of bad operator '%c'.\n", op)
#endif
    case ES_INSTR_NOP:
      return -1;
    // '.' for getting properties of objects
    case ES_INSTR_GET_PROP:
      return 16384;
    // unary operations:
    case ES_INSTR_NEG:
    case ES_INSTR_ABS:
    case ES_INSTR_BIT_NOT:
    case ES_INSTR_NOT:
      return 4096;
    // bitwise operations
    case ES_INSTR_BIT_AND:
    case ES_INSTR_BIT_OR:
    case ES_INSTR_BIT_XOR:
      return 2048
    // exponentiation and logarithm:
    case ES_INSTR_POW:
    case ES_INSTR_LOG:
      return 1024;
    // multiplication
    case ES_INSTR_MULT:
    case ES_INSTR_FDIV:
    case ES_INSTR_IDIV:
      return 512;
    // modulus
    case ES_INSTR_MOD:
      return 256;
    // addition
    case ES_INSTR_ADD:
    case ES_INSTR_SUB:
      return 128;
    // comparisons
    case ES_INSTR_LT:
    case ES_INSTR_LE:
    case ES_INSTR_EQ:
    case ES_INSTR_GE:
    case ES_INSTR_GT:
      return 64;
    // logical operations
    case ES_INSTR_AND:
    case ES_INSTR_OR:
      return 32;
  }
}

/*************************
 * Primitive Value Types *
 *************************/

es_bytecode* es_parse_value(es_parse_state *s) {
  es_bytecode *result;
  es_parse_state backup;
  es_int_t int_value;
  es_num_t num_value;
  string* str_value;

  SKIP_OR_RETURN(s, "value (beginning)", NULL);
  if (es_parse_atend(s)) {
    s->error = ES_PE_INCOMPLETE;
    s->context = "value (missing)";
    return NULL;
  }

  es_parse_copy_state(s, &backup);

  switch (s->input[s->pos]) {
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
    case '-': case '+':
      // Either an integer or a number:
      // Try to parse as an integer:
      int_value = es_parse_int(s);
      if (!es_parse_failed(s)) {
        // If successful, return our result:
        result = create_es_bytecode();
        es_add_int_literal(result, int_value);
        return result;
      } else {
        // Else back out and try parsing as a floating-point number:
        es_parse_copy_state(&backup, s);
      }
      num_value = es_parse_float(s);
      if (es_parse_failed(s)) {
        return NULL;
      }
      result = create_es_bytecode();
      es_add_num_literal(result, num_value);
      return result;
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'n': case 'm':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'N': case 'M':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case '_':
      // An identifier
      str_value = es_parse_identifier(s);
      if (es_parse_failed(s)) {
        return NULL;
      }
      result = create_es_bytecode();
      es_add_var(result, str_value);
      return result;
    case '"':
    case '\'':
      // A string literal
      str_value = es_parse_string(s);
      if (es_parse_failed(s)) {
        return NULL;
      }
      result = create_es_bytecode();
      es_add_str_literal(result, str_value);
      return result;
    default:
      s->error = ES_PE_MALFORMED;
      s->context = "value (invalid starting character)";
      return NULL;
  }
}

string* es_parse_identifier(es_parse_state *s) {
  char c;
  char const *start;
  size_t len;

  es_parse_skip(s);
  if (es_parse_failed(s)) {
    s->context = "identifier";
    return NULL;
  }

  start = (s->input + s->pos);
  len = 0;

  if (es_parse_atend(s)) {
    s->error = ES_PE_INCOMPLETE;
    s->context = "identifier (missing)";
    return NULL;
  }

  c = s->input[s->pos];

  if (c >= '0' && c <= '9') {
    s->error = ES_PE_MALFORMED;
    s->context = "identifier (initial numeral not allowed)";
    return NULL;
  }

  // Scan until we hit the end of input or find a whitespace or special char:
  while (
    !es_parse_atend(s)
 && !(
      es_is_whitespace(c)
   || es_is_special(c)
    )
  ) {
    len += 1;
    s->pos += 1;
    c = s->input[s->pos];
  }
  if (len == 0) {
    s->error = ES_PE_MISSING;
    s->context = "identifier (missing)";
    return NULL;
  }
  return create_string_from_chars(start, len);
}

es_int_t es_parse_int(es_parse_state *s) {
  es_int_t result;
  es_int_t sign;
  es_int_t base;
  es_int_t digit;
  es_int_t count;
  es_int_state state;
  char c;

  es_parse_skip(s);
  if (es_parse_failed(s)) {
    s->context = "integer (pre)";
    return ES_BAD_INTEGER;
  }

  state = ES_INT_STATE_PRE;
  result = 0;
  sign = 1;
  base = 10;
  digit = 0;
  count = 0;
  while (!es_parse_atend(s) && state != ES_INT_STATE_DONE) {
    c = s->input[s->pos];
    if (c == '\n') {
      s->lineno += 1;
    }
    switch (c) {
      default:
        if (state == ES_INT_STATE_DIGITS || state == ES_INT_STATE_BASE) {
          state = ES_INT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        } else {
          s->error = ES_PE_MALFORMED;
          s->context = "integer";
          return ES_BAD_INTEGER;
        }
        break;

      case '-':
        if (state == ES_INT_STATE_PRE) {
          if (sign == 1) {
            sign = -1;
          } else {
            s->error = ES_PE_MALFORMED;
            s->context = "integer (multiple signs)";
            return ES_BAD_INTEGER;
          }
        } else if (state == ES_INT_STATE_DIGITS) {
          state = ES_INT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        } else if (state == ES_INT_STATE_BASE) {
          state = ES_INT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        } else if (state == ES_INT_STATE_PRE_DIGITS) {
          state = ES_INT_STATE_DONE;
          s->pos -= 2; // about to be incremented before leaving while
        } else {
          s->error = ES_PE_MALFORMED;
          s->context = "integer (minus)";
          return ES_BAD_INTEGER;
        }
        break;
      case 'x':
        if (state == ES_INT_STATE_BASE) {
          base = 16;
          state = ES_INT_STATE_PRE_DIGITS;
        } else if (state == ES_INT_STATE_DIGITS) {
          state = ES_INT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        } else {
          s->error = ES_PE_MALFORMED;
          s->context = "integer (x)";
          return ES_BAD_INTEGER;
        }
        break;
      case '0':
        if (state == ES_INT_STATE_PRE) {
          state = ES_INT_STATE_BASE;
        } else if (
          state == ES_INT_STATE_PRE_DIGITS
       || state == ES_INT_STATE_DIGITS
        ) {
          state = ES_INT_STATE_DIGITS;
          result *= base;
          count += 1;
        } else {
          s->error = ES_PE_MALFORMED;
          s->context = "integer (00)";
          return ES_BAD_INTEGER;
        }
        break;
      case '1':
        digit = 1;
        goto lbl_int_digit;
      case '2':
        digit = 2;
        goto lbl_int_digit;
      case '3':
        digit = 3;
        goto lbl_int_digit;
      case '4':
        digit = 4;
        goto lbl_int_digit;
      case '5':
        digit = 5;
        goto lbl_int_digit;
      case '6':
        digit = 6;
        goto lbl_int_digit;
      case '7':
        digit = 7;
        goto lbl_int_digit;
      case '8':
        digit = 8;
        goto lbl_int_digit;
      case '9':
        digit = 9;
        goto lbl_int_digit;
      case 'a':
      case 'A':
        digit = 10;
        goto lbl_int_hex_digit;
      case 'b':
      case 'B':
        digit = 11;
        goto lbl_int_hex_digit;
      case 'c':
      case 'C':
        digit = 12;
        goto lbl_int_hex_digit;
      case 'd':
      case 'D':
        digit = 13;
        goto lbl_int_hex_digit;
      case 'e':
      case 'E':
        digit = 14;
        goto lbl_int_hex_digit;
      case 'f':
      case 'F':
        digit = 15;
        goto lbl_int_hex_digit;
      case '\0': // impossible
      lbl_int_hex_digit:
        if (base != 16) {
          if (state == ES_INT_STATE_DIGITS) {
            state = ES_INT_STATE_DONE;
            s->pos -= 1; // about to be incremented before leaving while
            break;
          } else {
            s->error = ES_PE_MALFORMED;
            s->context = "integer (unexpected hex digit)";
            return ES_BAD_INTEGER;
          }
        }
        // flow through...
      lbl_int_digit:
        if (state == ES_INT_STATE_BASE) {
          s->error = ES_PE_MALFORMED;
          s->context = "integer (bad base)";
          return ES_BAD_INTEGER;
        }
        state = ES_INT_STATE_DIGITS;
        result *= base;
        result += digit;
        count += 1;
        break;
    }
    s->pos += 1;
    if (count > ES_PARSER_MAX_DIGITS) {
      s->error = ES_PE_MALFORMED;
      s->context = "integer (too many digits)";
      return ES_BAD_INTEGER;
    }
  }
  if (
    state == ES_INT_STATE_DONE
 || state == ES_INT_STATE_BASE
 || state == ES_INT_STATE_DIGITS
  ) {
    return sign * result;
  } else if (state == ES_INT_STATE_PRE_DIGITS) {
    s->pos -= 1;
    return 0;
  } else {
    s->error = ES_PE_MALFORMED;
    s->context = "integer";
    return ES_BAD_INTEGER;
  }
}

es_num_t es_parse_float(es_parse_state *s) {
  es_num_t sign;
  es_num_t characteristic;
  es_num_t mantissa;
  es_num_t mant_div;
  es_num_t expsign;
  es_num_t exponent;
  es_num_t digit;
  intptr_t count;
  es_float_state state;
  char c;

  es_parse_skip(s);
  if (es_parse_failed(s)) {
    s->context = "integer (pre)";
    return ES_BAD_FLOAT;
  }

  state = ES_FLOAT_STATE_PRE;
  sign = 1;
  characteristic = 0;
  mantissa = 0;
  mant_div = 1;
  expsign = 0;
  exponent = 0;
  digit = 0;
  count = 0;

  while (!es_parse_atend(s) && state != ES_FLOAT_STATE_DONE) {
    c = s->input[s->pos];
    if (c == '\n') {
      s->lineno += 1;
    }
    switch (c) {
      default:
        if (state == ES_FLOAT_STATE_EXP_SIGN) {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 2; // skip back to the 'e' or 'E' before us
        } else if (state == ES_FLOAT_STATE_EXP) {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 3; // skip back over the '+'/'-' to the 'e' or 'E' before
        } else if (state != ES_FLOAT_STATE_PRE) {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        } else {
          s->error = ES_PE_MALFORMED;
          s->context = "number";
          return ES_BAD_FLOAT;
        }
        break;

      case '-':
        if (state == ES_FLOAT_STATE_PRE) {
          if (sign == 1) {
            sign = -1;
          } else {
            s->error = ES_PE_MALFORMED;
            s->context = "number (multiple signs)";
            return ES_BAD_FLOAT;
          }
        } else if (state == ES_FLOAT_STATE_EXP_SIGN) {
          if (expsign == 0) {
            expsign = -1;
            state = ES_FLOAT_STATE_EXP;
          } else {
            state = ES_FLOAT_STATE_DONE;
            s->pos -= 3; // skip back to the 'e' or 'E'
          }
        } else if (state == ES_FLOAT_STATE_EXP && count == 0) {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 3; // skip back to the 'e' or 'E'
        } else {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        }
        break;

      case '+':
        if (state == ES_FLOAT_STATE_EXP_SIGN) {
          if (expsign == 0) {
            expsign = 1;
            state = ES_FLOAT_STATE_EXP;
          } else {
            state = ES_FLOAT_STATE_DONE;
            s->pos -= 3; // skip back to the 'e' or 'E'
          }
        } else if (state == ES_FLOAT_STATE_EXP && count == 0) {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 3; // skip back to the 'e' or 'E'
        } else if (state == ES_FLOAT_STATE_PRE) {
          s->error = ES_PE_MALFORMED;
          s->context = "number (initial '+')";
          return ES_BAD_FLOAT;
        } else {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        }
        break;

      case '.':
        if (
          state == ES_FLOAT_STATE_PRE
       || state == ES_FLOAT_STATE_ZERO
       || state == ES_FLOAT_STATE_CHAR
        ) {
          state = ES_FLOAT_STATE_MANT;
          count = 0; // reset the count
        } else if (state == ES_FLOAT_STATE_EXP_SIGN) {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 2; // back to the 'e' or 'E'
        } else if (state == ES_FLOAT_STATE_EXP) {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 3; // back to the 'e' or 'E' before the '+' or '-'
        } else {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        }
        break;

      case 'e':
      case 'E':
        if (state == ES_FLOAT_STATE_CHAR || state == ES_FLOAT_STATE_MANT) {
          state = ES_FLOAT_STATE_EXP_SIGN;
          count = 0;
        } else {
          state = ES_FLOAT_STATE_DONE;
          s->pos -= 1; // about to be incremented before leaving while
        }
        break;

      case '0':
        if (state == ES_FLOAT_STATE_PRE) {
          state = ES_FLOAT_STATE_ZERO;
        } else if (state == ES_FLOAT_STATE_ZERO) {
          s->error = ES_PE_MALFORMED;
          s->context = "number (00)";
          return ES_BAD_FLOAT;
        } else if (
          state == ES_FLOAT_STATE_CHAR
       || state == ES_FLOAT_STATE_MANT
       || state == ES_FLOAT_STATE_EXP_SIGN
       || state == ES_FLOAT_STATE_EXP
       || state == ES_FLOAT_STATE_EXP_DIGITS
        ) {
          digit = 0;
          goto lbl_float_digit;
        }
        break;
      case '1':
        digit = 1;
        goto lbl_float_digit;
      case '2':
        digit = 2;
        goto lbl_float_digit;
      case '3':
        digit = 3;
        goto lbl_float_digit;
      case '4':
        digit = 4;
        goto lbl_float_digit;
      case '5':
        digit = 5;
        goto lbl_float_digit;
      case '6':
        digit = 6;
        goto lbl_float_digit;
      case '7':
        digit = 7;
        goto lbl_float_digit;
      case '8':
        digit = 8;
        goto lbl_float_digit;
      case '9':
        digit = 9;
        goto lbl_float_digit;
      case '\0': // impossible
      lbl_float_digit:
        if (state == ES_FLOAT_STATE_PRE || state == ES_FLOAT_STATE_CHAR) {
          state = ES_FLOAT_STATE_CHAR;
          characteristic *= 10;
          characteristic += digit;
        } else if (state == ES_FLOAT_STATE_MANT) {
          mantissa *= 10;
          mantissa += digit;
          mant_div *= 10;
        } else if (
          state == ES_FLOAT_STATE_EXP_SIGN
       || state == ES_FLOAT_STATE_EXP
       || state == ES_FLOAT_STATE_EXP_DIGITS
        ) {
          if (expsign == 0) {
            expsign = 1;
          }
          state = ES_FLOAT_STATE_EXP_DIGITS;
          exponent *= 10;
          exponent += digit;
        }
        count += 1;
        break;
    }
    s->pos += 1;
    if (count > ES_PARSER_MAX_DIGITS) {
      s->error = ES_PE_MALFORMED;
      s->context = "number (too many digits)";
      return ES_BAD_FLOAT;
    }
  }
  if (
    state == ES_FLOAT_STATE_DONE
 || state == ES_FLOAT_STATE_ZERO
 || (state == ES_FLOAT_STATE_CHAR && count > 0)
 || state == ES_FLOAT_STATE_MANT
 || (state == ES_FLOAT_STATE_EXP_DIGITS && count > 0)
  ) {
    return (
      (
        sign * characteristic
      + (mantissa / mant_div)
      )
    * pow(10, expsign * exponent)
    );
  } else {
    s->error = ES_PE_MALFORMED;
    s->context = "number";
    return ES_BAD_FLOAT;
  }
}

string* es_parse_string(es_parse_state *s) {
  ptrdiff_t start, end;
  char *pure;
  string *result;

  es_find_string_limits(s, &start, &end);
  if (es_parse_failed(s)) {
    return NULL;
  }

  pure = es_purify_string(s->input, start, end);

  result = create_string_from_ntchars(pure);

  free(pure);

  return result;
}

void es_find_string_limits(
  es_parse_state *s,
  ptrdiff_t *r_start,
  ptrdiff_t *r_end
) {
  es_quote_state state = ES_QUOTE_STATE_NORMAL;
  char c;
  char q;

  // skip to the starting quote:
  SKIP_OR_ELSE(s, "quoted string (pre)") //;

  c = s->input[s->pos];
  q = c;
  if (!(q == '"' || q == '\'')) {
    *r_start = -1;
    *r_end = -1;
    s->error = ES_PE_MALFORMED;
    s->context = "quoted string (open)";
    return;
  }
  *r_start = s->pos;
  s->pos += 1;
  while (!es_parse_atend(s)) {
    c = s->input[s->pos];
    if (c == '\n') {
      s->lineno += 1;
    }
    switch (state) {
      default:
      case ES_QUOTE_STATE_NORMAL:
        if (c == q) {
          state = ES_QUOTE_STATE_MAYBE_DONE;
        }
        break;
      case ES_QUOTE_STATE_MAYBE_DONE:
        if (c == q) {
          state = ES_QUOTE_STATE_NORMAL;
        } else {
          state = ES_QUOTE_STATE_DONE;
        }
        break;
    }
    if (state == ES_QUOTE_STATE_DONE) {
      break;
    } else {
      s->pos += 1;
    }
  }
  switch (state) {
    default:
    case ES_QUOTE_STATE_NORMAL:
      s->context = "quoted string (main)";
      s->error = ES_PE_INCOMPLETE;
      break;
    case ES_QUOTE_STATE_MAYBE_DONE:
    case ES_QUOTE_STATE_DONE:
      *r_end = s->pos - 1;
      break;
  }
}

char * es_purify_string(
  char const * const input,
  ptrdiff_t start,
  ptrdiff_t end
) {
  ptrdiff_t len = end - start - 1;
  char *result = (char*) malloc(sizeof(char) * (len + 1));
  ptrdiff_t i, offset;
  char q, c;
  int skip = 0;
  q = input[start];
  i = 0;
  for (offset = start + 1; offset < end; ++offset) {
    c = input[offset];
    if (c == q) {
      if (skip) {
        skip = 0;
        i -= 1; // counteract the increment
      } else {
        skip = 1;
        result[i] = c;
      }
#ifdef DEBUG
    } else if (skip) {
      fprintf(stderr, "Warning: bad quote found in string while purifying.\n");
      fprintf(stderr, "  String: %.*s\n", (int) len, input + start + 1);
      skip = 0;
      result[i] = c;
#endif
    } else {
      result[i] = c;
    }
    i += 1;
  }
  result[i] = '\0';
  return result;
}

void es_parse_skip(es_parse_state *s) {
  es_skip_state state = ES_SKIP_STATE_NORMAL;
  char c;
  while (!es_parse_atend(s)) {
    c = s->input[s->pos];
    if (c == '\n') {
      s->lineno += 1;
    }
    switch (state) {
      default:
      case ES_SKIP_STATE_NORMAL:
        if (c == ES_CH_COMMENT) {
          state = ES_SKIP_STATE_MAYBE_COMMENT;
        } else if (!es_is_whitespace(c)) {
          state = ES_SKIP_STATE_DONE;
        }
        break;
      case ES_SKIP_STATE_MAYBE_COMMENT: // maybe starting a comment
        if (c == ES_CH_COMMENT) {
          state = ES_SKIP_STATE_LINE_COMMENT;
        } else if (c == ES_CH_BLOCK_COMMENT) {
          state = ES_SKIP_STATE_BLOCK_COMMENT;
        } else {
          s->pos -= 1; // go back to the delim immediately preceding us
          if (c == ES_CH_NEWLINE) {
            s->lineno -= 1;
          }
          state = ES_SKIP_STATE_DONE; // not actually the start of a comment
        }
        break;
      case ES_SKIP_STATE_LINE_COMMENT:
        if (c == ES_CH_NEWLINE) {
          state = ES_SKIP_STATE_NORMAL;
        }
        break;
      case ES_SKIP_STATE_BLOCK_COMMENT: // comment until */
        if (c == ES_CH_BLOCK_COMMENT) {
          state = ES_SKIP_STATE_MAYBE_BLOCK_END;
        }
        break;
      case ES_SKIP_STATE_MAYBE_BLOCK_END:
        if (c == ES_CH_COMMENT) {
          state = ES_SKIP_STATE_NORMAL; // done with this block comment
        } else {
          state = ES_SKIP_STATE_BLOCK_COMMENT; // still in a block comment
        }
        break;
    }
    if (state == ES_SKIP_STATE_DONE) { // don't increment position
      break;
    } else { // increment our position and keep going
      s->pos += 1;
    }
  }
  if  (
    !(state == ES_SKIP_STATE_DONE
   || state == ES_SKIP_STATE_NORMAL
   || state == ES_SKIP_STATE_LINE_COMMENT)
  ) {
    if (
      state == ES_SKIP_STATE_BLOCK_COMMENT
   || state == ES_SKIP_STATE_MAYBE_BLOCK_END
    ) {
      s->context = "block comment";
    } // otherwise leave whatever context was already there
    s->error = ES_PE_INCOMPLETE;
  }
}

void es_print_parse_error(es_parse_state *s) {
  char const *context;
  char before[ES_PARSER_ERROR_LINE+1];
  char after[ES_PARSER_ERROR_LINE+1];
  char above[ES_PARSER_ERROR_LINE+1];
  char below[ES_PARSER_ERROR_LINE+1];
  int offset;
  int sursize;
  if (s->error != ES_PE_NO_ERROR) {
    // figure out our surroundings:
    memset(before, ' ', ES_PARSER_ERROR_LINE / 2);
    memset(after, '~', ES_PARSER_ERROR_LINE / 2);
    memset(before + (ES_PARSER_ERROR_LINE /2), '~', ES_PARSER_ERROR_LINE / 2);
    memset(after + (ES_PARSER_ERROR_LINE / 2), ' ', ES_PARSER_ERROR_LINE / 2);
    memset(above, '-', ES_PARSER_ERROR_LINE);
    memset(below, '-', ES_PARSER_ERROR_LINE);
    before[ES_PARSER_ERROR_LINE] = '\0';
    after[ES_PARSER_ERROR_LINE] = '\0';
    above[ES_PARSER_ERROR_LINE / 2] = 'v';
    strncpy(above + (ES_PARSER_ERROR_LINE / 2) + 2, "here", 4);
    below[ES_PARSER_ERROR_LINE / 2] = '^';
    strncpy(below + (ES_PARSER_ERROR_LINE / 2) - 5, "here", 4);
    above[ES_PARSER_ERROR_LINE] = '\0';
    below[ES_PARSER_ERROR_LINE] = '\0';

    // before:
    offset = ES_PARSER_ERROR_LINE / 2;
    offset -= s->pos;
    if (offset < (ES_PARSER_ERROR_LINE / 2) - ES_PARSER_ERROR_BEFORE) {
      offset = (ES_PARSER_ERROR_LINE / 2) - ES_PARSER_ERROR_BEFORE;
      before[offset-3] = '.';
      before[offset-2] = '.';
      before[offset-1] = '.';
    }
    sursize = (ES_PARSER_ERROR_LINE / 2) - offset;
    strncpy(before + offset, s->input + (s->pos - sursize), sursize);

    // after
    for (sursize = 0; sursize < ES_PARSER_ERROR_AFTER; ++sursize) {
      if (
        s->pos + sursize >= s->input_length
     || s->input[s->pos + sursize] == '\0'
      ) {
        break;
      }
    }
    if (
      s->pos + sursize < s->input_length
   && s->input[s->pos + sursize] != '\0'
    ) {
      after[(ES_PARSER_ERROR_LINE / 2) + sursize] = '.';
      after[(ES_PARSER_ERROR_LINE / 2) + sursize + 1] = '.';
      after[(ES_PARSER_ERROR_LINE / 2) + sursize + 2] = '.';
    }
    strncpy(after + (ES_PARSER_ERROR_LINE / 2), s->input + s->pos, sursize);

    // replace newlines, tabs, and carriage returns:
    for (offset = 0; offset < ES_PARSER_ERROR_LINE / 2; ++offset) {
      if (
        before[offset] == '\n'
     || before[offset] == '\r'
     || before[offset] == '\t'
      ) {
        before[offset] = '';
      }
      if (
        after[(ES_PARSER_ERROR_LINE / 2) + offset] == '\n'
     || after[(ES_PARSER_ERROR_LINE / 2) + offset] == '\r'
     || after[(ES_PARSER_ERROR_LINE / 2) + offset] == '\t'
      ) {
        after[(ES_PARSER_ERROR_LINE / 2) + offset] = '';
      }
    }

    // check for manual abort:
    if (
      s->pos <= s->input_length - 2
   && s->input[s->pos] == ES_CH_ABORT_PARSING
   && s->input[s->pos+1] == ES_CH_ABORT_PARSING
    ) {
      s->error = ES_PE_ABORT;
    }

    // figure out our context:
    context = s->context;
    if (context == NULL) {
      context = "<unknown>";
    }

    // print the file and line number:
    fprintf(
      stderr,
      "[%.*s:%ld] ",
      ES_PARSER_MAX_FILENAME_DISPLAY,
      s->filename,
      s->lineno
    );

    // print the error:
    switch (s->error) {
      default:
      case ES_PE_UNKNOWN:
        fprintf(
          stderr,
          "Unknown parsing error while parsing %.*s.\n",
          ES_PARSER_MAX_CONTEXT_DISPLAY,
          context
        );
        break;
      case ES_PE_ABORT:
        fprintf(
          stderr,
          "Manual abort while parsing %.*s.\n",
          ES_PARSER_MAX_CONTEXT_DISPLAY,
          context
        );
        break;
      case ES_PE_MALFORMED:
        fprintf(
          stderr,
          "Malformed input while parsing %.*s.\n",
          ES_PARSER_MAX_CONTEXT_DISPLAY,
          context
        );
        break;
      case ES_PE_MISSING:
        fprintf(
          stderr,
          "Missing element while parsing %.*s.\n",
          ES_PARSER_MAX_CONTEXT_DISPLAY,
          context
        );
        break;
      case ES_PE_INCOMPLETE:
        fprintf(
          stderr,
          "Input ended while parsing %.*s.\n",
          ES_PARSER_MAX_CONTEXT_DISPLAY,
          context
        );
        break;
    }

    // print the surroundings:
    fprintf(stderr, "%.*s\n", ES_PARSER_ERROR_LINE, above);
    fprintf(stderr, "%.*s\n", ES_PARSER_ERROR_LINE, before);
    fprintf(stderr, "%.*s\n", ES_PARSER_ERROR_LINE, after);
    fprintf(stderr, "%.*s\n", ES_PARSER_ERROR_LINE, below);
  }
}
