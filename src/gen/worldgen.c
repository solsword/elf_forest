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

/*************
 * Functions *
 *************/

void setup_worldgen(ptrdiff_t seed) {
  setup_terrain_gen(seed);
  seed = prng(seed);
  THE_WORLD = create_world_map(seed, WORLD_WIDTH, WORLD_HEIGHT);
  printf("  ...initializing world...\n");
  init_world_map(THE_WORLD);
  printf("  ...generating geology...\n");
  generate_geology(THE_WORLD);
  printf("  ...generating hydrology...\n");
  generate_hydrology(THE_WORLD);
  printf("  ...generating climate...\n");
  generate_climate(THE_WORLD);
  printf("  ...writing world maps...\n");
  texture *base_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *regions_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *temp_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *wind_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *evap_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *cloud_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *pq_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  texture *rain_map = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  render_map_layer(THE_WORLD, base_map, &ly_terrain_height);
  render_map_layer(THE_WORLD, regions_map, &ly_georegions);
  render_map_layer(THE_WORLD, temp_map, &ly_temperature);
  render_map_layer(THE_WORLD, wind_map, &ly_terrain_height);
  render_map_vectors(THE_WORLD, wind_map, PX_BLACK,PX_WHITE, &vly_wind_vectors);
  render_map_layer(THE_WORLD, evap_map, &ly_evaporation);
  render_map_layer(THE_WORLD, cloud_map, &ly_cloud_cover);
  render_map_layer(THE_WORLD, pq_map, &ly_precipitation_quotient);
  render_map_layer(THE_WORLD, rain_map, &ly_precipitation);
  write_texture_to_png(base_map, WORLD_MAP_FILE_BASE);
  printf("    ...elevation...\n");
  write_texture_to_png(regions_map, WORLD_MAP_FILE_REGIONS);
  printf("    ...geographic regions...\n");
  write_texture_to_png(temp_map, WORLD_MAP_FILE_TEMP);
  printf("    ...temperature...\n");
  write_texture_to_png(wind_map, WORLD_MAP_FILE_WIND);
  printf("    ...wind...\n");
  write_texture_to_png(evap_map, WORLD_MAP_FILE_EVAP);
  printf("    ...evaporation...\n");
  write_texture_to_png(cloud_map, WORLD_MAP_FILE_CLOUDS);
  printf("    ...clouds...\n");
  write_texture_to_png(pq_map, WORLD_MAP_FILE_PQ);
  printf("    ...precipitation_quotient...\n");
  write_texture_to_png(rain_map, WORLD_MAP_FILE_RAIN);
  printf("    ...precipitation...\n");
  cleanup_texture(base_map);
  cleanup_texture(regions_map);
  cleanup_texture(temp_map);
  cleanup_texture(wind_map);
  cleanup_texture(evap_map);
  cleanup_texture(cloud_map);
  cleanup_texture(pq_map);
  cleanup_texture(rain_map);
}

void cleanup_worldgen() {
  cleanup_world_map(THE_WORLD);
}

void init_world_map(world_map *wm) {
  size_t sofar = 0;
  float samples_per_region, min_neighbor_height;
  manifold_point gross, stone, dirt;
  world_map_pos xy, iter;
  world_region *wr, *dh;
  region_pos sample_point;

  samples_per_region = (
    WORLD_REGION_SIZE * WORLD_REGION_SIZE
  /
    (REGION_HEIGHT_SAMPLE_FREQUENCY * REGION_HEIGHT_SAMPLE_FREQUENCY)
  );
  for (xy.x = 0; xy.x < wm->width; xy.x += 1) {
    for (xy.y = 0; xy.y < wm->height; xy.y += 1) {
      sofar += 1;
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      // Set position information:
      wr->pos.x = xy.x;
      wr->pos.y = xy.y;
      // Probe chunk heights in the region to get min/max and average:
      wmpos__rpos(&xy, &(wr->anchor));
      wr->min_height = TR_MAX_HEIGHT;
      wr->mean_height = 0;
      wr->max_height = TR_MIN_HEIGHT;
      wr->gross_height.z = 0;
      wr->gross_height.dx = 0;
      wr->gross_height.dy = 0;
      for (
        sample_point.x = wr->anchor.x;
        sample_point.x < wr->anchor.x + WORLD_REGION_BLOCKS;
        sample_point.x += CHUNK_SIZE * REGION_HEIGHT_SAMPLE_FREQUENCY
      ) {
        for (
          sample_point.y = wr->anchor.y;
          sample_point.y < wr->anchor.y + WORLD_REGION_BLOCKS;
          sample_point.y += CHUNK_SIZE * REGION_HEIGHT_SAMPLE_FREQUENCY
        ) {
          compute_terrain_height(&sample_point, &gross, &stone, &dirt);

          // update min
          if (dirt.z < wr->min_height) {
            wr->min_height = dirt.z;
          }
          // update max
          if (dirt.z > wr->max_height) {
            wr->max_height = dirt.z;
          }

          // update mean
          wr->mean_height += dirt.z / samples_per_region;

          // update gross
          wr->gross_height.z += gross.z / samples_per_region;
          wr->gross_height.dx += gross.dx / samples_per_region;
          wr->gross_height.dy += gross.dy / samples_per_region;
        }
      }

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
  // Loop again now that heights are known to find downhill links:
  for (xy.x = 0; xy.x < wm->width; xy.x += 1) {
    for (xy.y = 0; xy.y < wm->height; xy.y += 1) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      wr->downhill = NULL;
      min_neighbor_height = wr->min_height;
      for (iter.x = xy.x - 1; iter.x <= xy.x + 1; iter.x += 1) {
        for (iter.y = xy.y - 1; iter.y <= xy.y + 1; iter.y += 1) {
          if (iter.x == xy.x && iter.y == xy.y) {
            continue;
          }
          dh = get_world_region(wm, &iter);
          if (dh != NULL && dh->min_height < min_neighbor_height) {
            wr->downhill = dh;
            min_neighbor_height = dh->min_height;
          }
        }
      }
    }
  }
}

void world_cell(world_map *wm, region_pos *rpos, cell *result) {
  world_map_pos wmpos, iter;
  world_region *neighborhood[9];
  size_t i = 0;
  rpos__wmpos(rpos, &wmpos);
  // default values:
  result->primary = b_make_block(B_VOID);
  result->secondary = b_make_block(B_VOID);
  result->p_data = 0;
  result->s_data = 0;
  i = 0;
  for (iter.x = wmpos.x - 1; iter.x <= wmpos.x + 1; iter.x += 1) {
    for (iter.y = wmpos.y - 1; iter.y <= wmpos.y + 1; iter.y += 1) {
      neighborhood[i] = get_world_region(wm, &iter);
      i += 1;
    }
  }
  if (rpos->z < 0) {
    result->primary = b_make_block(B_BOUNDARY);
  } else if (neighborhood[4] == NULL) {
    // Outside the world:
    result->primary = b_make_block(B_AIR);
  } else {
    // TODO: Oceans here!
    terrain_cell(wm, neighborhood, rpos, result);
  }
  if (b_is(result->primary, B_VOID)) {
    // TODO: Other things like plants here...
    result->primary = b_make_block(B_AIR);
  }
}
