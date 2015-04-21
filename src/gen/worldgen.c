// worldgen.c
// World map generation.

#include <math.h>

#include "datatypes/bitmap.h"
#include "noise/noise.h"
#include "world/blocks.h"
#include "world/world_map.h"
#include "data/data.h"
#include "txgen/cartography.h"
#include "tex/tex.h"
#include "math/manifold.h"
#include "util.h"

#include "terrain.h"
#include "geology.h"
#include "climate.h"
#include "biology.h"

#include "worldgen.h"

/*************
 * Constants *
 *************/

char const * const WORLD_MAP_FILE_BASE = "out/world_map.png";
char const * const WORLD_MAP_FILE_REGIONS = "out/world_map_regions.png";
char const * const WORLD_MAP_FILE_TEMP = "out/world_map_temp.png";
char const * const WORLD_MAP_FILE_WIND = "out/world_map_wind.png";
char const * const WORLD_MAP_FILE_EVAP = "out/world_map_evaporation.png";
char const * const WORLD_MAP_FILE_CLOUDS = "out/world_map_clouds.png";
char const * const WORLD_MAP_FILE_PQ = "out/world_map_pq.png";
char const * const WORLD_MAP_FILE_RAIN = "out/world_map_rain.png";
char const * const WORLD_MAP_FILE_LRAIN = "out/world_map_land_rain.png";

/*************
 * Functions *
 *************/

void setup_worldgen(ptrdiff_t seed) {
  setup_terrain_gen();
  THE_WORLD = create_world_map(prng(seed + 71), WORLD_WIDTH, WORLD_HEIGHT);
  printf("  ...initializing world...\n");
  init_world_map(THE_WORLD);
  printf("  ...generating tectonics...\n");
  generate_tectonics(THE_WORLD);
  printf("  ...generating topology...\n");
  generate_topology(THE_WORLD);
  printf("  ...generating hydrology...\n");
  generate_hydrology(THE_WORLD);
  printf("  ...generating climate...\n");
  generate_climate(THE_WORLD);
  printf("  ...generating geology...\n");
  generate_geology(THE_WORLD);
  printf("  ...writing world maps...\n");

  texture *base_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *regions_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *temp_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *wind_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *evap_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *cloud_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *pq_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *rain_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *lrain_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);

  render_map_layer(THE_WORLD, base_map, &ly_terrain_height);
  render_map_layer(THE_WORLD, regions_map, &ly_georegions);
  render_map_layer(THE_WORLD, temp_map, &ly_temperature);
  render_map_layer(THE_WORLD, wind_map, &ly_terrain_height);
  render_map_vectors(THE_WORLD, wind_map, PX_BLACK,PX_WHITE, &vly_wind_vectors);
  render_map_layer(THE_WORLD, evap_map, &ly_evaporation);
  render_map_layer(THE_WORLD, cloud_map, &ly_cloud_cover);
  render_map_layer(THE_WORLD, pq_map, &ly_precipitation_quotient);
  render_map_layer(THE_WORLD, rain_map, &ly_precipitation);
  render_map_layer(THE_WORLD, lrain_map, &ly_land_precipitation);

  printf("    ...elevation...\n");
  write_texture_to_png(base_map, WORLD_MAP_FILE_BASE);
  printf("    ...geographic regions...\n");
  write_texture_to_png(regions_map, WORLD_MAP_FILE_REGIONS);
  printf("    ...temperature...\n");
  write_texture_to_png(temp_map, WORLD_MAP_FILE_TEMP);
  printf("    ...wind...\n");
  write_texture_to_png(wind_map, WORLD_MAP_FILE_WIND);
  printf("    ...evaporation...\n");
  write_texture_to_png(evap_map, WORLD_MAP_FILE_EVAP);
  printf("    ...clouds...\n");
  write_texture_to_png(cloud_map, WORLD_MAP_FILE_CLOUDS);
  printf("    ...precipitation_quotient...\n");
  write_texture_to_png(pq_map, WORLD_MAP_FILE_PQ);
  printf("    ...precipitation...\n");
  write_texture_to_png(rain_map, WORLD_MAP_FILE_RAIN);
  printf("    ...land precipitation...\n");
  write_texture_to_png(lrain_map, WORLD_MAP_FILE_LRAIN);

  cleanup_texture(base_map);
  cleanup_texture(regions_map);
  cleanup_texture(temp_map);
  cleanup_texture(wind_map);
  cleanup_texture(evap_map);
  cleanup_texture(cloud_map);
  cleanup_texture(pq_map);
  cleanup_texture(rain_map);
  cleanup_texture(lrain_map);
}

void cleanup_worldgen() {
  cleanup_world_map(THE_WORLD);
}

void init_world_map(world_map *wm) {
  size_t sofar = 0;
  world_map_pos xy;
  world_region *wr;

  for (xy.x = 0; xy.x < wm->width; xy.x += 1) {
    for (xy.y = 0; xy.y < wm->height; xy.y += 1) {
      sofar += 1;
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      // Set position information:
      wr->pos.x = xy.x;
      wr->pos.y = xy.y;
      // Default height info:
      wr->topology.terrain_height.z = 0;
      wr->topology.terrain_height.dx = 0;
      wr->topology.terrain_height.dy = 0;
      wr->topology.geologic_height = 0;
      wr->topology.flow_potential = 0;
      wr->topology.next_height = 0;
      wr->topology.downhill = NULL;
      wr->topology.uphill = NULL;

      // Default hydrology info:
      wr->climate.water.state = HYDRO_LAND;
      wr->climate.water.body = NULL;
      wr->climate.water.water_table = 0; // TODO: get rid of water table?
      wr->climate.water.salt = SALINITY_FRESH;

      // Pick a seed for this world region:
      wr->seed = hash_3d(xy.x, xy.y, wm->seed + 8731);
      // Randomize the anchor position:
      compute_region_anchor(wm, &xy, &(wr->anchor));

      // Print a progress message:
      if (sofar % 100 == 0) {
        printf(
          "    ...%zu / %zu regions initialized...\r",
          sofar,
          (size_t) (wm->width * wm->height)
        );
      }
    }
  }
  printf(
    "    ...%zu / %zu regions initialized...\r",
    (size_t) (wm->width * wm->height),
    (size_t) (wm->width * wm->height)
  );
  printf("\n");
}

void compute_manifold(world_map *wm) {
  float z, nbz, min_neighbor_height, max_neighbor_height;
  float xdivisor, ydivisor;
  world_map_pos xy, iter;
  world_region *wr, *nb;

  for (xy.x = 0; xy.x < wm->width; xy.x += 1) {
    for (xy.y = 0; xy.y < wm->height; xy.y += 1) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      wr->downhill = NULL;
      wr->uphill = NULL;
      z = wr->topology.terrain_height.z;
      wr->topology.terrain_height.dx = 0;
      wr->topology.terrain_height.dy = 0;
      min_neighbor_height = z;
      max_neighbor_height = z;
      xdivisor = 0;
      ydivisor = 0;
      for (iter.x = xy.x - 1; iter.x <= xy.x + 1; iter.x += 1) {
        for (iter.y = xy.y - 1; iter.y <= xy.y + 1; iter.y += 1) {
          if (iter.x == xy.x && iter.y == xy.y) { continue; }
          nb = get_world_region(wm, &iter);
          if (nb == NULL) { continue; }
          nbz = nb->topology.terrain_height.z;
          // figure out up- and down-hill neighbors:
          if (nbz < min_neighbor_height) {
            wr->topology.downhill = nb;
            min_neighbor_height = nbz;
          }
          if (nbz > max_neighbor_height) {
            wr->topology.uphill = nb;
            max_neighbor_height = nbz;
          }
          // add up nearby height differences:
          if (iter.x < xy.x) {
            if (iter.y == xy.y) {
              wr->topology.terrain_height.dx += (z - nbz);
              xdivisor += 1;
            } else {
              wr->topology.terrain_height.dx += (z - nbz) * 0.5;
              xdivisor += 0.5;
            }
          } else if (iter.x > xy.x) {
            if (iter.y == xy.y) {
              wr->topology.terrain_height.dx += (nbz - z);
              xdivisor += 1;
            } else {
              wr->topology.terrain_height.dx += (nbz - z) * 0.5;
              xdivisor += 0.5;
            }
          }
          if (iter.y < xy.y) {
            if (iter.x == xy.x) {
              wr->topology.terrain_height.dy += (z - nbz);
              xdivisor += 1;
            } else {
              wr->topology.terrain_height.dy += (z - nbz) * 0.5;
              xdivisor += 0.5;
            }
          } else if (iter.y > xy.y) {
            if (iter.x == xy.x) {
              wr->topology.terrain_height.dy += (nbz - z);
              xdivisor += 1;
            } else {
              wr->topology.terrain_height.dy += (nbz - z) * 0.5;
              xdivisor += 0.5;
            }
          }
        }
      }
      wr->topology.terrain_height.dx /= xdivisor;
      wr->topology.terrain_height.dy /= ydivisor;
    }
  }
}

void world_cell(world_map *wm, global_pos *glpos, cell *result) {
  world_map_pos wmpos, iter;
  world_region *neighborhood[9];
  size_t i = 0;
  glpos__wmpos(glpos, &wmpos);
  // default values:
  result->primary = b_make_block(B_VOID);
  result->secondary = b_make_block(B_VOID);
  i = 0;
  for (iter.x = wmpos.x - 1; iter.x <= wmpos.x + 1; iter.x += 1) {
    for (iter.y = wmpos.y - 1; iter.y <= wmpos.y + 1; iter.y += 1) {
      neighborhood[i] = get_world_region(wm, &iter);
      i += 1;
    }
  }
  if (glpos->z < 0) {
    result->primary = b_make_block(B_BOUNDARY);
  } else if (neighborhood[4] == NULL) {
    // Outside the world:
    result->primary = b_make_block(B_AIR);
  } else {
    // TODO: Oceans here!
    terrain_cell(wm, neighborhood, glpos, result);
  }
  if (b_is(result->primary, B_VOID)) {
    // TODO: Other things like plants here...
    result->primary = b_make_block(B_AIR);
  }
}
