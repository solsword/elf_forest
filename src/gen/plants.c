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

extern int const SMALL_LEAF_MAX_HEIGHT = 4;
extern int const SMALL_LEAF_MAX_WIDTH = 4;

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
      for (x = 0; x < tx->width; ++x) {
        for (y = 0; y < tx->height; ++y) {
          tx_set_px(tx, PX_EMPTY, x, y);
        }
      }
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

void fltr_leaf(texture *tx, void *fargs);
