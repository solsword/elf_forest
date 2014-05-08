// plants.c
// Plant generation.

#include "world/blocks.h"
#include "world/world.h"
#include "noise/noise.h"
#include "graphics/tex.h"

#include "txgen.h"

#include "plants.h"

/*************
 * Constants *
 *************/

int const SMALL_LEAF_MAX_HEIGHT = 7;
int const SMALL_LEAF_MAX_WIDTH = 5;

float const CURVE_DRAWING_RESOLUTION = 0.05;

float const MAX_BULB_SPREAD = 0.9;

/****************************
 * Example filter arguments *
 ****************************/

branch_filter_args const example_branch_args = {
  .seed = 0,
  .rough = 0,
  .scale = 0.125,
  .width = 1.0,
  .dscale = 0.125,
  .distortion = 5.0,
  .squash = 1.2,
  .center_color = 0xff001133, // dark brown
  .mid_color = 0xff004466, // mid brown
  .outer_color = 0xff007799 // light brown
};

leaf_filter_args const example_leaf_args = {
  .seed = 0,
  .type = LT_SIMPLE,
  .size = LS_LARGE,
  .main_color = 0xff00bb22, // medium green
  .vein_color = 0xff55dd77, // light green
  .dark_color = 0xff007711, // dark green
};

/*************
 * Functions *
 *************/

void draw_bulb_leaf(
  texture *tx,
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
  // TODO: HERE!
  curve_witheach(&c, CURVE_DRAWING_RESOLUTION, arg, fn);
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
void fltr_leaves_helper(int x, int y, void *arg) {
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

void fltr_branches(texture *tx, void *fargs) {
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

void fltr_leaf(texture *tx, void *fargs) {
  leaf_filter_args *lfargs = (leaf_filter_args *) fargs;
  ptrdiff_t mxhash = mixed_hash_1d(lfargs->seed);
  if (lfargs->size == LS_SMALL) {
    if (lfargs->type == LT_SIMPLE) {
      int notch_lr = hash_2d(mxhash, 0) & 0x1; // is the notch left or right?
      int notch_dfe = hash_2d(mxhash, 1) & 0x3; // is it dark, full, or empty?
      int hang_lr = hash_2d(mxhash, 2) & 0x1; // does it hang left or right?
      int hang_height = hash_2d(mxhash, 3) & 0x1; // how many pixels of hang?
      int height = hash_2d(mxhash, 4) % SMALL_LEAF_MAX_HEIGHT;
      int width = hash_2d(mxhash, 5) % SMALL_LEAF_MAX_WIDTH;
      int x, y;
      tx_clear(tx);
      for (x = 0; x < width; ++x) {
        for (y = 0; y < height; ++y) {
          if (
            y == 0
          &&
            (
              (notch_lr == 0 && x == 0)
            ||
              (notchlr == 1 && x == width - 1)
            )
          ) {
            // the notch
            if (notch_dfe == 0) {
              tx_set_px(tx, lfargs->dark_color, x, y);
            } else if (notch_dfe == 1) {
              tx_set_px(tx, lfargs->main_color, x, y);
            } // else do nothing; leave it transparent (2/4 cases)
          } else if (
            height - y <= hang_height
          &&
            (
              (hang_lr == 0 && x < width/2)
            ||
              (hanglr == 1 && x > width/2)
            )
          ) {
            // the hanging part
            tx_set_px(tx, lfargs->main_color, x, y);
          } else if (height - y > hang_height) {
            // a normal part of the leaf
            tx_set_px(tx, lfargs->main_color, x, y);
          }
        }
      }
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
      int notch_lr = hash_2d(mxhash, 0) & 0x1; // is the notch left or right?
      int notch_dfe = hash_2d(mxhash, 1) & 0x3; // is it dark, full, or empty?
      int hang_lr = hash_2d(mxhash, 2) & 0x1; // does it hang left or right?
      int height = hash_2d(mxhash, 3) % SMALL_LEAF_MAX_HEIGHT;
      int width = hash_2d(mxhash, 4) % SMALL_LEAF_MAX_WIDTH;
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

void fltr_leaves(texture *tx, void *fargs) {
  leaves_filter_args *lfargs = (leaves_filter_args *) fargs;
  // Manually set up a texture on the stack to generate leaves into:
  pixel pixels[16*16];
  texture one_leaf;
  one_leaf.width = 16;
  one_leaf.height = 16;
  one_leaf.pixels = pixels;

  // Set up arguments for our helper function:
  struct leaves_helper_args_s lhargs;
  lhargs.tx = tx;
  lhargs.leaf = &one_leaf;
  lhargs.lfargs = lfargs;

  ateach_scattered(
    sfargs->seed,
    sfargs->x_freq, sfargs->y_freq,
    sfargs->x_freq/3, sfargs->y_freq/3,
    0, BLOCK_TEXTURE_SIZE,
    0, BLOCK_TEXTURE_SIZE,
    &lhargs,
    &fltr_leaves_helper
  );
}

void fltr_bulb_leaves(texture *tx, void *fargs) {
  bulb_leaves_filter_args *blfargs = (bulb_leaves_filter_args *) fargs;
  ptrdiff_t mxhash = mixed_hash_1d(blfargs->seed);
  int i = 0;
  float noise = 0.8 + 0.4 * float_hash_1d(mxhash); // [0.8, 1.2]
  int nstalks = (int) (i * noise);
  float x = 0, xo = 0;
  float y = BLOCK_TEXTURE_SIZE, yo = 0;
  float mid = (BLOCK_TEXTURE_SIZE / 2);
  noise = 0.75 + 0.5 * float_hash_1d(mxhash); // [0.75, 1.25]
  float spread = blfargs->spread * noise;
  if (spread > MAX_BULB_SPREAD) {
    spread = MAX_BULB_SPREAD;
  }
  spread *= BLOCK_TEXTURE_SIZE / 2.0;
  for (i; i < nstalks; ++i) {
    // generate x in [mid - spread, mid + spread]
    x = mid - spread + spread * float_hash_1d(spread * mxhash);
    noise = 1.0 - 2 * float_hash_1d(x * noise * mxhash); // [-1, 1]
    xo = blfargs->bend * noise;
    noise = 1.0 - 2 * float_hash_1d(x * noise * mxhash); // [-1, 1]
    yo = blfargs->bend * noise;
    draw_bulb_leaf(
      tx,
      x, y,
      x, y,
      x, y - mid + 0.5 * blfargs->bend,
      x + xo, y - mid + yo,
    );
    noise = 0.75 + 0.5 * float_hash_1d(x * noise * mxhash); // [0.75, 1.25]
    step = blfargs->step * noise;
  }
}
