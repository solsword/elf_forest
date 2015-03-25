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

char* S_LOCALE;
char const * S_LCHARSET;

/********
 * Init *
 ********/

void init_strings(void) {
  S_LOCALE = setlocale(LC_ALL, "");
  S_LCHARSET = locale_charset();
}

/*************************
 * Structure Definitions *
 *************************/

struct string_s {
  size_t length; // raw number of bytes including null terminator
  uint8_t* bytes;
};

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

string* create_string_from_ntchars(char const * const chars) {
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
  s->length = strlen((char*) s->bytes); // orphan the NUL byte
  return s;
}

string* create_string_from_chars(char const * const nchars, size_t len) {
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
  return s;
}

void cleanup_string(string* s) {
  if (s->bytes != NULL) {
    free(s->bytes);
  }
  free(s);
}

/*************
 * Functions *
 *************/

size_t s_contains_nul(string* s) {
  size_t i;
  for (i = 0; i < s->length; ++i) {
    if (s->bytes[i] == 0) {
      return 1;
    }
  }
  return 0;
}

char* s_encode(string* s, size_t* rlen) {
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

char* s_encode_nt(string* s) {
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

void s_append(string* base, string const * const extension) {
  uint8_t* newbytes = (uint8_t*) realloc(
    base->bytes,
    sizeof(uint8_t) * (base->length + extension->length)
  );
  if (newbytes == NULL) {
    perror("Failed to allocate new bytes for appending.");
    exit(errno);
  }
  u8_cpy(newbytes + base->length, extension->bytes, extension->length);
  base->bytes = newbytes; // realloc means we don't need to free
  base->length += extension->length;
}

string* s_concat(string* first, string* second) {
  string* result = create_string();
  result->length = first->length + second->length;
  result->bytes = (uint8_t*) malloc(sizeof(uint8_t)*result->length);
  u8_cpy(result->bytes, first->bytes, first->length);
  u8_cpy(result->bytes + first->length, second->bytes, second->length);
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