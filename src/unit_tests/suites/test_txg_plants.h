#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME txg_plants
#define TEST_SUITE_TESTS { \
    &test_plants_branch_filter, \
    &test_plants_leaves_filter, \
    &test_plants_branches_and_leaves, \
    &test_plants_herb_leaves, \
    &test_plants_herb_stems, \
    NULL, \
  }

#ifndef TEST_TXG_PLANTS_H
#define TEST_TXG_PLANTS_H

#include "txgen/txg_plants.h"
#include "tex/tex.h"

#include "unit_tests/test_suite.h"

/*******************
 * Example Structs *
 *******************/

branch_filter_args example_branch_args = {
  .seed = 17,
  .gnarled = 0,
  .direction = 4,
  //.scale = 0.125,
  .scale = 0.095,
  .width = 0.4,
  .dscale = 0.12,
  .distortion = 3.0,
  .squash = 1.0,
  .center_color = 0xff113366, // dark brown
  .mid_color = 0xff114477, // mid brown
  .outer_color = 0xff115588 // light brown
};

leaf_filter_args example_leaf_args = {
  .seed = 25,
  .type = LT_SIMPLE,
  .shape = LS_DELTOID,
  .angle = M_PI / 4.0,
  .angle_var = M_PI / 8.0,
  .bend = M_PI / 6.0,
  .width = 5,
  .length = 9,
  .stem_length = 0.2,
  .main_color = 0xff33dd66, // medium green
  .vein_color = 0xff55ee77, // light green
  .dark_color = 0xff007711 // dark green
};

leaves_filter_args example_leaves_args = {
  .seed = 37,
  //*
  .x_spacing = 6,
  .y_spacing = 6,
  /*/
  .x_spacing = 12,
  .y_spacing = 16,
  // */
  .leaf_args = {
    .seed = 38,
    .type = LT_SIMPLE,
    .shape = LS_OVAL,
    .angle = 0,
    .angle_var = M_PI / 4.0,
    .bend = M_PI / 6.0,
    .width = 6,
    .length = 7,
    .stem_length = 0.1,
    .main_color = 0xff33dd33, // medium green
    .vein_color = 0xff44ff44, // light green
    .dark_color = 0xff22bb22 // dark green
  },
};

herb_leaves_filter_args example_herb_leaves_args = {
  .seed = 42,
  .count = 4,
  .spread = 0.2,
  .angle = M_PI / 4.0,
  .bend = -M_PI / 8.0,
  .shape = 0.6,
  .length = 27,
  .width = 4,
  .main_color = 0xff00bb22, // medium green
  .vein_color = 0xff11cc33, // slightly lighter green
  .dark_color = 0xff007711 // dark green
};

herb_leaves_filter_args example_herb_stems_args = {
  .seed = 47,
  .count = 12,
  .spread = 0.9,
  .angle = M_PI / 12.0,
  .bend = -M_PI / 4.0,
  .shape = 0.5,
  .length = 29,
  .width = 2,
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

size_t test_plants_branches_and_leaves(void) {
  run_grammar(&example_branches_literal);
  run_grammar(&example_leaves_literal);
  // Put the leaves on the branches:
  tx_draw(
    example_branches_literal.result,
    example_leaves_literal.result,
    0, 0
  );
  write_texture_to_png(
    example_branches_literal.result,
    "out/test/test-leaves-branches.png"
  );
  cleanup_grammar_results(&example_branches_literal);
  cleanup_grammar_results(&example_leaves_literal);
  return 0;
}

size_t test_plants_herb_leaves(void) {
  texture *tx = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_herb_leaves(tx, &example_herb_leaves_args);
  write_texture_to_png(tx, "out/test/test-herb_leaves.png");
  // TODO: doublecheck
  /*
  texture *doublecheck = load_texture_from_png("out/test/tmoss.png");
  if (tx_get_px(doublecheck, 0, 0) != 0xff00b40f) { return 1; }
  if (tx_get_px(doublecheck, 1, 0) != 0xbf10870d) { return 2; }
  cleanup_texture(doublecheck);
  // */
  return 0;
}

size_t test_plants_herb_stems(void) {
  texture *leaves_tx = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  texture *stems_tx = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  texture *combined = create_texture(BLOCK_TEXTURE_SIZE*3, BLOCK_TEXTURE_SIZE*3);
  fltr_herb_leaves(leaves_tx, &example_herb_stems_args);
  fltr_herb_stems(stems_tx, &example_herb_stems_args);
  write_texture_to_png(stems_tx, "out/test/test-herb_stems.png");
  tx_paste(combined, leaves_tx, 0, 0);
  tx_paste(combined, leaves_tx, BLOCK_TEXTURE_SIZE, 0);
  tx_paste(combined, leaves_tx, BLOCK_TEXTURE_SIZE*2, 0);
  tx_paste(combined, stems_tx, 0, BLOCK_TEXTURE_SIZE);
  tx_paste(combined, stems_tx, 0, BLOCK_TEXTURE_SIZE*2);
  tx_paste(combined, stems_tx, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  tx_paste(combined, stems_tx, BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE*2);
  tx_paste(combined, stems_tx, BLOCK_TEXTURE_SIZE*2, BLOCK_TEXTURE_SIZE);
  tx_paste(combined, stems_tx, BLOCK_TEXTURE_SIZE*2, BLOCK_TEXTURE_SIZE*2);
  write_texture_to_png(combined, "out/test/test-herb_combined.png");
  // TODO: doublecheck
  /*
  texture *doublecheck = load_texture_from_png("out/test/tmoss.png");
  if (tx_get_px(doublecheck, 0, 0) != 0xff00b40f) { return 1; }
  if (tx_get_px(doublecheck, 1, 0) != 0xbf10870d) { return 2; }
  cleanup_texture(doublecheck);
  // */
  return 0;
}

#endif //ifndef TEST_TXG_PLANTS_H
