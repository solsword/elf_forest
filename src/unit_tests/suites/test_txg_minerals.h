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

mineral_filter_args example_stone_args = {
  .seed = 31,
  .scale = 0.095,

  .gritty = 0.14,
  .contoured = 0.4,
  .porous = 0.8,
  .bumpy = 0.3,

  .inclusions = 0.4,

  .dscale = 0.165,
  .distortion = 3.3,
  .squash = 0.9,
  .base_color = 0xff999999, // gray
  .alt_color = 0xff888078, // blue-gray
  .brightness = 0.2,
};

tx_grammar_literal example_stone_literal = FILTER_TX_LITERAL(
  fltr_mineral,
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
