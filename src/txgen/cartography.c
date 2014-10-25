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

gradient const BW_GRADIENT = {
  .count = 2,
  .colors = {
    0xff000000, // black
    0xffffffff, // white
  },
  .oob_below = 0xff000088, // maroon
  .oob_above = 0xff0000ff, // red
};

gradient const SEA_GRADIENT = {
  .count = 16,
  .colors = {
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
  },
  .oob_below = 0xff0000ff, // red
  .oob_above = 0xff8800ff, // pink
};

gradient const LAND_GRADIENT = {
  .count = 19,
  .colors = {
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
  },
  .oob_below = 0xff66ddff, // light yellow
  .oob_above = 0xffff0088, // purple
};

gradient const RAIN_GRADIENT = {
  .count = 24,
  .colors = {
    0xff88ddff, // 1
    0xff77dddd,
    0xff66cccc,
    0xff55ccbb, // 4
    0xff44cc99,
    0xff33cc77,
    0xff22c466,
    0xff22bb55, // 8
    0xff22b444,
    0xff22aa33,
    0xff22a428,
    0xff229922, // 12
    0xff229018,
    0xff228811,
    0xff228808,
    0xff228800, // 16
    0xff228000,
    0xff227700,
    0xff287000,
    0xff337000, // 20
    0xff226600,
    0xff115500,
    0xff004400,
    0xff004400, // 24
  },
  .oob_below = 0xff0088ff, // orange
  .oob_above = 0xffff0088, // purple
};

gradient const CLOUDS_GRADIENT = {
  .count = 2,
  .colors = {
    0xff883300, // deep blue
    0xffffffff, // "cloud" white
  },
  .oob_below = 0xff000000, // black
  .oob_above = 0xff888888, // gray
};

gradient const TEMPERATURE_GRADIENT = {
  .count = 3,
  .colors = {
    0xff000000, // black
    0xff777777, // grey
    0xff77ffff, // light yellow
  },
  .oob_below = 0xff000000, // black
  .oob_above = 0xff888888, // gray
};

/*************
 * Functions *
 *************/

void render_map_layer(
  world_map *wm,
  texture *tx,
  pixel (*layer_fn)(world_region*)
) {
  size_t row, col;
  float x, y;
  world_map_pos wmpos;
  world_region *wr;
  pixel color;
  for (col = 0; col < tx->width; ++col) {
    for (row = 0; row < tx->height; ++row) {
      x = (col + 0.5) / ((float) tx->width);
      y = (row + 0.5) / ((float) tx->height);
      wmpos.x = (wm_pos_t) ffloor(x * wm->width);
      wmpos.y = (wm_pos_t) ffloor(y * wm->height);
      wr = get_world_region(wm, &wmpos);
      if (wr == NULL) {
        // out-of-bounds shouldn't be possible
        color = 0xff0088ff; // orange
      } else {
        color = layer_fn(wr);
      }
      tx_set_px(tx, color, col, row);
    }
  }
}

void render_map_vectors(
  world_map *wm,
  texture *tx,
  pixel start, pixel end,
  void (*vector_layer_fn)(world_region*, float*, float*)
) {
  size_t row, col;
  float x, y;
  float r, theta;
  world_map_pos wmpos;
  world_region *wr;
  vector from, to;
  from.z = 0;
  to.z = 0;
  for (col = 0; col < tx->width; col += CART_VECTOR_SPACING) {
    for (row = 0; row < tx->height; row += CART_VECTOR_SPACING) {
      x = (col + 0.5) / ((float) tx->width);
      y = (row + 0.5) / ((float) tx->height);
      wmpos.x = (wm_pos_t) ffloor(x * wm->width);
      wmpos.y = (wm_pos_t) ffloor(y * wm->height);
      wr = get_world_region(wm, &wmpos);
      if (wr != NULL) {
        vector_layer_fn(wr, &r, &theta);
        from.x = col;
        from.y = row;
        to.x = from.x + CART_MAX_VECTOR_LENGTH * r * cos(theta);
        to.y = from.y + CART_MAX_VECTOR_LENGTH * r * sin(theta);
        draw_line_gradient(tx, &from, &to, start, end);
      }
    }
  }
}


/*******************
 * Layer Functions *
 *******************/

pixel ly_terrain_height(world_region *wr) {
  float h;
  if (wr->climate.water.body != NULL) {
    // draw water depth
    h = (
      (wr->climate.water.body->level - wr->min_height)
    /
      (float) (TR_HEIGHT_SEA_LEVEL)
    );
    if (h < 0) {
      h = 0;
    }
    h = 1 - h;
    return gradient_result(&SEA_GRADIENT, h);
  } else {
    // draw land elevation
    h = (
      (wr->mean_height - TR_HEIGHT_SEA_LEVEL)
    /
      (float) (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL)
    );
    return gradient_result(&LAND_GRADIENT, h);
  }
}

pixel ly_temperature(world_region *wr) {
  float t = (
    (wr->climate.atmosphere.mean_temp - TEMP_LOW)
  /
    (TEMP_HIGH - TEMP_LOW)
  );
  return gradient_result(&TEMPERATURE_GRADIENT, t);
}

pixel ly_evaporation(world_region *wr) {
  float t = evaporation(wr) / HUGE_CLOUD_POTENTIAL;
  return gradient_result(&CLOUDS_GRADIENT, t);
}

pixel ly_cloud_cover(world_region *wr) {
  float t = wr->climate.atmosphere.cloud_potential / HUGE_CLOUD_POTENTIAL;
  return gradient_result(&CLOUDS_GRADIENT, t);
}

pixel ly_precipitation_quotient(world_region *wr) {
  float t = wr->climate.atmosphere.precipitation_quotient;
  return gradient_result(&BW_GRADIENT, t);
}

pixel ly_precipitation(world_region *wr) {
  float t = wr->climate.atmosphere.cloud_potential;
  t *= wr->climate.atmosphere.precipitation_quotient;
  t /= HUGE_CLOUD_POTENTIAL;
  t *= 12; // TODO: GET RID OF THIS!
  return gradient_result(&RAIN_GRADIENT, t);
}

void vly_wind_vectors(world_region *wr, float *r, float *theta) {
  *r = wr->climate.atmosphere.wind_strength / WIND_UPPER_STRENGTH;
  *theta = wr->climate.atmosphere.wind_direction;
}
