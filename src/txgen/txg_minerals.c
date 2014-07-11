// txg_minerals.c
// Mineral and soil texture generation.

#include "noise/noise.h"
#include "tex/tex.h"
#include "tex/draw.h"

#include "txgen.h"

#include "txg_minerals.h"

/********************
 * Filter Functions *
 ********************/

// Generates stone textures.
void fltr_stone(texture *tx, void const * const fargs) {
  int row, col;
  float x, y;
  float offset;
  float ds;
  float noise, veins, bumps;
  stone_filter_args *sfargs = (stone_filter_args *) fargs;
  pixel base_hsv = rgb__hsv(sfargs->base_color);
  pixel alt_hsv = rgb__hsv(sfargs->alt_color);
  pixel hsv;
  pixel rgb;
  // We want to incorporate the seed value, but we don't want to deal with
  // overflow. 4 bytes of seed-based noise should be plenty while giving
  // comfortable overhead against overflow.
  offset = expanded_hash_1d(sfargs->seed) & 0xffff;
  for (col = 0; col < tx->width; col += 1) {
    for (row = 0; row < tx->height; row += 1) {
      ds = sxnoise_2d(col * sfargs->dscale, row * sfargs->dscale);
      x = (col * sfargs->squash + ds * sfargs->distortion) * sfargs->scale;
      y = (row / sfargs->squash + ds * sfargs->distortion) * sfargs->scale;
      x += tx->width * offset;
      y += tx->height * offset;
      noise = sfargs->noisy * hash_2d(
        col + fastfloor(x),
        row + fastfloor(y)
      ) / HASH_MASK;
      veins = sfargs->veins * wrnoise_2d_fancy(
        x, y,
        BLOCK_TEXTURE_SIZE * sfargs->scale, BLOCK_TEXTURE_SIZE * sfargs->scale,
        WORLEY_FLAG_INCLUDE_NEXTBEST
      );
      bumps = sfargs->bumpy * (1 + sxnoise_2d(x, y)) / 2.0;
      // figure out the color:
      hsv = base_hsv;
      px_set_green(&hsv, CHANNEL_MAX * (noise + veins + bumps/3.0));
      rgb = hsv__rgb(hsv);
      tx_set_px(tx, rgb, col, row);
    }
  }
}
