// list.c
// Simple array-based lists.

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>

#include "list.h"

/***********
 * Globals *
 ***********/

// We're more worried about mallocs than about saving space:
const size_t LIST_CHUNK_SIZE = 64;
const size_t LIST_KEEP_CHUNKS = 4;

/*************************
 * Structure Definitions *
 *************************/

struct list_s {
  size_t size; // Measured in chunks, not entries.
  size_t count; // Measured in entries.
  void **elements;
};

/*********************
 * Private Functions *
 *********************/

static inline void grow_if_necessary(list *l) {
  if (l->count == l->size*LIST_CHUNK_SIZE) { // We need more memory.
    void ** new_elements = (void **) realloc(
      l->elements,
      sizeof(void *) * ((l->size + 1)*LIST_CHUNK_SIZE)
    );
    if (new_elements == NULL) {
      perror("Failed to allocate additional list chunk.");
      exit(errno);
    }
    l->elements = new_elements;
    l->size += 1;
  }
}

static inline void shrink_if_necessary(list *l) {
  if (
    l->size > LIST_KEEP_CHUNKS
  &&
    l->count < (l->size - LIST_KEEP_CHUNKS)*LIST_CHUNK_SIZE
  ) {
    // We should free our extra elements.
    void ** new_elements = (void **) realloc(
      l->elements,
      sizeof(void *) * ((l->size - LIST_KEEP_CHUNKS)*LIST_CHUNK_SIZE)
    );
    if (new_elements == NULL) {
      perror("Failed to remove empty list chunks.");
      exit(errno);
    }
    l->elements = new_elements;
    l->size -= LIST_KEEP_CHUNKS;
  }
}

/*************
 * Functions *
 *************/

list *create_list(void) {
  list *l = (list *) malloc(sizeof(list));
  if (l == NULL) {
    perror("Failed to create list.");
    exit(errno);
  }
  l->elements = (void **) malloc(sizeof(void *)*LIST_CHUNK_SIZE);
  if (l->elements == NULL) {
    perror("Failed to create initial list chunk.");
    exit(errno);
  }
  l->size = 1;
  l->count = 0;
  return l;
}

void cleanup_list(list *l) {
  l->count = 0;
  l->size = 0;
  free(l->elements);
  free(l);
}

void destroy_list(list *l) {
  size_t i;
  for (i = 0; i < l->count; ++i) {
    free(l->elements[i]);
  }
  l->count = 0;
  l->size = 0;
  free(l->elements);
  free(l);
}


int l_is_empty(list *l) {
  return (l->count == 0);
}

int l_contains(list *l, void *element) {
  size_t i;
  int result = 0;
  for (i = 0; i < l->count; ++i) {
    if (l->elements[i] == element) {
      result = 1;
      break;
    }
  }
  return result;
}

size_t l_get_length(list *l) {
  return l->count;
}

void * l_get_element(list *l, size_t i) {
  if (i >= l->count) {
    return NULL;
  }
  return l->elements[i];
}

void l_append_element(list *l, void *element) {
  grow_if_necessary(l);
  l->elements[l->count] = element;
  l->count += 1;
}

void * l_pop_element(list *l) {
  void *result = NULL;
  if (l->count == 0) {
    return NULL;
  }
  result = l->elements[l->count - 1];
  l->count -= 1;
  shrink_if_necessary(l);
  return result;
}

void* l_remove_element(list *l, void *element) {
  size_t i, j;
  void *result = NULL;
  for (i = 0; i < l->count; ++i) {
    if (l->elements[i] == element) {
      result = l->elements[i];
      for (j = i; j < (l->count - 1); ++j) {
        l->elements[j] = l->elements[j+1];
      }
      l->count -= 1;
      shrink_if_necessary(l);
      break;
    }
  }
  return result;
}

int l_remove_all_elements(list *l, void *element) {
  size_t i;
  size_t removed = 0;
  size_t skip = 0;
  for (i = 0; i < l->count; ++i) {
    while (l->elements[i + skip] == element) {
      skip += 1;
      removed += 1;
      l->count -= 1;
    }
    if (skip > 0 && i < l->count) {
      l->elements[i] = l->elements[i + skip];
    }
  }
  shrink_if_necessary(l);
  return removed;
}

// Same code as remove_all but with an extra free()
int l_destroy_all_elements(list *l, void *element) {
  size_t i;
  size_t removed = 0;
  size_t skip = 0;
  for (i = 0; i < l->count; ++i) {
    while (l->elements[i + skip] == element) {
      free(element);
      skip += 1;
      removed += 1;
      l->count -= 1;
    }
    if (skip > 0 && i < l->count) {
      l->elements[i] = l->elements[i + skip];
    }
  }
  shrink_if_necessary(l);
  return removed;
}

void l_reverse(list *l) {
  size_t i;
  void *phased;
  for (i = 0; i < (l->count / 2); ++i) {
    phased = l->elements[l->count - i - 1];
    l->elements[l->count - i - 1] = l->elements[i];
    l->elements[i] = phased;
  }
}

void l_foreach(list *l, void (*f)(void *)) {
  size_t i;
  for (i = 0; i < l->count; ++i) {
    f(l->elements[i]);
  }
}

void * l_find_element(list *l, int (*match)(void *)) {
  size_t i;
  for (i = 0; i < l->count; ++i) {
    if (match(l->elements[i])) {
      return l->elements[i];
    }
  }
  return NULL;
}
