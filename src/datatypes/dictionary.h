#ifndef DICTIONARY_H
#define DICTIONARY_H

// dictionary.h
// Simple hash-table-based ordered dictionaries that can story almost any
// object by running a common hash function on its bytes. Unlike maps,
// dictionaries can contain multiple entries per key, and don't limit the size
// of key information.

#include <stdlib.h>
#include <stdint.h>

#include "list.h"

#include "boilerplate.h"

/*********
 * Types *
 *********/

// A dictionary's keys are just arbitrary data:
typedef char d_key_t;

/**************
 * Structures *
 **************/

// A hash-table-based dictionary that stores items indexed by arbitrary keys:
struct dictionary_s;
typedef struct dictionary_s dictionary;

// Note: the actual structure is defined in dictionary.c, not here. This is
// intentional: external code should only use pointers and shouldn't deal with
// the internals of dictionaries directly.

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocate and set up a new empty dictionary with the given table size:
dictionary *create_dictionary(size_t table_size);

// Frees the memory associated with a dictionary.
CLEANUP_DECL(dictionary);

// Frees the memory associated with a dictionary, and also calls free on each
// value in the dictionary.
void destroy_dictionary(dictionary *d);

/***********
 * Locking *
 ***********/

void d_lock(dictionary *d);
void d_unlock(dictionary *d);

/*************
 * Functions *
 *************/

// Tests whether the given dictionary is empty.
int d_is_empty(dictionary *d);

// Returns the number of values in the given dictionary.
size_t d_get_count(dictionary *d);

// Gets the nth item in the dictionary, according to the order in which items
// were added.
void * d_get_item(dictionary *d, size_t index);

// Gets the nth key in the dictionary, according to the order in which items
// were added. Note that when multiple items are entered under the same key,
// that key appears multiple times in the dictionary's key list. The key is
// returned via the r_key and r_size parameters, which are set to point to the
// key data (needless to say said data shouldn't be altered).
void d_get_key(dictionary *d, size_t index, d_key_t **r_key, size_t *r_size);

// Test whether the given dictionary contains any values under the given key.
int d_contains_key(dictionary *d, d_key_t *key, size_t key_size);

// Returns the first value stored under the given key, or NULL if no values are
// present for that key.
void * d_get_value(dictionary *d, d_key_t *key, size_t key_size);

// Returns a newly allocated list containing all values stored under the given
// key. Although the list is fresh, its values are the same as those in the
// dictionary, so they shouldn't be freed. If there are no matches for the
// given key, an empty list is returned.
list * d_get_all(dictionary *d, d_key_t *key, size_t key_size);

// Adds the given value to the dictionary under the given key. Allocates new
// memory to expand the dictionary as necessary. If there is already a value
// indexed by the given key, the given value will be inserted after it (and
// thus will not be returned by d_get_value).
void d_add_value(dictionary *d, d_key_t *key, size_t key_size, void *value);

// Works like d_add_value, but orders the value first instead of last both
// globally and within values with the same key.
void d_prepend_value(dictionary *d, d_key_t *key, size_t key_size, void *value);

// Removes all previous values under the given key and adds the given value.
void d_set_value(dictionary *d, d_key_t *key, size_t key_size, void *value);

// Removes all previous values under the given key and copies values from the
// given list as the new values for that key. The given list may be cleaned up
// by the caller, although its values should not be freed.
void d_set_all(dictionary *d, d_key_t *key, size_t key_size, list *values);

// Removes and returns the first value for the given key. If there are no
// matching values it returns NULL.
void * d_pop_value(dictionary *d, d_key_t *key, size_t key_size);

// Removes the first entry with the given value from the dictionary. Returns
// the removed value, or NULL if no matching value was found.
void * d_remove_value(dictionary *d, void* target);

// Removes all values for the given key.
void d_clear_values(dictionary *d, d_key_t *key, size_t key_size);

// Removes all values from the dictionary, leaving it empty.
void d_clear(dictionary *d);

// Runs the given function sequentially on each value in the dictionary,
// according to the order they were added in.
void d_foreach(dictionary *d, void (*f)(void *));

// Runs the given function sequentially on each value in the dictionary with
// the given extra argument as its second argument.
void d_witheach(dictionary *d, void *arg, void (*f)(void *, void *));

// Counts the number of bytes of data/overhead used by the given dictionary.
// For a dictionary, the data size is the space devoted to storing keys and
// values, while the overhead is all other space.
size_t d_data_size(dictionary *d);
size_t d_overhead_size(dictionary *d);

// Returns a number between 0 and 1 representing the fraction of the
// dictionary's table entries that are being used.
float d_utilization(dictionary *d);

// Returns the average number of entries per cell. This correlates with lookup
// times.
float d_crowding(dictionary *d);

#endif // ifndef DICTIONARY_H
