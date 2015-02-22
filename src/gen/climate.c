// climate.c
// Hydrology and climate.

#include <stdint.h>
#ifdef DEBUG
  #include <stdio.h>
#endif

#include "datatypes/list.h"
#include "datatypes/queue.h"
#include "datatypes/map.h"

#include "world/world_map.h"
#include "world/world.h"

#include "climate.h"

/******************************
 * Constructors & Destructors *
 ******************************/

body_of_water* create_body_of_water(float level, salinity salt) {
  body_of_water *result = (body_of_water*) malloc(sizeof(body_of_water));
  result->level = level;
  result->salt = salt;
  result->area = 0;
  result->shore_area = 0;
  return result;
}

void cleanup_body_of_water(body_of_water *body) {
  free(body);
}

river* create_river(void) {
  river *result = (river*) malloc(sizeof(river));
  result->path = create_list();
  result->control_points = create_list();
  return result;
}

void cleanup_river(river *r) {
  cleanup_list(r->path);
  cleanup_list(r->control_points);
  free(r);
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
  if (f > CL_LAKE_PROBABILITY) { // Not our lucky day
    return;
  }

  // Determine salinity:
  *seed = prng(*seed);
  f = ptrf(*seed); // random on [0, 1]
  if (f < CL_LAKE_SALINITY_THRESHOLD_BRINY) {
    salt = SALINITY_BRINY;
  } else if (f < CL_LAKE_SALINITY_THRESHOLD_SALINE) {
    salt = SALINITY_SALINE;
  } else if (f < CL_LAKE_SALINITY_THRESHOLD_BRACKISH) {
    salt = SALINITY_BRACKISH;
  } else {
    salt = SALINITY_FRESH;
  }

  // Determine base depth:
  *seed = prng(*seed);
  f = ptrf(*seed); // random on [0, 1]
  // exponential redistribution over [CL_MIN_LAKE_DEPTH, CL_MAX_LAKE_DEPTH]:
  // average for 22, 250, 4 is: 83.4
  f = (
    CL_MIN_LAKE_DEPTH
  +
    CL_MAX_LAKE_DEPTH * exp(CL_LAKE_DEPTH_DIST_SQUASH * (f-1))
  );

  // Create a body of water and attempt to fill with it:
  water = create_body_of_water(wr->min_height + f, salt);
  while (
    water->level >= wr->min_height + CL_MIN_LAKE_DEPTH
  &&
    !breadth_first_iter(
      wr->world,
      &(wr->pos),
      CL_MIN_LAKE_SIZE,
      CL_MAX_LAKE_SIZE,
      (void*) water,
      &fill_water
    )
  ) {
    f *= 0.9;
    water->level = wr->min_height + f;
  }
  if (water->level < wr->min_height + CL_MIN_LAKE_DEPTH) {
    // we failed to make a lake here
    cleanup_body_of_water(water);
  } else {
    l_append_element(wr->world->all_water, water);
  }
}

void _iter_seed_rivers(void *v_body, void *v_wm) {
  body_of_water *body = (body_of_water*) v_body;
  world_map *wm = (world_map*) v_wm;
  breadth_first_iter(
    wm,
    &(body->shorigin),
    0,
    -1,
    (void*) body,
    &seed_rivers
  );
}

// Simulates cloud movement to compute precipitation (cloud_potential) values.
static inline void _water_sim_step(world_region *wr) {
  world_region *neighbors[9];
  world_region *neighbor;
  float nbweights[9];
  float wtotal = 0;
  world_map_pos iter;
  size_t i = 0;
  float nbdir, nbwind, nbelev;
  float potential = wr->climate.atmosphere.cloud_potential;
  float windstr = (wr->climate.atmosphere.wind_strength/CL_WIND_UPPER_STRENGTH);
  if (windstr > 1) { windstr = 1; }
  // Store our neighbors in an array:
  for (iter.x = wr->pos.x - 1; iter.x <= wr->pos.x + 1; iter.x += 1) {
    for (iter.y = wr->pos.y - 1; iter.y <= wr->pos.y + 1; iter.y += 1) {
      neighbors[i] = get_world_region(wr->world, &iter); // might be NULL
      if (i == 4) { // ourself
        nbweights[i] = 1 - windstr;
        // focus on moving clouds around:
        nbweights[i] *= nbweights[i];
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
        nbwind = pow(nbwind, CL_WIND_FOCUS_EXP); // tighten the envelope a bit
        if (neighbors[i] != NULL) {
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
          nbelev = (1.5 + nbelev) * CL_WIND_ELEVATION_FORCING;
          if (wr->mean_height < TR_HEIGHT_SEA_LEVEL) {
            nbelev = 1.0;
          }
        } else {
          // out-of-bounds neighbors are considered to have an equal elevation
          nbelev = 1.0;
          // if our wind is blowing away from an out-of-bounds neighbor, we
          // want to grab some extra cloud as if they were sending some our way
          //*
          wr->climate.atmosphere.next_cloud_potential += (
            (1 - nbwind)
          * 
            windstr
          *
            temp_evap_influence(wr->climate.atmosphere.mean_temp)
          *
            CL_EDGE_CLOUD_POTENTIAL
          );
        }
        nbwind *= CL_WIND_FOCUS;
        // nbweights[i] = nbelev * (
        nbweights[i] = (
          windstr * nbwind
        +
          (1 - windstr) * CL_CALM_CLOUD_DIFFUSION_RATE
        );
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

// Water simulation next-step adjustment: flips states and handles
// precipitation and evaporation.
static inline void _water_sim_next(world_region *wr) {
  float evap = evaporation(wr);
  float rainfall = 0;
  // Flip states:
  wr->climate.atmosphere.cloud_potential =
    wr->climate.atmosphere.next_cloud_potential;
  wr->climate.atmosphere.next_cloud_potential = 0;
  // Precipitation:
  // DEBUG:
  // wr->climate.atmosphere.cloud_potential *= 0.985;
  //*
  if (
    1 - wr->climate.atmosphere.precipitation_quotient > 1
  ||
    1 - wr->climate.atmosphere.precipitation_quotient < 0
  ) {
    printf("Bad pq: %.3f\n", wr->climate.atmosphere.precipitation_quotient);
  }
  rainfall = (
    wr->climate.atmosphere.cloud_potential
  *
    wr->climate.atmosphere.precipitation_quotient
  );
  wr->climate.atmosphere.cloud_potential -= rainfall;
  wr->climate.atmosphere.total_precipitation += rainfall;
  // */
  // Evaporation recharge:
  //*
  if (wr->climate.atmosphere.cloud_potential < evap) {
    wr->climate.atmosphere.cloud_potential = (
      CL_CLOUD_RECHARGE_RATE * evap
    +
      (1 - CL_CLOUD_RECHARGE_RATE) * wr->climate.atmosphere.cloud_potential
    );
  }
  // */
}

// Water sim finishing: a final averaging pass to smooth out high-frequency
// variation in both cloud potential and precipitation quotients:
static inline void _water_sim_finish(world_region *wr) {
  world_region *neighbor;
  world_map_pos iter;
  float cloud_avg = 0, precip_avg = 0, divisor = 9.0;
  // Iterate over our neighbors:
  for (iter.x = wr->pos.x - 1; iter.x <= wr->pos.x + 1; iter.x += 1) {
    for (iter.y = wr->pos.y - 1; iter.y <= wr->pos.y + 1; iter.y += 1) {
      neighbor = get_world_region(wr->world, &iter); // might be NULL
      if (neighbor == NULL) {
        divisor -= 1.0;
        continue;
      }
      cloud_avg += neighbor->climate.atmosphere.cloud_potential;
      precip_avg += neighbor->climate.atmosphere.total_precipitation;
    }
  }
  // Divide to get a local averages:
  wr->climate.atmosphere.next_cloud_potential = cloud_avg / divisor;
  wr->climate.atmosphere.next_total_precipitation = precip_avg / divisor;
}

// Just flips states:
static inline void _water_sim_finish_next(world_region *wr) {
  // Flip states:
  wr->climate.atmosphere.cloud_potential =
    wr->climate.atmosphere.next_cloud_potential;
  wr->climate.atmosphere.next_cloud_potential = 0;
  wr->climate.atmosphere.total_precipitation =
    wr->climate.atmosphere.next_total_precipitation;
  wr->climate.atmosphere.next_total_precipitation = 0;
}

/*************
 * Functions *
 *************/

void generate_hydrology(world_map *wm) {
  world_map_pos xy;
  world_region *wr;
  body_of_water *next_water;
  list *lake_sites = create_list();
  ptrdiff_t lakes_seed = wm->seed + 8177342;

  next_water = create_body_of_water(TR_HEIGHT_SEA_LEVEL, SALINITY_SALINE);

  // First generate world oceans and take note of potential lake sites:
  printf("    ...filling oceans...\n");
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      wr->world = wm;
      // Note that hydrology defaults are set up in create_world_map
      if (
        wr->climate.water.body == NULL
      &&
        breadth_first_iter(
          wm,
          &xy,
          CL_MIN_OCEAN_SIZE,
          CL_MAX_OCEAN_SIZE,
          (void*) next_water,
          &fill_water
        )
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
  printf("    ...processing %zu lake sites...\n", l_get_length(lake_sites));
  l_witheach(lake_sites, (void*) &lakes_seed, &_iter_fill_lake_site);
  cleanup_list(lake_sites);

  // Finally, run rivers upwards from shores:
  printf("    ...generating rivers...\n");
  l_witheach(wm->all_water, (void*) wm, &_iter_seed_rivers);
  // TODO: Grow the river seeds!
}

void generate_climate(world_map *wm) {
  world_map_pos xy;
  world_region *wr;
  size_t i;
  float lat, lon;
  float r, theta, r2, theta2;
  float base_temp, elev, windstr, pq;
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

      // compute "elevation:"
      elev = elevation(wr->mean_height);

      // Winds:
      // ------
      get_standard_distortion(
        wr->anchor.x, wr->anchor.y, &salt,
        TR_DFQ_CONTINENTS * CL_WIND_CELL_DISTORTION_SIZE,
        TR_DS_CONTINENTS * CL_WIND_CELL_DISTORTION_STRENGTH,
        &dst_x, &dst_y
      );
      trig_component(
        &winds_base,
        wr->anchor.x + dst_x.z, wr->anchor.y + dst_y.z,
        1 + dst_x.dx, dst_x.dy,
        dst_y.dx, 1 + dst_y.dy,
        TR_FREQUENCY_CONTINENTS * CL_WIND_CELL_SIZE,
        &salt
      );
      mani_offset_const(&winds_base, 1);
      mani_scale_const(&winds_base, 0.5);

      // Put slopes at around a comparable magnitude with the actual terrain:
      mani_scale_const(
        &winds_base,
        (1.0 / (TR_FREQUENCY_CONTINENTS * CL_WIND_CELL_SIZE))
      );
      mani_scale_const(&winds_base, CL_WIND_BASE_STRENGTH);

      // Compute wind strength and direction:
      r = mani_slope(&winds_base);
      theta = mani_contour(&winds_base);
      // TODO: Choose between contour and opposite direction...
      if (wr->min_height > TR_HEIGHT_SEA_LEVEL) {
        r2 = mani_slope(&(wr->gross_height)) * CL_WIND_LAND_INFLUENCE;
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
      windstr = (wr->climate.atmosphere.wind_strength / CL_WIND_UPPER_STRENGTH);
      if (windstr > 1) { windstr = 1; }

      // Base temperatures:
      // ------------------
      // Start with a cosine curve modulated by some simplex noise:
      base_temp = (1 - cosf(lat * 2 * M_PI)) / 2.0;
      base_temp = pow(base_temp, 0.6);
      base_temp += CL_GLOBAL_TEMP_DISTORTION_STRENGTH * sxnoise_2d(
        lat * CL_GLOBAL_TEMP_DISTORTION_SCALE,
        lon * CL_GLOBAL_TEMP_DISTORTION_SCALE,
        salt
      );
      salt = prng(salt);
      // Scale the result to fit between arctic and equatorial temperatures
      // (note that the simplex noise may push it slightly outside of the
      // strict range):
      base_temp = (
        CL_ARCTIC_BASE_TEMP
      +
        (CL_EQUATOR_BASE_TEMP - CL_ARCTIC_BASE_TEMP) * base_temp
      );
      // Now adjust for elevation above sea level:
      if (elev > 0) {
        base_temp += CL_ELEVATION_TEMP_ADJUST * elev;
      }
      // Reign-in ultra-cold temperatures:
      if (base_temp < CL_ARCTIC_BASE_TEMP) {
        base_temp = CL_ARCTIC_BASE_TEMP - sqrtf(CL_ARCTIC_BASE_TEMP-base_temp);
      }
      // Set the mean_temp value:
      wr->climate.atmosphere.mean_temp = base_temp;

      // Precipitation:
      // --------------
      // First handle the precipitation quotient.
      if (elev < 0) {
        if (wr->climate.water.body != NULL) {
          pq = CL_WATER_PRECIPITATION_QUOTIENT;
        } else {
          pq = CL_LAND_PRECIPITATION_QUOTIENT;
        }
      } else {
        pq = (
          CL_LAND_PRECIPITATION_QUOTIENT
        +
          CL_ELEVATION_PRECIPITATION_QUOTIENT * elev
        );
      }
      if (pq > 1.0) { pq = 1.0; } // truncate into [0, 1]
      wr->climate.atmosphere.precipitation_quotient = pq;
      // Set base cloud potentials:
      wr->climate.atmosphere.cloud_potential = evaporation(wr);
      wr->climate.atmosphere.next_cloud_potential = 0;
      wr->climate.atmosphere.total_precipitation = 0;
      wr->climate.atmosphere.next_total_precipitation = 0;
    }
  }

  // Now that most of the climate values have been determined, simulate the
  // water cycle to get precipitation values:
  simulate_water_cycle(wm);

  // Loop over the world again to compute precipitation-dependent values:
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here

      // TODO: Real generation past this point!
      for (i = 0; i < WM_N_SEASONS; ++i) {
        wr->climate.atmosphere.rainfall[i] = MEAN_AVG_PRECIPITATION;
        wr->climate.atmosphere.temp_low[i] = 16;
        wr->climate.atmosphere.temp_mean[i] = 24;
        wr->climate.atmosphere.temp_high[i] = 32;
      }
      wr->climate.soil.base_dirt = 0;
      for (i = 0; i < WM_MAX_ALT_DIRTS; ++i) {
        wr->climate.soil.alt_dirt_blocks[i] = B_DIRT;
        wr->climate.soil.alt_dirt_species[i] = 0;
        wr->climate.soil.alt_dirt_strengths[i] = 0;
        wr->climate.soil.alt_dirt_hdeps[i] = 0;
      }
      wr->climate.soil.base_sand = 0;
      for (i = 0; i < WM_MAX_ALT_SANDS; ++i) {
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

  for (step = 0; step < CL_WATER_CYCLE_SIM_STEPS; ++step) {
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
    printf(
      "    ...%zu / %d water cycle simulation steps completed...\r",
      step,
      CL_WATER_CYCLE_SIM_STEPS
    );
  }
  printf(
    "    ...%zu / %d water cycle simulation steps completed...\n",
    step,
    CL_WATER_CYCLE_SIM_STEPS
  );
  // Divide out precipitation totals:
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy);
      wr->climate.atmosphere.total_precipitation /=
        ((float) CL_WATER_CYCLE_SIM_STEPS);
      wr->climate.atmosphere.total_precipitation *= CL_WATER_CYCLE_AVG_ADJ;
    }
  }
  // Finish the water simulation with some final averaging:
  for (step = 0; step < CL_WATER_CYCLE_FINISH_STEPS; ++step) {
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = 0; xy.y < wm->height; ++xy.y) {
        wr = get_world_region(wm, &xy);
        _water_sim_finish(wr);
      }
    }
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = 0; xy.y < wm->height; ++xy.y) {
        wr = get_world_region(wm, &xy);
        _water_sim_finish_next(wr);
      }
    }
  }
}

step_result fill_water(
  search_step step,
  world_region *wr,
  void* v_body
) {
  static list *interior, *shore;
  body_of_water *body = (body_of_water*) v_body;
  if (step == SSTEP_INIT) {
    interior = create_list();
    shore = create_list();
    copy_wmpos(&(wr->pos), &(body->origin));
    return SRESULT_CONTINUE;
  } else if (step == SSTEP_CLEANUP) {
    cleanup_list(interior);
    cleanup_list(shore);
    return SRESULT_CONTINUE;
  } else if (step == SSTEP_FINISH) {
    l_witheach(interior, v_body, &_iter_flag_as_water_interior);
    l_witheach(shore, v_body, &_iter_flag_as_water_shore);
    return SRESULT_CONTINUE;
  } else if (step == SSTEP_PROCESS) {
    if (wr->climate.water.body != NULL) {
      return SRESULT_ABORT;
    } else if (
      wr->min_height - (TR_SCALE_MOUNDS + TR_SCALE_DETAILS + TR_SCALE_BUMPS)
    >
      body->level
    ) {
      // This is a pure land region: do nothing
      return SRESULT_IGNORE;
    } else if (wr->max_height < body->level) {
      // This is an interior region
      l_append_element(interior, wr);
      body->area += 1;
      return SRESULT_CONTINUE;
    } else {
      // This is a shore region
      if (l_get_length(shore) == 0) { // the first one becomes the shorigin
        copy_wmpos(&(wr->pos), &(body->shorigin));
      }
      l_append_element(shore, wr);
      body->shore_area += 1;
      return SRESULT_CONTINUE;
    }
#ifdef DEBUG
  } else {
    printf("Unknown search/fill step: %d\n", step);
    return SRESULT_ABORT;
#endif
  }
}

step_result seed_rivers(
  search_step step,
  world_region *wr,
  void* v_body
) {
  static float prob = 0;
  static ptrdiff_t salt = 0;
  geopt rseed;
  region_pos rpos;
  river *r;
  float uph;
  size_t i;
  body_of_water *body = (body_of_water*) v_body;
  // TODO: Use this variable!
  if (step == SSTEP_INIT) {
    prob = CL_RIVER_SEED_PROB_FLOOR;
    salt = prng(wr->seed + 22192);
    return SRESULT_CONTINUE;
  } else if (step == SSTEP_CLEANUP) {
    return SRESULT_CONTINUE;
  } else if (step == SSTEP_FINISH) {
    return SRESULT_CONTINUE;
  } else if (step == SSTEP_PROCESS) {
    if (wr->climate.water.body != body) {
      return SRESULT_IGNORE;
    } else if (wr->climate.water.state == HYDRO_LAND) {
      return SRESULT_IGNORE;
    } else if (wr->climate.water.state != HYDRO_SHORE) {
      return SRESULT_CONTINUE;
    }
    salt = prng(salt);
    if (ptrf(salt) < prob && wr->climate.water.rivers[WM_MAX_RIVERS-1] == NULL){
      // Seed a new river here:
      r = create_river();
      rseed = inner_pt(wr, &salt);
      l_append_element(r->path, rseed); // A random starting point
      geopt__rpos(wr->world, &rseed, &rpos);
      uph = mani_uphill(&(wr->gross_height)); // flow from uphill
      // TODO: Add a random element here?
      rpos.x += WORLD_REGION_BLOCKS * CL_RIVER_SEED_CPT_DIST * cosf(uph);
      rpos.y += WORLD_REGION_BLOCKS * CL_RIVER_SEED_CPT_DIST * sinf(uph);
      if (rpos.x < 0) { rpos.x = 0; } // snap in control point if necessary
      if (rpos.y < 0) { rpos.y = 0; }
      if (rpos.x > wr->world->width * WORLD_REGION_BLOCKS) {
        rpos.x = wr->world->width * WORLD_REGION_BLOCKS;
      }
      if (rpos.y > wr->world->height * WORLD_REGION_BLOCKS) {
        rpos.y = wr->world->height * WORLD_REGION_BLOCKS;
      }
      rpos__geopt(wr->world, &rpos, &rseed);
      l_append_element(r->control_points, rseed);
      for (i = 0; i < WM_MAX_RIVERS; ++i) {
        if (wr->climate.water.rivers[i] == NULL) {
          wr->climate.water.rivers[i] = r;
        }
      }
      l_append_element(wr->world->all_rivers, r);

      // Reset our probability:
      prob = CL_RIVER_SEED_PROB_FLOOR;
    } else {
      // Slowly increase probability:
      if (prob < CL_RIVER_SEED_PROB_CEILING) {
        prob *= CL_RIVER_SEED_PROB_CLIMB;
      }
    }
    return SRESULT_CONTINUE;
#ifdef DEBUG
  } else {
    printf("Unknown search/fill step: %d\n", step);
    return SRESULT_ABORT;
#endif
  }
}
