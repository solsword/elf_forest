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
  .seed = 31,
  .scale = 0.095,

  .gritty = 0.12,
  .contoured = 0.8,
  .porous = 0.3,
  .bumpy = 0.5,

  .inclusions = 0.2,

  .dscale = 0.065,
  .distortion = 1.3,
  .squash = 0.9,
  .base_color = 0xffccaa88, // blue-gray
  .alt_color = 0xff778899, // orange-gray
  .brightness = 0.0,
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
