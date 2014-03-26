#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME map
#define TEST_SUITE_TESTS { \
    &test_map_setup_cleanup, \
    &test_map_growth, \
    &test_map_put_pop, \
    &test_map_collision, \
    NULL, \
  }

#ifndef TEST_MAP_H
#define TEST_MAP_H

#include "datatypes/map.h"

int test_map_setup_cleanup(void) {
  int i;
  map *m;
  for (i = 0; i < 1000; ++i) {
    m = create_map(i + 1);
    cleanup_map(m);
  }
  return 1;
}

int test_map_growth(void) {
  size_t i = 0;
  map *m = create_map(1);
  for (i = 0; i < 1000; ++i) {
    if (m_put_value(m, NULL, (map_key_t) i) != NULL) {
      return 0;
    }
  }
  cleanup_map(m);
  return 1;
}

int test_map_put_pop(void) {
  map *m = create_map(1);
  if (m_put_value(m, (void *) 17, (map_key_t) 3) != NULL) { return 0; }
  if (m_get_value(m, (map_key_t) 3) != (void *) 17) { return 0; }
  if (m_pop_value(m, (map_key_t) 3) != (void *) 17) { return 0; }
  if (m_pop_value(m, (map_key_t) 3) != NULL) { return 0; }
  if (m_pop_value(m, (map_key_t) 3) != NULL) { return 0; }
  if (m_pop_value(m, (map_key_t) 7) != NULL) { return 0; }
  if (m_put_value(m, (void *) 8, (map_key_t) 1) != NULL) { return 0; }
  if (m_put_value(m, (void *) 9, (map_key_t) 3) != (void *) 17) { return 0; }
  if (m_put_value(m, (void *) 10, (map_key_t) 5) != NULL) { return 0; }
  if (m_put_value(m, (void *) 11, (map_key_t) 5) != (void *) 10) { return 0; }
  if (m_get_value(m, (map_key_t) 1) != (void *) 8) { return 0; }
  if (m_get_value(m, (map_key_t) 3) != (void *) 9) { return 0; }
  if (m_get_value(m, (map_key_t) 5) != (void *) 11) { return 0; }
  if (m_pop_value(m, (map_key_t) 3) != (void *) 9) { return 0; }
  if (m_pop_value(m, (map_key_t) 1) != (void *) 8) { return 0; }
  if (m_pop_value(m, (map_key_t) 5) != (void *) 11) { return 0; }
  if (m_pop_value(m, (map_key_t) 1) != NULL) { return 0; }
  cleanup_map(m);
  return 1;
}

int test_map_collision(void) {
  static size_t const batch_size = 100;
  static size_t const small_batch_size = 97;
  size_t x = 0, y = 0, z = 0;
  map *m = create_map(3);
  for (x = 0; x < batch_size; ++x) {
    for (y = 0; y < batch_size; ++y) {
      for (z = 0; z < batch_size; ++z) {
        m_put_value(
          m,
          (void *) 17,
          (map_key_t) x,
          (map_key_t) y,
          (map_key_t) z
        );
      }
    }
  }
  if (
    m_get_value(m, (map_key_t) 0, (map_key_t) 0, (map_key_t) 0) != (void *) 17
  ) { return 0; }
  if (
    m_get_value(
      m,
      (map_key_t) 7,
      (map_key_t) 96,
      (map_key_t) 12
    ) != (void *) 17
  ) { return 0; }
  if (
    m_get_value(
      m,
      (map_key_t) batch_size - 1,
      (map_key_t) batch_size - 1,
      (map_key_t) batch_size - 1
    ) != (void *) 17
  ) { return 0; }

  for (x = 0; x < small_batch_size; ++x) {
    for (y = 0; y < small_batch_size; ++y) {
      for (z = 0; z < small_batch_size; ++z) {
        if (
          m_pop_value(
            m,
            (map_key_t) x,
            (map_key_t) y,
            (map_key_t) z
          ) != (void *) 17
        ) { return 0; }
      }
    }
  }
  for (x = 0; x < batch_size; ++x) {
    m_put_value(
      m,
      (void *) 35,
      (map_key_t) (x % 7),
      (map_key_t) ((x+2) % 7),
      (map_key_t) ((x+4) % 7)
    );
  }
  if (
    m_get_value(
      m,
      (map_key_t) (small_batch_size + 1),
      (map_key_t) 0,
      (map_key_t) 0
    ) != (void *) 17
  ) { return 0; }
  if (
    m_get_value(
      m,
      (map_key_t) 0,
      (map_key_t) 2,
      (map_key_t) 4
    ) != (void *) 35
  ) { return 0; }
  if (
    m_get_value(
      m,
      (map_key_t) 0,
      (map_key_t) 0,
      (map_key_t) 0
    ) != NULL
  ) { return 0; }
  cleanup_map(m);
  return 1;
}

#endif //ifndef TEST_MAP_H
