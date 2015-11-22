#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME bitmap
#define TEST_SUITE_TESTS { \
    &test_bitmap_setup_cleanup, \
    &test_bitmap_fill_empty, \
    &test_bitmap_selection, \
    NULL, \
  }

#ifndef TEST_BITMAP_H
#define TEST_BITMAP_H

#include <stdio.h>

#include "util.h"

#include "datatypes/bitmap.h"

// TODO: Test bm_find_space

size_t test_bitmap_setup_cleanup(void) {
  int i;
  bitmap *bm;
  for (i = 0; i < 1000; ++i) {
    bm = create_bitmap(i + 1);
    cleanup_bitmap(bm);
  }
  return 0;
}

size_t test_bitmap_fill_empty(void) {
  size_t i, size, tmp;
  bitmap *bm;
  // Small size:
  size = 17;
  bm = create_bitmap(size);
  for (i = 0; i < size; ++i) {
    bm_set_bits(bm, i, 1);
    if (bm_popcount(bm) != i + 1) {
      return 100 + i;
    }
  }
  for (i = 0; i < size; ++i) {
    bm_clear_bits(bm, i, 1);
    if (bm_popcount(bm) != size - i - 1) {
      return 200 + i;
    }
  }
  if (bm_popcount(bm) != 0) { return 250; }
  for (i = 0; i < size / 5; ++i) {
    bm_set_bits(bm, i*5, 4);
    if (bm_popcount(bm) != (i+1)*4) {
      return 300 + i;
    }
  }
  tmp = i*4;
  if (bm_popcount(bm) != tmp) { return 350; }
  for (i = 0; i < size / 5; ++i) {
    bm_clear_bits(bm, i*5, 5);
    if (bm_popcount(bm) != tmp - (i+1)*4) {
      return 400 + i;
    }
  }
  if (bm_popcount(bm) != 0) { return 450; }
  cleanup_bitmap(bm);
  // Exact size:
  size = 64;
  bm = create_bitmap(size);
  bm_set_bits(bm, 0, size);
  if (bm_popcount(bm) != size) { return 500; }
  bm_clear_bits(bm, 0, size);
  if (bm_popcount(bm) != 0) { return 550; }
  for (i = 0; i < size; ++i) {
    bm_set_bits(bm, i, 1);
    if (bm_popcount(bm) != i + 1) {
      return 600 + i;
    }
  }
  for (i = 0; i < size; ++i) {
    bm_clear_bits(bm, i, 1);
    if (bm_popcount(bm) != size - i - 1) {
      return 700 + i;
    }
  }
  if (bm_popcount(bm) != 0) { return 750; }

  // Medium size:
  size = 137;
  bm = create_bitmap(size);
  bm_set_bits(bm, 0, size);
  if (bm_popcount(bm) != size) { return 900; }
  bm_clear_bits(bm, 0, size);
  if (bm_popcount(bm) != 0) { return 950; }
  for (i = 0; i < size; ++i) {
    bm_set_bits(bm, i, 1);
    if (bm_popcount(bm) != i + 1) {
      return 1000 + i;
    }
  }
  for (i = 0; i < size; ++i) {
    bm_clear_bits(bm, i, 1);
    if (bm_popcount(bm) != size - i - 1) {
      return 2000 + i;
    }
  }
  if (bm_popcount(bm) != 0) { return 2500; }
  cleanup_bitmap(bm);
  // Large size:
  size = 102424;
  bm = create_bitmap(size);
  bm_set_bits(bm, 0, size);
  if (bm_popcount(bm) != size) { return 10000; }
  bm_clear_bits(bm, 0, size);
  if (bm_popcount(bm) != 0) { return 15000; }
  for (i = 0; i < size; ++i) {
    bm_set_bits(bm, i, 1);
    if (bm_popcount(bm) != i + 1) {
      return 1000000 + i;
    }
  }
  for (i = 0; i < size; ++i) {
    bm_clear_bits(bm, i, 1);
    if (bm_popcount(bm) != size - i - 1) {
      return 2000000 + i;
    }
  }
  if (bm_popcount(bm) != 0) { return 2500; }

  cleanup_bitmap(bm);
  return 0;
}

size_t test_bitmap_selection(void) {
  size_t i, count, choice, index;
  bitmap *bm = create_bitmap(1024);
  count = 812;
  for (i = 0; i < count; ++i) {
    bm_set_bits(bm, bm_select_open(bm, count - i - 1), 1);
    if (bm_popcount(bm) != (i + 1)) {
      return 1000 + i;
    }
  }
  if (bm_popcount(bm) != count) { return 1; }

  for (i = 0; i < 450; ++i) {
    bm_clear_bits(bm, bm_select_closed(bm, 0), 1);
    if (bm_popcount(bm) != count - (i + 1)) {
      return 2000 + i;
    }
  }
  if (bm_popcount(bm) != count - 450) { return 2; }

  choice = 18;
  for (i = 0; i < 200; ++i) {
    bm_clear_bits(bm, bm_select_closed(bm, choice), 1);
    choice = posmod(prng(choice), bm_popcount(bm));
    if (bm_popcount(bm) != count - 450 - (i + 1)) {
      return 3000 + i;
    }
  }
  if (bm_popcount(bm) != count - 450 - 200) { return 3; }

  choice = 23;
  for (i = 0; i < 100; ++i) {
    bm_set_bits(bm, bm_select_open(bm, choice), 1);
    choice = posmod(prng(choice), bm_size(bm) - bm_popcount(bm));
    if (bm_popcount(bm) != count - 450 - 200 + (i + 1)) {
      return 4000 + i;
    }
  }
  if (bm_popcount(bm) != count - 450 - 200 + 100) { return 4; }

  choice = 23;
  index = bm_select_open(bm, choice);
  while (index != -1) {
    bm_set_bits(bm, index, 1);
    if (bm_size(bm) - bm_popcount(bm) == 0) { break; }
    choice = posmod(prng(choice), bm_size(bm) - bm_popcount(bm));
    index = bm_select_open(bm, choice);
  }
  if (bm_popcount(bm) != bm_size(bm)) { return 5; }

  choice = count - 1;
  index = bm_select_closed(bm, choice);
  while (index != -1) {
    bm_clear_bits(bm, index, 1);
    if (bm_popcount(bm) == 0) { break; }
    choice = posmod(prng(choice), bm_popcount(bm));
    index = bm_select_closed(bm, choice);
  }
  if (bm_popcount(bm) != 0) { return 6; }

  cleanup_bitmap(bm);
  return 0;
}


#endif //ifndef TEST_BITMAP_H
