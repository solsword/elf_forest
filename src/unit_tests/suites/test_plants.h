#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME plants
#define TEST_SUITE_TESTS { \
    &test_plants_branch_filter, \
    &test_plants_bulb_leaves, \
    NULL, \
  }

#ifndef TEST_PLANTS_H
#define TEST_PLANTS_H

#include "gen/plants.h"
#include "graphics/tex.h"

#include "unit_tests/test_suite.h"

size_t test_plants_branch_filter(void) {
  run_grammar(&example_branches_literal);
  write_texture_to_png(example_branches_literal.result, "out/test/brtest.png");
  cleanup_grammar_results(&example_branches_literal);
  return 0;
}

size_t test_plants_bulb_leaves(void) {
  texture *tx = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_bulb_leaves(tx, &example_bulb_leaves_args);
  write_texture_to_png(tx, "out/test/test-bulb_leaves.png");
  // TODO: doublecheck
  /*
  texture *doublecheck = load_texture_from_png("out/test/tmoss.png");
  if (tx_get_px(doublecheck, 0, 0) != 0xff00b40f) { return 1; }
  if (tx_get_px(doublecheck, 1, 0) != 0xbf10870d) { return 2; }
  cleanup_texture(doublecheck);
  // */
  return 0;
}

#endif //ifndef TEST_PLANTS_H
