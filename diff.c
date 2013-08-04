// diff.c
// Changes to the world.

#include <assert.h>

#include "list.h"
#include "diff.h"

/*************
 * Functions *
 *************/

void setup_diff(diff *d) {
  d->runs = (rl *) malloc(sizeof(rl));
  d->runs->block = B_VOID;
  d->runs->length = DIFF_LENGTH;
  d->runs->next = NULL;
}

void d_put_block(diff *d, diff_pos *dpos, block b) {
  uint32_t index = 0;
  uint32_t target = dindex(dpos);
  rl *prev = NULL;
  rl *run = d->runs;
  do {
    if (index + run->length > target) {
      if (block_is(run->block, b)) {
        return;
      }
      // We need to split this run: start by decrementing its length:
      run->length -= 1;
      if (run->length == 0) { // the run was just a single block
        run->block = b;
        run->length = 1;
      } else if (target == index) { // at the start of the run
        if (prev == NULL) {
          d->runs = (rl *) malloc(sizeof(rl));
          d->runs->block = b;
          d->runs->length = 1;
          d->runs->next = run;
        } else if (prev->block == b) {
          prev->length += 1;
        } else {
          prev->next = (rl *) malloc(sizeof(rl));
          prev->next->block = b;
          prev->next->length = 1;
          prev->next->next = run;
        }
      } else if (target < index + run->length - 1) { // in the middle of the run
        // construct our first new run:
        prev = (rl *) malloc(sizeof(rl));
        prev->block = b;
        prev->length = 1;
        // and our second new run:
        prev->next = (rl *) malloc(sizeof(rl));
        prev->next->block = run->block;
          // note we've already subtracted 1 from run->length
        prev->next->length = run->length - target;
        // calculate the new length of the original run:
        run->length -= prev->length;
        // and wire everything back together:
        prev->next->next = run->next;
        run->next = prev;
      } else { // at the end of the run
        if (run->next == NULL) {
          run->next = (rl *) malloc(sizeof(rl));
          run->next->block = b;
          run->next->length = 1;
          run->next->next = NULL;
        } else if (run->next->block == b) {
          run->next->length += 1;
        } else {
          prev = run->next;
          run->next = (rl *) malloc(sizeof(rl));
          run->next->block = b;
          run->next->length = 1;
          run->next->next = prev;
        }
      }
      // we're done here
      return;
    }
    prev = run;
    run = run->next;
  } while (run != NULL);
  assert(0);
}
