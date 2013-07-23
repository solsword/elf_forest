#ifndef LIST_H
#define LIST_H

// list.h
// Simple singly-linked lists.

#include <stdint.h>

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
uint8_t is_empty(list *l);

// Test whether the given list contains the given element (uses address
// comparison).
uint8_t contains(void *element, list *l);

// Allocates an entry for and adds the given element to the front (push) or
// back (append) of the given list.
void push_element(void *element, list *l);
void append_element(void *element, list *l);

// Removes just the first copy of the given element from the given list (uses
// address comparison). Returns the removed element, or NULL if the given
// element wasn't found.
void * remove_element(void *element, list *l);

// Removes all copies of the given element from the given list (uses address
// comparison). If the destroy version is used free() is called on each element
// as it is removed from the list.
void remove_elements(void *element, list *l);
void destroy_elements(void *element, list *l);

// Reverses the given list. Doesn't allocate or free any memory.
void reverse(list *l);

// Runs the given function sequentially on each element in the list.
void foreach(list *l, void (*f)(void *));

// Frees the memory associated with a list.
void cleanup_list(list *l);

// Frees the memory associated with a list, and also calls free on each element
// in the list.
void destroy_list(list *l);

#endif //ifndef LIST_H
