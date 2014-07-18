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
  int row, col; // position within texture
  float x, y; // distorted/squashed x/y coordinates
  float alx, aly; // distorted/squashed alternate x/y coordinates

  float scx, scy; // rounded x/y scales
  float dscx, dscy; // rounded x/y distortion scales
  float alscx, alscy; // rounded alternate x/y scales

  float wx, wy; // wrap scales
  float dwx, dwy; // distortion wrap scales
  float alwx, alwy; // alternate wrap scales

  ptrdiff_t offset; // absolute offset for seed effect
  float ds; // distortion
  float grit, contours, matrix, bumps, alternate; // noise layers
  float saturation, value; // color values
  pixel base_hsv, alt_hsv; // base and alternate colors in hsv
  pixel hsv; // temp hsv color
  pixel rgb; // final rgb color
  stone_filter_args *sfargs = (stone_filter_args *) fargs;
  rgb__hsv(sfargs->base_color, &base_hsv);
  rgb__hsv(sfargs->alt_color, &alt_hsv);
  // We want to incorporate the seed value, but we don't want to deal with
  // overflow. 4 bytes of seed-based noise should be plenty while giving
  // comfortable overhead against overflow.
  offset = expanded_hash_1d(sfargs->seed) & 0xffff;
  // Round all scale values so that the scaled coordinate-space texture
  // boundaries are integers (for simplex noise wrapping purposes):
  scx = rounddenom(sfargs->scale*sfargs->squash, tx->width);
  scy = rounddenom(sfargs->scale/sfargs->squash, tx->height);
  wx = (float) tx->width * scx;
  wy = (float) tx->height * scy;

  dscx = rounddenom(sfargs->dscale, tx->width);
  dscy = rounddenom(sfargs->dscale, tx->height);
  dwx = tx->width * dscx;
  dwy = tx->height * dscy;

  alscx = (tx->width/5 + scx*tx->width)/((float) tx->width);
  alscy = (tx->height/5 + scy*tx->height)/((float) tx->height);
  alwx = tx->width * alscx;
  alwy = tx->height * alscy;

  // Loop over each pixel of the texture:
  for (col = 0; col < tx->width; col += 1) {
    for (row = 0; row < tx->height; row += 1) {
      ds = tiled_func(
        &sxnoise_2d,
        col * dscx, row * dscy,
        dwx, dwy,
        sfargs->seed
      );
      x = (col + ds * sfargs->distortion) * scx;
      y = (row + ds * sfargs->distortion) * scy;
      x += tx->width * offset;
      y += tx->height * offset;
      grit = hash_2d(
        col + fastfloor(x),
        row + fastfloor(y)
      ) / (float) HASH_MASK;
      contours = (
        1 + tiled_func(&sxnoise_2d, x, y, wx, wy, sfargs->seed+1)
      ) / 2.0;
      matrix = sqrtf(
        wrnoise_2d_fancy(
          x, y,
          fastfloor(wx), fastfloor(wy),
          0
        )
      );
      bumps = sqrtf(
        wrnoise_2d_fancy(
          x + tx->width*offset, y,
          fastfloor(wx), fastfloor(wy),
          WORLEY_FLAG_INCLUDE_NEXTBEST
        )
      );

      alx = col * alscx + tx->width*offset*3;
      aly = row * alscy + tx->height*offset*3;
      alternate = wrnoise_2d_fancy(
        alx, aly,
        fastfloor(wx), fastfloor(wy),
        WORLEY_FLAG_INCLUDE_NEXTBEST
      ) * (
        (
          1 + tiled_func(
            &sxnoise_2d,
            alx, aly,
            alwx, alwy,
            sfargs->seed+2
          )
        ) / 2.0
      );
      alternate = sqrtf(alternate);

      // value construction:
      value = (
        sfargs->gritty * grit +
        sfargs->contoured * contours +
        sfargs->porous * matrix +
        sfargs->bumpy * bumps +
        0
      ) / (
        sfargs->gritty +
        sfargs->contoured +
        sfargs->porous +
        sfargs->bumpy +
        0
      );

      // saturation variance:
      saturation = (
        0.7 + 0.3 * tiled_func(
          &sxnoise_2d,
          x, y,
          wx, wy,
          sfargs->seed+3
        )
      );

      // figure out the base color:
      hsv = base_hsv;
      px_set_sat(&hsv, px_sat(hsv) * saturation);
      px_set_val(&hsv, CHANNEL_MAX * value);
      if (alternate > (1 - sfargs->inclusions)) {
        px_set_hue(&hsv, px_hue(alt_hsv));
        px_set_sat(&hsv, px_sat(alt_hsv));
        px_set_val(&hsv, CHANNEL_MAX * alternate);
      } else if (alternate > (1 - (sfargs->inclusions + 0.05))) {
        px_set_sat(&hsv, 0.5*px_sat(hsv));
        px_set_val(&hsv, (px_val(hsv) + CHANNEL_MAX * alternate)/2.0);
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
