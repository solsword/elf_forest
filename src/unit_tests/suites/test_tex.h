#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME tex
#define TEST_SUITE_TESTS { \
    &test_texture_pixels, \
    &test_load_png, \
    NULL, \
  }

#ifndef TEST_TEX_H
#define TEST_TEX_H

#include "tex/tex.h"

#include "unit_tests/test_suite.h"

size_t test_texture_pixels(void) {
  texture * tx = create_texture(3, 3);
  tx_set_px(tx, 0xffffffff, 0, 0);
  pixel p;
  p = tx_get_px(tx, 0, 0);
  if (p != 0xffffffff) { return 1; }
  if (px_red(p) != 0xff) { return 17; }
  if (px_green(p) != 0xff) { return 3; }
  if (px_blue(p) != 0xff) { return 4; }
  if (px_alpha(p) != 0xff) { return 5; }
  tx_set_px(tx, 0x11223344, 0, 1);
  p = tx_get_px(tx, 0, 1);
  if (px_red(p) != 0x44) { return 6; }
  if (px_green(p) != 0x33) { return 7; }
  if (px_blue(p) != 0x22) { return 8; }
  if (px_alpha(p) != 0x11) { return 9; }
  px_set_red(tx_get_addr(tx, 0, 1), 0xff);
  p = tx_get_px(tx, 0, 1);
  if (p != 0x112233ff) { return 10; }
  if (tx_get_px(tx, 1, 1) != 0x00000000) { return 11; }
  cleanup_texture(tx);
  return 0;
}

size_t test_load_png(void) {
  texture *tx = load_texture_from_png(
    "res/textures/plants/mosses/template-sparse.png"
  );
  pixel p;
  p = tx_get_px(tx, 0, 0);
  if (p != 0xff0000fe) { return 1; }
  if (px_red(p) != 0xfe) { return 2; }
  if (px_green(p) != 0x00) { return 3; }
  if (px_blue(p) != 0x00) { return 4; }
  if (px_alpha(p) != 0xff) { return 5; }
  p = tx_get_px(tx, 1, 0);
  if (p != 0x00ffffff) { return 6; }
  if (px_red(p) != 0xff) { return 7; }
  if (px_green(p) != 0xff) { return 8; }
  if (px_blue(p) != 0xff) { return 9; }
  if (px_alpha(p) != 0x00) { return 10; }
  cleanup_texture(tx);
  return 0;
}

#endif //ifndef TEST_TEX_H
