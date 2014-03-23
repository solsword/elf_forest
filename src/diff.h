#ifndef DIFF_H
#define DIFF_H

// diff.h
// Changes to the world.

#include <stdlib.h>

#include "datatypes/list.h"
#include "world/world.h"

/**************
 * Structures *
 **************/

// A block position within a diff:
struct diff_pos_s;
typedef struct diff_pos_s diff_pos;

// A single entry in a run-length-encoded list:
struct rl_s;
typedef struct rl_s rl;

// A diff is stored as a run-length-encoded list of blocks, where B_VOID
// represents no difference.
struct diff_s;
typedef struct diff_s diff;

/*************
 * Constants *
 *************/

#define DIFF_SHIFT 10
#define DIFF_SIZE (1 << DIFF_SHIFT)
#define DIFF_LENGTH (DIFF_SIZE * DIFF_SIZE * DIFF_SIZE)

/***********
 * Globals *
 ***********/

/*************************
 * Structure Definitions *
 *************************/

struct diff_pos_s {
  uint32_t x, y, z;
};

struct rl_s {
  block block;
  uint32_t length;
  rl *next;
};

struct diff_s {
  region_pos offset;
  rl *runs;
};

/********************
 * Inline Functions *
 ********************/

static inline uint32_t dindex(const diff_pos *dpos) {
  return dpos->z + dpos->y * DIFF_SIZE + dpos->x * DIFF_SIZE * DIFF_SIZE;
}

static inline void dpos__rpos(
  const diff_pos *dpos,
  const diff *d,
  region_pos *rpos
) {
  rpos->x = dpos->x + d->offset.x;
  rpos->y = dpos->y + d->offset.y;
  rpos->z = dpos->z + d->offset.z;
}

static inline void rpos__dpos(
  const region_pos *rpos,
  const diff *d,
  diff_pos *dpos
) {
  dpos->x = rpos->x - d->offset.x;
  dpos->y = rpos->y - d->offset.y;
  dpos->z = rpos->z - d->offset.z;
}

static inline void dpos__fpos(
  const diff_pos *dpos,
  const diff *d,
  const frame *f,
  frame_pos *fpos
) {
  region_pos roff;
  rcpos__rpos(&(f->region_offset), &roff);
  fpos->x = dpos->x + d->offset.x - roff.x;
  fpos->y = dpos->y + d->offset.y - roff.y;
  fpos->z = dpos->z + d->offset.z - roff.z;
}

static inline void fpos__dpos(
  const frame_pos *fpos,
  const frame *f,
  const diff *d,
  diff_pos *dpos
) {
  region_pos roff;
  rcpos__rpos(&(f->region_offset), &roff);
  dpos->x = fpos->x + roff.x - d->offset.x;
  dpos->y = fpos->y + roff.y - d->offset.y;
  dpos->z = fpos->z + roff.z - d->offset.z;
}

static inline block d_get_block(const diff *d, diff_pos *dpos) {
  uint32_t index = 0;
  uint32_t target = dindex(dpos);
  rl *run = d->runs;
  do {
    index += run->length;
    if (index > target) {
      return run->block;
    }
    run = run->next;
  } while (run != NULL);
  return B_VOID;
}

/*************
 * Functions *
 *************/

void setup_diff(diff *d);

void d_put_block(diff *d, diff_pos *dpos, block b);

#endif // ifndef DIFF_H
