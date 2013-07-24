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

// A singly-linked list with a tail-pointer:
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

// Tests whether the given list is empty.
int is_empty(list *l);

// Test whether the given list contains the given element (uses address
// comparison).
int contains(list *l, void *element);

// Adds the given element to the end of the given list. Allocates new memory to
// expand the list as necessary.
void append_element(list *l, void *element);

// Removes just the first copy of the given element from the given list (uses
// address comparison). Returns the removed element, or NULL if the given
// element wasn't found.
void * remove_element(list *l, void *element);

// Removes all copies of the given element from the given list (uses address
// comparison). If the destroy version is used free() is called on each element
// as it is removed from the list. Both versions return the number of elements
// removed.
int remove_all_elements(list *l, void *element);
int destroy_all_elements(list *l, void *element);

// Reverses the given list. Doesn't allocate or free any memory.
void reverse(list *l);

// Runs the given function sequentially on each element in the list.
void foreach(list *l, void (*f)(void *));

// Scans the list until the given function returns non-zero, and returns the
// element that matched. Returns NULL if no match was found.
void * find_element(list *l, int (*match)(void *));

// Frees the memory associated with a list.
void cleanup_list(list *l);

// Frees the memory associated with a list, and also calls free on each element
// in the list.
void destroy_list(list *l);

#endif //ifndef LIST_H
