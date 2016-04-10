// bitmap.c
// Bitmaps for recording booleans.

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h> // DEBUG
#include <omp.h>

#include "bitmap.h"

/*********
 * Types *
 *********/

// A single row of a bitmap (only used internally)
typedef long unsigned int bitmap_row;

#define BITMAP_ROW_WIDTH sizeof(bitmap_row)

/*************************
 * Structure Definitions *
 *************************/

struct bitmap_s {
  size_t size; // how many entries are in this bitmap
  size_t rows; // how many rows there are
  bitmap_row *data; // the bits
  omp_lock_t lock; // lock for thread safety
};

/*********************
 * Private Functions *
 *********************/

// Returns the specified bit (either 1 or 0). Returns 0 for out-of-range
// indices.
static inline size_t bm_get_bit(bitmap *bm, size_t index) {
#ifdef DEBUG_CHECK_BITMAP_ACCESS
  if (index >= bm->size) {
    fprintf(stderr, "Error: out-of-range bitmap access in bm_get_bit.\n");
    exit(1);
  }
#endif
  return (bm->data[index / BITMAP_ROW_WIDTH] >> (index % BITMAP_ROW_WIDTH)) & 1;
}

// Sets the specified bit to 1.
static inline void bm_set_bit(bitmap *bm, size_t index) {
#ifdef DEBUG_CHECK_BITMAP_ACCESS
  if (index >= bm->size) {
    fprintf(stderr, "Error: out-of-range bitmap access in bm_set_bit.\n");
    exit(1);
  }
#endif
  bm->data[index / BITMAP_ROW_WIDTH] |= 1 << (index % BITMAP_ROW_WIDTH);
}

// Sets the specified bit to 0.
static inline void bm_clear_bit(bitmap *bm, size_t index) {
#ifdef DEBUG_CHECK_BITMAP_ACCESS
  if (index >= bm->size) {
    fprintf(stderr, "Error: out-of-range bitmap access in bm_clear_bit.\n");
    exit(1);
  }
#endif
  bm->data[index / BITMAP_ROW_WIDTH] &= ~(1 << (index % BITMAP_ROW_WIDTH));
}

/******************************
 * Constructors & Destructors *
 ******************************/

bitmap *create_bitmap(size_t bits) {
  bitmap *bm = (bitmap *) malloc(sizeof(bitmap));
  bm->size = bits;
  bm->rows = (bits / BITMAP_ROW_WIDTH) + (bits % BITMAP_ROW_WIDTH > 0);
  bm->data = (bitmap_row *) calloc(bm->rows, sizeof(bitmap_row));
  omp_init_lock(&(bm->lock));
  return bm;
}

CLEANUP_IMPL(bitmap) {
  omp_set_lock(&(doomed->lock));
  omp_destroy_lock(&(doomed->lock));
  free(doomed->data);
  free(doomed);
}

/***********
 * Locking *
 ***********/

void bm_lock(bitmap *bm) { omp_set_lock(&(bm->lock)); }
void bm_unlock(bitmap *bm) { omp_unset_lock(&(bm->lock)); }

/*************
 * Functions *
 *************/

size_t bm_size(bitmap *bm) {
  return bm->size;
}

size_t bm_check_bit(bitmap *bm, size_t index) {
  return bm_get_bit(bm, index);
}

void bm_set_bits(bitmap *bm, size_t index, size_t size) {
  size_t i;
  for (i = index; i < index + size && i < bm->size; ++i) {
    bm_set_bit(bm, i);
  }
}
void bm_clear_bits(bitmap *bm, size_t index, size_t size) {
  size_t i;
  for (i = index; i < index + size && i < bm->size; ++i) {
    bm_clear_bit(bm, i);
  }
}

ptrdiff_t bm_find_space(bitmap *bm, size_t required) {
  size_t i, j;
  int hit = 0;
  for (i = 0; i <= bm->size - required;) {
    hit = 1;
    for (j = i; j < i + required; ++j) {
      if (bm_get_bit(bm, j)) {
        i = j + 1;
        hit = 0;
        break;
      }
    }
    if (hit) {
      return i;
    }
  }
  return -1;
}

size_t bm_popcount(bitmap *bm) {
  size_t result = 0;
  size_t i;
  for (i = 0; i < bm->rows; ++i) {
    // TODO: Integer-size-dependent compilation?!?
    result += __builtin_popcountl(bm->data[i]);
    // No need to worry about trailing bits in the row beyond the size, because
    // they should always be zeroes.
  }
  return result;
}

ptrdiff_t bm_select_open(bitmap *bm, size_t n) {
  size_t i, j;
  size_t bits_here;
  ptrdiff_t result;
  bitmap_row row;
  i = 0;
  result = 0;
  for (i = 0; i < bm->rows; ++i) {
    bits_here = BITMAP_ROW_WIDTH - __builtin_popcountl(bm->data[i]);
    // Ignore zeroes on the last row past the exact size:
    if (i == bm->rows - 1) {
      bits_here -= BITMAP_ROW_WIDTH - (
        bm->size
      - (BITMAP_ROW_WIDTH * (bm->rows - 1))
      );
    }
    if (bits_here > n) {
      row = bm->data[i];
      for (j = 0; j < BITMAP_ROW_WIDTH; ++j) {
        if (!(row & 1)) {
          if (n == 0) {
            return result;
          } else {
            n -= 1;
          }
        }
        result += 1;
        row = row >> 1;
      }
    } else {
      n -= bits_here;
      result += BITMAP_ROW_WIDTH;
    }
  }
  // n is too large
  return -1;
}

ptrdiff_t bm_select_closed(bitmap *bm, size_t n) {
  size_t i, j;
  size_t bits_here;
  ptrdiff_t result;
  bitmap_row row;
  i = 0;
  result = 0;
  for (i = 0; i < bm->rows; ++i) {
    bits_here = __builtin_popcountl(bm->data[i]);
    // Fine even for the last row, because excess bits should be zeroes.
    row = bm->data[i];
    if (bits_here > n) {
      for (j = 0; j < BITMAP_ROW_WIDTH; ++j) {
        if (row & 1) {
          if (n == 0) {
            return result;
          } else {
            n -= 1;
          }
        }
        result += 1;
        row = row >> 1;
      }
    }
    n -= bits_here;
    result += BITMAP_ROW_WIDTH;
  }
  // n is too large
  return -1;
}
