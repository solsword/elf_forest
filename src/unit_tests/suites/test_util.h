#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME util
#define TEST_SUITE_TESTS { \
    &test_util_fxy__ptr, \
    NULL, \
  }

#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include "util.h"

#include "unit_tests/test_suite.h"

static float TOLERANCE = 0.0001;

size_t test_util_fxy__ptr(void) {
  void *ptr;
  float x, y;
  ptr = fxy__ptr(0, 0);
  x = ptr__fx(ptr);
  if (x != 0) { return 1; }
  y = ptr__fx(ptr);
  if (y != 0) { return 2; }
  ptr = fxy__ptr(0.75, 0.25);
  x = ptr__fx(ptr);
  if (x - 0.75 > TOLERANCE) { return 3; }
  y = ptr__fy(ptr);
  if (y - 0.25 > TOLERANCE) { return 4; }
  ptr = fxy__ptr(0.99, 0.01);
  x = ptr__fx(ptr);
  if (x - 0.99 > TOLERANCE) { return 5; }
  y = ptr__fy(ptr);
  if (y - 0.01 > TOLERANCE) { return 6; }
  ptr = fxy__ptr(0.58, 0.37);
  x = ptr__fx(ptr);
  if (x - 0.58 > TOLERANCE) { return 7; }
  y = ptr__fy(ptr);
  if (y - 0.37 > TOLERANCE) { return 8; }
  return 0;
}

#endif //ifndef TEST_UTIL_H
