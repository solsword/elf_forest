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

block tree_block(
  const region_pos *pos, const region_pos *trunk,
  int canopy_height, float noise
) {
  int dx = pos->x - trunk->x;
  int dy = pos->y - trunk->y;
  int dz = pos->z - trunk->z;
  if (dx == 0 && dy == 0) {
    if (canopy_height >= TREE_MIN_HEIGHT_REAL_TRUNK) {
      if (
        dz < canopy_height - fastceil(canopy_height * TREE_CROWN_FRACTION)
      ) {
        return B_TRUNK;
      } else {
        return B_BRANCHES;
      }
    } else {
      return B_BRANCHES;
    }
  } else {
    int branch_length = fastceil(canopy_height * TREE_BRANCH_FRACTION);
    branch_length -= dz % 2;
    branch_length -= dz % 3;
    branch_length = fmax(
      TREE_BRANCH_MIN_LENGTH,
      branch_length
    );
    branch_length = fmin(
      canopy_height - dz,
      branch_length
    );
    if (dx*dx + dy*dy <= branch_length*branch_length) {
      return B_LEAVES;
    } else {
      return B_AIR;
    }
  }
}
