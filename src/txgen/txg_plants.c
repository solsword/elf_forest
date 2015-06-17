// txg_plants.c
// Plant texture generation.

#include "world/blocks.h"
#include "world/world.h"
#include "noise/noise.h"
#include "tex/tex.h"
#include "tex/draw.h"
#include "math/curve.h"

#include "util.h"

#include "txgen.h"

#include "txg_plants.h"

/*************
 * Constants *
 *************/

size_t const LEAF_TEXTURE_SIZE = 16;

float const STEM_WIDTH_SHARPNESS = 5.0;

float const MAX_BULB_SPREAD = 0.9;

leaf_width_func leaf_width_functions_table[N_LEAF_SHAPES] = {
  &needle_width_func, // needle-shaped; just a line
  &oval_width_func, // circular
  NULL, // diamond-shaped
  NULL, // rounded at the base but tapers to a point
  NULL, // fan-shaped
  NULL, // egg-shaped w/ wide base
  NULL, // egg-shaped w/ narrow base
  NULL, // triangular with lobes at the base
  NULL, // spoon-shaped
  NULL, // pointed at both ends
  NULL, // parallel margins for much of length
  &deltoid_width_func, // simple triangular
  NULL, // fish-shaped w/ stem as tail
};

/*************
 * Functions *
 *************/

void draw_leaf(texture *tx, curve *c, leaf_filter_args *lfargs) {
  width_func_args wfargs;
  wfargs.stem_length = lfargs->stem_length;
  wfargs.base_width = lfargs->width;
  draw_thick_curve(
    tx,
    c,
    lfargs->main_color,
    lfargs->dark_color,
    (void *) &wfargs,
    leaf_width_functions_table[lfargs->shape]
  );
  draw_curve(tx, c, lfargs->vein_color);
  // TODO: secondary veins
}

/*******************
 * Width Functions *
 *******************/

float stem_width_func(float t, void *args) {
  width_func_args *wfargs = (width_func_args *) args;
  float start = wfargs->stem_length;
  float end = wfargs->stem_length;
  start -= wfargs->stem_length / STEM_WIDTH_SHARPNESS;
  end += wfargs->stem_length / STEM_WIDTH_SHARPNESS;
  return sigmoid(t, start, end);
}

float needle_width_func(float t, void *args) {
  return 0;
}

float oval_width_func(float t, void *args) {
  width_func_args *wfargs = (width_func_args *) args;
  float v = 0;
  float tprime = (t - wfargs->stem_length) / (1 - wfargs->stem_length);
  if (0 < tprime && tprime <= 0.5) {
    v = sqrtf(0.25 - (0.5 - tprime) * (0.5 - tprime));
  } else if (tprime < 1) {
    v = sqrtf(0.25 - (tprime - 0.5) * (tprime - 0.5));
  }
  return stem_width_func(t, args) * wfargs->base_width * v;
}

float deltoid_width_func(float t, void *args) {
  width_func_args *wfargs = (width_func_args *) args;
  return (1 - t) * wfargs->base_width * stem_width_func(t, args);
}

/******************
 * Filter Helpers *
 ******************/

// Private helper function for fltr_leaves w/ its own argument structure.
struct leaves_helper_args_s {
  texture *tx;
  texture *leaf;
  leaf_filter_args *lfargs;
};
void fltr_leaves_helper(int x, int y, void * arg) {
  struct leaves_helper_args_s *lhargs = (struct leaves_helper_args_s *) arg;
  // Scramble the leaf filter seed:
  lhargs->lfargs->seed = prng(lhargs->lfargs->seed * x * y);
  // Render a single randomized leaf into the leaf texture:
  fltr_leaf(lhargs->leaf, lhargs->lfargs);
  // Draw the fresh leaf onto the main texture at the given position:
  tx_draw_wrapped(lhargs->tx, lhargs->leaf, x, y);
}

// Private helper functions for fltr_branches: underlying manifold functions
// for different branch directions.
float branches_direction_up(float x, float y, ptrdiff_t seed) { return -y; }
float branches_direction_down(float x, float y, ptrdiff_t seed) { return y; }
float branches_direction_outwards(float x, float y, ptrdiff_t seed) {
  // TODO: What if the texture isn't 32x32?
  float d = (x - 16)*(x - 16) + (y - 16 * y - 16);
  return -d;
}
float branches_direction_inwards(float x, float y, ptrdiff_t seed) {
  // TODO: What if the texture isn't 32x32?
  float d = (x - 16)*(x - 16) + (y - 16 * y - 16);
  return d;
}
float branches_direction_random(float x, float y, ptrdiff_t seed) {
  // TODO: Properly control scaling...
  return sxnoise_2d(x*1/16.0, y*1/16.0, seed);
}


/********************
 * Filter Functions *
 ********************/

void fltr_branches(texture *tx, void const * const fargs) {
  int row, col;
  float dx, dy, x, y;
  float noise;
  float dontcare;
  ptrdiff_t salt1, salt2, salt3;
  float (*dirfunc)(float, float, ptrdiff_t);
  gradient_map grmap;
  branch_filter_args *bfargs = (branch_filter_args *) fargs;
  salt1 = prng(bfargs->seed+73);
  salt2 = prng(salt1);
  salt3 = prng(salt2);
  if (bfargs->gnarled) {
    grmap.colors[0] = PX_EMPTY;
    grmap.colors[1] = bfargs->outer_color;
    grmap.colors[2] = bfargs->mid_color;
    grmap.colors[3] = bfargs->center_color;
    grmap.colors[4] = bfargs->mid_color;
    grmap.colors[5] = bfargs->outer_color;
    grmap.colors[6] = PX_EMPTY;
#ifdef DEBUG
    // Use orange for out-of-range noise results.
    grmap.colors[7] = 0xff0088ff;
#else
    // Ignore out-of-range results (make them transparent).
    grmap.colors[7] = 0x00000000;
#endif
    grmap.thresholds[0] = 0.08;
    grmap.thresholds[1] = 0.1;
    grmap.thresholds[2] = 0.15;
    grmap.thresholds[3] = 0.22;
    grmap.thresholds[4] = 0.27;
    grmap.thresholds[5] = 0.31;
    grmap.thresholds[6] = 1.0;
    grmap.thresholds[7] = 1000.0;
  } else {
    grmap.colors[0] = bfargs->center_color;
    grmap.colors[1] = bfargs->mid_color;
    grmap.colors[2] = bfargs->outer_color;
    grmap.colors[3] = PX_EMPTY;
#ifdef DEBUG
    // Use orange for out-of-range noise results.
    grmap.colors[4] = 0xff0088ff;
#else
    // Ignore out-of-range results (make them transparent).
    grmap.colors[4] = 0x00000000;
#endif
    grmap.thresholds[0] = 0.07;
    grmap.thresholds[1] = 0.15;
    grmap.thresholds[2] = 0.22;
    grmap.thresholds[3] = 1.0;
    grmap.thresholds[4] = 1000.0;
  }
  switch (bfargs->direction) {
    case 0:
      dirfunc = &branches_direction_up;
      break;
    case 1:
      dirfunc = &branches_direction_down;
      break;
    case 2:
      dirfunc = &branches_direction_outwards;
      break;
    case 3:
      dirfunc = &branches_direction_inwards;
      break;
    default:
    case 4:
      dirfunc = &branches_direction_random;
      break;
  }
  for (col = 0; col < tx->width; col += 1) {
    for (row = 0; row < tx->height; row += 1) {
      // TODO: consider wrapping?
      // TODO: wrap w/ squash taken into account
      dx = tiled_func(
        &sxnoise_2d,
        col * bfargs->dscale,
        row * bfargs->dscale,
        tx->width * bfargs->dscale,
        tx->height * bfargs->dscale,
        salt1
      );
      dy = tiled_func(
        &sxnoise_2d,
        col * bfargs->dscale,
        row * bfargs->dscale,
        tx->width * bfargs->dscale,
        tx->height * bfargs->dscale,
        salt2
      );
      x = (col * bfargs->squash + dx * bfargs->distortion) * bfargs->scale;
      y = (row / bfargs->squash + dy * bfargs->distortion) * bfargs->scale;
      if (bfargs->gnarled) {
        noise = wrnoise_2d_fancy(
          x, y, salt3,
          tx->width * bfargs->scale, tx->height * bfargs->scale,
          &dontcare, &dontcare,
          0
        );
      } else {
        noise = dnnoise_2d(
          x, y,
          salt3,
          dirfunc,
          salt3
        );
      }
      if (noise > 1) { noise = 1; }
      if (bfargs->gnarled) {
        noise = smooth(noise, -bfargs->width, 0.185);
      } else {
        noise = smooth(noise, -bfargs->width, 0.05);
      }
      tx_set_px(
        tx,
        gradient_map_result_sharp(&grmap, noise),
        col,
        row
      );
    }
  }
}

void fltr_leaf(texture *tx, void const * const fargs) {
  leaf_filter_args *lfargs = (leaf_filter_args *) fargs;
  ptrdiff_t hash = hash_1d(lfargs->seed);
  float noise = 0;
  float angle = 0, bend = 0;
  float mid = LEAF_TEXTURE_SIZE / 2.0;
  curve c;
  c.from.x = 0; c.from.y = 0; c.from.z = 0;
  c.go_towards.x = 0; c.go_towards.y = 0; c.go_towards.z = 0;
  c.come_from.x = 0; c.come_from.y = 0; c.come_from.z = 0;
  c.to.x = 0; c.to.y = 0; c.to.z = 0;
  tx_clear(tx);
  if (lfargs->type == LT_SIMPLE) {
    // pick an angle in [-1, 1] * lfargs->angle_var (0 will be straight down)
    noise = -1 + 2.0 * float_hash_1d((hash + lfargs->seed)*lfargs->width);
    angle = lfargs->angle + lfargs->angle_var * noise;

    // the leaf starts opposite that angle:
    c.from.x = mid + (mid - 1) * sinf(angle + M_PI);
    c.from.y = mid + (mid - 1) * cosf(angle + M_PI);

    // its midpoint is in the direction of the given angle:
    c.go_towards.x = c.from.x + (lfargs->length / 2.0) * sinf(angle);
    c.go_towards.y = c.from.y + (lfargs->length / 2.0) * cosf(angle);
    // make sure we're still inside the texture:
    if (c.go_towards.x < 1) {
      c.go_towards.x = 1;
    } else if (c.go_towards.x > LEAF_TEXTURE_SIZE - 2) {
      c.go_towards.x = LEAF_TEXTURE_SIZE - 2;
    }

    // its endpoint bends:
    noise = 0.7 + 0.6 * float_hash_1d((hash + 2)*noise); // [0.7, 1.3]
    bend = lfargs->bend * noise;
    if ((angle > 0 && angle < M_PI) || (angle < -M_PI) ) {
      bend = angle - bend;
    } else {
      bend = angle + bend;
    }
    c.to.x = c.go_towards.x + (lfargs->length / 2.0) * sinf(bend);
    c.to.y = c.go_towards.y + (lfargs->length / 2.0) * cosf(bend);
    // check bounds again:
    if (c.to.x < 1) {
      c.go_towards.x += (1 - c.to.x) / 2.0;
      c.to.x = 1;
    } else if (c.to.x > LEAF_TEXTURE_SIZE - 2) {
      c.go_towards.x -= (c.to.x - (LEAF_TEXTURE_SIZE - 2)) / 2.0;
      c.to.x = LEAF_TEXTURE_SIZE - 2;
    }

    // copy the go_towards point as the come_from point:
    vcopy_as(&c.come_from, &c.go_towards);

    // draw the leaf:
    draw_leaf(tx, &c, lfargs);
  } else if (lfargs->type == LT_TRIPARTITE) {
    // TODO: HERE!
  } else if (lfargs->type == LT_NEEDLES) {
    // TODO: HERE!
  }
#ifdef DEBUG
  else {
    fprintf(stderr, "Bad leaf filter type %d!\n", lfargs->type);
  }
#endif
}

void fltr_leaves(texture *tx, void const * const fargs) {
  leaves_filter_args *lfargs = (leaves_filter_args *) fargs;
  // Manually set up a texture on the stack to generate leaves into:
  pixel pixels[LEAF_TEXTURE_SIZE*LEAF_TEXTURE_SIZE];
  texture one_leaf;
  one_leaf.width = LEAF_TEXTURE_SIZE;
  one_leaf.height = LEAF_TEXTURE_SIZE;
  one_leaf.pixels = pixels;
  tx_clear(&one_leaf);

  // Set up arguments for our helper function:
  struct leaves_helper_args_s lhargs;
  lhargs.tx = tx;
  lhargs.leaf = &one_leaf;
  lhargs.lfargs = &(lfargs->leaf_args);

  tx_clear(tx);
  ateach_scattered(
    lfargs->seed,
    lfargs->x_spacing, lfargs->y_spacing,
    lfargs->x_spacing/3, lfargs->y_spacing/3,
    0, BLOCK_TEXTURE_SIZE,
    0, BLOCK_TEXTURE_SIZE,
    &lhargs,
    &fltr_leaves_helper
  );
}

void fltr_bulb_leaves(texture *tx, void const * const fargs) {
  bulb_leaves_filter_args *blfargs = (bulb_leaves_filter_args *) fargs;
  ptrdiff_t hash = hash_1d(blfargs->seed);
  int i = 0;
  float noise = 0.7 + 0.6 * float_hash_1d(hash); // [0.7, 1.3]
  int nstalks = (int) (blfargs->count * noise + 0.5);
  if (nstalks < 1) {
    nstalks = 1;
  }
  curve c; // stores individual leaf shapes
  c.from.x = BLOCK_TEXTURE_SIZE / 2.0;
  c.from.y = BLOCK_TEXTURE_SIZE;
  c.from.z = 0;
  c.go_towards.x = 0; c.go_towards.y = 0; c.go_towards.z = 0;
  c.come_from.x = 0; c.come_from.y = 0; c.come_from.z = 0;
  c.to.x = 0; c.to.y = 0; c.to.z = 0;
  leaf_filter_args lfargs; // stores individual leaf parameters
  lfargs.main_color = blfargs->main_color;
  lfargs.vein_color = blfargs->vein_color;
  lfargs.dark_color = blfargs->dark_color;
  lfargs.width = 1;
  lfargs.stem_length = 0;
  lfargs.shape = LS_DELTOID;
  float th_base = 0, th_mid = 0;
  float mid = (BLOCK_TEXTURE_SIZE / 2);
  noise = 0.75 + 0.5 * float_hash_1d(hash); // [0.75, 1.25]
  float spread = blfargs->spread * noise;
  if (spread > MAX_BULB_SPREAD) {
    spread = MAX_BULB_SPREAD;
  }
  spread *= BLOCK_TEXTURE_SIZE / 2.0;
  for (i = 0; i < nstalks; ++i) {
    // generate x in [mid - spread, mid + spread]
    c.from.x = mid - spread;
    c.from.x += 2 * spread * float_hash_1d(spread * hash * c.from.x + i);
    // pick a starting angle in [-1, 1]
    noise = 1.0 - 2 * float_hash_1d(c.from.x*noise*hash + i*c.from.x);
    th_base = blfargs->angle * noise;
    // pick a bend angle in [0.6, 1.4] of the specified angle
    noise = 0.6 + 0.8 * float_hash_1d(c.from.x*noise*hash + i*c.from.x);
    th_mid = blfargs->bend * noise * th_base / blfargs->angle;
    th_mid += th_base;
    // compute midpiont and endpoint
    c.go_towards.x = c.from.x + sinf(th_base) * blfargs->length*blfargs->shape;
    c.go_towards.y = c.from.y - cosf(th_base) * blfargs->length*blfargs->shape;
    c.to.x = c.go_towards.x + sinf(th_mid) * blfargs->length*(1-blfargs->shape);
    c.to.y = c.go_towards.y - cosf(th_mid) * blfargs->length*(1-blfargs->shape);
    // keep things in-bounds
    /*
    if (c.to.x < 1) {
      c.go_towards.x += (1 - c.to.x) / 2.0;
      c.to.x = 1;
    } else if (c.to.x > BLOCK_TEXTURE_SIZE - 2) {
      c.go_towards.x -= (c.to.x - (BLOCK_TEXTURE_SIZE - 2)) / 2.0;
      c.to.x = BLOCK_TEXTURE_SIZE - 2;
    }
    if (c.to.y < 1) {
      c.go_towards.y += (1 - c.to.y) / 2.0;
      c.to.y = 1;
    }
    // */
    // Copy the come_from vector from the go_towards vector:
    vcopy_as(&c.come_from, &c.go_towards);
    /* DEBUG:
    printf("th_base: %.3f\n", th_base);
    printf("th_mid: %.3f\n", th_mid);
    printf("th_mid_atten: %.3f\n", th_base / blfargs->angle);
    printf("th_mid_roll: %.3f\n", noise);
    printf("xy_mid: %.2f, %.2f\n", c.go_towards.x, c.go_towards.y);
    printf("xy_end: %.2f, %.2f\n", c.to.x, c.to.y);
    // */
    // pick a base width in [0.75, 1.25]
    noise = 0.75 + 0.5 * float_hash_1d(c.from.x * noise * hash);
    lfargs.width = blfargs->width * noise;

    // draw the leaf:
    draw_leaf(tx, &c, &lfargs);
  }
}
