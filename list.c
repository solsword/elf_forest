// list.c
// Simple singly-linked lists.

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include "list.h"

/**********************
 * Private Structures *
 **********************/

// A node in a list:
struct node_s;
typedef struct node_s node;

/*************************
 * Structure Definitions *
 *************************/

struct list_s {
  node *first;
  node *last;
};

struct node_s {
  void *contents;
  node *next;
};

/*************
 * Functions *
 *************/

list *create_list(void) {
  list *l = (list *) malloc(sizeof(list));
  if (l == NULL) {
    perror("Failed to create list.");
    exit(errno);
  }
  l->first = NULL;
  l->last = NULL;
  return l;
}

int is_empty(list *l) {
  return (l->first == NULL);
}

int contains(void *element, list *l) {
  node *current = l->first;
  int result = 0;
  while (current != NULL) {
    current = current->next;
    if (current->contents == element) {
      result = 1;
      break;
    }
  }
  return result;
}

void push_element(void *element, list *l) {
  node *new_node = (node *) malloc(sizeof(node));
  if (new_node == NULL) {
    perror("Failed to create node.");
    exit(errno);
  }
  new_node->contents = element;
  if (l->first == NULL) {
    assert(l->last == NULL); // Integrity check
    new_node->next = NULL;
    l->first = new_node;
    l->last = new_node;
  } else {
    new_node->next = l->first;
    l->first = new_node;
  }
}

void append_element(void *element, list *l) {
  node *new_node = (node *) malloc(sizeof(node));
  if (new_node == NULL) {
    perror("Failed to create node.");
    exit(errno);
  }
  new_node->contents = element;
  new_node->next = NULL;
  if (l->first == NULL) {
    assert(l->last == NULL); // Integrity check
    l->first = new_node;
    l->last = new_node;
  } else {
    l->last->next = new_node;
    l->last = new_node;
  }
}

void* remove_element(void *element, list *l) {
  node *current = l->first;
  node *last = NULL;
  void *result = NULL;
  while (current != NULL && current->contents != element) {
    last = current;
    current = current->next;
  }
  if (current != NULL) {
    result = current->contents;
    if (last == NULL) { // At the start of the list.
      assert(l->first == current); // Integrity check.
      if (current->next == NULL) { // Which is also the end of the list!
        assert(l->last == current); // Integrity check.
        l->first = NULL;
        l->last = NULL;
      } else { // Just at the start.
        l->first = current->next;
      }
    } else if (current->next == NULL) { // At the end of the list.
      assert(l->last == current); // Integrity check.
      last->next = NULL;
      l->last = last;
    } else { // Somewhere in the middle of the list.
      last->next = current->next;
    }
    free(current);
  }
  return result;
}

void remove_elements(void *element, list *l) {
  node *current = l->first;
  node *last = NULL;
  node *removed;
  while (current != NULL) {
    if (current->contents == element) {
      if (last == NULL) { // At the start of the list.
        assert(l->first == current); // Integrity check.
        if (current->next == NULL) { // Which is also the end of the list!
          assert(l->last == current); // Integrity check.
          // Empty the list:
          l->first = NULL;
          l->last = NULL;
          // Capture the node that was removed:
          removed = current;
          // We're done here:
          current = NULL;
          // Free the removed node:
          free(removed);
        } else { // Just at the start.
          // Splice out the current node:
          l->first = current->next;
          // Capture the node that was removed:
          removed = current;
          // Walk the list forward:
          last = NULL;
          current = current->next;
          // Free the removed node:
          free(removed);
        }
      } else if (current->next == NULL) { // At the end of the list.
        assert(l->last == current); // Integrity check.
        // Splice out the current node:
        last->next = NULL;
        l->last = last;
        // Capture the node that was removed:
        removed = current;
        // We're done with the list:
        current = NULL;
        // Free the removed node:
        free(removed);
      } else { // Somewhere in the middle of the list.
        // Splice out the current node:
        last->next = current->next;
        // Capture the node that was removed:
        removed = current;
        // Walk the list forward (last remains the same):
        current = current->next;
        // Free the removed element:
        free(removed);
      }
    } else {
      // Walk forward without removing the current node:
      last = current;
      current = current->next;
    }
  }
}

void destroy_elements(void *element, list *l) {
  node *current = l->first;
  node *last = NULL;
  node *removed;
  while (current != NULL) {
    if (current->contents == element) {
      if (last == NULL) { // At the start of the list.
        assert(l->first == current); // Integrity check.
        if (current->next == NULL) { // Which is also the end of the list!
          assert(l->last == current); // Integrity check.
          // Empty the list:
          l->first = NULL;
          l->last = NULL;
          // Capture the node that was removed:
          removed = current;
          // We're done here:
          current = NULL;
          // Free the removed node and its contents:
          free(removed->contents);
          free(removed);
        } else { // Just at the start.
          // Splice out the current node:
          l->first = current->next;
          // Capture the node that was removed:
          removed = current;
          // Walk the list forward:
          last = NULL;
          current = current->next;
          // Free the removed node:
          free(removed->contents);
          free(removed);
        }
      } else if (current->next == NULL) { // At the end of the list.
        assert(l->last == current); // Integrity check.
        // Splice out the current node:
        last->next = NULL;
        l->last = last;
        // Capture the node that was removed:
        removed = current;
        // We're done with the list:
        current = NULL;
        // Free the removed node:
        free(removed->contents);
        free(removed);
      } else { // Somewhere in the middle of the list.
        // Splice out the current node:
        last->next = current->next;
        // Capture the node that was removed:
        removed = current;
        // Walk the list forward (last remains the same):
        current = current->next;
        // Free the removed element:
        free(removed->contents);
        free(removed);
      }
    } else {
      // Walk forward without removing the current node:
      last = current;
      current = current->next;
    }
  }
}

void reverse(list *l) {
  node *current = l->first;
  node *last = NULL;
  node *next;
  // Rewire the last pointer to the first node:
  l->last = current;
  // Flop the next pointer at each node:
  while (current != NULL) {
    next = current->next;
    current->next = last;
    last = current;
    current = next;
  }
  // Rewire the first pointer:
  l->first = last;
}

void foreach(list *l, void (*f)(void *)) {
  node *current = l->first;
  while (current != NULL) {
    f(current->contents);
    current = current->next;
  }
}

void * find_element(list *l, int (*match)(void *)) {
  node *current = l->first;
  while (current != NULL) {
    if (match(current->contents)) {
      return current->contents;
    };
    current = current->next;
  }
  return NULL;
}

void cleanup_list(list *l) {
  node *current = l->first;
  node *next = l->first;
  l->first = NULL;
  l->last = NULL;
  while (current != NULL) {
    next = current->next;
    free(current);
    current = next;
  }
  free(l);
}

void destroy_list(list *l) {
  node *current = l->first;
  node *next = l->first;
  l->first = NULL;
  l->last = NULL;
  while (current != NULL) {
    next = current->next;
    free(current->contents);
    free(current);
    current = next;
  }
  free(l);
}
