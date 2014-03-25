#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME queue
#define TEST_SUITE_TESTS { \
    &test_queue_setup_cleanup, \
    &test_queue_growth, \
    &test_queue_push_pop, \
    &test_queue_push_pop_lots, \
    NULL, \
  }

#ifndef TEST_QUEUE_H
#define TEST_QUEUE_H

#include "datatypes/queue.h"

int test_queue_setup_cleanup(void) {
  int i;
  queue *q;
  for (i = 0; i < 100000; ++i) {
    q = create_queue();
    cleanup_queue(q);
  }
  return 1;
}

int test_queue_growth(void) {
  int i = 0;
  queue *q = create_queue();
  for (i = 0; i < 100000; ++i) {
    q_push_element(q, NULL);
  }
  cleanup_queue(q);
  return 1;
}

int test_queue_push_pop(void) {
  queue *q = create_queue();
  q_push_element(q, (void *) 17);
  if (q_get_element(q, 0) != (void *) 17) { return 0; }
  if (q_pop_element(q) != (void *) 17) { return 0; }
  if (q_pop_element(q) != NULL) { return 0; }
  if (q_pop_element(q) != NULL) { return 0; }
  q_push_element(q, (void *) 8);
  q_push_element(q, (void *) 9);
  q_push_element(q, (void *) 10);
  if (q_get_element(q, 0) != (void *) 8) { return 0; }
  if (q_get_element(q, 1) != (void *) 9) { return 0; }
  if (q_get_element(q, 2) != (void *) 10) { return 0; }
  if (q_get_element(q, q_get_length(q) - 1) != (void *) 10) { return 0; }
  if (q_pop_element(q) != (void *) 8) { return 0; }
  if (q_pop_element(q) != (void *) 9) { return 0; }
  if (q_pop_element(q) != (void *) 10) { return 0; }
  if (q_pop_element(q) != NULL) { return 0; }
  cleanup_queue(q);
  return 1;
}

int test_queue_push_pop_lots(void) {
  static int const batch_size = 10000;
  static int const small_batch_size = 9760;
  int i = 0;
  queue *q = create_queue();
  for (i = 0; i < batch_size; ++i) {
    q_push_element(q, (void *) 17);
  }
  if (q_get_element(q, 0) != (void *) 17) { return 0; }
  if (q_get_element(q, batch_size - 1) != (void *) 17) { return 0; }

  for (i = 0; i < small_batch_size; ++i) {
    if (q_pop_element(q) != (void *) 17) { return 0; }
  }
  for (i = 0; i < batch_size; ++i) {
    q_push_element(q, (void *) 35);
  }
  if (q_get_element(q, 0) != (void *) 17) { return 0; }
  if (q_get_element(q, q_get_length(q) - 1) != (void *) 35) { return 0; }
  cleanup_queue(q);
  return 1;
}

#endif //ifndef TEST_QUEUE_H
