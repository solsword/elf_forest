#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME dictionary
#define TEST_SUITE_TESTS { \
    &test_dictionary_prepare, \
    &test_dictionary_setup_cleanup, \
    &test_dictionary_growth, \
    &test_dictionary_add_pop, \
    &test_dictionary_string_keys, \
    &test_dictionary_collision, \
    NULL, \
  }

#ifndef TEST_DICTIONARY_H
#define TEST_DICTIONARY_H

#include <stdio.h>

#include "datatypes/dictionary.h"
#include "datatypes/string.h"
#include "datatypes/dict_string_keys.h"

size_t test_dictionary_prepare(void) {
  init_strings();
  return 0;
}

size_t test_dictionary_setup_cleanup(void) {
  dictionary *d = create_dictionary(1024);
  cleanup_dictionary(d);
  return 0;
}

size_t test_dictionary_growth(void) {
  size_t i = 0;
  char *key = "key";
  size_t keylen = 3;
  dictionary *d = create_dictionary(512);
  for (i = 0; i < 10000; ++i) {
    d_add_value(d, (d_key_t*) key, keylen, (void*) i);
  }
  cleanup_dictionary(d);
  return 0;
}

size_t test_dictionary_add_pop(void) {
  dictionary *d = create_dictionary(1024);
  d_add_value(d, "key", 3, (void *) 17);
  if (d_get_value(d, "key", 3) != (void *) 17) { return 1; }
  if (d_pop_value(d, "key", 3) != (void *) 17) { return 2; }
  if (d_pop_value(d, "key", 3) != NULL) { return 3; }
  if (d_pop_value(d, "key", 3) != NULL) { return 4; }
  if (d_pop_value(d, "never_put_in", 12) != NULL) { return 5; }
  d_add_value(d, "key1", 4, (void *) 8);
  d_add_value(d, "key2", 4, (void *) 9);
  d_add_value(d, "key3", 4, (void *) 10);
  d_add_value(d, "key3", 4, (void *) 11);
  if (d_get_value(d, "key1", 4) != (void *) 8) { return 6; }
  if (d_get_value(d, "key2", 4) != (void *) 9) { return 7; }
  if (d_get_value(d, "key3", 4) != (void *) 10) { return 8; }
  if (d_pop_value(d, "key1", 4) != (void *) 8) { return 9; }
  if (d_pop_value(d, "key2", 4) != (void *) 9) { return 10; }
  if (d_pop_value(d, "key3", 4) != (void *) 10) { return 11; }
  if (d_get_value(d, "key3", 4) != (void *) 11) { return 12; }
  if (d_pop_value(d, "key3", 4) != (void *) 11) { return 13; }
  if (d_pop_value(d, "key3", 4) != NULL) { return 14; }
  d_add_value(d, "samekey", 7, (void *) 25);
  d_add_value(d, "samekey", 7, (void *) 26);
  d_add_value(d, "samekey", 7, (void *) 27);
  d_add_value(d, "samekey", 7, (void *) 28);
  d_add_value(d, "samekey", 7, (void *) 29);
  list *multi = d_get_all(d, "samekey", 7);
  if (l_get_item(multi, 0) != (void*) 25) { return 15; }
  if (l_get_item(multi, 1) != (void*) 26) { return 16; }
  if (l_get_item(multi, 2) != (void*) 27) { return 17; }
  if (l_get_item(multi, 3) != (void*) 28) { return 18; }
  if (l_get_item(multi, 4) != (void*) 29) { return 19; }
  cleanup_list(multi);
  d_prepend_value(d, "samekey", 7, (void*) 3);
  d_prepend_value(d, "samekey", 7, (void*) 2);
  multi = d_get_all(d, "samekey", 7);
  if (l_get_item(multi, 0) != (void*) 2) { return 20; }
  if (l_get_item(multi, 1) != (void*) 3) { return 21; }
  if (l_get_item(multi, 2) != (void*) 25) { return 22; }
  if (l_get_item(multi, 3) != (void*) 26) { return 23; }
  if (l_get_item(multi, 4) != (void*) 27) { return 24; }
  if (l_get_item(multi, 5) != (void*) 28) { return 25; }
  if (l_get_item(multi, 6) != (void*) 29) { return 26; }
  cleanup_list(multi);
  if (d_remove_value(d, (void*) 26) != (void*) 26) { return 27; }
  if (d_get_value(d, "samekey", 7) != (void*) 2) { return 28; }
  if (d_pop_value(d, "samekey", 7) != (void*) 2) { return 29; }
  if (d_pop_value(d, "samekey", 7) != (void*) 3) { return 30; }
  if (d_pop_value(d, "samekey", 7) != (void*) 25) { return 31; }
  if (d_get_value(d, "samekey", 7) != (void*) 27) { return 32; }
  d_clear_values(d, "samekey", 7);
  if (d_get_value(d, "samekey", 7) != NULL) { return 33; }
  d_add_value(d, "test", 4, (void*) 88);
  d_add_value(d, "test", 4, (void*) 89);
  d_add_value(d, "test", 4, (void*) 90);
  d_add_value(d, "test", 4, (void*) 90);
  d_add_value(d, "other", 5, (void*) 140);
  d_add_value(d, "other", 5, (void*) 141);
  d_clear(d);
  if (d_get_value(d, "test", 4) != NULL) { return 34; }
  if (d_get_value(d, "other", 5) != NULL) { return 35; }
  cleanup_dictionary(d);
  return 0;
}

size_t test_dictionary_string_keys(void) {
  dictionary *d = create_dictionary(100);
  string *key = create_string_from_ntchars("test");
  string *test;
  char* tkey = NULL;
  size_t tlen = 0;
  d_add_value_s(d, key, (void*) 99);
  if (d_get_value_s(d, key) != (void*) 99) { return 1; }
  d_add_value_s(d, key, (void*) 100);
  d_add_value_s(d, key, (void*) 101);
  if (d_pop_value_s(d, key) != (void*) 99) { return 2; }
  if (d_pop_value_s(d, key) != (void*) 100) { return 3; }
  if (d_pop_value_s(d, key) != (void*) 101) { return 4; }
  d_add_value_s(d, key, (void*) 102);
  d_get_key(d, 0, &tkey, &tlen);
  if (tkey == NULL) { return 5; }
  if (tlen != 4) { return 6; }
  if (strncmp(tkey, "test", 4) != 0) { return 7; }
  test = create_string_from_chars(tkey, tlen);
  if (!s_equals(test, key)) { return 8; }
  cleanup_string(test);
  cleanup_string(key);
  cleanup_dictionary(d);
  return 0;
}


size_t test_dictionary_collision(void) {
  static size_t const batch_size = 1000;
  size_t i;
  dictionary *d = create_dictionary(128);
  for (i = 0; i < batch_size; ++i) {
    d_add_value(d, (d_key_t*) &i, sizeof(size_t), (void*) i);
  }
  for (i = 0; i < batch_size; ++i) {
    if (d_get_value(d, (d_key_t*) &i, sizeof(size_t)) != (void*) i) {
      return i+1;
    }
  }
  cleanup_dictionary(d);
  return 0;
}

#endif //ifndef TEST_DICTIONARY_H
