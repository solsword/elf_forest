#undef TEST_SUITE_NAME
#undef TEST_SUITE_TESTS
#define TEST_SUITE_NAME color
#define TEST_SUITE_TESTS { \
    &test_color_conversion, \
    &test_precise_blending, \
    NULL, \
  }

#ifndef TEST_COLOR_H
#define TEST_COLOR_H

#include "tex/color.h"

#include "tex/tex.h"

#include "unit_tests/test_suite.h"

size_t test_color_conversion(void) {
  pixel b = PX_BLACK;
  pixel w = PX_WHITE;
  pixel t = 0xff334455;
  precise_color pr;
  float x, y, z;
  float epsilon = 0.00000000001;

  if (rgb__hsv(hsv__rgb(b)) != b) { return 1; }
  if (rgb__hsv(hsv__rgb(w)) != w) { return 2; }
  if (rgb__hsv(hsv__rgb(t)) != t) { return 3; }
  if (hsv__rgb(rgb__hsv(b)) != b) { return 4; }
  if (hsv__rgb(rgb__hsv(w)) != w) { return 5; }
  if (hsv__rgb(rgb__hsv(t)) != t) { return 6; }

  rgb__xyz(b, &pr);
  if (xyz__rgb(&pr) != b) { return 7; }
  x = pr.x;
  y = pr.y;
  z = pr.z;

  xyz__lab(&pr);
  lab__xyz(&pr);

  if (fabs(pr.x - x) > epsilon) { return 8; }
  if (fabs(pr.y - y) > epsilon) { return 9; }
  if (fabs(pr.z - z) > epsilon) { return 10; }

  if (xyz__rgb(&pr) != b) { return 11; }

  rgb__xyz(t, &pr);
  if (xyz__rgb(&pr) != t) { return 12; }
  x = pr.x;
  y = pr.y;
  z = pr.z;

  xyz__lab(&pr);
  lab__xyz(&pr);

  if (fabs(pr.x - x) > epsilon) { return 13; }
  if (fabs(pr.y - y) > epsilon) { return 14; }
  if (fabs(pr.z - z) > epsilon) { return 15; }

  if (xyz__rgb(&pr) != t) { return 16; }

  return 0;
}

size_t test_precise_blending(void) {
  size_t i;
  pixel p = PX_BLACK;
  float interp = 0;
  pixel start = 0xffff0000;
  pixel end = 0xff00ff00;
  texture * tx = create_texture(8, 2);

  for (i = 0; i < 8; ++i) {
    interp = ((float) i / 7.0);
    // dumb interpolation:
    px_set_red(
      &p,
      (channel) (interp * px_red(end) + (1 - interp) * px_red(start))
    );
    px_set_green(
      &p,
      (channel) (interp * px_green(end) + (1 - interp) * px_green(start))
    );
    px_set_blue(
      &p,
      (channel) (interp * px_blue(end) + (1 - interp) * px_blue(start))
    );
    tx_set_px(tx, p, i, 0);

    // smart interpolation:
    p = blend_precisely(start, end, interp);
    tx_set_px(tx, p, i, 1);
  }

  write_texture_to_png(tx, "out/test/test-gradient.png");
  cleanup_texture(tx);
  return 0;
}

#endif //ifndef TEST_COLOR_H
