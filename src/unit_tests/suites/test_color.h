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

#include <stdio.h>

static inline float color_error(pixel p1, pixel p2) {
  float dr = px_red(p1) - px_red(p2);
  float dg = px_green(p1) - px_green(p2);
  float db = px_blue(p1) - px_blue(p2);
  float da = px_alpha(p1) - px_alpha(p2);
  float meanr = (float) (px_red(p1) + px_red(p2)) / 2.0;
  float meang = (float) (px_green(p1) + px_green(p2)) / 2.0;
  float meanb = (float) (px_blue(p1) + px_blue(p2)) / 2.0;
  float meana = (float) (px_alpha(p1) + px_alpha(p2)) / 2.0;
  return (
    (dr / meanr)
  + (dg / meang)
  + (db / meanb)
  + (da / meana)
  );
}

#define COLOR_ERROR_LIMIT 0.03
#define COLOR_FLOAT_ERROR_LIMIT 1e-6

size_t test_color_conversion(void) {
  pixel b = PX_BLACK;
  pixel w = PX_WHITE;
  pixel t = 0xff334455;
  precise_color pr;
  float x, y, z;

  if (color_error(rgb__hsv(hsv__rgb(b)), b) > COLOR_ERROR_LIMIT) { return 1; }
  if (color_error(rgb__hsv(hsv__rgb(w)), w) > COLOR_ERROR_LIMIT) { return 2; }
  if (color_error(rgb__hsv(hsv__rgb(t)), t) > COLOR_ERROR_LIMIT) { return 3; }
  if (color_error(hsv__rgb(rgb__hsv(b)), b) > COLOR_ERROR_LIMIT) { return 4; }
  if (color_error(hsv__rgb(rgb__hsv(w)), w) > COLOR_ERROR_LIMIT) { return 5; }
  if (color_error(hsv__rgb(rgb__hsv(t)), t) > COLOR_ERROR_LIMIT) { return 6; }

  rgb__xyz(b, &pr);
  if (color_error(xyz__rgb(&pr), b) > COLOR_ERROR_LIMIT) { return 7; }
  x = pr.x;
  y = pr.y;
  z = pr.z;

  xyz__lab(&pr);
  lab__xyz(&pr);

  if (fabs(pr.x - x) > COLOR_FLOAT_ERROR_LIMIT) { return 8; }
  if (fabs(pr.y - y) > COLOR_FLOAT_ERROR_LIMIT) { return 9; }
  if (fabs(pr.z - z) > COLOR_FLOAT_ERROR_LIMIT) { return 10; }

  if (color_error(xyz__rgb(&pr), b) > COLOR_ERROR_LIMIT) { return 11; }

  rgb__xyz(t, &pr);
  if (color_error(xyz__rgb(&pr), t) > COLOR_ERROR_LIMIT) { return 12; }
  x = pr.x;
  y = pr.y;
  z = pr.z;

  xyz__lab(&pr);
  lab__xyz(&pr);

  if (fabs(pr.x - x) > COLOR_FLOAT_ERROR_LIMIT) { return 13; }
  if (fabs(pr.y - y) > COLOR_FLOAT_ERROR_LIMIT) { return 14; }
  if (fabs(pr.z - z) > COLOR_FLOAT_ERROR_LIMIT) { return 15; }

  if (color_error(xyz__rgb(&pr), t) > COLOR_ERROR_LIMIT) { return 16; }

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
