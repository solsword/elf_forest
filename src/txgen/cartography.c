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
    0xff440000, // navy
    0xffaa4499, // lavender
    0xffffffff, // white
  },
  .oob_below = 0xff000000, // black
  .oob_above = 0xff888888, // gray
};

gradient_map const GEOREGIONS_GRADIENT = {
  .colors = {
    0xff440000, // navy
    0xff991100, // dark blue
    0xffffaaaa, // light blue
    0xff006600, // dark green (only visible if smooth is used)
    0xff229911, // mid green
    0xff33cc44, // light green
    0xff99ffff, // light yellow
    0xffcccccc, // gray
    0xffffffff, // white
    0xff0000ff  // red
  },
  .thresholds = {
    TR_HEIGHT_OCEAN_DEPTHS,       // navy
    TR_HEIGHT_CONTINENTAL_SHELF,  // dark blue
    TR_HEIGHT_SEA_LEVEL,          // light blue
    TR_HEIGHT_SEA_LEVEL + 1,      // dark green (only visible if smooth is used)
    TR_HEIGHT_COASTAL_PLAINS,     // mid green
    TR_HEIGHT_HIGHLANDS,          // light green
    TR_HEIGHT_MOUNTAIN_BASES,     // light yellow
    TR_HEIGHT_MOUNTAIN_TOPS,      // gray
    TR_MAX_HEIGHT,                // white
    2 * TR_MAX_HEIGHT             // red
  }
};

gradient_map const PRECIPITATION_GRADIENT = {
  .colors = {
    0xff66bbff,  // orange
    0xff55ffff,  // yellow
    0xff00ff33,  // lime green
    0xff22bb00,  // green
    0xffaa9900,  // teal
    0xffff4422,  // light blue
    0xff990000,  // dark blue
    0xff440000,  // navy
    0xff330033,  // deep purple
    0xff000000   // black
  },
  .thresholds = {
    250,   // orange (desert)
    500,   // yellow (dry)
    1000,  // lime green (moderate)
    1500,  // green (wet)
    2000,  // teal (drenched)
    2500,  // light blue (pouring)
    3000,  // dark blue (constant rain)
    3500,  // navy (flooded)
    4000,  // deep purple (off-the-charts)
    16000  // black (okay, actually off-the-charts)
  }
};

pixel const RIVER_COLOR = 0xffdd9955;

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

void render_heightmap(
  heightmap *hm,
  texture *tx,
  uint8_t use_color
) {
  size_t row, col;
  float x, y;
  size_t hx, hy;
  pixel color;
  for (col = 0; col < tx->width; ++col) {
    for (row = 0; row < tx->height; ++row) {
      x = (col + 0.5) / ((float) tx->width);
      y = (row + 0.5) / ((float) tx->height);
      hx = ffloor(x * hm->width);
      hy = ffloor(y * hm->height);
      if (use_color) {
        color = gradient_result(&LAND_GRADIENT, hm_height(hm, hx, hy));
      } else {
        color = gradient_result(&BW_GRADIENT, hm_height(hm, hx, hy));
      }
      tx_set_px(tx, color, col, row);
    }
  }
}


/*******************
 * Layer Functions *
 *******************/

pixel ly_terrain_height(world_region *wr) {
  float h;
  if (wr->climate.water.rivers[0] != NULL) {
    // draw rivers
    // DEBUG:
    return RIVER_COLOR;
  } else if (wr->climate.water.body != NULL) {
    // draw water depth
    h = (
      (wr->climate.water.body->level - wr->topography.terrain_height.z)
    /
      (float) (TR_HEIGHT_SEA_LEVEL)
    );
    if (h < 0) {
      h = 0;
    }
    h = 1 - h;
    pixel result = gradient_result(&SEA_GRADIENT, h);
    if (
       wr->climate.water.state == WM_HS_OCEAN_SHORE
    || wr->climate.water.state == WM_HS_LAKE_SHORE
    ) {
      // shores are a bit greener and a bit darker
      pixel hsv;
      hsv = rgb__hsv(result);
      channel hue = px_hue(hsv);
      channel val = px_val(hsv);
      hue += SHORE_HUE_ADJUST;
      if (hue < 0) { hue = 0; }
      else if (hue > CHANNEL_MAX) { hue -= CHANNEL_MAX; }
      val += SHORE_VAL_ADJUST;
      if (val < 0) { val = 0; }
      else if (val > CHANNEL_MAX) { val = CHANNEL_MAX; }
      px_set_hue(&hsv, hue);
      px_set_val(&hsv, val);
      result = hsv__rgb(hsv);
    }
    return result;
  } else {
    // draw land elevation
    h = (
      (wr->topography.terrain_height.z - TR_HEIGHT_SEA_LEVEL)
    /
      (float) (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL)
    );
    //return gradient_result(&LAND_GRADIENT, h);
    // TODO: Clean this up!
    return gradient_map_result(
      &GEOREGIONS_GRADIENT,
      wr->topography.terrain_height.z
    );
  }
}

pixel ly_georegions(world_region *wr) {
  return gradient_map_result_sharp(
    &GEOREGIONS_GRADIENT,
    wr->topography.terrain_height.z
  );
}

pixel ly_temperature(world_region *wr) {
  float t = (
    (wr->climate.atmosphere.mean_temp - CL_TEMP_LOW)
  /
    (CL_TEMP_HIGH - CL_TEMP_LOW)
  );
  if (t < 0) { t = 0; }
  if (t > 1) { t = 1.0; }
  return gradient_result(&TEMPERATURE_GRADIENT, t);
}

pixel ly_evaporation(world_region *wr) {
  float t = evaporation(wr) / CL_HUGE_CLOUD_POTENTIAL;
  if (t < 0) { t = 0; }
  if (t > 1) { t = 1.0; }
  return gradient_result(&CLOUDS_GRADIENT, t);
}

pixel ly_cloud_cover(world_region *wr) {
  float t = wr->climate.atmosphere.cloud_potential / CL_HUGE_CLOUD_POTENTIAL;
  if (t < 0) { t = 0; }
  if (t > 1) { t = 1.0; }
  return gradient_result(&CLOUDS_GRADIENT, t);
}

pixel ly_precipitation_quotient(world_region *wr) {
  float t = wr->climate.atmosphere.precipitation_quotient;
  if (t < 0) { t = 0; }
  if (t > 1) { t = 1.0; }
  return gradient_result(&BW_GRADIENT, t);
}

pixel ly_precipitation(world_region *wr) {
  //float t = wr->climate.atmosphere.total_precipitation;
  //t /= CL_HUGE_CLOUD_POTENTIAL;
  //return gradient_result(&BW_GRADIENT, t);
  return gradient_map_result(
    &PRECIPITATION_GRADIENT,
    wr->climate.atmosphere.total_precipitation
  );
}

pixel ly_land_precipitation(world_region *wr) {
  pixel rain = gradient_map_result(
    &PRECIPITATION_GRADIENT,
    wr->climate.atmosphere.total_precipitation
  );
  if (wr->climate.water.body == NULL) {
    return rain;
  } else {
    return 0xff000000;
  }
}

void vly_wind_vectors(world_region *wr, float *r, float *theta) {
  *r = wr->climate.atmosphere.wind_strength / CL_WIND_UPPER_STRENGTH;
  *theta = wr->climate.atmosphere.wind_direction;
}
