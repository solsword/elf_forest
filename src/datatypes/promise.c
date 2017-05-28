// promise.h
// A C implementation of promises, using OMP multithreading.

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <omp.h>

#include "list.h"

#include "promise.h"

/**************
 * Structures *
 **************/

struct promsie_s {
  list *parents; // promises that this one is waiting for
  list *children; // promises that are waiting for this one
  omp_lock_t lock; // lock for thread safety
};

/*********************
 * Private Functions *
 *********************/

// TODO: Any of these?

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and sets up a new unfulfilled promise:
promise *create_promise();

// Frees the memory associated with a promise.
CLEANUP_DECL(promise);

/***********
 * Locking *
 ***********/

void pr_lock(promise *pr);
void pr_unlock(promise *pr);

/*************
 * Functions *
 *************/

// Frees the memory associated with a promise, and also calls free on each
// other promise that was waiting for this one. Warning: if something in the
// destroyed promise chain is unfulfilled, work waiting for that will never
// happen, and future attempts to act on destroyed promises can potentially
// corrupt memory.
void destroy_promise(promise *pr);

// These functions test the state of a promise: it's always either unresolved,
// fulfilled, or broken.
int pr_is_unresolved(promise const * const pr);
int pr_is_fulfilled(promise const * const pr);
int pr_is_broken(promise const * const pr);

// Returns the number of parents that this promise is waiting for.
size_t pr_get_parent_count(promise const * const pr);

// Returns the number of children waiting for the given promise.
size_t pr_get_child_count(promise const * const pr);

// Functions for adding/removing parents and children:
void pr_add_parent(promise *pr);
void pr_add_child(promise *pr);
void pr_remove_parent(promise *pr, promise *parent);
void pr_remove_child(promise *pr, promise *child);

// Run the given function sequentially on each parent/child of the promise.
void pr_foreach_parent(promise const * const pr, void (*f)(void *));
void pr_foreach_child(promise const * const pr, void (*f)(void *));

// Runs the given function sequentially on each parent/child of the promise
// with the given extra argument as its second argument.
void pr_witheach_parent(
  promise const * const pr,
  void *arg,
  void (*f)(void *, void *)
);
void pr_witheach_child(
  promise const * const pr,
  void *arg,
  void (*f)(void *, void *)
);

// Primitives for manipulating promises:
void pr_fulfill(promise *pr, void * value);
void pr_reject(promise *pr, void * message);

// Creates a new promise as a child of this one that
void pr_then(promise *pr, void * (*f)(void **));
void pr_else(promise *pr, void * (*f)(void **));

// Counts the number of bytes of data/overhead used by the given promise.
size_t pr_data_size(promise const * const pr);
size_t pr_overhead_size(promise const * const pr);
