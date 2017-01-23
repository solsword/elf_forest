#ifndef LIST_H
#define LIST_H

// list.h
// Simple array-based lists.

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "boilerplate.h"

/***********
 * Globals *
 ***********/

// The size of list chunks determines the tradeoff between wasted space and
// wasted malloc time when adding and removing items from a list. Custom values
// can be used in specific situations.
#define LIST_DEFAULT_SMALL_CHUNK_SIZE 8
#define LIST_DEFAULT_LARGE_CHUNK_SIZE 64

// Defines how many empty chunks are kept at the end of a shrinking list before
// reallocating to a smaller size.
extern size_t const LIST_KEEP_CHUNKS;

/**************
 * Structures *
 **************/

// An array-based list:
struct list_s;
typedef struct list_s list;

// Note: list_s is defined in list.c, not here. This is intentional: external
// code should only use list pointers and shouldn't deal with the internals of
// lists directly.

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and sets up a new empty list using the given chunk size:
list *create_custom_list(size_t chunk_size);

// Same as create_custom_list but uses a default chunk size:
static inline list* create_list() {
  return create_custom_list(LIST_DEFAULT_SMALL_CHUNK_SIZE);
}

// Allocates and sets up a new empty list that uses the same chunk size as the
// given list.
list *create_list_like(list const * const src);

// Copies the given list, returning a newly-allocated list containing the same
// data. If list items are references, they will be shared by both lists.
list *copy_list(list const * const src);

// Frees the memory associated with a list.
CLEANUP_DECL(list);

/***********
 * Locking *
 ***********/

void l_lock(list *l);
void l_unlock(list *l);

/*************
 * Functions *
 *************/

// Frees the memory associated with a list, and also calls free on each element
// in the list.
void destroy_list(list *l);

// Tests whether the given list is empty.
int l_is_empty(list const * const l);

// Returns the length of the given list.
size_t l_get_length(list const * const l);

// Test whether the given list contains the given element (uses address
// comparison).
int l_contains(list const * const l, void *element);

// Scans the list until it finds the given element, and returns that element's
// index. If the element appears multiple times in the list, the index of the
// first occurrence is returned. Returns -1 if the given element isn't in the
// list.
ptrdiff_t l_index_of(list const * const l, void *element);

// Returns the ith element of the given list, or NULL if i is out of range.
void * l_get_item(list const * const l, size_t i);

// Returns the last element in the given list, or NULL if the list is empty.
void * l_get_last(list const * const l);

// Returns a pointer to the ith element of the given list. This function should
// normally be avoided and the pointers it returns aren't safe to use if the
// list grows or shrinks. It also doesn't do any bounds checking, and it isn't
// thread-safe. This is why it has an extra underscore at the beginning of its
// name.
void ** _l_get_pointer(list *l, size_t i);

// Removes the element at index i, returning the removed value. If i is
// out-of-range, it does nothing and returns NULL.
void * l_remove_item(list *l, size_t i);

// Removes all elements from the list.
void l_clear(list *l);

// Removes the given number of items starting at the given index. The delete
// version frees the items before removing them. If any part of the range is
// out-of-bounds, it does nothing.
void l_remove_range(list *l, size_t i, size_t n);
void l_delete_range(list *l, size_t i, size_t n);

// Replaces the element at index i with the given element, returning the
// replaced value. If i is out-of-range, it does nothing and returns NULL.
void * l_replace_item(list *l, size_t i, void *element);

// Adds the given element to the end of the given list. Allocates new memory to
// expand the list as necessary.
void l_append_element(list *l, void *element);

// Inserts the given element before the ith member of the given list. Allocates
// new memory to expand the list as necessary. Note that this is much more
// expensive than l_append_element because elements after the insertion have to
// be moved.
void l_insert_element(list *l, void *element, size_t i);

// Adds all members of the other list to this list, without altering the other
// list.
void l_extend(list *l, list *other);

// Removes and returns the last element of the given list. Returns NULL if the
// list is already empty.
void * l_pop_element(list *l);

// Removes just the first copy of the given element from the given list (uses
// address comparison). Returns the removed element, or NULL if the given
// element wasn't found.
void * l_remove_element(list *l, void *element);

// Removes all copies of the given element from the given list (uses address
// comparison). Returns the number of elements removed.
int l_remove_all_elements(list *l, void *element);

// Reverses the given list. Doesn't allocate or free any memory.
void l_reverse(list *l);

// Picks a random element from the list using a uniform distribution. Returns
// NULL if called on an empty list.
void* l_pick_random(list const * const l, ptrdiff_t seed);

// Shuffles the given list using the given seed. The shuffle is notably not
// cryptographically sound.
void l_shuffle(list *l, ptrdiff_t seed);

// Runs the given function sequentially on each element in the list.
void l_foreach(list const * const l, void (*f)(void *));

// Runs the given function sequentially on each element in the list with the
// given extra argument as its second argument.
void l_witheach(list const * const l, void *arg, void (*f)(void *, void *));

// Transforms each element of the given list using the given function.
void l_apply(list *l, void* (*f)(void*));

// Scans the list until the given function returns non-zero, and returns the
// index that matched. Returns -1 if no match was found.
ptrdiff_t l_find_index(list const * const l, int (*match)(void *));

// Scans the list until the given function returns non-zero given the list
// element as its first argument and the reference as its second argument.
// Returns the index that matched. Returns -1 if no match was found.
ptrdiff_t l_scan_indices(
  list const * const l,
  void *ref,
  int (*match)(void *, void *)
);

// Works like l_find_index but returns the matching element instead of its
// index.
void * l_find_element(list const * const l, int (*match)(void *));

// Works like l_scan_indices but returns the matching element instead of its
// index.
void * l_scan_elements(
  list const * const l,
  void *ref,
  int (*match)(void *, void *)
);

// Takes a transformation function and returns a newly allocated list
// constructed by applying the transformation function to each element of the
// given list.
list * l_map(list const * const input, void * (*map)(void *));

// Works like l_map, but the mapping function gets the given extra argument as
// its second argument.
list * l_map_with(
  list const * const input,
  void * arg,
  void * (*map)(void *, void *)
);

// Counts the number of bytes of data/overhead used by the given list.
size_t l_data_size(list const * const l);
size_t l_overhead_size(list const * const l);

#endif //ifndef LIST_H
