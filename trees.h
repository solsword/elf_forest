#ifndef TREES_H
#define TREES_H

// trees.h
// Tree generation functions.

#include "noise.h" // for the HASH array
#include "terrain.h"

/*************
 * Constants *
 *************/

// Coordinates signifying "no tree":
#define TREE_NOTREE_X (-17)
#define TREE_NOTREE_Y (-17)
#define TREE_NOTREE_Z (-17)

// Elevation above which no trees will grow:
static const int TREE_TREELINE = 150;

// Minimum canopy height for trees to grow:
static const int TREE_MIN_CANOPY_HEIGHT = 3;

// Minimum height for trees to develop full trunks vs. just branches:
static const int TREE_MIN_HEIGHT_REAL_TRUNK = 5;

// Percentage of tree height taken by crown branches:
static const float TREE_CROWN_FRACTION = 0.1;

// Branch length as a percentage of tree height:
static const float TREE_BRANCH_FRACTION = 0.2;

// Minimum branch length:
static const int TREE_BRANCH_MIN_LENGTH = 2;

// Geoform tree influences:
static const float TREE_DEPTHS_DENSITY = 0;
static const int TREE_DEPTHS_HEIGHT = 0;

static const float TREE_OCEANS_DENSITY = 0;
static const int TREE_OCEANS_HEIGHT = 0;

static const float TREE_PLAINS_DENSITY = 1.0;
static const int TREE_PLAINS_HEIGHT = 20;

static const float TREE_HILLS_DENSITY = 0.7;
static const int TREE_HILLS_HEIGHT = 18;

static const float TREE_MOUNTAINS_DENSITY = 0.5;
static const int TREE_MOUNTAINS_HEIGHT = 15;

// Canopy height factors:
static const int TREE_DIRT_EFFECT = 2;
static const int TREE_ALTITUDE_EFFECT = -1;
static const float TREE_ALTITUDE_STEP = 32;

static const int TREE_CANOPY_DETAIL_HIGH = 3;
static const int TREE_CANOPY_DETAIL_HIGHEST = 1;

// Density factors:
static const float TREE_DENSITY_OFFSET = -0.2;
static const int TREE_BASE_BIN_WIDTH = 25;

/***********
 * Globals *
 ***********/

// Hashing offset:
extern int TREE_HASH_OFFSET;

/********************
 * Inline Functions *
 ********************/

static inline int real_trunk(region_pos trunk) {
  return (
    trunk.x != TREE_NOTREE_X
  ||
    trunk.y != TREE_NOTREE_Y
  ||
    trunk.z != TREE_NOTREE_Z
  );
}

static inline int use_tree_block(
  int terrain,
  int height,
  int canopy_height,
  int sea_level, 
  region_pos trunk
) {
  return (
    height > 0
  &&
    height <= canopy_height
  &&
    terrain < TREE_TREELINE
  &&
    terrain > sea_level // TODO: Underwater seaweed "trees"
  &&
    real_trunk(trunk)
  );
}

static inline void compute_trunk_coords(
  int x, int y,
  region_pos *trunk, int *canopy_height,
  float nlst, float nlow, float nmid, float nhig, float nhst,
  float depths, float oceans, float plains, float hills, float mountains,
  int terrain, int dirt
) {
  int bin_width;
  float density;
  trunk->x = TREE_NOTREE_X;
  trunk->y = TREE_NOTREE_Y;
  trunk->z = TREE_NOTREE_Z;
  *canopy_height = (
    depths * TREE_DEPTHS_HEIGHT +
    oceans * TREE_OCEANS_HEIGHT +
    plains * TREE_PLAINS_HEIGHT +
    hills * TREE_HILLS_HEIGHT +
    mountains * TREE_MOUNTAINS_HEIGHT
  );
  *canopy_height += TREE_DIRT_EFFECT * (dirt - TR_DIRT_MID);
  *canopy_height += TREE_CANOPY_DETAIL_HIGH * nhig;
  *canopy_height += TREE_CANOPY_DETAIL_HIGHEST * nhst;
  *canopy_height += TREE_ALTITUDE_EFFECT * (terrain / TREE_ALTITUDE_STEP);
  if (*canopy_height < TREE_MIN_CANOPY_HEIGHT) {
    return;
  }
  density = (
    depths * TREE_DEPTHS_DENSITY +
    oceans * TREE_OCEANS_DENSITY +
    plains * TREE_PLAINS_DENSITY +
    hills * TREE_HILLS_DENSITY +
    mountains * TREE_MOUNTAINS_DENSITY
  ) + TREE_DENSITY_OFFSET;
  density = 1 - density;
  density *= density;
  // DEBUG:
  //bin_width = 1 + TREE_BASE_BIN_WIDTH * density;
  bin_width = 4;
  // TODO: liven things up a bit!
  trunk->x = ((x / bin_width) * bin_width) + bin_width / 2;
  trunk->y = ((y / bin_width) * bin_width) + bin_width / 2;
  trunk->z = 4;
  // compute terrain height at the discovered trunk x/y:
  /*
  get_noise(
    trunk->x, trunk->y, 
    &nlst, &nlow, &nmid, &nhig, &nhst
  );
  compute_geoforms(nlst, &depths, &oceans, &plains, &hills, &mountains);
  trunk->z = get_terrain_height(
    nlst, nlow, nmid, nhig, nhst,
    depths, oceans, plains, hills, mountains
  );
  */
}

/*************
 * Functions *
 *************/

block tree_block(
  const region_pos *pos, const region_pos *trunk,
  int canopy_height, float noise
);

#endif // ifndef TREES_H
