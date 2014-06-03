// terrain.c
// Legacy terrain generation (see worldgen.h/c).

#include <math.h>

//#include "trees.h"
#include "terrain.h"

#include "world/blocks.h"
#include "world/world.h"
#include "noise/noise.h"

/***********
 * Globals *
 ***********/

float TR_NOISE_OFFSET = 7300;

float TR_TERRAIN_HEIGHT_AMP = 1.0;

/*************
 * Functions *
 *************/

void terrain_cell(region_pos *pos, cell *result) {
  static int xcache = 3, ycache = 7;
  static int terrain = 0;
  static int dirt = 1;
  static int sandy = 0;
  static int cave_layer_1_b = 0, cave_layer_1_t = 0;
  static int cave_layer_2_b = 0, cave_layer_2_t = 0;
  static int cave_layer_3_b = 0, cave_layer_3_t = 0;
  static float nlst = 0, nlwr = 0, nlow = 0, nmid = 0, nhig = 0, nhst = 0;
  static float depths = 0, oceans = 0, plains = 0, hills = 0, mountains = 0;
  int tunnel = 0;
  int altitude = 0;
  if (xcache != pos->x || ycache != pos->y) {
    xcache = pos->x; ycache = pos->y;
    // recompute everything:
    // generate some noise at each frequency (which we'll reuse several times):
    get_noise(pos->x, pos->y, &nlst, &nlwr, &nlow, &nmid, &nhig, &nhst);
    // compute geoform mixing factors:
    depths = 0;
    oceans = 0;
    plains = 0;
    hills = 0;
    mountains = 0;
    compute_geoforms(nlst, &depths, &oceans, &plains, &hills, &mountains);
    // compute terrain height:
    terrain = fastfloor(get_terrain_height(
      nlst, nlwr, nlow, nmid, nhig, nhst,
      depths, oceans, plains, hills, mountains
    ) * TR_TERRAIN_HEIGHT_AMP);
    // compute cave layers:
    get_cave_layers(
      nlst, nlwr, nlow, nmid, nhig, nhst,
      depths, oceans, plains, hills, mountains,
      &cave_layer_1_b, &cave_layer_1_t,
      &cave_layer_2_b, &cave_layer_2_t,
      &cave_layer_3_b, &cave_layer_3_t
    );
    // dirt depth:
    dirt = TR_DIRT_MID + (int) (
      nmid * TR_DIRT_VAR
    );
    // compute a tree milieu:
    //compute_tree_milieu(pos->x, pos->y, &TREE_MILIEU);
    // sandiness:
    sandy = oceans > (TR_BEACH_THRESHOLD + (0.03 * terrain - TR_SEA_LEVEL));
  }
  // Altitude measures height above/below the base terrain height:
  altitude = pos->z - terrain;
  // DEBUG: (tunnels are expensive)
  tunnel = 0;
  /*
  // compute tunnel value:
  tunnel = get_tunnel(
    pos,
    depths, oceans, plains, hills, mountains // TODO: Use these arguments!
  );
  if (
    tunnel
  &&
    altitude <= 0
  &&
    (
      terrain > TR_SEA_LEVEL
    ||
      altitude < -TR_TUNNEL_UNDERSEA_OFFSET
    )
  ) {
    return B_AIR;
  }
  // */
  int aboveground = (altitude > 0);
  int surface = (altitude == 1);
  int topsoil = (altitude == 0 && dirt > 0);
  int soil = (altitude > -dirt);
  int underwater = (pos->z <= TR_SEA_LEVEL);
  int on_land = (pos->z > TR_SEA_LEVEL);
  int on_shore = (pos->z == TR_SEA_LEVEL);

  // Set the block types and data of the result cell:
  result->primary = b_make_block(B_VOID);
  result->secondary = b_make_block(B_VOID);
  result->p_data = 0;
  result->s_data = 0;
  if ((on_land || on_shore) && surface) {
    if (on_land && !sandy) {
      result->primary = b_make_block(B_GRASS);
    } else if (on_shore) {
      result->primary = b_make_block(B_WATER_FLOW);
    } else {
      result->primary = b_make_block(B_AIR);
    }
  } else if (topsoil) {
    if (sandy) {
      result->primary = b_make_block(B_SAND);
    } else {
      result->primary = b_make_block(B_DIRT);
      if (on_land || on_shore) {
        result->secondary = b_make_block(B_GRASS_ROOTS);
      }
    }
  } else if (aboveground) {
    if (underwater) {
      result->primary = b_make_block(B_WATER);
    } else {
      //if (altitude <= TREE_MAX_CANOPY_HEIGHT) {
        //result->primary = tree_block(pos, &TREE_MILIEU);
      //}
      result->primary = b_make_block(B_AIR);
    }
  } else {
    if (soil) {
      if (sandy) {
        result->primary = b_make_block(B_SAND);
      } else {
        result->primary = b_make_block(B_DIRT);
      }
    } else {
      result->primary = b_make_block(B_STONE);
    }
  }
}

void get_geoforms(
  int x, int y,
  float *depths, float *oceans, float *plains, float *hills, float *mountains
) {
  float noise = 0, ignore = 0;
  get_noise(x, y, &noise, &ignore, &ignore, &ignore, &ignore, &ignore);
  compute_geoforms(noise, depths, oceans, plains, hills, mountains);
}
