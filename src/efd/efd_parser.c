// efd_parser.c
// Parsing for the Elf Forest Data format.

/*************
 * Functions *
 *************/

// Top level parsing function that delegates to the more specific functions:
efd_node* efd_parse_any(efd_parse_state *s) {
  efd_node* result = (efd_node*) malloc(sizeof(efd_node));
  efd_parse_open(raw, pos);
  efd_check_parse_progress(pos);
  result->h.type = efd_parse_type(raw, pos);
  efd_check_parse_progress(pos);
}

// Parsing functions for the EFD primitive types:
//-----------------------------------------------

efd_node* efd_parse_collection(efd_parse_state *s);

efd_node* efd_parse_proto(efd_parse_state *s);

efd_node* efd_parse_integer(efd_parse_state *s);

efd_node* efd_parse_number(efd_parse_state *s);

efd_node* efd_parse_string(efd_parse_state *s);

efd_node* efd_parse_obj_array(efd_parse_state *s);

efd_node* efd_parse_int_array(efd_parse_state *s);

efd_node* efd_parse_num_array(efd_parse_state *s);

efd_node* efd_parse_str_array(efd_parse_state *s);

// Functions for parsing bits & pieces:
//-------------------------------------

void efd_parse_open(efd_parse_state *s);

void efd_parse_close(efd_parse_state *s);

efd_node_type efd_parse_type(efd_parse_state *s);

void efd_parse_name(efd_parse_state *s, char *r_name);

void efd_parse_schema(efd_parse_state *s, char *r_name);

ptrdiff_t efd_parse_int(efd_parse_state *s);

float efd_parse_float(efd_parse_state *s);

void efd_grab_string_limits(
  efd_parse_state *s
  ptrdiff_t *start,
  ptrdiff_t *end
) {
  efd_quote_state state = EFD_QUOTE_STATE_NORMAL;
  char c;
  char q;

  // skip to the starting quote:
  efd_parse_skip(s);
  efd_assume_parse_progress(s);

  c = s->input[s->pos];
  q = c;
  if (!(q == '"' || q == '\'')) {
    *start = -1;
    *end = -1;
    s->context = "quoted string";
    s->error = EFD_PE_MALFORMED;
    return;
  }
  s->pos += 1;
  *start = s->pos;
  while (!efd_parse_atend(s)) {
    c = s->input[s->pos];
    if (c == '\n') {
      s->lineno += 1;
    }
    switch (state) {
      default:
      case EFD_QUOTE_STATE_NORMAL:
        if (c == q) {
          state = EFD_QUOTE_STATE_MAYBE_DONE;
        }
        break;
      case EFD_QUOTE_STATE_MAYBE_DONE:
        if (c == q) {
          state = EFD_QUOTE_STATE_NORMAL;
        } else {
          state = EFD_QUOTE_STATE_DONE;
        }
        break;
    }
    if (state == EFD_QUOTE_STATE_DONE) {
      break;
    } else {
      s->pos += 1;
    }
  }
  switch (state) {
    default:
    case EFD_QUOTE_STATE_NORMAL:
      s->context = "quoted string";
      s->error = EFD_PE_INCOMPLETE;
      break;
    case EFD_QUOTE_STATE_MAYBE_DONE:
    case EFD_QUOTE_STATE_DONE:
      *end = s->pos - 2;
      break;
  }
}

void efd_parse_skip(efd_parse_state *s) {
  efd_skip_state state = EFD_SKIP_STATE_NORMAL;
  char c;
  while (!efd_parse_atend(s)) {
    c = s->input[s->pos];
    if (c == '\n') {
      s->lineno += 1;
    }
    switch (state) {
      default:
      case EFD_SKIP_STATE_NORMAL:
        if (c == '/') {
          state = EFD_SKIP_STATE_MAYBE_COMMENT;
        } else if (!is_whitespace(c)) {
          state = EFD_SKIP_STATE_DONE;
        }
        break;
      case EFD_SKIP_STATE_MAYBE_COMMENT: // maybe starting a comment
        if (c == '/') {
          state = EFD_SKIP_STATE_LINE_COMMENT;
        } else if (c == '*') {
          state = EFD_SKIP_STATE_BLOCK_COMMENT;
        } else {
          s->pos -= 1; // go back to the '/' immediately preceding us
          if (c == '\n') {
            s->lineno -= 1;
          }
          state = EFD_SKIP_STATE_DONE; // not actually the start of a comment
        }
        break;
      case EFD_SKIP_STATE_LINE_COMMENT:
        if (c == '\n') {
          state = EFD_SKIP_STATE_NORMAL;
        }
        break;
      case EFD_SKIP_STATE_BLOCK_COMMENT: // comment until */
        if (c == '*') {
          s->state = EFD_SKIP_STATE_MAYBE_BLOCK_END;
        }
        break;
      case EFD_SKIP_STATE_MAYBE_BLOCK_END:
        if (c == '/') {
          s->state = EFD_SKIP_STATE_NORMAL; // done with this block comment
        } else {
          s->state = EFD_SKIP_STATE_BLOCK_COMMENT; // still in a block comment
        }
        break;
    }
    if (s->state == EFD_SKIP_STATE_DONE) { // don't increment position
      break;
    } else { // increment our position and keep going
      s->pos += 1;
    }
  }
  if  (
    !(s->state == EFD_SKIP_STATE_DONE
   || s->state == EFD_SKIP_STATE_NORMAL
   || s->state == EFD_SKIP_STATE_LINE_COMMENT)
  ) {
    if (
      s->state == EFD_SKIP_STATE_BLOCK_COMMENT
   || s->state == EFD_SKIP_STATE_MAYBE_BLOCK_END
    ) {
      s->context = "block comment";
    } // otherwise leave whatever context was already there
    s->error = EFD_PE_INCOMPLETE;
  }
}

// Helper functions:
//------------------

void efd_assume_parse_progress(efd_parse_state *s) {
  if (s->error != EFD_PE_NO_ERROR) {
    fprintf(
      stderr,
      "[%.*s:%d] ",
      EFD_PARSER_MAX_FILENAME_DISPLAY,
      s->filename,
      s->lineno
    );
    switch (s->error) {
      default:
      case EFD_PE_UNKNOWN:
        s->error = EFD_PE_UNKNOWN;
        fprintf(
          stderr,
          "Unknown parsing error while parsing %.*s.\n"
          EFD_PARSER_MAX_CONTEXT_DISPLAY,
          s->context
        );
        break;
      case EFD_PE_MALFORMED:
        fprintf(
          stderr,
          "Malformed input while parsing %.*s at '%.*s'.\n",
          EFD_PARSER_MAX_CONTEXT_DISPLAY,
          s->context,
          EFD_PARSER_ERROR_CONTEXT, // TODO: pre-context as well?
          (s->input + s->pos)
        );
        break;
      case EFD_PE_INCOMPLETE:
        fprintf(
          stderr,
          "Input ended while parsing %.*s.\n",
          EFD_PARSER_MAX_CONTEXT_DISPLAY,
          s->context
        );
        break;
    }
    exit(-s->error);
  }
}

int efd_test_parse_progress(efd_parse_state *s, ptrdiff_t reset_to) {
  if (s->error == EFD_PE_NO_ERROR) {
    return 1;
  } else {
    s->pos = reset_to;
    return 0;
  }
}
