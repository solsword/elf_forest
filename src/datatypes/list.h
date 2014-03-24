#ifndef LIST_H
#define LIST_H

// list.h
// Simple array-based lists.

#include <stdlib.h>
#include <stdint.h>

/***********
 * Globals *
 ***********/

// Defines what size memory chunk lists use internally.
extern const size_t LIST_CHUNK_SIZE;

// Defines how many empty chunks we should keep at the end of a shrinking list
// before reallocating to a smaller size.
extern const size_t LIST_KEEP_CHUNKS;

/**************
 * Structures *
 **************/

// An array-based list:
struct list_s;
typedef struct list_s list;

// Note: list_s is defined in list.c, not here. This is intentional: external
// code should only use list pointers and shouldn't deal with the internals of
// lists directly.

/*************
 * Functions *
 *************/

// Allocates and sets up a new empty list:
list *create_list(void);

// Frees the memory associated with a list.
void cleanup_list(list *l);

// Frees the memory associated with a list, and also calls free on each element
// in the list.
void destroy_list(list *l);


// Tests whether the given list is empty.
int l_is_empty(list *l);

// Test whether the given list contains the given element (uses address
// comparison).
int l_contains(list *l, void *element);

// Returns the length of the given list.
size_t l_get_length(list *l);

// Returns the ith element of the given list, or NULL if i is out of range.
void * l_get_element(list *l, size_t i);

// Adds the given element to the end of the given list. Allocates new memory to
// expand the list as necessary.
void l_append_element(list *l, void *element);

// Removes and returns the last element of the given list. Returns NULL if the
// list is already empty.
void * l_pop_element(list *l);

// Removes just the first copy of the given element from the given list (uses
// address comparison). Returns the removed element, or NULL if the given
// element wasn't found.
void * l_remove_element(list *l, void *element);

// Removes all copies of the given element from the given list (uses address
// comparison). If the destroy version is used free() is called on each element
// as it is removed from the list. Both versions return the number of elements
// removed.
int l_remove_all_elements(list *l, void *element);
int l_destroy_all_elements(list *l, void *element);

// Reverses the given list. Doesn't allocate or free any memory.
void l_reverse(list *l);

// Runs the given function sequentially on each element in the list.
void l_foreach(list *l, void (*f)(void *));

// Scans the list until the given function returns non-zero, and returns the
// element that matched. Returns NULL if no match was found.
void * l_find_element(list *l, int (*match)(void *));

#endif //ifndef LIST_H
