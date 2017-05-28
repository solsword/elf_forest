// elfscript.c
// Definition and implementation of ElfScript.

#include <arpa/inet.h>

#include "elfscript.h"

/***********
 * Globals *
 ***********/

// A switch to control tracing and a list of error contexts for tracing:
int ELFSCRIPT_TRACK_ERROR_CONTEXTS = 1;
list *ELFSCRIPT_ERROR_CONTEXT = NULL;

CSTR(ELFSCRIPT_FILE_EXTENSION, "es", 2);

CSTR(ELFSCRIPT_SCRIPT_DIR_NAME, "script", 6);

CSTR(ELFSCRIPT_ADDR_SEP_STR, ".", 1);

CSTR(ELFSCRIPT_ANON_NAME, "__anon__", 8);
CSTR(ELFSCRIPT_GLOBAL_NAME, "__global__", 10);

char const * const ELFSCRIPT_DT_NAMES[] = {
  "invalid",
  "any",
  "integer",
  "number",
  "string",
  "object",
  "function",
  "method",
  "generator",
  "generator-method"
};

char const * const ELFSCRIPT_DT_ABBRS[] = {
  "X",
  "A",
  "i",
  "n",
  "s",
  "o",
  "f",
  "m",
  "g",
  "G"
};

char const * const ELFSCRIPT_GT_NAMES[] = {
  "invalid",
  "children",
  "indices",
  "function",
  "extend-restart",
  "extend-hold",
  "parallel"
};


es_scope *ELFSCRIPT_GLOBAL_SCOPE = NULL;

dictionary *ELFSCRIPT_GLOBAL_VALS = NULL;

/******************************
 * Constructors & Destructors *
 ******************************/

es_bytecode* create_es_bytecode(void) {
  es_bytecode *result = (es_bytecode*) malloc(sizeof(es_bytecode));
  result->len = 0;
  result->capacity = ELFSCRIPT_BYTECODE_STARTING_SIZE;
  result->bytes = (char*) malloc(sizeof(char)*ELFSCRIPT_BYTECODE_STARTING_SIZE);
  return result;
}

es_bytecode* create_es_bytecode_sized(size_t size) {
  es_bytecode *result = (es_bytecode*) malloc(sizeof(es_bytecode));
  result->len = 0;
  result->capacity = size;
  result->bytes = (char*) malloc(sizeof(char) * size);
  return result;
}

es_bytecode* copy_es_bytecode(es_bytecode *src) {
  es_bytecode *result = create_es_bytecode_sized(src->capacity);
  memcpy(result->bytes, src->bytes, src->len);
  return result;
}

CLEANUP_IMPL(es_bytecode) {
  free(code->bytes);
  free(code);
}

es_scope* create_es_scope(void) {
  es_scope *result = (es_scope*) malloc(sizeof(es_scope));
  result->variables = create_dictionary(ELFSCRIPT_DEFAULT_DICTIONARY_SIZE);
  // TODO: More here?
  return result;
}

CLEANUP_IMPL(es_scope) {
  d_foreach(doomed->variables, &es_v_decref);
  cleanup_dictionary(doomed->variables);
  // TODO: More here?
  free(doomed);
}

es_var* create_es_var(es_type type, es_val_t value);
es_var* create_es_int_var(es_int_t value) {
  create_es_var(ES_DT_INT, (es_val_t) value);
}
es_var* create_es_num_var(es_num_t value) {
  create_es_var(ES_DT_NUM, (es_val_t) value);
}
es_var* create_es_str_var(string* value) {
  create_es_var(ES_DT_STR, (es_val_t) value);
}

es_var* create_es_obj_var(es_scope* value) {
  create_es_var(ES_DT_OBJ, (es_val_t) value);
}

es_var* create_es_scp_var(es_scope* value) {
  create_es_var(ES_DT_SCP, (es_val_t) value);
}

CLEANUP_IMPL(es_var) {
  switch(doomed->type) {
    default:
    case ES_DT_INVALID:
    case ES_DT_ANY:
#ifdef DEBUG
      fprintf(stderr, "Warning: cleanup of elfscript var with invalid type!\n");
      // and fall through
#endif
    case ES_DT_INT:
    case ES_DT_NUM:
      break;
    case ES_DT_STR:
      cleanup_string((string*) doomed->value);
      break;
    case ES_DT_SCP:
      cleanup_es_scope((es_scope*) doomed->value);
      break;
    case ES_DT_OBJ:
      cleanup_es_obj((es_obj*) doomed->value);
      break;
    case ES_DT_FCN:
    case ES_DT_MTH:
      cleanup_es_bytecode((es_bytecode*) doomed->value);
      break;
    case ES_DT_GEN:
    case ES_DT_GNM:
      // TODO: Modify es_generator_state as necessary
      cleanup_es_generator_state((es_generator_state*) doomed->value);
      break;
  }
  free(doomed);
  return;
}

es_obj* create_es_obj(es_object_format *format, es_probj_t obj) {
  es_obj* result = (es_obj*) malloc(sizeof(es_obj));
  result->format = format;
  result->value = obj;
  return result;
}

CLEANUP_IMPL(es_obj) {
  doomed->format->destructor(doomed->value);
  // Note: format is not owned by object
  free(doomed);
}

es_slice* create_es_slice(void) {
  es_slice* result = (es_slice*) malloc(sizeof(es_slice));
  result->start = 0;
  result->end = 0;
  result->step = 0;
  return result;
}

// Clean up memory from a slice.
CLEANUP_IMPL(es_slice) {
  free(doomed);
}

/*************
 * Functions *
 *************/

void es_v_decref(void *v_var) {
  es_decref((es_var*) v_var);
}

void es_write_esb(char const * const filename, es_bytecode *code) {
  FILE *fout = fopen(filename, "w");
  fprintf(fout, "esb\n");
  size_t written = fwrite(code->bytes, 1, code->len, fout);
#ifdef DEBUG
  if (written != code->len) {
    fprintf(
      stderr,
      "ERROR: es_write_esb only wrote %zu/%zu bytes!\n",
      written,
      code->len
    );
  }
#endif
  fclose(fout);
}

es_bytecode * es_load_esb(char const * const filename) {
  FILE *fin = fopen(filename, "r");
  char c;
  char *expect = "esb\n";
  es_bytecode *result = create_es_bytecode_sized(ELFSCRIPT_BYTECODE_LARGE_SIZE);
  while (!feof(fin)) {
    c = fgetc(fin);
    if (*expect != '\0') { // check the header
      if (c != *expect) {
#ifdef DEBUG
        fprintf(stderr, "ERROR: es_load_esb: file is missing header.\n");
#endif
        fclose(fin);
        return NULL;
      }
      expect += 1;
    } else { // past the header
      es_add_instruction(result, (es_instruction) c);
    }
  }
  fclose(fin);
  return result;
}

es_bytecode* es_join_bytecode(es_bytecode *first, es_bytecode *second) {
  es_bytecode *result = create_es_bytecode_sized(
    first->len + second->len + ELFSCRIPT_BYTECODE_STARTING_SIZE
  );
  result->len = first->len + second->len;
  memcpy(result->bytes, first->bytes, first->len);
  memcpy(result->bytes + first->len, second->bytes, second->len);
  cleanup_es_bytecode(first);
  cleanup_es_bytecode(second);
  return result;
}

void es_add_instruction(es_bytecode *code, es_instruction instr) {
  es_ensure_bytecode_capacity(code, 1);
  code->bytes[code->len] = (char) instr;
  code->len += 1;
}

void es_add_int_literal(es_bytecode *code, es_int_t value) {
  es_ensure_bytecode_capacity(code, 1 + sizeof(es_int_t));
  code->bytes[code->len] = ES_INSTR_LINT;
  code->len += 1;
  // TODO: Something about byte order?
  memcpy((void*) (code->bytes + code->len), (void*) &value, sizeof(es_int_t));
  code->len += sizeof(es_int_t);
}

void es_add_num_literal(es_bytecode *code, es_num_t value) {
  es_ensure_bytecode_capacity(code, 1 + sizeof(es_num_t));
  code->bytes[code->len] = ES_INSTR_LNUM;
  code->len += 1;
  // TODO: Something about byte order?
  memcpy((void*) (code->bytes + code->len), (void*) &value, sizeof(es_num_t));
  code->len += sizeof(es_num_t);
}

void es_add_str_literal(es_bytecode *code, string *value) {
  es_int_t slen = s_count_bytes(value);
  es_ensure_bytecode_capacity(code, 1 + sizeof(es_int_t) + slen);
  code->bytes[code->len] = ES_INSTR_LSTR;
  code->len += 1;
  // TODO: Something about byte order?
  memcpy((void*) (code->bytes + code->len), (void*) &slen, sizeof(es_num_t));
  code->len += sizeof(es_num_t);
  memcpy(
    (void*) (code->bytes + code->len),
    (void*) s_raw(value),
    slen
  );
  code->len += sizeof(es_num_t);
  // TODO: Ending sentinel value?
}

void es_add_str_literal(es_bytecode *code, string *value) {
  es_int_t slen = s_count_bytes(value);
  es_ensure_bytecode_capacity(code, 1 + sizeof(es_int_t) + slen);
  code->bytes[code->len] = ES_INSTR_LSTR;
  code->len += 1;
  // TODO: Something about byte order?
  memcpy((void*) (code->bytes + code->len), (void*) &slen, sizeof(es_num_t));
  code->len += sizeof(es_num_t);
  memcpy(
    (void*) (code->bytes + code->len),
    (void*) s_raw(value),
    slen
  );
  code->len += sizeof(es_num_t);
  // TODO: Ending sentinel value?
}

void es_add_var(es_bytecode *code, string *varname) {
  es_int_t slen = s_count_bytes(value);
  es_ensure_bytecode_capacity(code, 1 + sizeof(es_int_t) + slen);
  code->bytes[code->len] = ES_INSTR_VAR;
  code->len += 1;
  // TODO: Something about byte order?
  memcpy((void*) (code->bytes + code->len), (void*) &slen, sizeof(es_num_t));
  code->len += sizeof(es_num_t);
  memcpy(
    (void*) (code->bytes + code->len),
    (void*) s_raw(value),
    slen
  );
  code->len += sizeof(es_num_t);
  // TODO: Ending sentinel value?
}

size_t es_scope_size(es_scope *sc) {
  return d_get_count(sc->variables);
}

es_var * es_read_nth(es_scope *sc, size_t n) {
  return d_get_item(sc->variables, n);
}

es_var * es_read_var(es_scope *sc, string *name) {
  return (es_var*) d_get_value_s(sc->variables, name);
}

void es_write_last(es_scope *sc, es_var *value) {
  d_add_value_s(sc->variables, ELFSCRIPT_ANON_NAME, (void*) value);
}

void es_write_var(es_scope *sc, string *name, es_var *value) {
  es_var *old = (es_var*) d_pop_value_s(sc->variables, name);
  while (old != NULL) {
    es_decref(old);
    old = d_pop_value_s(sc->variables, name)
  }
  d_set_value_s(sc->variables, name, (void*) value);
}

void es_report_error(string *message) {
  // TODO: line numbers etc. here
  fprintf(stderr, "Elfscript error during evaluation:\n");
  s_fprintln(stderr, message);
}
