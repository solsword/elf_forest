#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME list
#define TEST_SUITE_TESTS { \
    &test_list_setup_cleanup, \
    &test_list_growth, \
    &test_list_append_get_pop, \
    NULL, \
  }

#ifndef TEST_LIST_H
#define TEST_LIST_H

#include "datatypes/list.h"

#include "unit_tests/test_suite.h"

size_t test_list_setup_cleanup(void) {
  int i;
  list *l;
  for (i = 0; i < 100000; ++i) {
    l = create_list();
    cleanup_list(l);
  }
  return 0;
}

size_t test_list_growth(void) {
  int i = 0;
  list *l = create_list();
  for (i = 0; i < 100000; ++i) {
    l_append_element(l, NULL);
  }

  list *ex = create_list();
  for (i = 0; i < 10000; ++i) {
    l_append_element(ex, (void*) i);
  }

  for (i = 0; i < 3; ++i) {
    l_extend_element(l, ex);
  }

  if (l_get_item(l, 0) != NULL) { return 1; }
  if (l_get_item(l, 10000) != NULL) { return 2; }
  if (l_get_item(l, 99999) != NULL) { return 3; }
  if ((size_t) l_get_item(l, 100000) != 0) { return 4; }
  if ((size_t) l_get_item(l, 100001) != 1) { return 5; }
  if ((size_t) l_get_item(l, 100002) != 2) { return 6; }
  if ((size_t) l_get_item(l, 109999) != 9999) { return 7; }
  if ((size_t) l_get_item(l, 110000) != 0) { return 8; }
  if ((size_t) l_get_item(l, 110003) != 3) { return 9; }
  if ((size_t) l_get_item(l, 119999) != 9999) { return 10; }
  if ((size_t) l_get_item(l, 129999) != 9999) { return 11; }
  if (l_get_length(l) != 130000) { return 12; }

  cleanup_list(l);
  return 0;
}

size_t test_list_append_get_pop(void) {
  list *l = create_list();
  l_append_element(l, (void *) 17);
  if (l_get_item(l, 0) != (void *) 17) { return 1; }
  if (l_pop_element(l) != (void *) 17) { return 2; }
  if (l_pop_element(l) != NULL) { return 3; }
  if (l_pop_element(l) != NULL) { return 4; }
  l_append_element(l, (void *) 8);
  l_append_element(l, (void *) 9);
  l_append_element(l, (void *) 10);
  if (l_get_item(l, 0) != (void *) 8) { return 5; }
  if (l_get_item(l, 1) != (void *) 9) { return 6; }
  if (l_get_item(l, 2) != (void *) 10) { return 7; }
  if (l_get_item(l, l_get_length(l) - 1) != (void *) 10) { return 8; }
  if (l_pop_element(l) != (void *) 10) { return 9; }
  if (l_pop_element(l) != (void *) 9) { return 10; }
  if (l_pop_element(l) != (void *) 8) { return 11; }
  if (l_pop_element(l) != NULL) { return 12; }
  cleanup_list(l);
  return 0;
}

#endif //ifndef TEST_LIST_H
