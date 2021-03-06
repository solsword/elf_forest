// dictionary.c
// Simple hash-table-based ordered dictionaries that can story almost any
// object by running a common hash function on its bytes. Unlike maps,
// dictionaries can contain multiple entries per key, and don't limit the size
// of key information.

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <omp.h>

#include "dictionary.h"

#include "list.h"

#include "boilerplate.h"

/******************
 * Internal Types *
 ******************/

// A dictionary hash is just an unsigned integer:
typedef size_t dictionary_hash_t;

/***********************
 * Internal Structures *
 ***********************/

struct dictionary_entry_s;
typedef struct dictionary_entry_s dictionary_entry;

/**********************
 * Internal Constants *
 **********************/

#define DICT_BUCKET_SIZE 4

/**************
 * Structures *
 **************/

// A hash-table-based dictionary that stores items indexed by arbitrary keys:
struct dictionary_s {
  list *ordered;
  size_t table_size;
  list ** table;
  omp_lock_t lock; // For thread safety.
};

struct dictionary_entry_s {
  size_t index;
  d_key_t *key;
  size_t key_size;
  void *value;
};

/*********************
 * Private Functions *
 *********************/

// The hash function:
static inline dictionary_hash_t dict_hash(d_key_t *data, size_t size) {
  size_t i;
#ifdef DEBUG
  if (size < 1) {
    fprintf(stderr, "Error: Dictionary key of size 0.\n");
    exit(EXIT_FAILURE);
  }
#endif
  dictionary_hash_t result = 0;
  for (i = 0; i < size; ++i) {
    result ^= (
      ((dictionary_hash_t) (data[i]))
   << (
        (sizeof(d_key_t) * i)
      % (sizeof(dictionary_hash_t) - sizeof(d_key_t) + 1)
      )
    );
  }
  result = result ^ (result % 17) ^ (result % 6011);
  return result;
}

// Matching function for computing exact matches:
static inline int dict_is_match(
  dictionary_entry *e,
  d_key_t *key,
  size_t key_size
) {
#ifdef DEBUG
  if (key_size < 1) {
    fprintf(stderr, "Error: Dictionary key of size 0.\n");
    exit(EXIT_FAILURE);
  }
#endif
  if (key_size != e->key_size) {
    return 0;
  }
  size_t i;
  for (i = 0; i < key_size; ++i) {
    if (e->key[i] != key[i]) {
      return 0;
    }
  }
  return 1;
}

CLEANUP_DECL(dictionary_entry);

/******************************
 * Constructors & Destructors *
 ******************************/

dictionary_entry* create_dictionary_entry(
  size_t index,
  d_key_t *key,
  size_t key_size,
  void *value
) {
  dictionary_entry *result = (dictionary_entry*) malloc(
    sizeof(dictionary_entry)
  );
  result->index = index;
  result->key = (d_key_t*) malloc(sizeof(d_key_t) * key_size);
  memcpy((void*) result->key, (void*) key, sizeof(d_key_t) * key_size);
  result->key_size = key_size;
  result->value = value;
  return result;
}

CLEANUP_IMPL(dictionary_entry) {
  free(doomed->key);
  free(doomed);
}

void destroy_dictionary_entry(dictionary_entry *doomed) {
  free(doomed->value);
  free(doomed->key);
  free(doomed);
}

// Allocate and set up a new empty dictionary with the given table size:
dictionary* create_dictionary(size_t table_size) {
  dictionary *result = (dictionary*) malloc(sizeof(dictionary));
  result->ordered = create_list(table_size/2);
  result->table_size = table_size;
  result->table = (list**) calloc(table_size, sizeof(list*));
  omp_init_lock(&(result->lock));
  return result;
}

// Frees the memory associated with a dictionary.
CLEANUP_IMPL(dictionary) {
  size_t i;
  list *bucket;
  l_foreach(doomed->ordered, cleanup_v_dictionary_entry);
  cleanup_list(doomed->ordered);
  for (i = 0; i < doomed->table_size; ++i) {
    bucket = (list*) doomed->table[i];
    if (bucket != NULL) {
      cleanup_list(bucket);
    }
  }
  free(doomed->table);
  free(doomed);
}

void destroy_dictionary(dictionary *doomed) {
  size_t i;
  list *bucket;
  dictionary_entry *e;
  for (i = 0; i < l_get_length(doomed->ordered); ++i) {
    e = (dictionary_entry*) l_get_item(doomed->ordered, i);
    destroy_dictionary_entry(e);
  }
  cleanup_list(doomed->ordered);
  for (i = 0; i < doomed->table_size; ++i) {
    bucket = (list*) doomed->table[i];
    if (bucket != NULL) {
      cleanup_list(bucket);
    }
  }
  free(doomed->table);
  free(doomed);
}

/***********
 * Locking *
 ***********/

void d_lock(dictionary *d) { omp_set_lock(&(d->lock)); }
void d_unlock(dictionary *d) { omp_unset_lock(&(d->lock)); }

/*************
 * Functions *
 *************/

int d_is_empty(dictionary const * const d) {
  return l_get_length(d->ordered) == 0;
}

size_t d_get_count(dictionary const * const d) {
  return l_get_length(d->ordered);
}

void* d_get_item(dictionary const * const d, size_t index) {
  dictionary_entry *e = (dictionary_entry*) l_get_item(d->ordered, index);
  if (e == NULL) {
#ifdef DEBUG
    fprintf(stderr, "Warning: d_get_item on out-of-bounds item.\n");
#endif
    return NULL;
  }
  return e->value;
}

void d_get_key(
  dictionary const * const d,
  size_t index,
  d_key_t **r_key,
  size_t *r_size
) {
  dictionary_entry *e = (dictionary_entry*) l_get_item(d->ordered, index);
  *r_key = e->key;
  *r_size = e->key_size;
}

int d_contains_key(dictionary const * const d, d_key_t *key, size_t key_size) {
  size_t i;
  dictionary_entry *e;
  dictionary_hash_t hash = dict_hash(key, key_size);
  list * bucket = d->table[hash % d->table_size];
  if (bucket == NULL) {
    return 0;
  }
  for (i = 0; i < l_get_length(bucket); ++i) {
    e = (dictionary_entry*) l_get_item(bucket, i);
    if (dict_is_match(e, key, key_size)) {
      return 1;
    }
  }
  return 0;
}

void* d_get_value(dictionary const * const d, d_key_t *key, size_t key_size) {
  size_t i;
  dictionary_entry *e;
  dictionary_hash_t hash = dict_hash(key, key_size);
  list * bucket = d->table[hash % d->table_size];
  if (bucket == NULL) {
    return NULL;
  }
  for (i = 0; i < l_get_length(bucket); ++i) {
    e = (dictionary_entry*) l_get_item(bucket, i);
    if (dict_is_match(e, key, key_size)) {
      return e->value;
    }
  }
  return NULL;
}

list* d_get_all(dictionary const * const d, d_key_t *key, size_t key_size) {
  list *result;
  size_t i;
  dictionary_entry *e;
  dictionary_hash_t hash = dict_hash(key, key_size);
  list * bucket = d->table[hash % d->table_size];
  result = create_list(LIST_DEFAULT_SMALL_CHUNK_SIZE);
  if (bucket == NULL) {
    return result;
  }
  for (i = 0; i < l_get_length(bucket); ++i) {
    e = (dictionary_entry*) l_get_item(bucket, i);
    if (dict_is_match(e, key, key_size)) {
      l_append_element(result, e->value);
    }
  }
  return result;
}

void d_add_value(dictionary *d, d_key_t *key, size_t key_size, void *value) {
  dictionary_entry *e;
  dictionary_hash_t hash;
  list *bucket;

  // Create an entry and add it to our ordered list:
  e = create_dictionary_entry(
    l_get_length(d->ordered),
    key,
    key_size,
    value
  );
  l_append_element(d->ordered, e);

  // Find the appropriate bucket and add it there too:
  hash = dict_hash(key, key_size);
  bucket = d->table[hash % d->table_size];
  if (bucket == NULL) {
    bucket = create_list(DICT_BUCKET_SIZE);
    d->table[hash % d->table_size] = bucket;
  }
  l_append_element(bucket, e);
}

void d_prepend_value(dictionary *d, d_key_t *key, size_t key_size, void *value){
  size_t i;
  dictionary_entry *e;
  dictionary_hash_t hash;
  list *bucket;

  // Create an entry and add it to our ordered list:
  e = create_dictionary_entry(
    0,
    key,
    key_size,
    value
  );
  l_insert_element(d->ordered, e, 0);
  for (i = 1; i < l_get_length(d->ordered); ++i) {
    ((dictionary_entry*) l_get_item(d->ordered, i))->index += 1;
  }

  // Find the appropriate bucket and add it there too:
  hash = dict_hash(key, key_size);
  bucket = d->table[hash % d->table_size];
  if (bucket == NULL) {
    bucket = create_list(DICT_BUCKET_SIZE);
    d->table[hash % d->table_size] = bucket;
  }
  l_insert_element(bucket, e, 0);
}

void d_set_value(dictionary *d, d_key_t *key, size_t key_size, void *value) {
  d_clear_values(d, key, key_size);
  d_add_value(d, key, key_size, value);
}

void d_set_all(dictionary *d, d_key_t *key, size_t key_size, list *values) {
  size_t i;
  void *value;
  dictionary_entry *e;
  dictionary_hash_t hash;
  list *bucket;

  d_clear_values(d, key, key_size);

  hash = dict_hash(key, key_size);
  bucket = d->table[hash % d->table_size];
  if (bucket == NULL) {
    bucket = create_list(DICT_BUCKET_SIZE);
    d->table[hash % d->table_size] = bucket;
  }

  for (i = 0; i < l_get_length(values); ++i) {
    value = l_get_item(values, i);

    e = create_dictionary_entry(
      l_get_length(d->ordered),
      key,
      key_size,
      value
    );
    l_append_element(d->ordered, e);
    l_append_element(bucket, e);
  }
}

void* d_pop_value(dictionary *d, d_key_t *key, size_t key_size) {
  size_t i;
  void *value;
  dictionary_entry *e;
  dictionary_hash_t hash = dict_hash(key, key_size);
  list * bucket = d->table[hash % d->table_size];
  if (bucket == NULL) {
    return NULL;
  }
  for (i = 0; i < l_get_length(bucket); ++i) {
    e = (dictionary_entry*) l_get_item(bucket, i);
    if (dict_is_match(e, key, key_size)) {
      break;
    } else {
      e = NULL;
    }
  }
  if (e == NULL) {
    return NULL;
  }
  value = e->value;

  // remove it from the bucket:
  l_remove_item(bucket, i);

  // remove it from the ordered list:
  l_remove_item(d->ordered, e->index);
  for (i = e->index; i < l_get_length(d->ordered); ++i) {
    ((dictionary_entry*) l_get_item(d->ordered, i))->index -= 1;
  }

  // dispose of the entry:
  cleanup_dictionary_entry(e);

  // return just the value:
  return value;
}

// Helper for d_remove_value:
int _find_value_in_dict(void *v_entry, void *value) {
  dictionary_entry *e = (dictionary_entry*) v_entry;
  if (e->value == value) {
    return 1;
  }
  return 0;
}

void * d_remove_value(dictionary *d, void* target) {
  size_t i;
  ptrdiff_t match;
  dictionary_entry *e;
  void *value;

  match = l_scan_indices(d->ordered, target, &_find_value_in_dict);
  if (match < 0) {
    return NULL;
  }

  e = (dictionary_entry*) l_get_item(d->ordered, match);

  // Remove from the ordered list:
  l_remove_item(d->ordered, match);
  for (i = match; i < l_get_length(d->ordered); ++i) {
    ((dictionary_entry*) l_get_item(d->ordered, i))->index -= 1;
  }

  // Remove from it's bucket:
  dictionary_hash_t hash = dict_hash(e->key, e->key_size);
  list * bucket = d->table[hash % d->table_size];
#ifdef DEBUG
  if (bucket == NULL) { // shouldn't be possible
    fprintf(stderr, "Error: Entry in ordered list has no bucket!\n");
    exit(EXIT_FAILURE);
  }
#endif
  e = l_remove_element(bucket, e);
#ifdef DEBUG
  if (e == NULL) { // shouldn't be possible
    fprintf(stderr, "Error: Entry in ordered list was missing from bucket!\n");
    exit(EXIT_FAILURE);
  }
#endif

  value = e->value;

  // dispose of the entry:
  cleanup_dictionary_entry(e);

  // return just the value:
  return value;
}

void d_clear_values(dictionary *d, d_key_t *key, size_t key_size) {
  size_t i, j;
  dictionary_entry *e;
  dictionary_hash_t hash = dict_hash(key, key_size);
  list * bucket = d->table[hash % d->table_size];
  if (bucket == NULL) {
    return;
  }
  for (i = 0; i < l_get_length(bucket); ++i) {
    e = (dictionary_entry*) l_get_item(bucket, i);
    if (dict_is_match(e, key, key_size)) {
      l_remove_item(d->ordered, e->index);
      for (j = e->index; j < l_get_length(d->ordered); ++j) {
        ((dictionary_entry*) l_get_item(d->ordered, j))->index -= 1;
      }
      l_remove_item(bucket, i);
      cleanup_dictionary_entry(e);
      i -= 1;
    } else {
      e = NULL;
    }
  }
}

void d_clear(dictionary *d) {
  size_t i;
  list *bucket;
  for (i = 0; i < d->table_size; ++i) {
    bucket = d->table[i];
    if (bucket != NULL) {
      l_clear(bucket);
    }
  }
  l_foreach(d->ordered, &cleanup_v_dictionary_entry);
  l_clear(d->ordered);
}

void d_foreach(dictionary const * const d, void (*f)(void *)) {
  size_t i;
  dictionary_entry *e;
  for (i = 0; i < l_get_length(d->ordered); ++i) {
    e = (dictionary_entry*) l_get_item(d->ordered, i);
    f(e->value);
  }
}

void d_witheach(
  dictionary const * const d,
  void *arg,
  void (*f)(void *, void *)
) {
  size_t i;
  dictionary_entry *e;
  for (i = 0; i < l_get_length(d->ordered); ++i) {
    e = (dictionary_entry*) l_get_item(d->ordered, i);
    f(e->value, arg);
  }
}

list * d_as_list(dictionary const * const d) {
  size_t i, count, size;
  dictionary_entry *e;
  list *result;

  count = d_get_count(d);
  size = count/2;

  if (size < LIST_DEFAULT_SMALL_CHUNK_SIZE) {
    size = LIST_DEFAULT_SMALL_CHUNK_SIZE;
  }
  result = create_custom_list(size);
  for (i = 0; i < count; ++i) {
    e = (dictionary_entry*) l_get_item(d->ordered, i);
    l_append_element(result, e->value);
  }
  return result;
}

size_t d_data_size(dictionary const * const d) {
  size_t i;
  dictionary_entry *e;
  size_t result = 0;
  for (i = 0; i < l_get_length(d->ordered); ++i) {
    e = (dictionary_entry*) l_get_item(d->ordered, i);
    result += e->key_size * sizeof(d_key_t);
    result += sizeof(void*);
  }
  return result;
}
size_t d_overhead_size(dictionary const * const d) {
  size_t i;
  list *bucket;
  size_t result = (
    (sizeof(dictionary_entry) - sizeof(void*))
  * l_get_length(d->ordered)
  );
  result += l_data_size(d->ordered) + l_overhead_size(d->ordered);
  for (i = 0; i < d->table_size; ++i) {
    bucket = d->table[i];
    result += l_data_size(bucket) + l_overhead_size(bucket);
  }
  result += d->table_size * sizeof(list*);
  result += sizeof(dictionary);
  return result;
}

float d_utilization(dictionary const * const d) {
  size_t i;
  list *bucket;
  float result = 0;
  for (i = 0; i < d->table_size; ++i) {
    bucket = d->table[i];
    if (bucket != NULL) {
      result += 1;
    }
  }
  return result / ((float) d->table_size);
}

float d_crowding(dictionary const * const d) {
  size_t i;
  list *bucket;
  float result = 0;
  float denom = 0;
  for (i = 0; i < d->table_size; ++i) {
    bucket = d->table[i];
    if (bucket != NULL) {
      result += l_get_length(bucket);
      denom += 1;
    }
  }
  if (denom > 0) {
    return result / denom;
  } else {
    return 0;
  }
}
