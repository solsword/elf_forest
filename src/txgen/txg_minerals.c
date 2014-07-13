// txg_minerals.c
// Mineral and soil texture generation.

#include "noise/noise.h"
#include "tex/tex.h"
#include "tex/draw.h"
#include "world/species.h"

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
  float grit, contours, matrix, bumps, alternate;
  float saturation, value;
  stone_filter_args *sfargs = (stone_filter_args *) fargs;
  pixel base_hsv, alt_hsv;
  pixel hsv;
  pixel rgb;
  rgb__hsv(sfargs->base_color, &base_hsv);
  rgb__hsv(sfargs->alt_color, &alt_hsv);
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
      grit = hash_2d(
        col + fastfloor(x),
        row + fastfloor(y)
      ) / (float) HASH_MASK;
      contours = (1 + sxnoise_2d(x, y)) / 2.0;
      matrix = sqrtf(
        wrnoise_2d_fancy(
          x+1000, y,
          BLOCK_TEXTURE_SIZE * sfargs->scale,
          BLOCK_TEXTURE_SIZE * sfargs->scale,
          0
        )
      );
      bumps = sqrtf(
        wrnoise_2d_fancy(
          x, y,
          BLOCK_TEXTURE_SIZE * sfargs->scale,
          BLOCK_TEXTURE_SIZE * sfargs->scale,
          WORLEY_FLAG_INCLUDE_NEXTBEST
        )
      );

      alternate = wrnoise_2d(
        x * 1.4, y * 1.4 + 1000
      ) * (
        (1 + sxnoise_2d(x*1.2, y*1.2 + 2000)) / 2.0
      );
      //alternate = ((1 + sxnoise_2d(x*1.2, y*1.2 + 2000)) / 2.0);
      //alternate *= ((1 + sxnoise_2d(x*1.4, y*1.4 + 3000)) / 2.0);
      //alternate = exp(alternate)/exp(1);
      //alternate *= alternate;
      alternate = sqrtf(alternate);

      // value construction:
      value = (
        sfargs->gritty * grit +
        sfargs->contoured * contours +
        sfargs->porous * matrix +
        sfargs->bumpy * bumps
      ) / (
        sfargs->gritty + sfargs->contoured + sfargs->porous + sfargs->bumpy
      );

      // saturation variance:
      saturation = (0.7 + 0.3 * sxnoise_2d(x, y+1000));

      // figure out the base color:
      hsv = base_hsv;
      px_set_blue(&hsv, px_blue(hsv) * saturation);
      px_set_green(&hsv, CHANNEL_MAX * value);
      if (alternate > (1 - sfargs->inclusions)) {
        px_set_red(&hsv, px_red(alt_hsv));
        px_set_blue(&hsv, px_blue(alt_hsv));
        px_set_green(&hsv, CHANNEL_MAX * alternate);
      } else if (alternate > (1 - (sfargs->inclusions + 0.05))) {
        px_set_blue(&hsv, 0.5*px_blue(hsv));
        px_set_green(&hsv, (px_green(hsv) + CHANNEL_MAX * alternate)/2.0);
      }
      hsv__rgb(hsv, &rgb);
      rgb = px_relight(rgb, sfargs->brightness);

      tx_set_px(tx, rgb, col, row);
    }
  }
}

/*************
 * Functions *
 *************/

texture* gen_stone_texture(species s) {
  stone_species* ssp = get_stone_species(s);
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_stone(result, &(ssp->appearance));
  return result;
}
