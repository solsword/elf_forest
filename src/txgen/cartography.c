// cartography.c
// Map drawing functions.

#include "tex/tex.h"
#include "gen/worldgen.h"
#include "gen/terrain.h"

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
};

pixel const LAND_COLORS[EC_LAND_COLORS] = {
  0xff007700, // 1
  0xff008800,
  0xff119911,
  0xff22aa22, // 4
  0xff33bb33,
  0xff44cc44,
  0xff55cc55,
  0xff55cc66, // 8
  0xff55dd77,
  0xff66ee99,
  0xff66eeaa,
  0xff66ddcc, // 12
  0xff66dddd,
  0xff77eeee,
  0xff77eeee,
  0xff88ffff, // 16
  0xff99ffff,
  0xffaaeeee,
  0xffbbdddd,
  0xffccdddd, // 20
  0xffddeeee,
  0xffeeffff,
  0xffffffff, // 23
};

/*************
 * Functions *
 *************/

r_pos_t world_map_height(world_map *wm, float x, float y) {
  world_map_pos wmpos;
  world_region *wr;
  wmpos.x = (wm_pos_t) ffloor(x * wm->width);
  wmpos.y = (wm_pos_t) ffloor(y * wm->height);
  wr = get_world_region(THE_WORLD, &wmpos);
  if (wr == NULL) { // out-of-bounds
    return 0;
  }
  return wr->terrain_height;
}

void render_map(world_map *wm, texture *tx) {
  size_t row, col;
  float x, y;
  r_pos_t h;
  float hf;
  size_t i;
  region_pos test;
  for (col = 0; col < tx->width; ++col) {
    for (row = 0; row < tx->height; ++row) {
      x = col / ((float) tx->width);
      y = row / ((float) tx->height);
      test.x = (r_pos_t) (WORLD_WIDTH * WORLD_REGION_SIZE * CHUNK_SIZE * x);
      test.y = (r_pos_t) (WORLD_WIDTH * WORLD_REGION_SIZE * CHUNK_SIZE * y);
      test.z = 0;
      h = terrain_height(&test);
      //h = world_map_height(wm, x, y);
      if (h < TR_HEIGHT_SEA_LEVEL) {
        hf = h / (float) (TR_HEIGHT_SEA_LEVEL);
        i = (size_t) (hf*EC_SEA_COLORS);
        if (hf < 0 || hf > 1) {
          tx_set_px(tx, 0xff0088ff, col, row);
        } else {
          tx_set_px(tx, SEA_COLORS[i], col, row);
        }
      } else {
        hf = (h - TR_HEIGHT_SEA_LEVEL) /
             (float) (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL);
        i = (size_t) (hf*EC_LAND_COLORS);
        if (hf < 0 || hf > 1) {
          tx_set_px(tx, 0xff8800ff, col, row);
        }
        tx_set_px(tx, LAND_COLORS[i], col, row);
      }
    }
  }
}
