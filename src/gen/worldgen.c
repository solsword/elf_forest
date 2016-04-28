// worldgen.c
// World map generation.

#include <math.h>

#include "datatypes/bitmap.h"
#include "noise/noise.h"
#include "world/blocks.h"
#include "world/world_map.h"
#include "data/data.h"
#include "data/persist.h"
#include "txgen/cartography.h"
#include "tex/tex.h"
#include "math/manifold.h"
#include "util.h"

#include "elements.h"
#include "terrain.h"
#include "geology.h"
#include "climate.h"
#include "soil.h"
#include "ecology.h"
#include "biology.h"

#include "worldgen.h"

/*************
 * Constants *
 *************/

CSTR(WORLD_MAP_FILE_BASE, "world_map.png", 13);
CSTR(WORLD_MAP_FILE_REGIONS, "world_map_regions.png", 21);
CSTR(WORLD_MAP_FILE_TEMP, "world_map_temp.png", 18);
CSTR(WORLD_MAP_FILE_WIND, "world_map_wind.png", 18);
CSTR(WORLD_MAP_FILE_EVAP, "world_map_evaporation.png", 25);
CSTR(WORLD_MAP_FILE_CLOUDS, "world_map_clouds.png", 20);
CSTR(WORLD_MAP_FILE_PQ, "world_map_pq.png", 16);
CSTR(WORLD_MAP_FILE_RAIN, "world_map_rain.png", 18);
CSTR(WORLD_MAP_FILE_LRAIN, "world_map_land_rain.png", 23);

/*********************
 * Private Functions *
 *********************/

static inline void write_map_to_file(texture *map, string const * const file) {
  char *map_file;
  string *full_file;

  full_file = s_concat(PS_MAPS_DIR_PREFIX, file);
  map_file = s_encode_nt(full_file);

  write_texture_to_png(map, map_file);

  free(map_file);
  cleanup_string(full_file);
}

/*************
 * Functions *
 *************/

void setup_worldgen(ptrdiff_t seed) {
  setup_terrain_gen();
  setup_biology_gen();

  THE_WORLD = create_world_map(prng(seed + 71), WORLD_WIDTH, WORLD_HEIGHT);

  printf("  ...initializing world...\n");
  init_world_map(THE_WORLD);
  printf("  ...generating elements...\n");
  generate_elements(THE_WORLD);
  printf("  ...generating tectonics...\n");
  generate_tectonics(THE_WORLD);
  printf("  ...generating topography...\n");
  generate_topography(THE_WORLD);
  printf("  ...generating hydrology...\n");
  generate_hydrology(THE_WORLD);
  printf("  ...generating climate...\n");
  generate_climate(THE_WORLD);
  printf("  ...generating geology...\n");
  generate_geology(THE_WORLD);
  printf("  ...summarizing altitude and climate information...\n");
  summarize_all_regions(THE_WORLD);
  printf("  ...generating soil...\n");
  generate_soil(THE_WORLD);
  printf("  ...generating ecology...\n");
  generate_ecology(THE_WORLD);


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

  printf("    ...elevation...\n");
  render_map_layer(THE_WORLD, base_map, &ly_terrain_height);
  write_map_to_file(base_map, WORLD_MAP_FILE_BASE);

  printf("    ...geographic regions...\n");
  render_map_layer(THE_WORLD, regions_map, &ly_georegions);
  write_map_to_file(regions_map, WORLD_MAP_FILE_REGIONS);

  printf("    ...temperature...\n");
  render_map_layer(THE_WORLD, temp_map, &ly_temperature);
  write_map_to_file(temp_map, WORLD_MAP_FILE_TEMP);

  printf("    ...wind...\n");
  render_map_layer(THE_WORLD, wind_map, &ly_terrain_height);
  render_map_vectors(THE_WORLD, wind_map, PX_BLACK,PX_WHITE, &vly_wind_vectors);
  write_map_to_file(wind_map, WORLD_MAP_FILE_WIND);

  printf("    ...evaporation...\n");
  render_map_layer(THE_WORLD, evap_map, &ly_evaporation);
  write_map_to_file(evap_map, WORLD_MAP_FILE_EVAP);

  printf("    ...clouds...\n");
  render_map_layer(THE_WORLD, cloud_map, &ly_cloud_cover);
  write_map_to_file(cloud_map, WORLD_MAP_FILE_CLOUDS);

  printf("    ...precipitation_quotient...\n");
  render_map_layer(THE_WORLD, pq_map, &ly_precipitation_quotient);
  write_map_to_file(pq_map, WORLD_MAP_FILE_PQ);

  printf("    ...precipitation...\n");
  render_map_layer(THE_WORLD, rain_map, &ly_precipitation);
  write_map_to_file(rain_map, WORLD_MAP_FILE_RAIN);

  printf("    ...land precipitation...\n");
  render_map_layer(THE_WORLD, lrain_map, &ly_land_precipitation);
  write_map_to_file(lrain_map, WORLD_MAP_FILE_LRAIN);

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
  size_t i, sofar = 0;
  world_map_pos xy;
  world_region *wr;

  for (xy.x = 0; xy.x < wm->width; xy.x += 1) {
    for (xy.y = 0; xy.y < wm->height; xy.y += 1) {
      sofar += 1;
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      wr->world = wm;
      // Set position information:
      wr->pos.x = xy.x;
      wr->pos.y = xy.y;
      // Default height info:
      wr->topography.terrain_height.z = 0;
      wr->topography.terrain_height.dx = 0;
      wr->topography.terrain_height.dy = 0;
      wr->topography.geologic_height = 0;
      wr->topography.flow_potential = 0;
      wr->topography.downhill = NULL;
      wr->topography.uphill = NULL;

      // Default hydrology info:
      wr->climate.water.state = WM_HS_LAND;
      wr->climate.water.body = NULL;
      wr->climate.water.water_table = 0; // TODO: get rid of water table?
      wr->climate.water.salt = WM_SL_FRESH;

      // Default ecology info:
      wr->ecology.biome_count = 0;
      for (i = 0; i < WM_MAX_BIOME_OVERLAP; ++i) {
        wr->ecology.biomes[i] = NULL;
      }

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
      wr->topography.downhill = NULL;
      wr->topography.uphill = NULL;
      z = wr->topography.terrain_height.z;
      wr->topography.terrain_height.dx = 0;
      wr->topography.terrain_height.dy = 0;
      min_neighbor_height = z;
      max_neighbor_height = z;
      xdivisor = 0;
      ydivisor = 0;
      for (iter.x = xy.x - 1; iter.x <= xy.x + 1; iter.x += 1) {
        for (iter.y = xy.y - 1; iter.y <= xy.y + 1; iter.y += 1) {
          if (iter.x == xy.x && iter.y == xy.y) { continue; }
          nb = get_world_region(wm, &iter);
          if (nb == NULL) { continue; }
          nbz = nb->topography.terrain_height.z;
          // figure out up- and down-hill neighbors:
          if (nbz < min_neighbor_height) {
            wr->topography.downhill = nb;
            min_neighbor_height = nbz;
          }
          if (nbz > max_neighbor_height) {
            wr->topography.uphill = nb;
            max_neighbor_height = nbz;
          }
          // add up nearby height differences:
          if (iter.x < xy.x) {
            if (iter.y == xy.y) {
              wr->topography.terrain_height.dx += (z - nbz);
              xdivisor += 1;
            } else {
              wr->topography.terrain_height.dx += (z - nbz) * 0.5;
              xdivisor += 0.5;
            }
          } else if (iter.x > xy.x) {
            if (iter.y == xy.y) {
              wr->topography.terrain_height.dx += (nbz - z);
              xdivisor += 1;
            } else {
              wr->topography.terrain_height.dx += (nbz - z) * 0.5;
              xdivisor += 0.5;
            }
          }
          if (iter.y < xy.y) {
            if (iter.x == xy.x) {
              wr->topography.terrain_height.dy += (z - nbz);
              ydivisor += 1;
            } else {
              wr->topography.terrain_height.dy += (z - nbz) * 0.5;
              ydivisor += 0.5;
            }
          } else if (iter.y > xy.y) {
            if (iter.x == xy.x) {
              wr->topography.terrain_height.dy += (nbz - z);
              ydivisor += 1;
            } else {
              wr->topography.terrain_height.dy += (nbz - z) * 0.5;
              ydivisor += 0.5;
            }
          }
        }
      }
      wr->topography.terrain_height.dx /= xdivisor;
      wr->topography.terrain_height.dy /= ydivisor;
      wr->topography.terrain_height.dx /= (float) WORLD_REGION_BLOCKS;
      wr->topography.terrain_height.dy /= (float) WORLD_REGION_BLOCKS;
#ifdef DEBUG
      // TODO: Get rid of this?
      if (
        isnan(wr->topography.terrain_height.dx)
      ||
        isnan(wr->topography.terrain_height.dy)
      ) {
        printf("ERROR!\n");
        exit(EXIT_FAILURE);
      }
#endif
    }
  }
}

void world_cell(world_map *wm, global_pos *glpos, cell *result) {
  static world_region *neighborhood[9];
  world_map_pos wmpos;
  // default values:
  result->blocks[0] = b_make_block(B_VOID);
  result->blocks[1] = b_make_block(B_VOID);

  // get neighborhood pointers:
  glpos__wmpos(glpos, &wmpos);
  get_world_neighborhood_small(wm, &wmpos, neighborhood);

  if (glpos->z < 0 || neighborhood[4] == NULL) {
    // Outside the world:
    result->blocks[0] = b_make_block(B_BOUNDARY);
  } else {
    terrain_cell(wm, neighborhood, glpos, result);
  }
  if (b_is(result->blocks[0], B_VOID)) {
    result->blocks[0] = b_make_block(B_AIR);
  }
}

void generate_chunk(chunk *c) {
  block_index idx;
  global_pos glpos;
  // Generate base materials:
  idx.xyz.w = 0;
  for (idx.xyz.x = 0; idx.xyz.x < CHUNK_SIZE; ++idx.xyz.x) {
    for (idx.xyz.y = 0; idx.xyz.y < CHUNK_SIZE; ++idx.xyz.y) {
      for (idx.xyz.z = 0; idx.xyz.z < CHUNK_SIZE; ++idx.xyz.z) {
        cidx__glpos(c, &idx, &glpos);
        world_cell(THE_WORLD, &glpos, c_cell(c, idx));
      }
    }
  }
}
