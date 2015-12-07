#ifndef BITMAP_H
#define BITMAP_H

// bitmap.h
// Bitmaps for recording booleans.

#include <stddef.h>

/**************
 * Structures *
 **************/

// A bitmap holding boolean info for a bunch of things.
struct bitmap_s;
typedef struct bitmap_s bitmap;

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new bitmap. The new map starts with all zeroes.
bitmap *create_bitmap(size_t bits);

// Frees the data allocated for the given bitmap.
void cleanup_bitmap(bitmap *bm);

/***********
 * Locking *
 ***********/

void bm_lock(bitmap *bm);
void bm_unlock(bitmap *bm);

/*************
 * Functions *
 *************/

// Returns the bitmap's size.
size_t bm_size(bitmap *bm);

// Check a bit:
size_t bm_check_bit(bitmap *bm, size_t index);

// Set/clear size bits starting at index.
void bm_set_bits(bitmap *bm, size_t index, size_t size);
void bm_clear_bits(bitmap *bm, size_t index, size_t size);

// Finds an open block of the required size in the bitmap, and returns its
// index. Returns -1 if there is no open block of the requested size.
ptrdiff_t bm_find_space(bitmap *bm, size_t required);

// Returns the number of closed bits in the bitmap. To find open bits just
// subtract this number from the bitmap's size.
size_t bm_popcount(bitmap *bm);

// Return the index of the nth open/closed bit in the bitmap. These return -1
// if there is no nth open/closed bit to select.
ptrdiff_t bm_select_open(bitmap *bm, size_t n);
ptrdiff_t bm_select_closed(bitmap *bm, size_t n);

#endif //ifndef BITMAP_H
