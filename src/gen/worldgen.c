// worldgen.c
// World map generation.

#include <math.h>

#include "datatypes/bitmap.h"
#include "datatypes/list.h"
#include "datatypes/queue.h"
#include "datatypes/map.h"
#include "noise/noise.h"
#include "world/blocks.h"
#include "data/data.h"
#include "txgen/cartography.h"
#include "tex/tex.h"
#include "math/manifold.h"
#include "util.h"

#include "geology.h"

#include "worldgen.h"
#include "terrain.h"
#include "climate.h"

/*************
 * Constants *
 *************/

char const * const WORLD_MAP_FILE = "out/world_map.png";

/***********
 * Globals *
 ***********/

world_map* THE_WORLD = NULL;

/******************************
 * Constructors & Destructors *
 ******************************/

world_map *create_world_map(ptrdiff_t seed, wm_pos_t width, wm_pos_t height) {
  size_t sofar;
  float samples_per_region, min_neighbor_height;
  manifold_point gross, stone, dirt;
  world_map *result = (world_map*) malloc(sizeof(world_map));
  world_map_pos xy, iter;
  world_region *wr, *dh;
  region_pos sample_point;

  result->seed = prng(seed);
  result->width = width;
  result->height = height;
  result->regions = (world_region *) calloc(width*height, sizeof(world_region));
  result->all_strata = create_list();
  result->all_water = create_list();
  result->all_biomes = create_list();
  result->all_civs = create_list();

  sofar = 0;
  samples_per_region = (
    WORLD_REGION_SIZE * WORLD_REGION_SIZE
  /
    (REGION_HEIGHT_SAMPLE_FREQUENCY * REGION_HEIGHT_SAMPLE_FREQUENCY)
  );
  for (xy.x = 0; xy.x < result->width; xy.x += 1) {
    for (xy.y = 0; xy.y < result->height; xy.y += 1) {
      sofar += 1;
      wr = get_world_region(result, &xy); // no need to worry about NULL here
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
      wr->seed = hash_3d(xy.x, xy.y, seed + 8731);
      // Randomize the anchor position:
      compute_region_anchor(result, &xy, &(wr->anchor));

      // Print a progress message:
      if (sofar % 100 == 0) {
        printf(
          "    ...%zu / %zu regions initialized...\r",
          sofar,
          (size_t) (result->width * result->height)
        );
      }
    }
  }
  printf(
    "    ...%zu / %zu regions initialized...\r",
    (size_t) (result->width * result->height),
    (size_t) (result->width * result->height)
  );
  printf("\n");
  // Loop again now that heights are known to find downhill links:
  for (xy.x = 0; xy.x < result->width; xy.x += 1) {
    for (xy.y = 0; xy.y < result->height; xy.y += 1) {
      wr = get_world_region(result, &xy); // no need to worry about NULL here
      wr->downhill = NULL;
      min_neighbor_height = wr->min_height;
      for (iter.x = xy.x - 1; iter.x <= xy.x + 1; iter.x += 1) {
        for (iter.y = xy.y - 1; iter.y <= xy.y + 1; iter.y += 1) {
          if (iter.x == xy.x && iter.y == xy.y) {
            continue;
          }
          dh = get_world_region(result, &iter);
          if (dh != NULL && dh->min_height < min_neighbor_height) {
            wr->downhill = dh;
            min_neighbor_height = dh->min_height;
          }
        }
      }
    }
  }
  return result;
}

void cleanup_world_map(world_map *wm) {
  // none of these need special cleanup beyond a free()
  destroy_list(wm->all_strata);
  destroy_list(wm->all_water);
  destroy_list(wm->all_biomes);
  destroy_list(wm->all_civs);
  free(wm->regions);
  free(wm);
}

/*********************
 * Private Functions *
 *********************/

void _iter_flag_as_water_interior(void *v_region, void *v_body) {
  world_region *region = (world_region*) v_region;
  body_of_water *body = (body_of_water*) v_body;
  region->climate.water.state = HYDRO_WATER;
  region->climate.water.body = body;
  region->climate.water.water_table = body->level;
  region->climate.water.salt = body->salt;
}

void _iter_flag_as_water_shore(void *v_region, void *v_body) {
  world_region *region = (world_region*) v_region;
  body_of_water *body = (body_of_water*) v_body;
  region->climate.water.state = HYDRO_SHORE;
  region->climate.water.body = body;
  region->climate.water.water_table = body->level;
  region->climate.water.salt = body->salt - 1;
  if (region->climate.water.salt < 0) {
    region->climate.water.salt = 0;
  }
}

void _iter_fill_lake_site(void *v_wr, void *v_seed) {
  world_region *wr = (world_region*) v_wr;
  ptrdiff_t *seed = (ptrdiff_t*) v_seed;
  float f;
  body_of_water *water;
  salinity salt;

  // Test for existing water:
  if (wr->climate.water.body != NULL) { return; }

  // Test against base lake probability:
  *seed = prng(*seed);
  f = ptrf(*seed); // random on [0, 1]
  if (f > LAKE_PROBABILITY) { // Not our lucky day
    return;
  }

  // Determine salinity:
  *seed = prng(*seed);
  f = ptrf(*seed); // random on [0, 1]
  if (f < LAKE_SALINITY_THRESHOLD_BRINY) {
    salt = SALINITY_BRINY;
  } else if (f < LAKE_SALINITY_THRESHOLD_SALINE) {
    salt = SALINITY_SALINE;
  } else if (f < LAKE_SALINITY_THRESHOLD_BRACKISH) {
    salt = SALINITY_BRACKISH;
  } else {
    salt = SALINITY_FRESH;
  }

  // Determine base depth:
  *seed = prng(*seed);
  f = ptrf(*seed); // random on [0, 1]
  // exponential redistribution over [MIN_LAKE_DEPTH, MAX_LAKE_DEPTH]:
  // average for 22, 250, 4 is: 83.4
  f = MIN_LAKE_DEPTH + MAX_LAKE_DEPTH * exp(LAKE_DEPTH_DIST_SQUASH * (f-1));

  // Create a body of water and attempt to fill with it:
  water = create_body_of_water(wr->min_height + f, salt);
  while (
    water->level >= wr->min_height + MIN_LAKE_DEPTH
  &&
    !fill_water(
      wr->world,
      water,
      &(wr->pos),
      MIN_LAKE_SIZE,
      MAX_LAKE_SIZE
    )
  ) {
    f *= 0.9;
    water->level = wr->min_height + f;
  }
  if (water->level < wr->min_height + MIN_LAKE_DEPTH) {
    // we failed to make a lake here
    cleanup_body_of_water(water);
  } else {
    l_append_element(wr->world->all_water, water);
  }
}

// Computes base evaporation for the given world region.
static inline float _base_evap(world_region *wr) {
  float temp, elev, slope;
  temp = EVAPORATION_TEMP_SCALING * wr->climate.atmosphere.mean_temp;
  temp = 0.7 + 0.4 * temp;
  if (temp < 0) {
    temp = 0;
  }
  if (wr->climate.water.body != NULL) {
    return BASE_WATER_CLOUD_POTENTIAL * temp;
    // TODO: depth/size-based differentials?
  } else {
    elev = (
      (wr->mean_height - TR_HEIGHT_SEA_LEVEL)
    /
      (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL)
    );
    if (elev < 0) { elev = 0; }
    elev *= elev;
    slope = mani_slope(&(wr->gross_height));
    if (slope > 1.5) { slope = 1.5; }
    slope /= 1.5;
    slope = pow(slope, 0.8);
    return BASE_LAND_CLOUD_POTENTIAL * temp * (1 - elev) * (1 - slope);
  }
}

// Simulates cloud movement to compute precipitation (cloud_potential) values.
/* The averaging method
static inline void _water_sim_step(world_region *wr) {
  world_region *neighbor;
  world_map_pos iter;
  size_t i = 0;
  float nbdir, nbwind, nbwindstr;
  float avg = 0, divisor = 9.0;
  float base_evap;
  // Iterate over our neighbors:
  for (iter.x = wr->pos.x - 1; iter.x <= wr->pos.x + 1; iter.x += 1) {
    for (iter.y = wr->pos.y - 1; iter.y <= wr->pos.y + 1; iter.y += 1) {
      neighbor = get_world_region(wr->world, &iter); // might be NULL
      i += 1;
      if (neighbor == NULL) {
        divisor -= 1.0;
        continue;
      }
      if (wr == neighbor) {
        // Our own previous cloud potential is reduced by precipitation:
        avg += (
          wr->climate.atmosphere.cloud_potential
        *
          (1 - wr->climate.atmosphere.precipitation_quotient)
        );
      } else {
        // Direction from our neighbor to us:
        nbdir = atan2(
          wr->pos.y - neighbor->pos.y,
          wr->pos.x - neighbor->pos.x
        );
        // Our neighbor's wind direction:
        nbwind = (
          1 + cosf(neighbor->climate.atmosphere.wind_direction - nbdir)
        ) / 2.0;
        // Our neighbor's wind strength:
        nbwindstr = sqrtf(
          neighbor->climate.atmosphere.wind_strength / WIND_UPPER_STRENGTH
        );
        if (nbwindstr > 1.0) {
          nbwindstr = 1.0;
        }
        nbwind = nbwindstr * nbwind + (1 - nbwindstr) * 1.0;
        // Add in our neighbor's cloud potential:
        avg += nbwind * neighbor->climate.atmosphere.cloud_potential;
      }
    }
  }
  // Divide to get a local "weighted average:"
  avg /= divisor;

  // Recharge based on evaporation:
  base_evap = _base_evap(wr);
  if (avg < base_evap) {
    wr->climate.atmosphere.cloud_potential = (
      CLOUD_RECHARGE_RATE * base_evap
    +
      (1 - CLOUD_RECHARGE_RATE) * avg
    );
  } else {
    wr->climate.atmosphere.cloud_potential = avg;
  }
}
// */

//* The cloud-pushing method
static inline void _water_sim_step(world_region *wr) {
  world_region *neighbors[9];
  world_region *neighbor;
  float nbweights[9];
  float wtotal = 0;
  world_map_pos iter;
  size_t i = 0;
  float nbdir, nbwind, nbelev;
  float potential = wr->climate.atmosphere.cloud_potential;
  float windstr = (wr->climate.atmosphere.wind_strength / WIND_UPPER_STRENGTH);
  if (windstr > 1) { windstr = 1; }
  // Store our neighbors in an array:
  for (iter.x = wr->pos.x - 1; iter.x <= wr->pos.x + 1; iter.x += 1) {
    for (iter.y = wr->pos.y - 1; iter.y <= wr->pos.y + 1; iter.y += 1) {
      neighbors[i] = get_world_region(wr->world, &iter); // might be NULL
      if (i == 4) { // ourself
        nbweights[i] = 1 - windstr;
        // focus on moving clouds around:
        nbweights[i] *= nbweights[i];
      } else if (neighbors[i] == NULL) {
        nbweights[i] = 0;
      } else {
        // Direction to our neighbor:
        nbdir = atan2(
          iter.y - wr->pos.y,
          iter.x - wr->pos.x
        );
        // Our wind direction with respect to our neighbor:
        nbwind = (
          1 + cosf(wr->climate.atmosphere.wind_direction - nbdir)
        ) / 2.0;
        nbwind *= nbwind; // tighten the envelope a bit
        nbwind *= WIND_FOCUS;
        // Our neighbor's elevation with respect to us:
        nbelev = neighbors[i]->mean_height - wr->mean_height;
        // Convert to a slope:
        nbelev /= (float) (WORLD_REGION_BLOCKS);
        // truncate:
        if (nbelev < -1.5) {
          nbelev = -1.5;
        } else if (nbelev > 1.5) {
          nbelev = 1.5;
        }
        nbelev = (1.5 + nbelev) * WIND_ELEVATION_FORCING;
        if (wr->mean_height < TR_HEIGHT_SEA_LEVEL) {
          nbelev = 1.0;
        }
        nbweights[i] = windstr * nbwind * nbelev;
      }
      wtotal += nbweights[i];
      i += 1;
    }
  }
  // Now that we've got weights, divy up our cloud potential between our
  // neighbors:
  i = 0;
  for (iter.x = wr->pos.x - 1; iter.x <= wr->pos.x + 1; iter.x += 1) {
    for (iter.y = wr->pos.y - 1; iter.y <= wr->pos.y + 1; iter.y += 1) {
      neighbor = get_world_region(wr->world, &iter); // might be NULL
      if (neighbor != NULL) {
        neighbor->climate.atmosphere.next_cloud_potential += (
          potential * (nbweights[i] / wtotal)
        );
      }
      i += 1;
    }
  }
}
// */

static inline void _water_sim_next(world_region *wr) {
  float evap = _base_evap(wr);
  // Flip states:
  wr->climate.atmosphere.cloud_potential =
    wr->climate.atmosphere.next_cloud_potential;
  wr->climate.atmosphere.next_cloud_potential = 0;
  // Precipitation:
  // DEBUG:
  // wr->climate.atmosphere.cloud_potential *= 0.95;
  //*
  wr->climate.atmosphere.cloud_potential *=
    1 - wr->climate.atmosphere.precipitation_quotient;
  // */
  // Evaporation recharge:
  if (wr->climate.atmosphere.cloud_potential < evap) {
    wr->climate.atmosphere.cloud_potential = (
      CLOUD_RECHARGE_RATE * evap
    +
      (1 - CLOUD_RECHARGE_RATE) * wr->climate.atmosphere.cloud_potential
    );
  }
}

/*************
 * Functions *
 *************/

void setup_worldgen(ptrdiff_t seed) {
  setup_terrain_gen(seed);
  seed = prng(seed);
  THE_WORLD = create_world_map(seed, WORLD_WIDTH, WORLD_HEIGHT);
  printf("  ...generating geology...\n");
  generate_geology(THE_WORLD);
  printf("  ...generating hydrology...\n");
  generate_hydrology(THE_WORLD);
  printf("  ...generating climate...\n");
  generate_climate(THE_WORLD);
  printf("  ...writing world map to '%s'...\n", WORLD_MAP_FILE);
  texture *tx = create_texture(WORLD_WIDTH, WORLD_HEIGHT);
  render_map(THE_WORLD, tx);
  write_texture_to_png(tx, WORLD_MAP_FILE);
  cleanup_texture(tx);
}

void cleanup_worldgen() {
  cleanup_world_map(THE_WORLD);
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
    strata_cell(wm, neighborhood, rpos, result);
  }
  if (b_is(result->primary, B_VOID)) {
    // TODO: Other things like plants here...
    result->primary = b_make_block(B_AIR);
  }
}

void generate_geology(world_map *wm) {
  size_t i, j;
  world_map_pos xy;
  region_pos anchor;
  r_pos_t t;
  stratum *s;

  float avg_size = sqrtf(wm->width*wm->height);
  avg_size *= STRATA_AVG_SIZE;
  avg_size *= WORLD_REGION_BLOCKS;

  map_function profile = MFN_SPREAD_UP;
  geologic_source source = GEO_SEDIMENTAY;
  ptrdiff_t hash, h1, h2, h3, h4, h5;
  world_region *wr;
  for (i = 0; i < MAX_STRATA_LAYERS * STRATA_COMPLEXITY; ++i) {
    // Create a stratum and append it to the list of all strata:
    hash = prng(wm->seed + 567*i);
    h1 = hash_1d(hash);
    h2 = hash_1d(h1);
    h3 = hash_1d(h2);
    h4 = hash_1d(h3);
    h5 = hash_1d(h4);
    switch (h4 % 3) {
      case 0:
        profile = MFN_SPREAD_UP;
        break;
      case 1:
        profile = MFN_TERRACE;
        break;
      case 2:
      default:
        profile = MFN_HILL;
        break;
    }
    switch (h5 % 3) {
      case 0:
        source = GEO_IGNEOUS;
        break;
      case 1:
        source = GEO_METAMORPHIC;
        break;
      case 2:
      default:
        source = GEO_SEDIMENTAY;
        break;
    }
    s = create_stratum(
      hash,
      float_hash_1d(hash)*wm->width, float_hash_1d(h1)*wm->height,
      avg_size * (0.6 + float_hash_1d(h2)*0.8), // size
      BASE_STRATUM_THICKNESS * exp(-0.5 + float_hash_1d(h3)*3.5), // thickness
      profile, // profile
      source
    );
    l_append_element(wm->all_strata, (void*) s);
    // Render the stratum into the various regions:
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = 0; xy.y < wm->height; ++xy.y) {
        // Compute thickness to determine if this region needs to be aware of
        // this stratum:
        compute_region_anchor(wm, &xy, &anchor);
        t = compute_stratum_height(s, &anchor);
        // If any corner has material, add this stratum to this region:
        if (t > 0) {
          //TODO: Real logging/debugging
          wr = get_world_region(wm, &xy); // no need to worry about NULL here
          if (wr->geology.stratum_count < MAX_STRATA_LAYERS) {
            // adjust existing strata:
            for (j = 0; j < wr->geology.stratum_count; ++j) {
              wr->geology.bottoms[j] *= (
                wr->geology.total_height
              ) / (
                wr->geology.total_height + t
              );
            }
            wr->geology.total_height += t;
            wr->geology.bottoms[wr->geology.stratum_count] = 1 - (
              t / fmax(BASE_STRATUM_THICKNESS*6, wr->geology.total_height)
              // the higher of the new total height or approximately 6 strata
              // of height
            );
            wr->geology.strata[wr->geology.stratum_count] = s;
            wr->geology.stratum_count += 1;
          } // it's okay if some strata are zoned out by the layers limit
        }
      }
    }
    if (i % 10 == 0) {
      printf(
        "    ...%zu / %zu strata done...\r",
        i,
        (size_t) (MAX_STRATA_LAYERS * STRATA_COMPLEXITY)
      );
    }
  }
  printf(
    "    ...%zu / %zu strata done...\r",
    (size_t) (MAX_STRATA_LAYERS * STRATA_COMPLEXITY),
    (size_t) (MAX_STRATA_LAYERS * STRATA_COMPLEXITY)
  );
  printf("\n");
}

void generate_hydrology(world_map *wm) {
  world_map_pos xy;
  world_region *wr;
  body_of_water *next_water;
  list *lake_sites = create_list();
  ptrdiff_t lakes_seed = wm->seed + 8177342;

  next_water = create_body_of_water(TR_HEIGHT_SEA_LEVEL, SALINITY_SALINE);

  // First generate world oceans and take note of potential lake sites:
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      wr->world = wm;
      // Note that hydrology defaults are set up in create_world_map
      if (
        wr->climate.water.body == NULL
      &&
        fill_water(wm, next_water, &xy, MIN_OCEAN_SIZE, MAX_OCEAN_SIZE)
      ) {
        l_append_element(wm->all_water, next_water);
        next_water = create_body_of_water(TR_HEIGHT_SEA_LEVEL, SALINITY_SALINE);
      }
      if (wr->climate.water.body == NULL && wr->downhill == NULL) {
        l_append_element(lake_sites, wr);
      }
    }
  }
  // Clean up the extra body of water:
  cleanup_body_of_water(next_water);

  // Now probabilistically fill every valley with a lake:
  l_witheach(lake_sites, (void*) &lakes_seed, &_iter_fill_lake_site);
  cleanup_list(lake_sites);
}

void generate_climate(world_map *wm) {
  world_map_pos xy;
  world_region *wr;
  size_t i;
  float lat, lon;
  float r, theta, r2, theta2;
  float base_temp, elev, elev2, windstr, pq;
  manifold_point winds_base, dst_x, dst_y;

  ptrdiff_t seed = prng(wm->seed) + 12810;
  ptrdiff_t salt = seed;

  // Loop over the world and compute base climate values:
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      lon = xy.x / (float) (wm->width);
      lat = xy.y / (float) (wm->height);
      salt = seed;

      // Winds:
      // ------
      get_standard_distortion(
        wr->anchor.x, wr->anchor.y, &salt,
        TR_DFQ_CONTINENTS * WIND_CELL_DISTORTION_SIZE,
        TR_DS_CONTINENTS * WIND_CELL_DISTORTION_STRENGTH,
        &dst_x, &dst_y
      );
      trig_component(
        &winds_base,
        wr->anchor.x + dst_x.z, wr->anchor.y + dst_y.z,
        1 + dst_x.dx, dst_x.dy,
        dst_y.dx, 1 + dst_y.dy,
        TR_FREQUENCY_CONTINENTS * WIND_CELL_SIZE,
        &salt
      );
      mani_offset_const(&winds_base, 1);
      mani_scale_const(&winds_base, 0.5);

      // DEBUG:
      /*
      wr->mean_height = TR_HEIGHT_SEA_LEVEL + (
        TR_HEIGHT_MOUNTAIN_TOPS - TR_HEIGHT_SEA_LEVEL
      ) * winds_base.z;
      wr->climate.water.body = NULL;
      // */

      // Put slopes at around a comparable magnitude with the actual terrain:
      mani_scale_const(
        &winds_base,
        (1.0 / (TR_FREQUENCY_CONTINENTS * WIND_CELL_SIZE))
      );
      mani_scale_const(&winds_base, WIND_BASE_STRENGTH);

      // Compute wind strength and direction:
      r = mani_slope(&winds_base);
      theta = mani_contour(&winds_base);
      if (wr->min_height > TR_HEIGHT_SEA_LEVEL) {
        r2 = mani_slope(&(wr->gross_height)) * WIND_LAND_INFLUENCE;
        theta2 = mani_contour(&(wr->gross_height));
        // Note we're not changing magnitude here:
        theta = (
          (r / (r + r2)) * theta
        +
          (r2 / (r + r2)) * theta2
        );
      }
      wr->climate.atmosphere.wind_strength = r;
      wr->climate.atmosphere.wind_direction = theta;
      windstr = (wr->climate.atmosphere.wind_strength / WIND_UPPER_STRENGTH);
      if (windstr > 1) { windstr = 1; }

      // Base temperatures:
      // ------------------
      // Start with a cosine curve modulated by some simplex noise:
      base_temp = (1 - cosf(lat * 2 * M_PI)) / 2.0;
      base_temp = pow(base_temp, 0.8);
      base_temp += GLOBAL_TEMP_DISTORTION_STRENGTH * sxnoise_2d(
        lat * GLOBAL_TEMP_DISTORTION_SCALE,
        lon * GLOBAL_TEMP_DISTORTION_SCALE,
        salt
      );
      salt = prng(salt);
      // Scale the result to fit between arctic and equatorial temperatures
      // (note that the simplex noise may push it slightly outside of the
      // strict range):
      base_temp = (
        ARCTIC_BASE_TEMP
      +
        (EQUATOR_BASE_TEMP - ARCTIC_BASE_TEMP) * base_temp
      );
      // Now adjust for elevation:
      elev = (
        (wr->mean_height - TR_HEIGHT_SEA_LEVEL)
      /
        (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL)
      );
      if (elev < 0) { elev = 0; }
      elev2 = elev * elev;
      base_temp += ELEVATION_TEMP_ADJUST * elev2;
      // Reign-in ultra-cold temperatures:
      if (base_temp < ARCTIC_BASE_TEMP) {
        base_temp = ARCTIC_BASE_TEMP - sqrtf(ARCTIC_BASE_TEMP - base_temp);
      }
      // Set the mean_temp value:
      wr->climate.atmosphere.mean_temp = base_temp;
      // DEBUG:
      /*
      wr->mean_height = (
        TR_HEIGHT_SEA_LEVEL
      +
        (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL) * (
          (
            ((wr->climate.atmosphere.mean_temp) - ARCTIC_BASE_TEMP)
          /
            (EQUATOR_BASE_TEMP - ARCTIC_BASE_TEMP)
          )
        )
      );
      // */

      // Precipitation:
      // --------------
      // First handle the precipitation quotient.
      // Wind direction component:
      if (elev > 0) {
        pq = (
          1 + cosf(
            wr->climate.atmosphere.wind_direction
          -
            mani_uphill(&(wr->gross_height))
          )
        ) / 2.0;
        pq = pq * sqrtf(windstr) + 0.25 * (1 - sqrtf(windstr));
      } else {
        pq = OCEAN_PRECIPITATION_QUOTIENT;
      }
      // Straight elevation component:
      pq += ELEVATION_PRECIPITATION_QUOTIENT * elev2 * elev;
      pq /= 2.0; // average the effects
      if (pq > 1.0) { pq = 1.0; } // truncate into [0, 1]
      pq *= pq;
      wr->climate.atmosphere.precipitation_quotient = pq;
      // Set base cloud potentials:
      wr->climate.atmosphere.cloud_potential = _base_evap(wr);
      wr->climate.atmosphere.next_cloud_potential = 0;
    }
  }

  // Now that most of the climate values have been determined, simulate the
  // water cycle to get precipitation values:
  simulate_water_cycle(wm);

  // Loop over the world again to compute precipitation-dependent values:
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      // DEBUG -- remap:
      /* cloud potential
      wr->mean_height = (
        TR_HEIGHT_SEA_LEVEL
      +
        (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL) * (
          12
        *
          wr->climate.atmosphere.precipitation_quotient
        *
          (
            wr->climate.atmosphere.cloud_potential
          /
            BASE_WATER_CLOUD_POTENTIAL
          )
        )
      );
      // */
      /* gross slope
      wr->mean_height = (
        TR_HEIGHT_SEA_LEVEL
      +
        (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL) * mani_slope(&(wr->gross_height))
      );
      // */
      /* gross height
      wr->mean_height = wr->gross_height.z;
      // */

      // TODO: Real generation past this point!
      for (i = 0; i < N_SEASONS; ++i) {
        wr->climate.atmosphere.rainfall[i] = MEAN_AVG_PRECIPITATION;
        wr->climate.atmosphere.temp_low[i] = 16;
        wr->climate.atmosphere.temp_mean[i] = 24;
        wr->climate.atmosphere.temp_high[i] = 32;
      }
      wr->climate.soil.base_dirt = 0;
      for (i = 0; i < MAX_ALT_DIRTS; ++i) {
        wr->climate.soil.alt_dirt_blocks[i] = B_DIRT;
        wr->climate.soil.alt_dirt_species[i] = 0;
        wr->climate.soil.alt_dirt_strengths[i] = 0;
        wr->climate.soil.alt_dirt_hdeps[i] = 0;
      }
      wr->climate.soil.base_sand = 0;
      for (i = 0; i < MAX_ALT_SANDS; ++i) {
        wr->climate.soil.alt_sand_blocks[i] = B_SAND;
        wr->climate.soil.alt_sand_species[i] = 0;
        wr->climate.soil.alt_sand_strengths[i] = 0;
        wr->climate.soil.alt_sand_hdeps[i] = 0;
      }
    }
  }
}

void simulate_water_cycle(world_map *wm) {
  world_map_pos xy;
  world_region *wr;
  size_t step;

  for (step = 0; step < WATER_CYCLE_SIM_STEPS; ++step) {
    // Each cycle has four iteration phases with different orientations, which
    // should balance each other out in terms of running updates. The running
    // updates in turn allow precipitation information to reach far inland in
    // only a few cycles.

    // Safe order-independent updates:
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = 0; xy.y < wm->height; ++xy.y) {
        wr = get_world_region(wm, &xy);
        _water_sim_step(wr);
      }
    }
    // State flopping and water recharge:
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = 0; xy.y < wm->height; ++xy.y) {
        wr = get_world_region(wm, &xy);
        _water_sim_next(wr);
      }
    }
    /*
    for (xy.y = wm->height - 1; xy.y > -1; --xy.y) {
      for (xy.x = wm->width - 1; xy.x > -1; --xy.x) {
        wr = get_world_region(wm, &xy);
        _water_sim_step(wr);
      }
    }
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = wm->height - 1; xy.y > -1; --xy.y) {
        wr = get_world_region(wm, &xy);
        _water_sim_step(wr);
      }
    }
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      for (xy.x = wm->width - 1; xy.x > -1; --xy.x) {
        wr = get_world_region(wm, &xy);
        _water_sim_step(wr);
      }
    }
    */
    printf(
      "    ...%zu / %d water cycle simulation steps completed...\r",
      step,
      WATER_CYCLE_SIM_STEPS
    );
  }
  printf(
    "    ...%zu / %d water cycle simulation steps completed...\n",
    step,
    WATER_CYCLE_SIM_STEPS
  );
}

void strata_cell(
  world_map *wm,
  world_region* neighborhood[],
  region_pos *rpos,
  cell *result
) {
  static manifold_point dontcare, stone_height, dirt_height;
  static region_pos pr_rpos = { .x = -1, .y = -1, .z = -1 };
  float h;
  world_region *best, *secondbest; // best and second-best regions
  float strbest, strsecond; // their respective strengths

  // DEBUG: (to show the strata)
  //*
  if (
    (
      abs(
        rpos->x -
        ((WORLD_WIDTH / 2.0) * WORLD_REGION_BLOCKS + 2*CHUNK_SIZE)
      ) < CHUNK_SIZE
    ) && (
      rpos->z > (
        rpos->y - (WORLD_HEIGHT/2.0) * WORLD_REGION_BLOCKS
      ) + 8000
      //rpos->z > (rpos->y - (WORLD_HEIGHT*WORLD_REGION_BLOCKS)/2)
    )
  ) {
  //if (abs(rpos->x - 32770) < CHUNK_SIZE) {
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
    return;
  }
  // */

  // DEBUG: Caves to show things off more:
  /*
  if (
    sxnoise_3d(rpos->x*1/12.0, rpos->y*1/12.0, rpos->z*1/12.0, 17) >
      sxnoise_3d(rpos->x*1/52.0, rpos->y*1/52.0, rpos->z*1/52.0, 18)
  ) {
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
    return;
  }
  // */

  if (pr_rpos.x != rpos->x || pr_rpos.y != rpos->y) {
    // No need to recompute the surface height if we're at the same x/y.
    // TODO: Get rid of double-caching here?
    compute_terrain_height(rpos, &dontcare, &stone_height, &dirt_height);
  }

  // Keep track of our previous position:
  copy_rpos(rpos, &pr_rpos);

  // Compute our fractional height:
  h = rpos->z / stone_height.z;

  // Figure out the nearest regions:
  compute_region_contenders(
    wm,
    neighborhood,
    rpos, wm->seed + 317,
    &best, &secondbest, 
    &strbest, &strsecond
  );

  if (h <= 1.0) { // we're in the stone layers
    stone_cell(
      wm, rpos,
      h, stone_height.z,
      best, secondbest, strbest, strsecond,
      result
    );
  } else if (h <= dirt_height.z / stone_height.z) { // we're in dirt
    dirt_cell(
      wm, rpos,
      (rpos->z - stone_height.z) / (dirt_height.z - stone_height.z),
      dirt_height.z,
      best,
      result
    );
  } else if (rpos->z <= TR_HEIGHT_SEA_LEVEL) { // under the ocean
    result->primary = b_make_block(B_WATER);
    result->secondary = b_make_block(B_VOID);
  } else { // we're above the ground
    // TODO: HERE!
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
  }
}

void stone_cell(
  world_map *wm, region_pos *rpos,
  float h, float ceiling,
  world_region *best, world_region *secondbest, float strbest, float strsecond,
  cell *result
) {
  stratum *st;
  // Add some noise to distort the base height:
  // TODO: more spatial variance in noise strength?
  h += (STRATA_FRACTION_NOISE_STRENGTH / ceiling) * (
    sxnoise_3d(
      rpos->x * STRATA_FRACTION_NOISE_SCALE,
      rpos->y * STRATA_FRACTION_NOISE_SCALE,
      rpos->z * STRATA_FRACTION_NOISE_SCALE,
      7193
    ) + 
    0.5 * sxnoise_3d(
      rpos->x * STRATA_FRACTION_NOISE_SCALE * 1.7,
      rpos->y * STRATA_FRACTION_NOISE_SCALE * 1.7,
      rpos->z * STRATA_FRACTION_NOISE_SCALE * 1.7,
      7194
    )
  ) / 1.5;
  // Clamp out-of-range values after noise:
  // TODO: some low-frequency un-clamped noise?
  if (h > 1.0) { h = 1.0; } else if (h < 0.0) { h = 0.0; }

  // Where available, persistence values are also a factor:
  if (best != NULL) {
    st = get_stratum(best, h);
    strbest *= st->persistence;
  }
  if (secondbest != NULL) {
    st = get_stratum(secondbest, h);
    strsecond *= st->persistence;
  }

 // Now that we know which stratum to use, set the cell's block data:
  if (strsecond > strbest) {
    best = secondbest;
  }
  if (best == NULL) {
    // TODO: Various edge types here?
    result->primary = b_make_block(B_STONE);
  } else {
    st = get_stratum(best, h);
    // TODO: veins and inclusions here!
    result->primary = b_make_species(B_STONE, st->base_species);
  }
}

void dirt_cell(
  world_map *wm, region_pos *rpos,
  float h,
  float elev,
  world_region *wr,
  cell *result
) {
  size_t i, max_alts;
  block soil_type;
  species soil_species;
  float beach_height, rel_h, str, beststr;
  float *alt_strengths;
  float *alt_hdeps;
  block *alt_blocks;
  species *alt_species;

  // compute beach height:
  beach_height = BEACH_BASE_HEIGHT;
  beach_height += BEACH_HEIGHT_VAR * sxnoise_2d(
    rpos->x * BEACH_HEIGHT_NOISE_SCALE,
    rpos->y * BEACH_HEIGHT_NOISE_SCALE,
    wr->seed + 18294
  );

  rel_h = elev - TR_HEIGHT_SEA_LEVEL + beach_height;
  beststr = SOIL_ALT_THRESHOLD;
  if (
    rel_h > 0
  ||
    h < 1 - (-rel_h / BEACH_HEIGHT_VAR)
    // TODO: Test this!
  ) { // TODO: lakes!
    soil_type = B_DIRT;
    soil_species = wr->climate.soil.base_dirt;
    alt_strengths = wr->climate.soil.alt_dirt_strengths;
    alt_hdeps = wr->climate.soil.alt_dirt_hdeps;
    alt_blocks = wr->climate.soil.alt_dirt_blocks;
    alt_species = wr->climate.soil.alt_dirt_species;
    max_alts = MAX_ALT_DIRTS;
  } else {
    soil_type = B_SAND;
    soil_species = wr->climate.soil.base_sand;
    alt_strengths = wr->climate.soil.alt_sand_strengths;
    alt_hdeps = wr->climate.soil.alt_sand_hdeps;
    alt_blocks = wr->climate.soil.alt_sand_blocks;
    alt_species = wr->climate.soil.alt_sand_species;
    max_alts = MAX_ALT_SANDS;
  }
  for (i = 0; i < max_alts; ++i) {
    // TODO: Moisture dependence?
    str = sxnoise_3d(
      rpos->x * SOIL_ALT_NOISE_SCALE,
      rpos->y * SOIL_ALT_NOISE_SCALE,
      rpos->z * SOIL_ALT_NOISE_SCALE,
      wr->seed + 4920 * i
    ) + 0.7 * sxnoise_3d(
      rpos->x * SOIL_ALT_NOISE_SCALE * 2.4,
      rpos->y * SOIL_ALT_NOISE_SCALE * 2.4,
      rpos->z * SOIL_ALT_NOISE_SCALE * 2.4,
      wr->seed + 7482 * i
    ) + 0.3 * sxnoise_3d(
      rpos->x * SOIL_ALT_NOISE_SCALE * 3.8,
      rpos->y * SOIL_ALT_NOISE_SCALE * 3.8,
      rpos->z * SOIL_ALT_NOISE_SCALE * 3.8,
      wr->seed + 3194 * i
    );
    str = (2 + str)/4.0; // [0, 1]
    str *= alt_strengths[i];
    if (alt_hdeps[i] > 0) {
      str *= pow(h, alt_hdeps[i]);
    } else {
      str *= pow((1 - h), -alt_hdeps[i]);
    }
    if (str > beststr) {
      beststr = str;
      soil_type = alt_blocks[i];
      soil_species = alt_species[i];
    }
  }
  result->primary = b_make_species(soil_type, soil_species);
}

void compute_region_contenders(
  world_map *wm,
  world_region* neighborhood[],
  region_pos *rpos,
  ptrdiff_t salt,
  world_region **best, world_region **secondbest,
  float *strbest, float *strsecond
) {
  world_region *wr; // best and second-best regions
  int i;
  region_pos anchor;
  vector v, vbest, vsecond;
  float str, noise;
  ptrdiff_t bestseed, secondseed; // seeds for polar noise
  world_map_pos wmpos;

  rpos__wmpos(rpos, &wmpos);

 // Figure out the two nearest world regions:
  // Setup worst-case defaults:
  vbest.x = WORLD_REGION_BLOCKS;
  vbest.y = WORLD_REGION_BLOCKS;
  vbest.z = BASE_STRATUM_THICKNESS * MAX_STRATA_LAYERS;
  *strbest = 0;
  bestseed = 0;
  vsecond.x = WORLD_REGION_BLOCKS;
  vsecond.y = WORLD_REGION_BLOCKS;
  vsecond.z = BASE_STRATUM_THICKNESS * MAX_STRATA_LAYERS;
  *strsecond = 0;
  secondseed = 0;

  *secondbest = NULL;
  *best = NULL;

  // Figure out which of our neighbors are closest:
  wmpos.x -= 1;
  wmpos.y -= 1;
  for (i = 0; i < 9; i += 1) {
    wr = neighborhood[i];
    if (wr != NULL) {
      copy_rpos(&(wr->anchor), &anchor);
    } else {
      compute_region_anchor(wm, &wmpos, &anchor);
    }
    v.x = rpos->x - anchor.x;
    v.y = rpos->y - anchor.y;
    v.z = rpos->z - anchor.z;
    str = 1 - (vmag(&v) / MAX_REGION_ANCHOR_DISTANCE);
    noise = (
      sxnoise_3d(
        v.x * REGION_CONTENTION_NOISE_SCALE,
        v.y * REGION_CONTENTION_NOISE_SCALE,
        v.z * REGION_CONTENTION_NOISE_SCALE,
        9123
      ) + 
      0.6 * sxnoise_3d(
        v.x * REGION_CONTENTION_NOISE_SCALE * 2.1,
        v.y * REGION_CONTENTION_NOISE_SCALE * 2.1,
        v.z * REGION_CONTENTION_NOISE_SCALE * 2.1,
        9124
      )
    ) / 1.6;
    noise = (1 + noise * REGION_CONTENTION_NOISE_STRENGTH) / 2.0;
    // [
    //   0.5 - REGION_CONTENTION_NOISE_STRENGTH/2,
    //   0.5 + R REGION_CONTENTION_NOISE_STRENGTH/2
    // ]
    str *= noise;
    if (str > *strbest) {
      vcopy(&vsecond, &vbest);
      *strsecond = *strbest;
      *secondbest = *best;
      secondseed = bestseed;

      vcopy(&vbest, &v);
      *strbest = str;
      *best = wr;
      if (wr != NULL) {
        bestseed = wr->seed;
      } else {
        bestseed = hash_3d(wmpos.x, wmpos.y, 574);
      }
    } else if (str > *strsecond) {
      vcopy(&vsecond, &v);
      *strsecond = str;
      *secondbest = wr;
      if (wr != NULL) {
        secondseed = wr->seed;
      } else {
        secondseed = hash_3d(wmpos.x, wmpos.y, 574);
      }
    }
    // Update wmpos based on i:
    if (i == 2 || i == 5) {
      wmpos.x += 1;
      wmpos.y -= 2;
    } else {
      wmpos.y += 1;
    }
  }

 // Figure out which stratum dominates:
  // Polar base noise modifies strengths:
  vxyz__polar(&vbest, &v);
  *strbest *= (
    1 + REGION_GEO_STRENGTH_VARIANCE * sxnoise_2d(
      v.y * REGION_GEO_STRENGTH_FREQUENCY,
      v.z * REGION_GEO_STRENGTH_FREQUENCY,
      bestseed + salt
    )
  ); // result is in [0, 1 + REGION_GEO_STRENGTH_VARIANCE]
  vxyz__polar(&vsecond, &v);
  *strsecond *= (
    1 + REGION_GEO_STRENGTH_VARIANCE * sxnoise_2d(
      v.y * REGION_GEO_STRENGTH_FREQUENCY,
      v.z * REGION_GEO_STRENGTH_FREQUENCY,
      secondseed + salt
    )
  ); // result is in [0, 1 + REGION_GEO_STRENGTH_VARIANCE]
}

int fill_water(
  world_map *wm,
  body_of_water *body,
  world_map_pos *origin,
  int min_size,
  int max_size
) {
  world_map_pos wmpos;
  queue *open = create_queue();
  list *interior = create_list();
  list *shore = create_list();
  map *visited = create_map(1, 2048);
  world_region *this, *next;
  int size = 0;

  this = get_world_region(wm, origin);
  if (this != NULL) {
    q_push_element(open, this);
    m_put_value(visited, (void*) 1, (map_key_t) this);
  }

  while (!q_is_empty(open) && (max_size < 0 || size <= max_size) ) {
    // Grab the next open region:
    this = (world_region*) q_pop_element(open);
    if (this->climate.water.body != NULL) {
      // We've hit another body of water (shouldn't be possible?)!
      // All we can do is abort at this point.
      cleanup_queue(open);
      cleanup_list(interior);
      cleanup_list(shore);
      cleanup_map(visited);
      return 0;
    }
    if (
      this->min_height - (TR_SCALE_MOUNDS + TR_SCALE_DETAILS + TR_SCALE_BUMPS)
    >
      body->level
    ) {
      // This is a pure land region: do nothing
      continue;
    } else if (this->max_height > body->level) {
      // This is an interior region
      size += 1;
      l_append_element(interior, this);
    } else {
      // This is a shore region
      size += 1;
      l_append_element(shore, this);
    }
    // Add our neighbors to the open list:
    wmpos.x = this->pos.x + 1;
    wmpos.y = this->pos.y;
    next = get_world_region(wm, &wmpos);
    if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
      q_push_element(open, next);
      m_put_value(visited, (void*) 1, (map_key_t) next);
    }
    wmpos.x -= 2;
    next = get_world_region(wm, &wmpos);
    if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
      q_push_element(open, next);
      m_put_value(visited, (void*) 1, (map_key_t) next);
    }
    wmpos.x += 2;
    wmpos.y -= 1;
    next = get_world_region(wm, &wmpos);
    if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
      q_push_element(open, next);
      m_put_value(visited, (void*) 1, (map_key_t) next);
    }
    wmpos.y += 2;
    next = get_world_region(wm, &wmpos);
    if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
      q_push_element(open, next);
      m_put_value(visited, (void*) 1, (map_key_t) next);
    }
  }
  if ((max_size >= 0 && size > max_size) || size < min_size) {
    // we hit one of the size limits: cleanup and return 0
    cleanup_queue(open);
    cleanup_list(interior);
    cleanup_list(shore);
    cleanup_map(visited);
    return 0;
  }
  // Otherwise we should flag each region as belonging to this body of water
  // and return 1.
  l_witheach(interior, (void*) body, &_iter_flag_as_water_interior);
  l_witheach(shore, (void*) body, &_iter_flag_as_water_shore);
  return 1;
}

void find_valley(world_map *wm, world_map_pos *pos) {
  world_region *wr;
  wr = get_world_region(wm, pos);
  if (wr == NULL) {
    return;
  }

  while (wr->downhill != NULL) {
    wr = wr->downhill;
  }
  copy_wmpos(&(wr->pos), pos);
}
