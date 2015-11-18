// color.c
// Color manipulation and format conversion.

#include <math.h>

#include "color.h"

/*************
 * Functions *
 *************/

pixel float_color(float r, float g, float b, float a) {
  pixel result = PX_EMPTY;
  int cr = fastfloor(CHANNEL_MAX * r);
  int cg = fastfloor(CHANNEL_MAX * g);
  int cb = fastfloor(CHANNEL_MAX * b);
  int ca = fastfloor(CHANNEL_MAX * a);
  if (cr < 0) { cr = 0; }
  if (cr > CHANNEL_MAX) { cr = CHANNEL_MAX; }
  if (cg < 0) { cg = 0; }
  if (cg > CHANNEL_MAX) { cg = CHANNEL_MAX; }
  if (cb < 0) { cb = 0; }
  if (cb > CHANNEL_MAX) { cb = CHANNEL_MAX; }
  if (ca < 0) { ca = 0; }
  if (ca > CHANNEL_MAX) { ca = CHANNEL_MAX; }
  px_set_red(&result, (channel) cr);
  px_set_green(&result, (channel) cg);
  px_set_blue(&result, (channel) cb);
  px_set_alpha(&result, (channel) ca);
  return result;
}

pixel rgb__hsv(pixel p) {
  pixel result = PX_BLACK;
  float r = px_red(p) / (float) CHANNEL_MAX;
  float g = px_green(p) / (float) CHANNEL_MAX;
  float b = px_blue(p) / (float) CHANNEL_MAX;
  float max = 0, min = CHANNEL_MAX;
  float chroma = 0;
  if (r > max) { max = r; }
  if (g > max) { max = g; }
  if (b > max) { max = b; }
  if (r < min) { min = r; }
  if (g < min) { min = g; }
  if (b < min) { min = b; }
  chroma = max - min;

  // saturation (first 'cause of the shortcut when max == 0)
  if (max == 0) {
    px_set_red(&result, 0);
    px_set_blue(&result, 0);
    px_set_green(&result, 0);
    return result;
  } else {
    px_set_sat(&result, CHANNEL_MAX * (chroma / max));
  }

  // hue
  if (max == r) {
    px_set_hue(
      &result,
      CHANNEL_MAX * (
        (1/6.0) * (g - b) / chroma + (b > g ? 1 : 0)
      )
    );
  } else if (max == g) {
    px_set_hue(
      &result,
      CHANNEL_MAX * (
        (1/6.0) * (b - r) / chroma + (1/3.0)
      )
    );
  } else if (max == b) {
    px_set_hue(
      &result,
      CHANNEL_MAX * (
        (1/6.0) * (r - g) / chroma + (2/3.0)
      )
    );
  } else {
    px_set_hue(
      &result,
      CHANNEL_MAX * (
        0
      )
    );
  }

  // value
  if (r >= g && r >= b) {
    px_set_val(&result, CHANNEL_MAX * r);
  } else if (g >= b) {
    px_set_val(&result, CHANNEL_MAX * g);
  } else {
    px_set_val(&result, CHANNEL_MAX * b);
  }
  return result;
}

pixel hsv__rgb(pixel p) {
  pixel result = PX_BLACK;
  float h = px_hue(p) / (float) CHANNEL_MAX;
  float s = px_sat(p) / (float) CHANNEL_MAX;
  float v = px_val(p) / (float) CHANNEL_MAX;
  int sector = floor(h / (1/6.0)); // which sector of the color wheel?
  float remainder = 6 * (h - (1/6.0)*sector); // [0, 1] position in sector
  // The three possible channel values (along with v):
  float c1 = v * (1 - s);
  float c2 = v * (1 - s * remainder);
  float c3 = v * (1 - s * (1 - remainder));

  // shortcut for gray:
  if (s == 0) {
    px_set_red(&result, CHANNEL_MAX*v);
    px_set_green(&result, CHANNEL_MAX*v);
    px_set_blue(&result, CHANNEL_MAX*v);
    return result;
  }
  switch (sector) {
    case 0:
      px_set_red(&result, CHANNEL_MAX * v);
      px_set_green(&result, CHANNEL_MAX * c3);
      px_set_blue(&result, CHANNEL_MAX * c1);
      break;
    case 1:
      px_set_red(&result, CHANNEL_MAX * c2);
      px_set_green(&result, CHANNEL_MAX * v);
      px_set_blue(&result, CHANNEL_MAX * c1);
      break;
    case 2:
      px_set_red(&result, CHANNEL_MAX * c1);
      px_set_green(&result, CHANNEL_MAX * v);
      px_set_blue(&result, CHANNEL_MAX * c3);
      break;
    case 3:
      px_set_red(&result, CHANNEL_MAX * c1);
      px_set_green(&result, CHANNEL_MAX * c2);
      px_set_blue(&result, CHANNEL_MAX * v);
      break;
    case 4:
      px_set_red(&result, CHANNEL_MAX * c3);
      px_set_green(&result, CHANNEL_MAX * c1);
      px_set_blue(&result, CHANNEL_MAX * v);
      break;
    case 5:
    default:
      px_set_red(&result, CHANNEL_MAX * v);
      px_set_green(&result, CHANNEL_MAX * c1);
      px_set_blue(&result, CHANNEL_MAX * c2);
      break;
  }
  return result;
}

void rgb__xyz(pixel p, precise_color *result) {
  float r = px_red(p) / (float) CHANNEL_MAX;
  float g = px_green(p) / (float) CHANNEL_MAX;
  float b = px_blue(p) / (float) CHANNEL_MAX;
  float a = px_alpha(p) / (float) CHANNEL_MAX;
  if (r > 0.04045) {
    r = pow(((r + 0.055) / 1.055), 2.4);
  } else {
    r = r / 12.92;
  }
  if (b > 0.04045) {
    b = pow(((b + 0.055) / 1.055), 2.4);
  } else {
    b = b / 12.92;
  }
  if (g > 0.04045) {
    g = pow(((g + 0.055) / 1.055), 2.4);
  } else {
    g = g / 12.92;
  }

  r *= 100.0;
  b *= 100.0;
  g *= 100.0;

  result->x = r * 0.4124 + g * 0.3576 + b * 0.1805;
  result->y = r * 0.2126 + g * 0.7152 + b * 0.0722;
  result->z = r * 0.0193 + g * 0.1192 + b * 0.9505;
  result->alpha = a;
}

pixel xyz__rgb(precise_color* color) {
  pixel result = PX_BLACK;
  float x = color->x / 100.0;
  float y = color->y / 100.0;
  float z = color->z / 100.0;

  float r = x *  3.2406 + y * -1.5372 + z * -0.4986;
  float g = x * -0.9689 + y *  1.8758 + z *  0.0415;
  float b = x *  0.0557 + y * -0.2040 + z *  1.0570;
  float a = color->alpha;

  if (r > 0.0031308) {
    r = 1.055 * (pow(r, 1.0/2.4)) - 0.055;
  } else {
    r *= 12.92;
  }
  if (g > 0.0031308) {
    g = 1.055 * (pow(g, 1.0/2.4)) - 0.055;
  } else {
    g *= 12.92;
  }
  if (b > 0.0031308) {
    b = 1.055 * (pow(b, 1.0/2.4)) - 0.055;
  } else {
    b *= 12.92;
  }

  // clamp out-of-gamut colors
  r *= 255.0;
  if (r < 0) { r = 0; }
  if (r > 255) { r = 255; }
  g *= 255.0;
  if (g < 0) { g = 0; }
  if (g > 255) { g = 255; }
  b *= 255.0;
  if (b < 0) { b = 0; }
  if (b > 255) { b = 255; }
  a *= 255.0;
  if (a < 0) { a = 0; }
  if (a > 255) { a = 255; }

  // write results
  px_set_red(&result, (channel) r);
  px_set_green(&result, (channel) g);
  px_set_blue(&result, (channel) b);
  px_set_alpha(&result, (channel) a);

  return result;
}

void xyz__lab(precise_color *color) {
  // These values correspond to a 2-degree FoV w/ illuminant D65:
  float tx = color->x / 95.047;
  float ty = color->y / 100.0;
  float tz = color->z / 108.883;

  if (tx > 0.008856) {
    tx = pow(tx, 1.0/3.0);
  } else {
    tx = (7.787 * tx) + (16.0 / 116.0);
  }
  if (ty > 0.008856) {
    ty = pow(ty, 1.0/3.0);
  } else {
    ty = (7.787 * ty) + (16.0 / 116.0);
  }
  if (tz > 0.008856) {
    tz = pow(tz, 1.0/3.0);
  } else {
    tz = (7.787 * tz) + (16.0 / 116.0);
  }

  color->x = (116.0 * ty) - 16.0;
  color->y = 500.0 * (tx - ty);
  color->z = 200.0 * (ty - tz);
}

void lab__xyz(precise_color *color) {
  float ty = (color->x + 16.0) / 116.0;
  float tx = color->y / 500.0 + ty;
  float tz = ty - (color->z / 200.0);

  if (tx > 0.206893) {
    tx = pow(tx, 3);
  } else {
    tx = (tx - (16.0 / 116.0)) / 7.787;
  }
  if (ty > 0.206893) {
    ty = pow(ty, 3);
  } else {
    ty = (ty - (16.0 / 116.0)) / 7.787;
  }
  if (tz > 0.206893) {
    tz = pow(tz, 3);
  } else {
    tz = (tz - (16.0 / 116.0)) / 7.787;
  }

  // These values correspond to a 2-degree FoV w/ illuminant D65:
  color->x = 95.047 * tx;
  color->y = 100.0 * ty;
  color->z = 108.883 * tz;
}

void lab__lch(precise_color *color) {
  // Note: L ('x' in our struct) is unchanged
  float h = atan2(color->z, color->y);
  color->y = sqrtf( pow(color->y, 2) + pow(color->z, 2) );
  color->z = h;
}

void lch__lab(precise_color *color) {
  float y = cosf(color->z) * color->y;
  float z = sinf(color->z) * color->y;
  color->y = y;
  color->z = z;
}

pixel blend_precisely(pixel a, pixel b, float blend) {
  precise_color ca, cb;
  rgb__xyz(a, &ca); /*->*/ xyz__lab(&ca); /*->*/ lab__lch(&ca);
  rgb__xyz(b, &cb); /*->*/ xyz__lab(&cb); /*->*/ lab__lch(&cb);
  ca.x = blend * ca.x + (1 - blend) * cb.x;
  ca.y = blend * ca.y + (1 - blend) * cb.y;
  ca.z = blend * ca.z + (1 - blend) * cb.z;
  ca.alpha = blend * ca.alpha + (1 - blend) * cb.alpha;
  lch__lab(&ca); /*->*/ lab__xyz(&ca);
  return xyz__rgb(&ca);
}
