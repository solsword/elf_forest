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

void es_parse_copy_state(es_parse_state *from, es_parse_state *to) {
  to->pos = from->pos;
  to->filename = from->filename;
  to->lineno = from->lineno;
  to->next_closing_brace = from->next_closing_brace;
  to->context = from->context;
  to->error = from->error;
}

es_bytecode* es_parse_statement(es_parse_state *s) {
  es_parse_state back;
  es_storage_target st;
  es_bytecode *assign, *eval;
  assign = es_parse_lhs(s);
  if (es_parse_failed(s)) {
    return NULL;
  }
  eval = es_parse_expression(s);
  if (es_parse_failed(s)) {
    cleanup_es_bytecode(assign);
    return NULL;
  }
  return es_join_bytecode(eval, assign);
}

es_bytecode* es_parse_lhs(es_parse_state *s) {
  es_bytecode *result;
  string *target;
  es_slice *slice;

  SKIP_OR_RETURN(s, "assignment target (start)", NULL);
  target = es_parse_identifier(s);
  // TODO: HERE
  // TODO: What does a target look like? Also free whatever this becomes when
  // aborting below!
  if (es_parse_failed(s)) {
    return NULL;
  }

  SKIP_OR_RETURN(s, "assignment target (after identifier)", NULL);
  slice = es_parse_subscript(s); // look for a subscript
  if (s->error == ES_PE_MISSING) {
    // ignore if we didn't find anything
    s->error = ES_PE_NO_ERROR;
    s->context = NULL;
  } else if (es_parse_failed(s)) {
    // propagate error
    return NULL;
  } else {
    // there is a valid slice; allow whitespace
    es_parse_skip(s);
    if (es_parse_failed(s)) {
      es_cleanup_slice(slice);
      s->error = ES_PE_MALFORMED;
      s->context = "assignment target (after slice)";
      return NULL;
    }
  }

  if (s->input[s->pos] == ES_CH_EQUALS) {
    s->pos += 1;
  } else {
    s->error = ES_PE_MALFORMED;
    s->context = "assignment target (equals sign)";
    return NULL;
  }

  es_parse_skip(s);
  if (es_parse_failed(s)) {
    if (slice != NULL) {
      es_cleanup_slice(slice);
    }
    s->error = ES_PE_MALFORMED;
    s->context = "assignment target (after equals sign)";
    return NULL;
  }

  // TODO: HERE

  // TODO: Something else
  result = create_es_bytecode();
  return result;
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
        s->pos += 1;
        done = 1;
        break;
      default:
        // might be a unary operation in front:
        fragment = create_es_expr_fragment(pdetph, NULL, ES_INSTR_NOP);
        fragment->op = es_parse_unop(s);
        if (!es_parse_failed(s)) {
          // transfers ownership of fragment:
          l_append_element(fragments, (void*) fragment);
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
  es_add_instruction(tree->here->valcode, tree->here->op);
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
    case ES_CH_OPEN_SLICE:
      // TODO: HERE!
      sub = es_parse_subscript(s);
      if (es_parse_failed(s)) {
        es_scrub_value(t, v);
        return NULL;
      }
      break;
    case ES_CH_ACCESS:
      s->pos += 1;
      // TODO: how to determine op type by lhs?
      // Do that during aggregation?
      return ES_INSTR_OPROP;
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
      s->pos -= 1;
      s->error = ES_PE_MALFORMED;
      s->context = "binary operator (single equals is not an operator)";
      return ES_INSTR_INVALID;
    default:
      s->error = ES_PE_MALFORMED;
      s->context = "binary operator (unknown operator)";
      return ES_INSTR_INVALID;
  }
}

// Strings

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
  ptrdiff_t *start,
  ptrdiff_t *end
) {
  es_quote_state state = ES_QUOTE_STATE_NORMAL;
  char c;
  char q;

  // skip to the starting quote:
  SKIP_OR_ELSE(s, "quoted string (pre)") //;

  c = s->input[s->pos];
  q = c;
  if (!(q == '"' || q == '\'')) {
    *start = -1;
    *end = -1;
    s->error = ES_PE_MALFORMED;
    s->context = "quoted string (open)";
    return;
  }
  *start = s->pos;
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
      *end = s->pos - 1;
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

// Whitespace and comments:

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
        } else if (!is_whitespace(c)) {
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
