// trees.c
// Tree generation functions.

#include <math.h>

#include "util.h"

#include "trees.h"

#include "world/blocks.h"
#include "noise/noise.h"

/***********
 * Globals *
 ***********/

int TREE_HASH_OFFSET = 11;

tree_milieu TREE_MILIEU;

/*************
 * Functions *
 *************/

block trunk_block(region_pos pos, trunk const * const trk) {
  int dx = pos.x - trk->root.x;
  int dy = pos.y - trk->root.y;
  int dz = pos.z - trk->root.z;
  float radius;
  if (dz > trk->height) {
    return B_AIR;
  }
  if (dx == 0 && dy == 0) {
    if (trk->height >= TREE_MIN_HEIGHT_REAL_TRUNK) {
      if (
        dz < trk->height - fastceil(trk->height * TREE_CROWN_FRACTION)
      ) {
        return B_TREE_TRUNK;
      } else {
        return B_TREE_BRANCHES;
      }
    } else {
      return B_TREE_BRANCHES;
    }
  } else if (dz > 1) {
    radius = trk->radius + 0.6;
    radius -= dz % 2;
    if (radius > 2) { radius -= dz % 3; }
    radius = fmax(
      TREE_BRANCH_MIN_LENGTH,
      radius
    );
    radius = fmin(
      trk->height - dz,
      radius
    );
    assert(dz >= trk->height || radius > 0);
    if (dx*dx + dy*dy <= radius*radius) {
      return B_TREE_LEAVES;
    } else {
      return B_AIR;
    }
  } else {
    return B_AIR;
  }
}

block tree_block(region_pos pos, tree_milieu const * const trm) {
  int i;
  block result = B_AIR;
  for (i = 0; i < trm->a.n_trunks; ++i) {
    result = tree_growth(result, trunk_block(pos, &(trm->a.trunks[i])));
  }
  for (i = 0; i < trm->b.n_trunks; ++i) {
    result = tree_growth(result, trunk_block(pos, &(trm->b.trunks[i])));
  }
  return result;
}
