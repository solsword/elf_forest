#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME txgen
#define TEST_SUITE_TESTS { \
    &test_txgen_template_moss, \
    &test_txgen_scatter_filter_moss, \
    &test_worley_noise, \
    NULL, \
  }

#ifndef TEST_TXGEN_H
#define TEST_TXGEN_H

#include "gen/txgen.h"
#include "graphics/tex.h"
#include "../res/textures/plants/mosses/moss.h"

#include "unit_tests/test_suite.h"

size_t test_txgen_template_moss(void) {
  run_grammar(&template_sparse_moss);
  write_texture_to_png(template_sparse_moss.result, "out/test/tmoss.png");
  write_texture_to_ppm(template_sparse_moss.result, "out/test/tmoss.ppm");
  cleanup_grammar(&template_sparse_moss);
  texture *doublecheck = load_texture_from_png("out/test/tmoss.png");
  if (tx_get_px(doublecheck, 0, 0) != 0xff00b40f) { return 1; }
  if (tx_get_px(doublecheck, 1, 0) != 0xbf10870d) { return 2; }
  cleanup_texture(doublecheck);
  return 0;
}

size_t test_txgen_scatter_filter_moss(void) {
  run_grammar(&scattered_moss);
  write_texture_to_png(scattered_moss.result, "out/test/smoss.png");
  write_texture_to_ppm(scattered_moss.result, "out/test/smoss.ppm");
  cleanup_grammar(&scattered_moss);
  return 0;
}

size_t test_worley_noise(void) {
  run_grammar(&worley_test);
  write_texture_to_png(worley_test.result, "out/test/wtest.png");
  write_texture_to_ppm(worley_test.result, "out/test/wtest.ppm");
  cleanup_grammar(&worley_test);
  return 0;
}

#endif //ifndef TEST_TXGEN_H
