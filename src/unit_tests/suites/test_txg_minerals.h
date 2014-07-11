#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME txg_minerals
#define TEST_SUITE_TESTS { \
    &test_minerals_stone_filter, \
    NULL, \
  }

#ifndef TEST_TXG_MINERALS_H
#define TEST_TXG_MINERALS_H

#include "txgen/txg_minerals.h"
#include "tex/tex.h"

#include "unit_tests/test_suite.h"

/*******************
 * Example Structs *
 *******************/

stone_filter_args example_stone_args = {
  .seed = 27,
  .scale = 0.125,
  .noisy = 0.1,
  .bumpy = 1.0,
  .veins = 0.3,
  .dscale = 0.135,
  .distortion = 0.3,
  .squash = 0.9,
  .base_color = 0xff778877, // greenish-gray
  .alt_color = 0xff778877, // greenish-gray
};

tx_grammar_literal example_stone_literal = FILTER_TX_LITERAL(
  fltr_stone,
  example_stone_args,
  BLOCK_TEXTURE_SIZE,
  BLOCK_TEXTURE_SIZE
);

/******************
 * Test Functions *
 ******************/

size_t test_minerals_stone_filter(void) {
  run_grammar(&example_stone_literal);
  write_texture_to_png(
    example_stone_literal.result,
    "out/test/test-stone.png"
  );
  cleanup_grammar_results(&example_stone_literal);
  return 0;
}

#endif //ifndef TEST_TXG_MINERALS_H
