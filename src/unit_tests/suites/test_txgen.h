#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME txgen
#define TEST_SUITE_TESTS { \
    &test_txgen_template_moss, \
    NULL, \
  }

#ifndef TEST_TXGEN_H
#define TEST_TXGEN_H

#include "gen/txgen.h"
#include "graphics/tex.h"
#include "../res/textures/plants/mosses/moss.h"

#include "unit_tests/test_suite.h"

size_t test_txgen_scatter_filter_moss(void) {
  run_grammar(&scattered_moss);
  write_texture_to_png(scattered_moss.result, "out/test/smoss.png");
  write_texture_to_ppm(scattered_moss.result, "out/test/smoss.ppm");
  cleanup_grammar(&scattered_moss);
  return 0;
}

size_t test_txgen_template_moss(void) {
  run_grammar(&template_moss);
  write_texture_to_png(template_moss.result, "out/test/tmoss.png");
  write_texture_to_ppm(template_moss.result, "out/test/tmoss.ppm");
  /* TODO: Turn this into a reading/writing unit test!
  texture *result = load_texture_from_png(template_moss.filename);
  write_texture_to_png(result, "out/test/tmoss.png");
  write_texture_to_ppm(result, "out/test/tmoss.ppm");
  printf("Test:\n");
  printf(
    "  px(0, 0) r g b: %d, %d, %d // value: 0x%08x\n",
    px_red(tx_get_px(result, 0, 0)),
    px_green(tx_get_px(result, 0, 0)),
    px_blue(tx_get_px(result, 0, 0)),
    tx_get_px(result, 0, 0)
  );
  printf(
    "  px(1, 0) r g b: %d, %d, %d // value: 0x%08x\n",
    px_red(tx_get_px(result, 1, 0)),
    px_green(tx_get_px(result, 1, 0)),
    px_blue(tx_get_px(result, 1, 0)),
    tx_get_px(result, 1, 0)
  );
  cleanup_texture(result);
  // */
  cleanup_grammar(&template_moss);
  return 0;
}

#endif //ifndef TEST_TXGEN_H
