#ifndef DICT_STRING_KEYS_H
#define DICT_STRING_KEYS_H

// dict_string_keys.h
// A header file that augments dictionary.h with functions for using strings as
// keys directly. Be careful: strings are mutable, so looking up "the same
// string" won't work if the string's data has changed.

#include "dictionary.h"
#include "string.h"

// Returns a newly-allocated string holding the key data for the given key (see
// d_get_key).
static inline string* d_get_key_s(
  dictionary *d,
  size_t index
) {
  uint8_t *value;
  size_t length;
  d_get_key(d, index, (d_key_t**) &value, &length);
  return create_raw_string(
    value,
    length*sizeof(d_key_t)/sizeof(char)
  );
}

static inline int d_contains_key_s(dictionary *d, string *key) {
  return d_contains_key(
    d,
    (d_key_t*) s_raw(key),
    s_get_length(key) * sizeof(char) / sizeof(d_key_t)
  );
}

static inline void* d_get_value_s(dictionary *d, string *key) {
  return d_get_value(
    d,
    (d_key_t*) s_raw(key),
    s_get_length(key) * sizeof(char) / sizeof(d_key_t)
  );
}

static inline list* d_get_all_s(dictionary *d, string *key) {
  return d_get_all(
    d,
    (d_key_t*) s_raw(key),
    s_get_length(key) * sizeof(char) / sizeof(d_key_t)
  );
}

static inline void d_add_value_s(dictionary *d, void *value, string *key) {
  d_add_value(
    d,
    value,
    (d_key_t*) s_raw(key),
    s_get_length(key) * sizeof(char) / sizeof(d_key_t)
  );
}

static inline void d_set_value_s(dictionary *d, void *value, string *key) {
  d_set_value(
    d,
    value,
    (d_key_t*) s_raw(key),
    s_get_length(key) * sizeof(char) / sizeof(d_key_t)
  );
}

static inline void d_set_all_s(dictionary *d, list *values, string *key) {
  d_set_all(
    d,
    values,
    (d_key_t*) s_raw(key),
    s_get_length(key) * sizeof(char) / sizeof(d_key_t)
  );
}

static inline void* d_pop_value_s(dictionary *d, string *key) {
  return d_pop_value(
    d,
    (d_key_t*) s_raw(key),
    s_get_length(key) * sizeof(char) / sizeof(d_key_t)
  );
}

static inline void d_clear_values_s(dictionary *d, string *key) {
  d_clear_values(
    d,
    (d_key_t*) s_raw(key),
    s_get_length(key) * sizeof(char) / sizeof(d_key_t)
  );
}

#endif // ifndef DICT_STRING_KEYS_H
