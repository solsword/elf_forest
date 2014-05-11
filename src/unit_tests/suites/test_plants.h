#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME plants
#define TEST_SUITE_TESTS { \
    &test_plants_branch_filter, \
    &test_plants_leaves_filter, \
    &test_plants_bulb_leaves, \
    NULL, \
  }

#ifndef TEST_PLANTS_H
#define TEST_PLANTS_H

#include "gen/plants.h"
#include "tex/tex.h"

#include "unit_tests/test_suite.h"

/*******************
 * Example Structs *
 *******************/

branch_filter_args example_branch_args = {
  .seed = 17,
  .rough = 0,
  .scale = 0.125,
  .width = 1.0,
  .dscale = 0.125,
  .distortion = 5.0,
  .squash = 1.2,
  .center_color = 0xff001133, // dark brown
  .mid_color = 0xff004466, // mid brown
  .outer_color = 0xff007799 // light brown
};

leaf_filter_args example_leaf_args = {
  .seed = 25,
  .type = LT_SIMPLE,
  .size = LS_SMALL,
  .main_color = 0xff00bb22, // medium green
  .vein_color = 0xff55dd77, // light green
  .dark_color = 0xff007711 // dark green
};

leaves_filter_args example_leaves_args = {
  .seed = 37,
  .x_spacing = 6,
  .y_spacing = 6,
  .leaf_args = {
    .seed = 59,
    .type = LT_SIMPLE,
    .size = LS_SMALL,
    .main_color = 0xff00bb22,
    .vein_color = 0xff55dd77,
    .dark_color = 0xff007711
  },
};

bulb_leaves_filter_args example_bulb_leaves_args = {
  .seed = 41,
  .count = 5,
  .spread = 0.3,
  .angle = M_PI / 4.0,
  .bend = -M_PI / 8.0,
  .shape = 0.6,
  .length = 27,
  .width = 3,
  .main_color = 0xff00bb22, // medium green
  .vein_color = 0xff11cc33, // slightly lighter green
  .dark_color = 0xff007711 // dark green
};

tx_grammar_literal example_branches_literal = FILTER_TX_LITERAL(
  fltr_branches,
  example_branch_args,
  BLOCK_TEXTURE_SIZE,
  BLOCK_TEXTURE_SIZE
);

tx_grammar_literal example_leaves_literal = FILTER_TX_LITERAL(
  fltr_leaves,
  example_leaves_args,
  BLOCK_TEXTURE_SIZE,
  BLOCK_TEXTURE_SIZE
);

/******************
 * Test Functions *
 ******************/

size_t test_plants_branch_filter(void) {
  run_grammar(&example_branches_literal);
  write_texture_to_png(
    example_branches_literal.result,
    "out/test/test-branches.png"
  );
  cleanup_grammar_results(&example_branches_literal);
  return 0;
}

size_t test_plants_leaves_filter(void) {
  run_grammar(&example_leaves_literal);
  write_texture_to_png(
    example_leaves_literal.result,
    "out/test/test-leaves.png"
  );
  cleanup_grammar_results(&example_leaves_literal);
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
