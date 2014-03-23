#ifndef TREES_H
#define TREES_H

// trees.h
// Tree generation functions.

// DEBUG:
#include <stdio.h>
#include <assert.h>

#include "terrain.h"

#include "noise/noise.h" // for the HASH array

/**************
 * Structures *
 **************/

// Information about a single trunk.
struct trunk_s;
typedef struct trunk_s trunk;

// A tree cell stores information about several trunks.
struct tree_cell_s;
typedef struct tree_cell_s tree_cell;

// The set of tree cells that a given region position falls into:
struct tree_milieu_s;
typedef struct tree_milieu_s tree_milieu;

/*************
 * Constants *
 *************/

/// Max trunks/grid cell:
#define TREE_MAX_TRUNKS 5

// Tree grid sizes:
/*
static const int TREE_GRID_SMALL = 7;
static const int TREE_GRID_MEDIUM = 14;
static const int TREE_GRID_LARGE = 21;
*/

#define TREE_GRID_BITS 5

static const int TREE_GRID_BASE = 1 << TREE_GRID_BITS;
static const int TREE_GRID_MIN = 1 << (TREE_GRID_BITS - 2);

// Tree grid offsets:
static const int TREE_GRID_OFFSET = 20;
//static const int TREE_GRID_OFFSET_A = 2;
//static const int TREE_GRID_OFFSET_B = 5;
// Offsets by size:
// Small: 0, A, B
// Medium: 0, B, A + SMALL
// Large: 0, A + SMALL, B + MEDIUM

// Tree widths:
/*
static const int TREE_WIDTH_MIN = 3;
static const int TREE_WIDTH_STEP = 2;
static const int TREE_WIDTH_COUNT = 7;
*/

// Elevation above which no trees will grow:
static const int TREE_TREELINE = 150;

// Minimum/maximum canopy height for trees:
static const int TREE_MIN_CANOPY_HEIGHT = 3;
static const int TREE_MAX_CANOPY_HEIGHT = 35;

// Minimum height for trees to develop full trunks vs. just branches:
static const int TREE_MIN_HEIGHT_REAL_TRUNK = 5;

// Percentage of tree height taken by crown branches:
static const float TREE_CROWN_FRACTION = 0.1;

// Branch length as a percentage of tree height:
static const float TREE_BRANCH_FRACTION = 0.2;

// Branches part of each branch vs. leaves part:
static const float TREE_BRANCH_PROPORTION = 0.34;

// Minimum branch length:
static const int TREE_BRANCH_MIN_LENGTH = 1;

// Geoform tree influences:
static const int TREE_DEPTHS_HEIGHT = -50;
static const int TREE_OCEANS_HEIGHT = -20;
static const int TREE_PLAINS_HEIGHT = 20;
static const int TREE_HILLS_HEIGHT = 18;
static const int TREE_MOUNTAINS_HEIGHT = 15;

// Tree density frequencies:
static const float TREE_DENSITY_FREQUENCY_LOW = 0.021;
static const float TREE_DENSITY_FACTOR_LOW = 3*4*4;

static const float TREE_DENSITY_FREQUENCY_HIGH = 0.063;
static const float TREE_DENSITY_FACTOR_HIGH = 2*4*4;

// Canopy height factors:
static const int TREE_DIRT_EFFECT = 2;
static const int TREE_ELEVATION_EFFECT = -1;
static const float TREE_ELEVATION_STEP = 32;

static const int TREE_CANOPY_DETAIL_HIGH = 3;
static const int TREE_CANOPY_DETAIL_HIGHEST = 1;

/***********
 * Globals *
 ***********/

// Hashing offset:
extern int TREE_HASH_OFFSET;

// Reserved space for performing tree milieu calculations:
extern tree_milieu TREE_MILIEU;

/*************************
 * Structure Definitions *
 *************************/

struct trunk_s {
  region_pos root;
  int height;
  int radius;
};

struct tree_cell_s {
  int scale;
  region_pos origin;
  int n_trunks;
  trunk trunks[TREE_MAX_TRUNKS];
};

struct tree_milieu_s {
  tree_cell a, b;
};

/********************
 * Inline Functions *
 ********************/

static inline int valid_trunk_elevation(int root) {
  return (
    root < TREE_TREELINE
  &&
    root > TR_SEA_LEVEL // TODO: Underwater seaweed "trees"
  );
}

static inline block tree_growth(block existing, block grow) {
  int can_grow = (
    (block_is(existing, B_AIR) || block_is(existing, B_LEAVES))
      || 
    (block_is(existing, B_BRANCHES) && block_is(grow, B_TRUNK))
  );
  return can_grow * grow + (!can_grow) * existing;
}

static inline int compute_tree_trunk(int tx, int ty, int radius, trunk *trk) {
  float nlst = 0, nlow = 0, nmid = 0, nhig = 0, nhst = 0;
  float depths = 0, oceans = 0, plains = 0, hills = 0, mountains = 0;
  trk->radius = radius;
  trk->root.x = tx;
  trk->root.y = ty;
  // compute terrain at the trunk location:
  get_noise(tx, ty, &nlst, &nlow, &nmid, &nhig, &nhst);
  compute_geoforms(nlst, &depths, &oceans, &plains, &hills, &mountains);
  // fill in the trunk height (return 0 if the height is too low):
  trk->height = (
    depths * TREE_DEPTHS_HEIGHT +
    oceans * TREE_OCEANS_HEIGHT +
    plains * TREE_PLAINS_HEIGHT +
    hills * TREE_HILLS_HEIGHT +
    mountains * TREE_MOUNTAINS_HEIGHT
  );
  trk->root.z = get_terrain_height(
    nlst, nlow, nmid, nhig, nhst,
    depths, oceans, plains, hills, mountains
  ) + 1;
  if (!valid_trunk_elevation(trk->root.z)) {
    return 0;
  }
  trk->height += TREE_DIRT_EFFECT * ((1 + nmid) / 2.0) * TR_DIRT_VAR;
  trk->height += TREE_CANOPY_DETAIL_HIGH * ((1 + nhig) / 2.0);
  trk->height += TREE_CANOPY_DETAIL_HIGHEST * nhst;
  trk->height += TREE_ELEVATION_EFFECT * (trk->root.z / TREE_ELEVATION_STEP);
  return trk->height >= TREE_MIN_CANOPY_HEIGHT;
}

static inline void compute_tree_cell(
  long int x, long int y,
  int offset,
  tree_cell *cell
) {
  int nudge = 0;
  int hash = 0;
  float nlow = 0, nhig = 0;

  // Start at the base scale:
  cell->scale = TREE_GRID_BASE;

  // Compute base origin:
  cell->origin.x = fastfloor((x - offset) / (float) cell->scale) * cell->scale;
  cell->origin.x += offset;
  cell->origin.y = fastfloor((y - offset) / (float) cell->scale) * cell->scale;
  cell->origin.y += offset;
  cell->origin.z = 0;

  // Get some noise to use for tree density:
  nlow = sxnoise_2d(
    cell->origin.x * TREE_DENSITY_FREQUENCY_LOW,
    cell->origin.y * TREE_DENSITY_FREQUENCY_LOW
  );
  nhig = sxnoise_2d(
    cell->origin.x * TREE_DENSITY_FREQUENCY_HIGH,
    cell->origin.y * TREE_DENSITY_FREQUENCY_HIGH
  );
  // Compute tree density:
  cell->n_trunks = ((1 + nlow) / 2.0) * TREE_DENSITY_FACTOR_LOW;
  cell->n_trunks += ((1 + nhig) / 2.0) * TREE_DENSITY_FACTOR_HIGH;

  assert(cell->n_trunks > 0);

  // Subdivide as necessary while throwing in a bit of randomness:
  while (cell->n_trunks > TREE_MAX_TRUNKS) {
    cell->scale = cell->scale >> 1;
    if (cell->scale < TREE_GRID_MIN) {
      cell->scale = TREE_GRID_MIN;
      cell->n_trunks = TREE_MAX_TRUNKS;
      break;
    }
    hash = HASH[
      (cell->origin.x & 0xff) + HASH[(
        cell->origin.y & 0xff) + TREE_HASH_OFFSET
      ]
    ];
    nudge = ( (int) (hash / 86.0)) - 1;
    cell->n_trunks = (cell->n_trunks >> 2) + nudge;
    cell->origin.x += ((x - cell->origin.x) / cell->scale) * cell->scale;
    cell->origin.y += ((y - cell->origin.y) / cell->scale) * cell->scale;
  }

  // Fill in the actual trunks:
  int i = 0;
  int radius = 1;
  int from = 0, to = cell->scale - 1;
  int tx = 0, ty = 0;
  for (i = 0; i < cell->n_trunks; ++i) {
    hash = HASH[(cell->origin.x & 0xff) + HASH[(cell->origin.y & 0xff) + hash]];
    radius = 2 + nudge + (2 * nudge) + (cell->scale / 8);
    radius = radius < 1 ? 1 : radius;
    radius = radius > cell->scale/2 ? cell->scale/2 : radius;
    from = radius;
    to = cell->scale - (radius + 1);
    tx = from + ( (int) (hash * (to - from) / 256.0) );
    hash = HASH[(cell->origin.x & 0xff) + HASH[(cell->origin.y & 0xff) + hash]];
    ty = from + ( (int) (hash * (to - from) / 256.0) );
    tx += cell->origin.x;
    ty += cell->origin.y;
    if ( !compute_tree_trunk(tx, ty, radius, &(cell->trunks[i])) ) {
      cell->n_trunks -= 1;
      i -= 1;
    };
  }
}

static inline void compute_tree_milieu(int x, int y, tree_milieu *trm) {
  compute_tree_cell(x, y, 0, &(trm->a));
  compute_tree_cell(x, y, TREE_GRID_OFFSET, &(trm->b));
}

/*************
 * Functions *
 *************/

// Computes a block ID given a tree, a location, and some noise.
block trunk_block(region_pos pos, const trunk *trk);

// Computes a block ID given a tree milieu, a location, and some noise.
block tree_block(region_pos pos, const tree_milieu *trm);

#endif // ifndef TREES_H
