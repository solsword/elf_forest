// cartography.c
// Map drawing functions.

#include "tex/tex.h"
#include "tex/draw.h"
#include "gen/worldgen.h"
#include "gen/terrain.h"
#include "math/manifold.h"

#include "cartography.h"

/*************
 * Constants *
 *************/

pixel const SEA_COLORS[EC_SEA_COLORS] = {
  0xff882200, // 1
  0xff992211,
  0xff993311,
  0xffaa3322, // 4
  0xffaa4433,
  0xffbb5544,
  0xffcc5555,
  0xffdd6655, // 8
  0xffee6666,
  0xffff7766,
  0xffff7777,
  0xffff8866, // 12
  0xffff9966,
  0xffffaa77,
  0xffffbb88,
  0xffffcc88, // 16
};

pixel const LAND_COLORS[EC_LAND_COLORS] = {
  0xff004400, // 1
  0xff005500,
  0xff006600,
  0xff007700, // 4
  0xff008811,
  0xff119922,
  0xff22aa33,
  0xff33bb44, // 8
  0xff44cc66,
  0xff55dd88, // last green
  0xff66dddd,
  0xff77eeee, // 12
  0xff99ffff,
  0xffaaeeee, // last yellow
  0xffccdddd,
  0xffdddddd, // 16
  0xffeeeeee,
  0xffffffff, // white
  0xffffffff,
};

/*************
 * Functions *
 *************/

float world_map_height(world_map *wm, float x, float y) {
  world_map_pos wmpos;
  world_region *wr;
  wmpos.x = (wm_pos_t) ffloor(x * wm->width);
  wmpos.y = (wm_pos_t) ffloor(y * wm->height);
  wr = get_world_region(THE_WORLD, &wmpos);
  if (wr == NULL) { // out-of-bounds
    return 0;
  }
  return wr->mean_height;
}

float water_depth(world_map *wm, float x, float y) {
  world_map_pos wmpos;
  world_region *wr;
  wmpos.x = (wm_pos_t) ffloor(x * wm->width);
  wmpos.y = (wm_pos_t) ffloor(y * wm->height);
  wr = get_world_region(THE_WORLD, &wmpos);
  if (wr == NULL) { // out-of-bounds
    return 0;
  }
  if (wr->climate.water.body == NULL) {
    return 0;
  }
  // DEBUG:
  // return 0;
  return wr->climate.water.body->level - wr->min_height;
}

void render_map(world_map *wm, texture *tx) {
  size_t row, col;
  float x, y;
  float h, wd, hf;
  float cinterp;
  size_t i;
  pixel color;
  for (col = 0; col < tx->width; ++col) {
    for (row = 0; row < tx->height; ++row) {
      x = col / ((float) tx->width);
      y = row / ((float) tx->height);
      h = world_map_height(wm, x, y);
      wd = water_depth(wm, x, y);
      //hf = h / (float) (TR_MAX_HEIGHT);
      //hf *= 1.3;
      //color = float_color(hf, hf, hf, 1);
      //tx_set_px(tx, color, col, row);
      //*
      if (wd > 0) {
        hf = 1 - (wd / (float) (TR_HEIGHT_SEA_LEVEL));
        i = (size_t) (hf*(EC_SEA_COLORS - 1));
        cinterp = (hf*(EC_SEA_COLORS - 1)) - i;
        if (hf < 0 || hf > 1) {
          tx_set_px(tx, 0xff0088ff, col, row);
        } else {
          color = SEA_COLORS[i];
          px_set_red(
            &color,
            (1 - cinterp)*px_red(color) + cinterp*px_red(SEA_COLORS[i+1])
          );
          px_set_green(
            &color,
            (1 - cinterp)*px_green(color) + cinterp*px_green(SEA_COLORS[i+1])
          );
          px_set_blue(
            &color,
            (1 - cinterp)*px_blue(color) + cinterp*px_blue(SEA_COLORS[i+1])
          );
          tx_set_px(tx, color, col, row);
        }
      } else {
        hf = (h - TR_HEIGHT_SEA_LEVEL) /
             (float) (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL);
        i = (size_t) (hf*(EC_LAND_COLORS - 1));
        cinterp = (hf*(EC_LAND_COLORS - 1)) - i;
        if (hf < 0 || hf > 1) {
          tx_set_px(tx, 0xff8800ff, col, row);
        } else {
          color = LAND_COLORS[i];
          px_set_red(
            &color,
            (1 - cinterp)*px_red(color) + cinterp*px_red(LAND_COLORS[i+1])
          );
          px_set_green(
            &color,
            (1 - cinterp)*px_green(color) + cinterp*px_green(LAND_COLORS[i+1])
          );
          px_set_blue(
            &color,
            (1 - cinterp)*px_blue(color) + cinterp*px_blue(LAND_COLORS[i+1])
          );
          tx_set_px(tx, color, col, row);
        }
      }
      // */
    }
  }
  // DEBUG:
  // Draw wind/downhill vectors:
  /*
  region_pos rpos;
  world_map_pos wmpos;
  manifold_point gross, dontcare;
  float r, theta;
  vector from, to;
  world_region *wr;
  color = 0xff000000; // black
  pixel end_color = 0xffffffff; // white
  for (col = 0; col < tx->width; col += 8) {
    for (row = 0; row < tx->height; row += 8) {
      x = col / ((float) tx->width);
      y = row / ((float) tx->height);
      rpos.x = WORLD_REGION_SIZE * CHUNK_SIZE * wm->width * x;
      rpos.y = WORLD_REGION_SIZE * CHUNK_SIZE * wm->height * y;
      rpos__wmpos(&rpos, &wmpos);
      // Downhill direction
      // compute_terrain_height(&rpos, &gross, &dontcare, &dontcare);
      // r = 4;
      // theta = mani_downhill(&gross);
      // Wind vector
      wr = get_world_region(wm, &wmpos);
      r = wr->climate.atmosphere.wind_strength;
      theta = wr->climate.atmosphere.wind_direction;
      from.x = col;
      from.y = row;
      to.x = from.x + r * cos(theta);
      to.y = from.y + r * sin(theta);
      draw_line_gradient(tx, &from, &to, color, end_color);
    }
  }
  // */
}
