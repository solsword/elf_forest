#ifndef MAP_H
#define MAP_H

// map.h
// Simple hash-table-based maps that accept a fixed number of keys per element.
// Different map objects may have different key counts.

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

#include "boilerplate.h"

/*********
 * Types *
 *********/

// A map key is just a void pointer:
typedef void* map_key_t;

/**************
 * Structures *
 **************/

// A hash-table-based map that stores pointer values indexed by pointer keys:
struct map_s;
typedef struct map_s map;

// Note: the actual structure is defined in map.c, not here. This is
// intentional: external code should only use map pointers and shouldn't deal
// with the internals of maps directly.

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocate and set up a new empty map with the given key arity:
map *create_map(size_t key_arity, size_t table_size);

// Frees the memory associated with a map.
CLEANUP_DECL(map);

// Frees the memory associated with a map, and also calls free on each value in
// the map (but not on each key, as keys are commonly not real pointers).
void destroy_map(map *m);

/***********
 * Locking *
 ***********/

void m_lock(map *m);
void m_unlock(map *m);

/*************
 * Functions *
 *************/

// Returns the key arity of the given map. All of the varargs functions must
// always be passed exactly as many arguments as the key arity of the map they
// are given.
int m_get_key_arity(map *m);

// Tests whether the given map is empty.
int m_is_empty(map *m);

// Returns the number of values in the given map.
size_t m_get_count(map *m);

// Test whether the given map contains a value with the given key(s).
int m_contains_key(map *m, ...);
int m1_contains_key(map *m, map_key_t x);
int m2_contains_key(map *m, map_key_t x, map_key_t y);
int m3_contains_key(map *m, map_key_t x, map_key_t y, map_key_t z);

// Returns the value corresponding to the given key(s), or NULL if no value is
// present for those key(s). Note that NULL values may be stored in the map,
// and if this is the case, m_contains_key can be used to verify whether a NULL
// return value is due to a missing key or a stored NULL value.
void * m_get_value(map *m, ...);
void * m1_get_value(map *m, map_key_t x);
void * m2_get_value(map *m, map_key_t x, map_key_t y);
void * m3_get_value(map *m, map_key_t x, map_key_t y, map_key_t z);

// Adds the given value to the map under the given key(s). Allocates new memory
// to expand the map as necessary. If there is already a value indexed by the
// given key(s), it returns that value after overwriting it, otherwise it
// returns NULL.
void * m_put_value(map *m, void *value, ...);
void * m1_put_value(map *m, void *value, map_key_t x);
void * m2_put_value(map *m, void *value, map_key_t x, map_key_t y);
void * m3_put_value(map *m, void *value, map_key_t x, map_key_t y, map_key_t z);

// Removes and returns the value indexed under the given key(s). Returns NULL
// if there is no such value in the map.
void * m_pop_value(map *m, ...);
void * m1_pop_value(map *m, map_key_t x);
void * m2_pop_value(map *m, map_key_t x, map_key_t y);
void * m3_pop_value(map *m, map_key_t x, map_key_t y, map_key_t z);

// Searches through the map to find if it contains the given *value* (not key).
int m_contains_value(map *m, void *value);

// Removes all copies of the given value from the given map (uses address
// comparison). Returns the number of elements removed (possibly 0).
size_t m_remove_all_values(map *m, void *value);

// Functions for reverse lookups which return key(s) via return arguments.
// Their return values indicate success (1) or failure (0) of the lookup. If
// the sought-after value exists multiple times in the map, an arbitrary valid
// key for it will be returned (this is stable but not if the map is modified).
int m1_reverse_lookup(map *m, void *value, map_key_t *r_key);
int m2_reverse_lookup(
  map *m,
  void *value,
  map_key_t *r_k1,
  map_key_t *r_k2
);
int m3_reverse_lookup(
  map *m,
  void *value,
  map_key_t *r_k1,
  map_key_t *r_k2,
  map_key_t *r_k3
);

// Runs the given function sequentially on each value in the map.
void m_foreach(map *m, void (*f)(void *));

// Functions for calling a function for each keyset in the map (as opposed to
// each value):
void m1_foreach_key(map *m, void (*f)(map_key_t));
void m2_foreach_key(map *m, void (*f)(map_key_t, map_key_t));
void m3_foreach_key(map *m, void (*f)(map_key_t, map_key_t, map_key_t));

// Runs the given function sequentially on each value in the map with the given
// extra argument as its second argument.
void m_witheach(map *m, void *arg, void (*f)(void *, void *));

// Functions for calling a function for each keyset in the map (as opposed to
// each value) with an extra argument:
void m1_witheach_key(map *m, void *arg, void (*f)(map_key_t, void*));
void m2_witheach_key(map *m, void *arg, void (*f)(map_key_t, map_key_t, void*));
void m3_witheach_key(
  map *m,
  void *arg,
  void (*f)(map_key_t, map_key_t, map_key_t, void*)
);

// Counts the number of bytes of data/overhead used by the given map. For a
// map, the data size is the space devoted to storing keys and values, while
// the overhead is all other space.
size_t m_data_size(map *m);

size_t m_overhead_size(map *m);

// Returns a number between 0 and 1 representing the fraction of the map's
// table entries that are being used.
float m_utilization(map *m);

// Returns the average number of entries per cell. This correlates with lookup
// times.
float m_crowding(map *m);

#endif //ifndef MAP_H
