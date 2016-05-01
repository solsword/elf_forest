// string.c
// Remotely safe strings.

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <unistr.h>
#include <uniconv.h>
#include <unistdio.h>

#include "string.h"

/***********
 * Globals *
 ***********/

char* S_LOCALE = NULL;
char const * S_LCHARSET = NULL;

/********
 * Init *
 ********/

void init_strings(void) {
  S_LOCALE = setlocale(LC_ALL, "");
  S_LCHARSET = locale_charset();
}

/******************************
 * Constructors & Destructors *
 ******************************/

string* create_string(void) {
  string* s = (string*) malloc(sizeof(string));
  if (s == NULL) {
    perror("Failed to create string.");
    exit(errno);
  }
  s->length = 0;
  s->bytes = NULL;
  return s;
}

string* copy_string(string const * const base) {
  string *s = (string*) malloc(sizeof(string));
  if (s == NULL) {
    perror("Failed to create string for copy.");
    exit(errno);
  }
  s->bytes = (uint8_t*) malloc(sizeof(uint8_t) * (base->length + 1));
  if (s->bytes == NULL) {
    perror("Failed to create bytes for string copy.");
    exit(errno);
  }
  s->length = base->length;
  memcpy(s->bytes, base->bytes, base->length + 1);
  return s;
}

string* create_empty_string(void) {
  string* s = (string*) malloc(sizeof(string));
  if (s == NULL) {
    perror("Failed to create empty string.");
    exit(errno);
  }
  s->bytes = (uint8_t*) malloc(sizeof(uint8_t));
  if (s->bytes == NULL) {
    perror("Failed to create empty string byte.");
    exit(errno);
  }
  s->length = 0;
  s->bytes[0] = '\0';
  return s;
}

string* create_string_from_ntchars(char const * const chars) {
#ifdef DEBUG
  if (S_LCHARSET == NULL) {
    fprintf(
      stderr,
      "Error: create_string_from_ntchars called before init_strings.\n"
    );
  }
#endif // ifdef DEBUG
  string* s = (string*) malloc(sizeof(string));
  if (s == NULL) {
    perror("Failed to create string.");
    exit(errno);
  }
  s->bytes = u8_strconv_from_encoding(
    chars,
    S_LCHARSET,
    iconveh_escape_sequence
  );
  if (s->bytes == NULL) {
    perror("Failed to get string from local encoding.");
    exit(errno);
  }
  s->length = strlen((char*) (s->bytes)); // orphan the NUL byte
  s->bytes = realloc(s->bytes, sizeof(uint8_t) * (s->length + 1));
  s->bytes[s->length] = '\0';
  return s;
}

string* create_string_from_chars(char const * const nchars, size_t len) {
#ifdef DEBUG
  if (S_LCHARSET == NULL) {
    fprintf(
      stderr,
      "Error: create_string_from_chars called before init_strings.\n"
    );
  }
#endif // ifdef DEBUG
  string* s = (string*) malloc(sizeof(string));
  if (s == NULL) {
    perror("Failed to create string.");
    exit(errno);
  }
  s->bytes = u8_conv_from_encoding(
    S_LCHARSET,
    iconveh_escape_sequence,
    nchars,
    len,
    NULL,
    NULL,
    &(s->length)
  );
  if (s->bytes == NULL) {
    perror("Failed to get string from local encoding.");
    exit(errno);
  }
  s->bytes = realloc(s->bytes, sizeof(uint8_t) * (s->length + 1));
  s->bytes[s->length] = '\0';
  return s;
}

string* create_raw_string(uint8_t const * const raw, size_t len) {
  string* s = (string*) malloc(sizeof(string));
  if (s == NULL) {
    perror("Failed to create string.");
    exit(errno);
  }
  s->length = len;
  s->bytes = malloc(sizeof(uint8_t) * len+1);
  memcpy(s->bytes, raw, s->length);
  s->bytes[s->length] = '\0';
  return s;
}

CLEANUP_IMPL(string) {
  if (doomed->bytes != NULL) {
    free(doomed->bytes);
  }
  free(doomed);
}

/*************
 * Functions *
 *************/

int s_contains_nul(string const * const s) {
  size_t i;
  for (i = 0; i < s->length; ++i) {
    if (s->bytes[i] == '\0') {
      return 1;
    }
  }
  return 0;
}

size_t s_get_length(string const * const s) {
  return s->length;
}

int s_check_bytes(string const * const s, char const * const c) {
  size_t i;
  for (i = 0; i < s->length; ++i) {
    if ((char) s->bytes[i] != c[i]) {
      return 0;
    }
  }
  return 1;
}

char* s_encode(string const * const s, size_t* rlen) {
#ifdef DEBUG
  if (S_LCHARSET == NULL) {
    fprintf(
      stderr,
      "Error: s_encode called before init_strings.\n"
    );
  }
#endif // ifdef DEBUG
  char* result = u8_conv_to_encoding(
    S_LCHARSET,
    iconveh_escape_sequence,
    s->bytes,
    s->length,
    NULL,
    NULL,
    rlen
  );
  if (result == NULL) {
    perror("Failed to encode string.");
    exit(errno);
  }
  return result;
}

char* s_encode_nt(string const * const s) {
#ifdef DEBUG
  if (S_LCHARSET == NULL) {
    fprintf(
      stderr,
      "Error: s_encode_nt called before init_strings.\n"
    );
  }
#endif // ifdef DEBUG
  size_t len;
  char* result;
  char* tmp = u8_conv_to_encoding(
    S_LCHARSET,
    iconveh_escape_sequence,
    s->bytes,
    s->length,
    NULL,
    NULL,
    &len
  );
  if (tmp == NULL) {
    perror("Failed to encode string.");
    exit(errno);
  }
  result = tmp;
  tmp = (char*) realloc(result, sizeof(char) * (len+1));
  if (tmp == NULL) {
    perror("Failed to expand encoded string to add NUL.");
    exit(errno);
  }
  result = tmp;
  result[len] = '\0';
  return result;
}

char const * const s_raw(string const * const s) {
  return (char*) s->bytes;
}

int s_equals(string const * const s, string const * const other) {
  return (
    (s->length == other->length)
 && (strncmp((char*) s->bytes, (char*) other->bytes, s->length) == 0)
  );
}

void s_append(string* base, string const * const extension) {
  uint8_t* newbytes;
  if (base->bytes == NULL) {
    newbytes = (uint8_t*) malloc(
      sizeof(uint8_t) * (base->length + extension->length + 1)
    );
  } else {
    newbytes = (uint8_t*) realloc(
      base->bytes,
      sizeof(uint8_t) * (base->length + extension->length + 1)
    );
  }
  if (newbytes == NULL) {
    perror("Failed to allocate new bytes for appending.");
    exit(errno);
  }
  u8_cpy(newbytes + base->length, extension->bytes, extension->length);
  base->bytes = newbytes; // realloc means we don't need to free
  base->length += extension->length;
  base->bytes[base->length] = '\0';
}

void s_devour(string *base, string *extension) {
  s_append(base, extension);
  cleanup_string(extension);
}

string* s_concat(string const * const first, string const * const second) {
  string* result = create_string();
  result->length = first->length + second->length;
  result->bytes = (uint8_t*) malloc(sizeof(uint8_t)*(result->length + 1));
  u8_cpy(result->bytes, first->bytes, first->length);
  u8_cpy(result->bytes + first->length, second->bytes, second->length);
  result->bytes[result->length] = '\0';
  return result;
}

string* s_join(string const * const sep, ...) {
  va_list args;
  va_start(args, sep);
  string* result = s_vjoin(sep, args);
  va_end(args);
  return result;
}

string* s_vjoin(string const * const sep, va_list args) {
  string* next = va_arg(args, string*);
  string* result = create_string();
  int first = 1;
  while (next != NULL) {
    if (first) {
      first = 0;
    } else {
      s_append(result, sep);
    }
    s_append(result, next);
    next = va_arg(args, string*);
  }
  return result;
}

string* s_sprintf(char const * const fmt, ...) {
  va_list args;
  va_start(args, fmt);
  string* result = s_vsprintf(fmt, args);
  va_end(args);
  return result;
}

string* s_vsprintf(char const * const fmt, va_list args) {
  string* result = create_string();
  result->bytes = u8_vasnprintf(
    NULL,
    &(result->length),
    fmt,
    args
  );
  return result;
}

void s_print(string const * const s) {
  s_fprint(stdout, s);
}

void s_println(string const * const s) {
  s_fprintln(stdout, s);
}

void s_fprint(FILE *out, string const * const s) {
  char* encoded = s_encode_nt(s);
  fputs(encoded, out);
  free(encoded);
}

void s_fprintln(FILE *out, string const * const s) {
  s_fprint(out, s);
  fputc('\n', out);
}
