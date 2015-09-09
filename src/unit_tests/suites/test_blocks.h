#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME blocks
#define TEST_SUITE_TESTS { \
    &test_blocks, \
    NULL, \
  }

#ifndef TEST_BLOCKS_H
#define TEST_BLOCKS_H

#include <stdio.h>

#include "world/blocks.h"

#include "unit_tests/test_suite.h"

size_t test_blocks(void) {
  init_blocks();
  if (bi_vis(b_make_block(B_AIR)) != BI_VIS_INVISIBLE) { return 1; }
  if (bi_sbst(b_make_block(B_AIR)) != BI_SBST_EMPTY) { return 2; }
  if (bi_geom(b_make_block(B_AIR)) != BI_GEOM_EMPTY) { return 3; }
  if (!b_is_invisible(b_make_block(B_AIR))) { return 4; }
  if (b_is_solid(b_make_block(B_AIR))) { return 5; }
  return 0;
}

#endif //ifndef TEST_BLOCKS_H
