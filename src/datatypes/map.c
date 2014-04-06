// map.c
// Simple hash-table-based maps that accept a variable number of keys per
// element. Map instances must always be given the same number of keys to work
// with of course.

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "list.h"
#include "map.h"

/******************
 * Internal Types *
 ******************/

// A map hash is the result of hashing one or more map keys.
typedef size_t map_hash_t;

/*************************
 * Structure Definitions *
 *************************/

struct map_s {
  uint8_t key_arity;
  size_t count; // Measured in key/value pairs.
  list * table[MAP_TABLE_SIZE];
};

struct map3_s {
  size_t count; // Measured in key/value pairs.
  list * table[MAP_TABLE_SIZE];
};

/*********************
 * Private Functions *
 *********************/

static inline map_hash_t fold_keys(size_t n_keys, va_list args) {
  size_t i = 0;
  map_hash_t k;
  map_hash_t result = 137 + n_keys;
  for (i = 0; i < n_keys; ++i) {
    k = (map_hash_t) va_arg(args, map_key_t);
    result += k;
    result = (result << 5) - result;
  }
  return result;
}

static inline map_hash_t get_key_hash(size_t arity, va_list args) {
  return (fold_keys(arity, args) % MAP_TABLE_SIZE);
}

// Non-loop variants for small N (the most common cases):
static inline map_hash_t get_hash_xyz(map_key_t x, map_key_t y, map_key_t z) {
  map_hash_t result = 731 + (map_hash_t) x;
  result = ((result << 5) - result) ^ (map_hash_t) y;
  result = ((result << 5) - result) ^ (map_hash_t) z;
  return ((result << 5) - result) % MAP_TABLE_SIZE;
}

/*************
 * Functions *
 *************/

map *create_map(size_t key_arity) {
  size_t i = 0;
  if (key_arity <= 0) {
    perror("Map key arity must be at least 1.");
    exit(1);
  }
  map * m = (map *) malloc(sizeof(map));
  if (m == NULL) {
    perror("Failed to create map.");
    exit(errno);
  }
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    m->table[i] = NULL;
  }
  m->key_arity = key_arity;
  m->count = 0;
  return m;
}
map3 *create_map3() {
  size_t i = 0;
  map3 * m = (map3 *) malloc(sizeof(map3));
  if (m == NULL) {
    perror("Failed to create map3.");
    exit(errno);
  }
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    m->table[i] = NULL;
  }
  m->count = 0;
  return m;
}

void cleanup_map(map *m) {
  size_t i = 0;
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    if (m->table[i] != NULL) {
      cleanup_list(m->table[i]);
    }
  }
  free(m);
}
void cleanup_map3(map3 *m) {
  size_t i = 0;
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    if (m->table[i] != NULL) {
      cleanup_list(m->table[i]);
    }
  }
  free(m);
}

void destroy_map(map *m) {
  size_t i = 0;
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    list *l = m->table[i];
    if (l == NULL) {
      continue;
    }
    size_t j = 0;
    for (
      j = m->key_arity;
      j < l_get_length(l);
      j += (m->key_arity + 1)
    ) {
      void * item = l_get_item(l, j);
      free(item);
    }
    cleanup_list(l);
  }
  free(m);
}
void destroy_map3(map3 *m) {
  size_t i = 0, j = 0;
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    list *l = m->table[i];
    if (l == NULL) {
      continue;
    }
    j = 0;
    for (
      j = 3;
      j < l_get_length(l);
      j += 4
    ) {
      free(l_get_item(l, j));
    }
    cleanup_list(l);
  }
  free(m);
}


inline int m_get_key_arity(map *m) { return m->key_arity; }

inline int m_is_empty(map *m) { return m->count == 0; }
inline int m3_is_empty(map3 *m) { return m->count == 0; }

inline size_t m_get_count(map *m) { return m->count; }
inline size_t m3_get_count(map3 *m) { return m->count; }

int m_contains_key(map *m, ...) {
  size_t i = 0, j = 0, length = 0, hit = 0;
  va_list args;
  va_start(args, m);
  map_hash_t hash = get_key_hash(m->key_arity, args);
  va_end(args);
  list *l = m->table[hash];
  if (l == NULL) {
    return 0;
  }
  length = l_get_length(l);
  for (i = 0; i < length; i += m->key_arity + 1) {
    hit = 1;
    va_start(args, m);
    for (j = 0; j < m->key_arity; ++j) {
      if (l_get_item(l, i + j) != va_arg(args, map_key_t)) {
        hit = 0;
      }
    }
    va_end(args);
    if (hit) {
      return 1;
    }
  }
  return 0;
}
int m3_contains_key(map3 *m, map_key_t x, map_key_t y, map_key_t z) {
  size_t i = 0, length = 0;
  void **lptr = NULL;
  map_hash_t hash = get_hash_xyz(x, y, z);
  list *l = m->table[hash];
  if (l == NULL) {
    return 0;
  }
  length = l_get_length(l);
  lptr = _l_get_pointer(l, 0);
  for (i = 0; i < length; i += 4) {
    if (
      lptr[i] == x
    &&
      lptr[i + 1] == y
    &&
      lptr[i + 2] == z
    ) {
      return 1;
    }
  }
  return 0;
}

// Returns the value corresponding to the given key(s), or NULL if no value is
// present for those key(s).
void * m_get_value(map *m, ...) {
  size_t i = 0, j = 0, hit = 0;
  va_list args;
  va_start(args, m);
  map_hash_t hash = get_key_hash(m->key_arity, args);
  va_end(args);
  list *l = m->table[hash];
  // An empty spot in the table means no such value:
  if (l == NULL) {
    return NULL;
  }
  // If there is a list in the table, search it for a matching entry:
  for (
    i = 0;
    i < l_get_length(l);
    i += m->key_arity + 1
  ) {
    hit = 1;
    // Check the keys for this entry against our varargs:
    va_start(args, m);
    for (j = 0; j < m->key_arity; ++j) {
      if (l_get_item(l, i + j) != va_arg(args, map_key_t)) {
        hit = 0;
      }
    }
    va_end(args);
    if (hit) {
      return l_get_item(l, i + m->key_arity);
    }
  }
  return NULL;
}
void * m3_get_value(map3 *m, map_key_t x, map_key_t y, map_key_t z) {
  size_t i = 0, length = 0;
  void **lptr = NULL;
  map_hash_t hash = get_hash_xyz(x, y, z);
  list *l = m->table[hash];
  if (l == NULL) {
    return NULL;
  }
  length = l_get_length(l);
  lptr = _l_get_pointer(l, i);
  for (i = 0; i < length; i += 4) {
    if (
      lptr[i] == x
    &&
      lptr[i + 1] == y
    &&
      lptr[i + 2] == z
    ) {
      return lptr[i + 3];
    }
  }
  return NULL;
}

// Adds the given value to the map under the given key(s). Allocates new memory
// to expand the map as necessary. If another value is already present under
// the given key(s), it will be overwritten. This function returns NULL unless
// it overwrites an existing value, in which case it returns that value.
void * m_put_value(map *m, void *value, ...) {
  size_t i = 0, j = 0, length = 0, hit = 0;
  va_list args;
  void *result = NULL;
  va_start(args, value);
  map_hash_t hash = get_key_hash(m->key_arity, args);
  va_end(args);
  list *l = m->table[hash];
  // Create a new list at the appropriate table entry if it was uninitialized:
  if (l == NULL) {
    m->table[hash] = create_list();
    l = m->table[hash];
  }

  // Search through the list for an existing entry with the same key(s):
  length = l_get_length(l);
  for (i = 0; i < length; i += m->key_arity + 1) {
    hit = 1;
    va_start(args, value);
    // Check the keys for this entry against our varargs:
    for (j = 0; j < m->key_arity; ++j) {
      if (l_get_item(l, i + j) != va_arg(args, map_key_t)) {
        hit = 0;
      }
    }
    va_end(args);
    if (hit) {
      break;
    }
  }

  // Insert the value:
  if (i < l_get_length(l)) {
    // If we hit an existing element with the right key(s), overwrite the old
    // value:
    result = l_replace_item(l, i + j, value);
  } else {
    // Otherwise, append a new set of elements: each key in turn followed by
    // the value:
    va_start(args, value);
    for (j = 0; j < m->key_arity; ++j) {
      l_append_element(l, va_arg(args, void *));
    }
    va_end(args);
    l_append_element(l, value);
    m->count += 1;
  }
  return result;
}
void * m3_put_value(
  map3 *m,
  void *value,
  map_key_t x,
  map_key_t y,
  map_key_t z
) {
  size_t i = 0, length = 0;
  void **lptr = NULL;
  map_hash_t hash = get_hash_xyz(x, y, z);
  list *l = m->table[hash];
  // If there's no list in the table at this point, create one:
  if (l == NULL) {
    m->table[hash] = create_list();
    l = m->table[hash];
  }
  length = l_get_length(l);
  // Search through the list for an existing entry with the same keys:
  lptr = _l_get_pointer(l, 0);
  for (i = 0; i < length; i += 4) {
    if (
      lptr[i] == x
    &&
      lptr[i + 1] == y
    &&
      lptr[i + 2] == z
    ) {
      // If we found a match, replace it:
      return l_replace_item(l, i + 3, value);
    }
  }
  // If there is no such entry, append a new entry to the appropriate bucket:
  l_append_element(l, x);
  l_append_element(l, y);
  l_append_element(l, z);
  l_append_element(l, value);
  m->count += 1;
  return NULL;
}

// Removes and returns the value indexed under the given key(s). Returns NULL
// if there is no such value in the map.
void * m_pop_value(map *m, ...) {
  size_t i = 0, j = 0, hit = 0;
  va_list args;
  void *result = NULL;
  va_start(args, m);
  map_hash_t hash = get_key_hash(m->key_arity, args);
  va_end(args);
  list *l = m->table[hash];
  // If there's no table entry, there's no value to pop:
  if (l == NULL) {
    return NULL;
  }

  // Search through the list for an existing entry with the right key(s):
  for (
    i = 0;
    i < l_get_length(l);
    i += m->key_arity + 1
  ) {
    hit = 1;
    va_start(args, m);
    // Check the keys for this entry against our varargs:
    for (j = 0; j < m->key_arity; ++j) {
      if (l_get_item(l, i + j) != va_arg(args, map_key_t)) {
        hit = 0;
      }
    }
    va_end(args);
    if (hit) {
      result = l_get_item(l, i + j);
      l_remove_range(l, i, m->key_arity + 1);
      m->count -= 1;
      break;
    }
  }
  return result;
}
void * m3_pop_value(map3 *m, map_key_t x, map_key_t y, map_key_t z) {
  size_t i = 0, length = 0;
  void **lptr = NULL;
  void *result = NULL;
  map_hash_t hash = get_hash_xyz(x, y, z);
  list *l = m->table[hash];
  if (l == NULL) {
    return NULL;
  }
  length = l_get_length(l);
  lptr = _l_get_pointer(l, 0);
  for (i = 0; i < length; i += 4) {
    if (
      lptr[i] == x
    &&
      lptr[i + 1] == y
    &&
      lptr[i + 2] == z
    ) {
      result = lptr[i + 3];
      l_remove_range(l, i, 4);
      m->count -= 1;
      break;
    }
  }
  return result;
}

size_t m_remove_all_values(map *m, void *value) {
  size_t i = 0, j = 0, length = 0;
  size_t removed = 0;
  list *l = NULL;
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    l = m->table[i];
    // If there's no table entry here, keep scanning the table:
    if (l == NULL) {
      continue;
    }

    // Search through the list for an existing entry with the right value:
    j = 0;
    length = l_get_length(l);
    while (j < length) {
      for (j = m->key_arity; j < length; j += m->key_arity + 1) {
        if (l_get_item(l, j) == value) {
          l_remove_range(l, j - m->key_arity, m->key_arity + 1);
          removed += 1;
          j -= (m->key_arity + 1); // warp backwards so we reconsider this
          // element, which after a remove_range will be the next element.
        }
      }
    }
  }
  m->count -= removed;
  return removed;
}
size_t m3_remove_all_values(map3 *m, void *value) {
  size_t i = 0, j = 0, length = 0;
  size_t removed = 0;
  list *l = NULL;
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    l = m->table[i];
    // If there's no table entry here, keep scanning the table:
    if (l == NULL) {
      continue;
    }

    // Search through the list for an existing entry with the right value:
    j = 0;
    length = l_get_length(l);
    for (j = 3; j < length; j += 4) {
      if (l_get_item(l, j) == value) {
        l_remove_range(l, j - 3, 4);
        removed += 1;
        j -= 4; // warp backwards so we reconsider this element, which after a
        // remove_range will be the next element.
      }
    }
  }
  m->count -= removed;
  return removed;
}

// Runs the given function sequentially on each value in the map.
void m_foreach(map *m, void (*f)(void *)) {
  size_t i = 0, j = 0, length = 0;
  list *l = NULL;
  // Iterate through each list in the table:
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    l = m->table[i];
    // If there's no table entry here, keep scanning the table:
    if (l == NULL) {
      continue;
    }

    // Iterate through each item in the list:
    j = 0;
    length = l_get_length(l);
    for (j = m->key_arity; j < length; j += m->key_arity + 1) {
      f(l_get_item(l, j));
    }
  }
}
void m3_foreach(map3 *m, void (*f)(void *)) {
  size_t i = 0, j = 0, length = 0;
  list *l = NULL;
  // Iterate through each list in the table:
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    l = m->table[i];
    // If there's no table entry here, keep scanning the table:
    if (l == NULL) {
      continue;
    }

    // Iterate through each item in the list:
    j = 0;
    length = l_get_length(l);
    for (j = 3; j < length; j += 4) {
      f(l_get_item(l, j));
    }
  }
}

void m_witheach(map *m, void *arg, void (*f)(void *, void *)) {
  size_t i = 0, j = 0, length = 0;
  list *l = NULL;
  // Iterate through each list in the table:
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    l = m->table[i];
    // If there's no table entry here, keep scanning the table:
    if (l == NULL) {
      continue;
    }

    // Iterate through each item in the list:
    j = 0;
    length = l_get_length(l);
    for (j = m->key_arity; j < length; j += m->key_arity + 1) {
      f(l_get_item(l, j), arg);
    }
  }
}
void m3_witheach(map3 *m, void *arg, void (*f)(void *, void *)) {
  size_t i = 0, j = 0, length = 0;
  list *l = NULL;
  // Iterate through each list in the table:
  for (i = 0; i < MAP_TABLE_SIZE; ++i) {
    l = m->table[i];
    // If there's no table entry here, keep scanning the table:
    if (l == NULL) {
      continue;
    }

    // Iterate through each item in the list:
    j = 0;
    length = l_get_length(l);
    for (j = 3; j < length; j += 4) {
      f(l_get_item(l, j), arg);
    }
  }
}