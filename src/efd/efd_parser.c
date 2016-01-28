// efd_parser.c
// Parsing for the Elf Forest Data format.

/*************
 * Functions *
 *************/

// Top level parsing function that delegates to the more specific functions:
efd_node* efd_parse_any(char const * const raw, ptrdiff_t *pos) {
  efd_node* result = (efd_node*) malloc(sizeof(efd_node));
  efd_parse_open(raw, pos);
  efd_check_parse_progress(pos);
  result->h.type = efd_parse_type(raw, pos);
  efd_check_parse_progress(pos);
}

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

void efd_parse_open(char const * const raw, ptrdiff_t *pos);

void efd_parse_close(char const * const raw, ptrdiff_t *pos);

efd_node_type efd_parse_type(char const * const raw, ptrdiff_t *pos);

void efd_parse_name(char const * const raw, ptrdiff_t *pos, char *r_name);

void efd_parse_schema(char const * const raw, ptrdiff_t *pos, char *r_name);

ptrdiff_t efd_parse_int(char const * const raw, ptrdiff_t *pos);

float efd_parse_float(char const * const raw, ptrdiff_t *pos);

void efd_grab_string_limits(
  char const * const raw,
  ptrdiff_t *start,
  ptrdiff_t *end
);

void efd_parse_skip(char const * const raw, ptrdiff_t *pos) {
  // TODO: HERE (comments)
  while (is_whitespace(raw[*pos]) && raw[*pos] != '\0') {
    *pos += 1;
  }
}

// Helper functions:
//------------------

void efd_check_parse_progress(ptrdiff_t *pos) {
  if (pos < 0) {
    // TODO: Better here!!
    fprintf(stderr, "Parsing Error!\n");
    exit(1);
  }
}
