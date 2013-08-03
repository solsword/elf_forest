// trees.c
// Tree generation functions.

#include <math.h>

#include "util.h"
#include "blocks.h"
#include "noise.h"
#include "trees.h"

/***********
 * Globals *
 ***********/

int TREE_HASH_OFFSET = 11;

tree_milieu TREE_MILIEU;

/*************
 * Functions *
 *************/

block trunk_block(region_pos pos, const trunk *trk) {
  int dx = pos.x - trk->root.x;
  int dy = pos.y - trk->root.y;
  int dz = pos.z - trk->root.z;
  int radius;
  if (dx == 0 && dy == 0) {
    if (trk->height >= TREE_MIN_HEIGHT_REAL_TRUNK) {
      if (
        dz < trk->height - fastceil(trk->height * TREE_CROWN_FRACTION)
      ) {
        return B_TRUNK;
      } else {
        return B_BRANCHES;
      }
    } else {
      return B_BRANCHES;
    }
  } else {
    return B_AIR;
    /*
    radius = trk->radius;
    radius -= dz % 2;
    radius -= dz % 3;
    radius = fmax(
      TREE_BRANCH_MIN_LENGTH,
      radius
    );
    radius = fmin(
      trk->height - dz,
      radius
    );
    if (dx*dx + dy*dy <= radius*radius) {
      return B_LEAVES;
    } else {
      return B_AIR;
    }
    // */
  }
}

block tree_block(region_pos pos, const tree_milieu *trm) {
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
