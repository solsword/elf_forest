#ifndef PLANT_TYPE_MOSS
#define PLANT_TYPE_MOSS

#include "gen/txgen.h"

// The individual moss clump textures (with anchor points):
tx_grammar_literal moss_clump_1 = SIMPLE_TX_LITERAL("res/textures/plants/mosses/leaf-moss-1.png", 3, 2);
tx_grammar_literal moss_clump_2 = SIMPLE_TX_LITERAL("res/textures/plants/mosses/leaf-moss-2.png", 3, 2);
tx_grammar_literal moss_clump_3 = SIMPLE_TX_LITERAL("res/textures/plants/mosses/leaf-moss-3.png", 2, 2);
tx_grammar_literal moss_clump_4 = SIMPLE_TX_LITERAL("res/textures/plants/mosses/leaf-moss-4.png", 2, 3);
tx_grammar_literal moss_clump_5 = SIMPLE_TX_LITERAL("res/textures/plants/mosses/leaf-moss-5.png", 3, 2);
tx_grammar_literal moss_clump_6 = SIMPLE_TX_LITERAL("res/textures/plants/mosses/leaf-moss-6.png", 3, 2);
tx_grammar_literal moss_clump_7 = SIMPLE_TX_LITERAL("res/textures/plants/mosses/leaf-moss-7.png", 2, 2);
tx_grammar_literal moss_clump_8 = SIMPLE_TX_LITERAL("res/textures/plants/mosses/leaf-moss-8.png", 2, 3);
tx_grammar_literal moss_clump_9 = SIMPLE_TX_LITERAL("res/textures/plants/mosses/leaf-moss-9.png", 2, 2);
tx_grammar_literal moss_clump_10 = SIMPLE_TX_LITERAL("res/textures/plants/mosses/leaf-moss-10.png", 2, 2);

// A disjunction over the clumps with equal weight for each:
TX_ANY_ENTRY(moss_clump, 10, NULL, 1);
TX_ANY_ENTRY(moss_clump, 9, &TX_ANY(moss_clump, 10), 1);
TX_ANY_ENTRY(moss_clump, 8, &TX_ANY(moss_clump, 9), 1);
TX_ANY_ENTRY(moss_clump, 7, &TX_ANY(moss_clump, 8), 1);
TX_ANY_ENTRY(moss_clump, 6, &TX_ANY(moss_clump, 7), 1);
TX_ANY_ENTRY(moss_clump, 5, &TX_ANY(moss_clump, 6), 1);
TX_ANY_ENTRY(moss_clump, 4, &TX_ANY(moss_clump, 5), 1);
TX_ANY_ENTRY(moss_clump, 3, &TX_ANY(moss_clump, 4), 1);
TX_ANY_ENTRY(moss_clump, 2, &TX_ANY(moss_clump, 3), 1);
TX_ANY_ENTRY(moss_clump, 1, &TX_ANY(moss_clump, 2), 1);

// Moss based on the template-pairs.png template:
tx_grammar_literal template_pairs_moss = {
  .filename = "res/textures/plants/mosses/template-pairs.png",
  .anchor_x = 0,
  .anchor_y = 0,
  .preprocess = NULL,
    .preargs = NULL,
  .postprocess = NULL,
    .postargs = NULL,
  .children = {
    &any_moss_clump_1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
  },
  .result = NULL
};

// Moss based on the template-sparse.png template:
tx_grammar_literal template_sparse_moss = {
  .filename = "res/textures/plants/mosses/template-sparse.png",
  .anchor_x = 0,
  .anchor_y = 0,
  .preprocess = NULL,
    .preargs = NULL,
  .postprocess = NULL,
    .postargs = NULL,
  .children = {
    &any_moss_clump_1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
  },
  .result = NULL
};

scatter_filter_args moss_scatter_args = {
  .x_freq = 6,
  .y_freq = 6,
  .color = GRAMMAR_KEY_0
};

gradient_map worley_test_map = {
  .colors = {
    0xff000000, 0xff111111, 0xff222222, 0xff333333,
    0xff444444, 0xff555555, 0xff666666, 0xff777777,
    0xff888888, 0xff999999, 0xffaaaaaa, 0xffbbbbbb,
    0xffcccccc, 0xffdddddd, 0xffeeeeee, 0xffffffff,
  },
  .thresholds = {
    1/16.0, 2/16.0, 3/16.0, 4/16.0,
    5/16.0, 6/16.0, 7/16.0, 8/16.0,
    9/16.0, 10/16.0, 11/16.0, 12/16.0,
    13/16.0, 14/16.0, 15/16.0, 16/16.0
  }
};

worley_filter_args worley_test_args = {
  .freq = 1/8.0,
  .grmap = &(worley_test_map)
};

branch_filter_args branch_test_args = {
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

// Moss based a 6x6 scatter grid (specified by the args struct above):
tx_grammar_literal scattered_moss = {
  .filename = NULL,
  .anchor_x = 32,
  .anchor_y = 32,
  .preprocess = &fltr_scatter,
    .preargs = (void *) (&moss_scatter_args),
  .postprocess = NULL,
    .postargs = NULL,
  .children = {
    &any_moss_clump_1,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
  },
  .result = NULL
};

// TODO: remove this
// A Worley noise test:
tx_grammar_literal worley_test = {
  .filename = NULL,
  .anchor_x = 32,
  .anchor_y = 32,
  .preprocess = &fltr_worley,
    .preargs = (void *) (&worley_test_args),
  .postprocess = NULL,
    .postargs = NULL,
  .children = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
  },
  .result = NULL
};

// TODO: move this
// A branch filter test:
tx_grammar_literal branch_filter_test = {
  .filename = NULL,
  .anchor_x = 32,
  .anchor_y = 32,
  .preprocess = &fltr_branches,
    .preargs = (void *) (&branch_test_args),
  .postprocess = NULL,
    .postargs = NULL,
  .children = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
  },
  .result = NULL
};

#endif // PLANT_TYPE_MOSS
