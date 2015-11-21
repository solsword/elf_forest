// txg_minerals.c
// Mineral and soil texture generation.

#include "noise/noise.h"
#include "tex/tex.h"
#include "tex/draw.h"
#include "world/species.h"
#include "math/functions.h"

#include "util.h"

#include "txgen.h"

#include "txg_minerals.h"

/********************
 * Filter Functions *
 ********************/

// Generates mineral textures.
void fltr_mineral(texture *tx, void const * const fargs) {
  int row, col; // position within texture
  float x, y; // distorted/squashed x/y coordinates
  float alx, aly; // distorted/squashed alternate x/y coordinates

  float scx, scy; // rounded x/y scales
  float dscx, dscy; // rounded x/y distortion scales
  float alscx, alscy; // rounded alternate x/y scales

  float wx, wy; // wrap scales
  float dwx, dwy; // distortion wrap scales
  float alwx, alwy; // alternate wrap scales

  float ds; // distortion
  float grit, contours, matrix, bumps, alternate, layers; // noise components
  float wavescale, layerscale, lphase; // layer phase
  float snoise; // saturation noise
  float saturation, value; // color values

  float dontcare;

  // Seeds for the different noise functions:
  ptrdiff_t salt1, salt2, salt3, salt4, salt5, salt6, salt7, salt8;

  pixel base_hsv, alt_hsv; // base and alternate colors in hsv
  pixel hsv; // temp hsv color
  pixel rgb; // final rgb color
  mineral_filter_args *mfargs = (mineral_filter_args *) fargs;
  base_hsv = rgb__hsv(mfargs->base_color);
  alt_hsv = rgb__hsv(mfargs->alt_color);

  salt1 = prng(mfargs->seed - 5);
  salt2 = prng(salt1);
  salt3 = prng(salt2);
  salt4 = prng(salt3);
  salt5 = prng(salt4);
  salt6 = prng(salt5);
  salt7 = prng(salt6);
  salt8 = prng(salt7);

  // Round all scale values so that the scaled coordinate-space texture
  // boundaries are integers (for simplex noise wrapping purposes):
  scx = rounddenom(mfargs->scale*mfargs->squash, tx->width);
  scy = rounddenom(mfargs->scale/mfargs->squash, tx->height);
  wx = (float) tx->width * scx;
  wy = (float) tx->height * scy;

  dscx = rounddenom(mfargs->dscale, tx->width);
  dscy = rounddenom(mfargs->dscale, tx->height);
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
        salt1
      );
      x = (col + ds * mfargs->distortion) * scx;
      y = (row + ds * mfargs->distortion) * scy;
      grit = hash_2d(
        col + fastfloor(x),
        row + fastfloor(y)
      ) / (float) HASH_MASK;
      contours = (
        1 + tiled_func(&sxnoise_2d, x, y, wx, wy, salt2)
      ) / 2.0;
      matrix = sqrtf(
        wrnoise_2d_fancy(
          x, y, salt3,
          fastfloor(wx), fastfloor(wy),
          &dontcare, &dontcare,
          0
        )
      );
      bumps = sqrtf(
        wrnoise_2d_fancy(
          x, y, salt4,
          fastfloor(wx), fastfloor(wy),
          &dontcare, &dontcare,
          WORLEY_FLAG_INCLUDE_NEXTBEST
        )
      );

      wavescale = ((float) tx->width) / roundf(1.0 / mfargs->wavescale);
      lphase = (1.0 + sinf(((float) col) * 2 * M_PI / wavescale)) / 2.0;
      wavescale = ((float) tx->width) / roundf(3.0 / mfargs->wavescale);
      lphase += 0.5 * (1.0 + sinf(((float) col) * 2 * M_PI / wavescale)) / 2.0;
      lphase /= 1.5;

      layerscale = ((float) tx->height) / roundf(1.0 / mfargs->layerscale);
      lphase = (((float) row) + lphase * mfargs->layerwaves) / layerscale;
      lphase -= fastfloor(lphase);
      if (ptrf(salt5) > 0.5) {
        layers = 1 - expdist(lphase, 2.5);
      } else {
        layers = expdist(lphase, 2.5);
      }

      alx = col * alscx;
      aly = row * alscy;
      alternate = wrnoise_2d_fancy(
        alx, aly, salt6,
        fastfloor(wx), fastfloor(wy),
        &dontcare, &dontcare,
        WORLEY_FLAG_INCLUDE_NEXTBEST
      ) * (
        (
          1 + tiled_func(
            &sxnoise_2d,
            alx, aly,
            alwx, alwy,
            salt7
          )
        ) / 2.0
      );
      alternate = sqrtf(alternate);

      // value construction:
      value = (
        mfargs->gritty * grit
      + mfargs->contoured * contours
      + mfargs->porous * matrix
      + mfargs->bumpy * bumps
      ) / (
        mfargs->gritty
      + mfargs->contoured
      + mfargs->porous
      + mfargs->bumpy
      );

      value *= (mfargs->layered) * layers + (1 - mfargs->layered);

      // saturation variance:
      saturation = (
        0.7 + 0.3 * tiled_func(
          &sxnoise_2d,
          x, y,
          wx, wy,
          salt8
        )
      );
      snoise = ptrf(prng(prng(col + fastfloor(x)) + row + fastfloor(y)));
      saturation = (
        (1.0 - mfargs->sat_noise) * saturation
      + mfargs->sat_noise * snoise
      );

      saturation *= (1 - mfargs->desaturate);

      // figure out the base color:
      hsv = base_hsv;
      px_set_sat(&hsv, px_sat(hsv) * saturation);
      px_set_val(&hsv, CHANNEL_MAX * value);
      if (alternate > (1 - mfargs->inclusions)) {
        px_set_hue(&hsv, px_hue(alt_hsv));
        px_set_sat(&hsv, px_sat(alt_hsv));
        px_set_val(&hsv, CHANNEL_MAX * alternate);
      } else if (alternate > (1 - (mfargs->inclusions + 0.05))) {
        px_set_sat(&hsv, 0.5*px_sat(hsv));
        px_set_val(&hsv, (px_val(hsv) + CHANNEL_MAX * alternate)/2.0);
      }
      rgb = hsv__rgb(hsv);
      rgb = px_relight(rgb, mfargs->brightness);

      tx_set_px(tx, rgb, col, row);
    }
  }
}

/*************
 * Functions *
 *************/

texture* gen_dirt_texture(species s) {
  dirt_species* ssp = get_dirt_species(s);
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_mineral(result, &(ssp->appearance));
  return result;
}

texture* gen_sand_texture(species s) {
  stone_species* ssp = get_stone_species(s);
  ptrdiff_t seed = prng(ssp->appearance.seed + 189912);

  // Store original stone texture parameters:
  float old_grit = ssp->appearance.gritty;
  float old_contoured = ssp->appearance.contoured;
  float old_porous = ssp->appearance.porous;
  float old_bumpy = ssp->appearance.bumpy;
  float old_layered = ssp->appearance.layered;
  float old_brightness = ssp->appearance.brightness;

  // Temporarily edit texture parameters:
  ssp->appearance.gritty = randf(seed, 0.7, 1.0);
  seed = prng(seed);
  ssp->appearance.contoured = randf(seed, 0.01, 0.1);
  seed = prng(seed);
  ssp->appearance.porous = randf(seed, 0.05, 0.5);
  seed = prng(seed);
  ssp->appearance.bumpy = randf(seed, 0.01, 0.1);
  seed = prng(seed);
  ssp->appearance.layered = randf(seed, 0.05, 0.5);
  seed = prng(seed);
  ssp->appearance.brightness = randf(seed, 0.1, 0.5);
  seed = prng(seed);

  // Generate our texture:
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_mineral(result, &(ssp->appearance));

  // Restore the old parameters:
  ssp->appearance.gritty = old_grit;
  ssp->appearance.contoured = old_contoured;
  ssp->appearance.porous = old_porous;
  ssp->appearance.bumpy = old_bumpy;
  ssp->appearance.layered = old_layered;
  ssp->appearance.brightness = old_brightness;

  return result;
}

texture* gen_clay_texture(species s) {
  clay_species* ssp = get_clay_species(s);
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_mineral(result, &(ssp->appearance));
  return result;
}

texture* gen_stone_texture(species s) {
  stone_species* ssp = get_stone_species(s);
  texture *result = create_texture(BLOCK_TEXTURE_SIZE, BLOCK_TEXTURE_SIZE);
  fltr_mineral(result, &(ssp->appearance));
  return result;
}
