#ifndef MAP_H
#define MAP_H

// map.h
// Simple hash-table-based maps that accept a variable number of keys per
// element. Map instances must always be given the same number of keys to work
// with of course.

#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>

/*********
 * Types *
 *********/

// A map key is just an unsigned integer:
typedef void* map_key_t;

/***********
 * Globals *
 ***********/

// Defines the size of a map's hash table.
#define MAP_TABLE_SIZE 16384

/**************
 * Structures *
 **************/

// A hash-table-based map that stores pointer values indexed by pointer keys:
struct map_s;
typedef struct map_s map;

// A hash-table-based map using exactly 3 keys:
struct map3_s;
typedef struct map3_s map3;

// Note: the actual structures are defined in map.c, not here. This is
// intentional: external code should only use map pointers and shouldn't deal
// with the internals of maps directly.

/*************
 * Functions *
 *************/

// Allocate and set up a new empty map with the given key arity:
map *create_map(size_t key_arity);
map3 *create_map3();

// Frees the memory associated with a map.
void cleanup_map(map *m);
void cleanup_map3(map3 *m);

// Frees the memory associated with a map, and also calls free on each value in
// the map (but not on each key, as keys are commonly not real pointers).
void destroy_map(map *m);
void destroy_map3(map3 *m);


// Returns the key arity of the given map. All of the varargs functions must
// always be passed exactly as many arguments as the key arity of the map they
// are given.
inline int m_get_key_arity(map *m);

// Tests whether the given map is empty.
inline int m_is_empty(map *m);
inline int m3_is_empty(map3 *m);

// Returns the number of values in the given map.
inline size_t m_get_count(map *m);
inline size_t m3_get_count(map3 *m);

// Test whether the given map contains a value with the given key(s).
int m_contains_key(map *m, ...);
int m3_contains_key(map3 *m, map_key_t x, map_key_t y, map_key_t z);

// Returns the value corresponding to the given key(s), or NULL if no value is
// present for those key(s).
void * m_get_value(map *m, ...);
void * m3_get_value(map3 *m, map_key_t x, map_key_t y, map_key_t z);

// Adds the given value to the map under the given key(s). Allocates new memory
// to expand the map as necessary. If there is already a value indexed by the
// given key(s), it returns that value after overwriting it.
void * m_put_value(map *m, void *value, ...);
void * m3_put_value(map3 *m, void *value, map_key_t x, map_key_t y,map_key_t z);

// Removes and returns the value indexed under the given key(s). Returns NULL
// if there is no such value in the map.
void * m_pop_value(map *m, ...);
void * m3_pop_value(map3 *m, map_key_t x, map_key_t y, map_key_t z);

// Removes all copies of the given value from the given map (uses address
// comparison). Returns the number of elements removed (possibly 0).
size_t m_remove_all_values(map *m, void *value);
size_t m3_remove_all_values(map3 *m, void *value);

// Runs the given function sequentially on each value in the map.
void m_foreach(map *m, void (*f)(void *));
void m3_foreach(map3 *m, void (*f)(void *));

// Runs the given function sequentially on each value in the map with the given
// extra argument as its second argument.
void m_witheach(map *m, void *arg, void (*f)(void *, void *));
void m3_witheach(map3 *m, void *arg, void (*f)(void *, void *));

#endif //ifndef MAP_H