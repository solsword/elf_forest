// plants.c
// Plant generation.

#include "world/blocks.h"
#include "world/world.h"
#include "noise/noise.h"
#include "tex/tex.h"
#include "tex/curve.h"
#include "tex/draw.h"

#include "txgen.h"

#include "plants.h"

/*************
 * Constants *
 *************/

int const SMALL_LEAF_MAX_HEIGHT = 7;
int const SMALL_LEAF_MAX_WIDTH = 5;
size_t const SMALL_LEAF_DEFAULT_BREADTH = 5;

float const MAX_BULB_SPREAD = 0.9;

/*************
 * Functions *
 *************/

float bulb_width_func(float t, void *args) {
  size_t base_width = (size_t) args;
  // TODO: something more interesting here?
  return (1 - t) * ((float) base_width);
}

void draw_bulb_leaf(
  texture *tx,
  size_t base_width,
  pixel main_color,
  pixel vein_color,
  pixel shade_color,
  float frx, float fry,
  float twx, float twy,
  float cfx, float cfy,
  float tox, float toy
) {
  curve c;
  c.from.x = frx; c.from.y = fry; c.from.z = 0;
  c.go_towards.x = twx; c.go_towards.y = twy; c.go_towards.z = 0;
  c.come_from.x = cfx; c.come_from.y = cfy; c.come_from.z = 0;
  c.to.x = tox; c.to.y = toy; c.to.z = 0;
  draw_thick_curve(
    tx,
    &c,
    main_color,
    shade_color,
    (void *) base_width,
    &bulb_width_func
  );
  draw_curve(tx, &c, vein_color);
}

/******************
 * Filter Helpers *
 ******************/

// private helper function for fltr_leaves w/ its own argument structure
struct leaves_helper_args_s {
  texture *tx;
  texture *leaf;
  leaf_filter_args *lfargs;
};
void fltr_leaves_helper(int x, int y, void * arg) {
  struct leaves_helper_args_s *lhargs = (struct leaves_helper_args_s *) arg;
  // Scramble the leaf filter seed:
  lhargs->lfargs->seed = expanded_hash_1d(lhargs->lfargs->seed * x * y);
  // Render a single randomized leaf into the leaf texture:
  fltr_leaf(lhargs->leaf, lhargs->lfargs);
  // Draw the fresh leaf onto the main texture at the given position:
  tx_draw_wrapped(lhargs->tx, lhargs->leaf, x, y);
}

/********************
 * Filter Functions *
 ********************/

void fltr_branches(texture *tx, void const * const fargs) {
  int row, col;
  float x, y;
  float offset;
  float noise, ds;
  gradient_map grmap;
  branch_filter_args *bfargs = (branch_filter_args *) fargs;
  grmap.colors[0] = bfargs->center_color;
  grmap.colors[1] = bfargs->mid_color;
  grmap.colors[2] = bfargs->outer_color;
  grmap.colors[3] = 0x00000000;
  if (bfargs->rough) {
    grmap.thresholds[0] = 0.43;
    grmap.thresholds[1] = 0.61;
    grmap.thresholds[2] = 0.72;
  } else {
    grmap.thresholds[0] = 0.06;
    grmap.thresholds[1] = 0.14;
    grmap.thresholds[2] = 0.2;
  }
  grmap.thresholds[3] = 1.0;
#ifdef DEBUG
  // Use orange for out-of-range noise results:
  grmap.colors[4] = 0xff0088ff;
  grmap.thresholds[4] = 1000.0;
#endif
  // We want to incorporate the seed value, but we don't want to deal with
  // overflow. 4 bytes of seed-based noise should be plenty while giving
  // comfortable overhead against overflow.
  offset = expanded_hash_1d(bfargs->seed) & 0xffff;
  for (col = 0; col < tx->width; col += 1) {
    for (row = 0; row < tx->height; row += 1) {
      // TODO: properly wrapped simplex noise.
      ds = sxnoise_2d(col * bfargs->dscale, row * bfargs->dscale);
      x = (col * bfargs->squash + ds * bfargs->distortion) * bfargs->scale;
      y = (row / bfargs->squash + ds * bfargs->distortion) * bfargs->scale;
      x += tx->width * offset;
      y += tx->height * offset;
      noise = wrnoise_2d(
        x, y,
        BLOCK_TEXTURE_SIZE * bfargs->scale, BLOCK_TEXTURE_SIZE * bfargs->scale,
        (!bfargs->rough) * WORLEY_FLAG_INCLUDE_NEXTBEST
      );
      if (bfargs->rough) {
        noise = 1 - noise;
        // TODO: a sigmoid for organizing branches
      }
      noise *= (2 - bfargs->width);
      if (noise > 1) { noise = 1; }
      tx_set_px(
        tx,
        gradient_result(&grmap, noise),
        col,
        row
      );
    }
  }
}

void fltr_leaf(texture *tx, void const * const fargs) {
  leaf_filter_args *lfargs = (leaf_filter_args *) fargs;
  ptrdiff_t mxhash = mixed_hash_1d(lfargs->seed);
  float noise = 0;
  curve c;
  c.from.x = 0; c.from.y = 0; c.from.z = 0;
  c.go_towards.x = 0; c.go_towards.y = 0; c.go_towards.z = 0;
  c.come_from.x = 0; c.come_from.y = 0; c.come_from.z = 0;
  c.to.x = 0; c.to.y = 0; c.to.z = 0;
  int x, y, corner;
  tx_clear(tx);
  if (lfargs->size == LS_SMALL) {
    if (lfargs->type == LT_SIMPLE) {
      noise = 0.5 + 0.5 * float_hash_1d(mxhash + 1);
      x =  (int) (noise * SMALL_LEAF_MAX_HEIGHT);
      noise = 0.5 + 0.5 * float_hash_1d(mxhash + 2);
      y =  (int) (noise * SMALL_LEAF_MAX_WIDTH);
      corner = mixed_hash_1d(mxhash + 3) & 0x3;
      if (corner == 1) {
        c.from.x = SMALL_LEAF_MAX_WIDTH;
        c.go_towards.x = SMALL_LEAF_MAX_WIDTH;
      } else if (corner == 2) {
        c.from.y = SMALL_LEAF_MAX_HEIGHT;
        c.go_towards.y = SMALL_LEAF_MAX_HEIGHT;
      } else if (corner >= 3) {
        c.from.x = SMALL_LEAF_MAX_WIDTH;
        c.go_towards.x = SMALL_LEAF_MAX_WIDTH;
        c.from.y = SMALL_LEAF_MAX_HEIGHT;
        c.go_towards.y = SMALL_LEAF_MAX_HEIGHT;
      }
      c.come_from.x = x;
      c.come_from.y = y;
      c.to.x = x;
      c.to.y = y;
      draw_thick_curve(
        tx,
        &c,
        lfargs->main_color,
        lfargs->dark_color,
        (void *) SMALL_LEAF_DEFAULT_BREADTH,
        // TODO: Different width func here?
        &bulb_width_func
      );
      draw_curve(
        tx,
        &c,
        lfargs->vein_color
      );
    } else if (lfargs->type == LT_TRIPARTITE) {
    } else if (lfargs->type == LT_NEEDLES) {
    }
#ifdef DEBUG
    else {
      fprintf(stderr, "Bad leaf filter type %d!\n", lfargs->size);
    }
#endif
  } else if (lfargs->size == LS_LARGE) {
    if (lfargs->type == LT_SIMPLE) {
    } else if (lfargs->type == LT_TRIPARTITE) {
    } else if (lfargs->type == LT_NEEDLES) {
    }
#ifdef DEBUG
    else {
      fprintf(stderr, "Bad leaf filter type %d!\n", lfargs->size);
    }
#endif
  }
#ifdef DEBUG
  else {
    fprintf(stderr, "Bad leaf filter size %d!\n", lfargs->size);
  }
#endif
}

void fltr_leaves(texture *tx, void const * const fargs) {
  leaves_filter_args *lfargs = (leaves_filter_args *) fargs;
  // Manually set up a texture on the stack to generate leaves into:
  pixel pixels[16*16];
  texture one_leaf;
  one_leaf.width = 16;
  one_leaf.height = 16;
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
  ptrdiff_t mxhash = mixed_hash_1d(blfargs->seed);
  int i = 0;
  float noise = 0.7 + 0.6 * float_hash_1d(mxhash); // [0.7, 1.3]
  int nstalks = (int) (blfargs->count * noise + 0.5);
  if (nstalks < 1) {
    nstalks = 1;
  }
  float x = BLOCK_TEXTURE_SIZE / 2.0;
  float y = BLOCK_TEXTURE_SIZE;
  float x_mid = 0, y_mid = 0;
  float x_end = 0, y_end = 0;
  float th_base = 0, th_mid = 0;
  float width = 1;
  float mid = (BLOCK_TEXTURE_SIZE / 2);
  noise = 0.75 + 0.5 * float_hash_1d(mxhash); // [0.75, 1.25]
  float spread = blfargs->spread * noise;
  if (spread > MAX_BULB_SPREAD) {
    spread = MAX_BULB_SPREAD;
  }
  spread *= BLOCK_TEXTURE_SIZE / 2.0;
  for (i = 0; i < nstalks; ++i) {
    // generate x in [mid - spread, mid + spread]
    x = mid - spread + 2 * spread * float_hash_1d(spread * mxhash * x + i);
    // pick a starting angle
    noise = 1.0 - 2 * float_hash_1d(x*noise*mxhash + i*x); // [-1, 1]
    th_base = blfargs->angle * noise;
    // pick a bend angle
    noise = 0.6 + 0.8 * float_hash_1d(x*noise*mxhash + i*x); // [0.6, 1.4]
    th_mid = blfargs->bend * noise * th_base / blfargs->angle;
    th_mid += th_base;
    // compute midpiont and endpoint
    x_mid = x + sinf(th_base) * blfargs->length * blfargs->shape;
    y_mid = y - cosf(th_base) * blfargs->length * blfargs->shape;
    x_end = x_mid + sinf(th_mid) * blfargs->length * (1 - blfargs->shape);
    y_end = y_mid - cosf(th_mid) * blfargs->length * (1 - blfargs->shape);
    // keep things in-bounds
    if (x_end < 1) {
      x_mid += (1 - x_end) / 2.0;
      x_end = 1;
    } else if (x_end > BLOCK_TEXTURE_SIZE - 2) {
      x_mid -= (x_end - (BLOCK_TEXTURE_SIZE - 2)) / 2.0;
      x_end = BLOCK_TEXTURE_SIZE - 2;
    }
    if (y_end < 1) {
      y_mid += (1 - y_end) / 2.0;
      y_end = 1;
    }
    /* DEBUG:
    printf("th_base: %.3f\n", th_base);
    printf("th_mid: %.3f\n", th_mid);
    printf("th_mid_atten: %.3f\n", th_base / blfargs->angle);
    printf("th_mid_roll: %.3f\n", noise);
    printf("xy_mid: %.2f, %.2f\n", x_mid, y_mid);
    printf("xy_end: %.2f, %.2f\n", x_end, y_end);
    // */
    // pick a base width:
    noise = 0.75 + 0.5 * float_hash_1d(x * noise * mxhash); // [0.75, 1.25]
    width = blfargs->width * noise;
    draw_bulb_leaf(
      tx,
      (size_t) width,
      blfargs->main_color,
      blfargs->vein_color,
      blfargs->dark_color,
      x, y,
      x_mid, y_mid,
      x_mid, y_mid,
      x_end, y_end
    );
  }
}
