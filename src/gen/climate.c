// climate.c
// Hydrology and climate.

#include <stdint.h>
#ifdef DEBUG
  #include <stdio.h>
#endif

#include <math.h>

#include "datatypes/list.h"
#include "datatypes/queue.h"
#include "datatypes/map.h"

#include "math/curve.h"

#include "world/world_map.h"
#include "world/world.h"

#include "climate.h"

/******************************
 * Constructors & Destructors *
 ******************************/

body_of_water* create_body_of_water(
  hydro_state type,
  float level,
  salinity salt
) {
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

river* create_river() {
  river *result = (river*) malloc(sizeof(river));
  result->path = create_list();
  result->control_points = create_list();
  result->widths = create_list();
  result->depths = create_list();
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
  region->climate.water.state = body->type;
  region->climate.water.body = body;
  region->climate.water.water_table = body->level;
  region->climate.water.salt = body->salt;
}

void _iter_flag_as_water_shore(void *v_region, void *v_body) {
  world_region *region = (world_region*) v_region;
  body_of_water *body = (body_of_water*) v_body;
  if (body->type == WM_HS_OCEAN) {
    region->climate.water.state = WM_HS_OCEAN_SHORE;
  } else if (body->type == WM_HS_LAKE) {
    region->climate.water.state = WM_HS_LAKE_SHORE;
#ifdef DEBUG
  } else {
    printf(
      "ERROR: Body of water has bad type '%x' in _iter_flag_as_water_shore.\n",
      body->type
    );
#endif
  }
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
    salt = WM_SL_BRINY;
  } else if (f < CL_LAKE_SALINITY_THRESHOLD_SALINE) {
    salt = WM_SL_SALINE;
  } else if (f < CL_LAKE_SALINITY_THRESHOLD_BRACKISH) {
    salt = WM_SL_BRACKISH;
  } else {
    salt = WM_SL_FRESH;
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
  water = create_body_of_water(
    WM_HS_LAKE,
    wr->topography.terrain_height.z + f,
    salt
  );
  while (
    water->level >= wr->topography.terrain_height.z + CL_MIN_LAKE_DEPTH
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
    water->level = wr->topography.terrain_height.z + f;
  }
  if (water->level < wr->topography.terrain_height.z + CL_MIN_LAKE_DEPTH) {
    // we failed to make a lake here
    cleanup_body_of_water(water);
  } else {
    l_append_element(wr->world->all_water, water);
  }
}

void _iter_grow_rivers(void *v_river, void *v_wm) {
  static ptrdiff_t salt = 18391204;
  river *r = (river*) v_river;
  river *r_merge;
  world_map *wm = (world_map*) v_wm;
  world_region *wr, *nextwr;
  world_map_pos wmpos;
  global_pos last_glpos, ctrl_glpos;
  float gross_dnh, cont_flow;
  float slope;
  float next_dist;
  float merge_dist;
  float test_dist;
  global_pos gd_glpos, cf_glpos, next;
  ptrdiff_t merge_index;
  geopt prev, phandle;
  geopt nextpt, ctlpt, mergept, testpt;
  size_t i, j;

  salt += wm->seed;
  salt = prng(salt);

  // Get the previous river point and control point:
  prev = (geopt) l_get_item(r->path, l_get_length(r->path) - 1);
  phandle = (geopt) l_get_item(
    r->control_points,
    l_get_length(r->control_points) - 1
  );
  geopt__wmpos(wm, &prev, &wmpos);
  geopt__glpos(wm, &prev, &last_glpos);
  geopt__glpos(wm, &phandle, &ctrl_glpos);
  wr = get_world_region(wm, &wmpos);
  if (
     wr == NULL
  || wr->climate.water.body != NULL
  || (intptr_t) l_get_item(r->widths, l_get_length(r->widths) - 1) == 0
  ) {
    // if we're off the map, in water, or dried up, ignore this river
    return;
  }

  // Figure out two flow-to directions: gross downhill and previous flow line:
  // Gross downhill:
  gross_dnh = mani_downhill(&(wr->topography.terrain_height)); // gross downhill
  // Previous flow line:
  // TODO: Investigate uniform flow continuation starting directions!
  cont_flow = atan2(ctrl_glpos.y - last_glpos.y, ctrl_glpos.x - last_glpos.x);
  // Compute the distance to the next control point:
  next_dist = ptrf(salt) * CL_RIVER_SEP_VAR + CL_RIVER_SEP_BASE;
  salt = prng(salt);
  // Compute region positions from both angles:
  gd_glpos.x = last_glpos.x + cosf(gross_dnh) * next_dist;
  gd_glpos.y = last_glpos.y + sinf(gross_dnh) * next_dist;
  cf_glpos.x = last_glpos.x + cosf(cont_flow) * next_dist;
  cf_glpos.y = last_glpos.y + sinf(cont_flow) * next_dist;
  // final flow direction:
  slope = mani_slope(&(wr->topography.terrain_height));
  if (slope > 1.0) {
    slope = 1.0;
  }
  slope = logdist(slope, 0.08);
  slope *= 0.3;
  next.x = (0.6 + slope) * gd_glpos.x + (0.4 - slope) * cf_glpos.x;
  next.y = (0.6 + slope) * gd_glpos.y + (0.4 - slope) * cf_glpos.y;
  // Make sure we travel a minimum distance:
  next_dist = sqrtf(
    (next.x - last_glpos.x) * (next.x - last_glpos.x) +
    (next.y - last_glpos.y) * (next.y - last_glpos.y)
  );
  if (next_dist < CL_RIVER_SEP_MIN) {
    next_dist = CL_RIVER_SEP_MIN;
    cont_flow = atan2(next.y - last_glpos.y, next.x - last_glpos.x);
    next.x = last_glpos.x + next_dist * cosf(cont_flow);
    next.y = last_glpos.y + next_dist * sinf(cont_flow);
  }
  // Check if we're going off the map:
  glpos__wmpos(&next, &wmpos);
  nextwr = get_world_region(wm, &wmpos);
  if (nextwr == NULL) { // if we are, snap to the edge of the world and end:
    if (next.x < 0) {
      next.x = 0;
    } else if (next.x >= wm->width * WORLD_REGION_BLOCKS) {
      next.x = wm->width * WORLD_REGION_BLOCKS - 1;
    }
    if (next.y < 0) {
      next.y = 0;
    } else if (next.y >= wm->height * WORLD_REGION_BLOCKS) {
      next.y = wm->height * WORLD_REGION_BLOCKS - 1;
    }
    glpos__geopt(wm, &next, &nextpt);
    l_append_element(r->path, (void*) nextpt);
    l_append_element(r->control_points, (void*) nextpt);
    // End the river here:
    l_append_element(r->widths, (void*) 0);
    l_append_element(r->depths, (void*) 0);
  } else { // if not grow the river as normal:
    glpos__geopt(wm, &next, &nextpt);

    // Add a new element to our river:
    l_append_element(r->path, (void*) nextpt);
    // Just use a downhill control point:
    gd_glpos.x = next.x + cosf(gross_dnh) * next_dist/3.0;
    gd_glpos.y = next.y + sinf(gross_dnh) * next_dist/3.0;
    glpos__geopt(wm, &gd_glpos, &ctlpt);
    l_append_element(r->control_points, (void*) ctlpt);
    // Add width/depth values:
    // TODO: Real width/depth values!
    l_append_element(r->widths, (void*) 1);
    l_append_element(r->depths, (void*) 1);
  }

  // Update our new region if necessary:
  geopt__wmpos(wm, &nextpt, &wmpos);
  nextwr = get_world_region(wm, &wmpos);
  if (nextwr != wr && nextwr != NULL) {
    // We don't mind crossing diagonals because world regions will look into
    // adjacent regions when looking for terrain features like rivers.
    if (nextwr->climate.water.rivers[WM_MAX_RIVERS-1] == NULL) {
      for (i = 0; i < WM_MAX_RIVERS; ++i) {
        if (nextwr->climate.water.rivers[i] == r) {
          break;
        } else if (nextwr->climate.water.rivers[i] == NULL) {
          nextwr->climate.water.rivers[i] = r;
          if (nextwr->climate.water.state == WM_HS_LAND) {
            nextwr->climate.water.state = WM_HS_RIVER;
          }
          if (i > 0) {
            // We've hit a region that contains at least one other river: merge
            // with the first river in that region.
            // TODO: Search over all rivers in that region to find the best
            // merge point instead...
            r_merge = nextwr->climate.water.rivers[0];
            merge_index = l_scan_indices(
              r_merge->path,
              (void*) nextwr,
              &find_geopt_in_wr
            ); // shouldn't be possible to miss
#ifdef DEBUG
            if (merge_index < 0) {
              fprintf(
                stderr,
                "Failed to find river node in traversed region for merging!\n"
              );
              exit(1);
            }
#endif
            // Find the first/closest path point for merging:
            mergept = (geopt) l_get_item(r_merge->path, merge_index);
            merge_dist = geodist(wm, nextpt, mergept);
            for (j = merge_index; j < l_get_length(r_merge->path); ++j) {
              testpt = (geopt) l_get_item(r_merge->path, j);
              test_dist = geodist(wm, nextpt, testpt);
              if (test_dist < merge_dist) {
                mergept = testpt;
                merge_dist = test_dist;
                merge_index = j;
              } else {
                // we've started to move farther away
                break;
              }
            }
            // Set our control point to point towards the merge:
            l_pop_element(r->control_points);
            l_append_element(
              r->control_points,
              (void*) geomid(nextpt, mergept)
            );
            // Merge rivers:
            l_append_element(r->path, mergept);
            l_append_element(
              r->control_points,
              l_get_item(r_merge->control_points, merge_index)
            );
            l_append_element(r->widths, (void*) 0);
            l_append_element(r->depths, (void*) 0);
          }
          break;
        }
      }
    } else {
      // Abort this expansion and set our previous node's width/depth to 0:
      l_pop_element(r->path);
      l_pop_element(r->control_points);
      l_pop_element(r->widths);
      l_pop_element(r->depths);
      // Pop/replace the previous path point's width/depth:
      l_pop_element(r->widths);
      l_pop_element(r->depths);
      l_append_element(r->widths, (void*) 0);
      l_append_element(r->depths, (void*) 0);
      return;
    }
  }
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
          nbelev = (
            neighbors[i]->topography.terrain_height.z
          - wr->topography.terrain_height.z
          );
          // Convert to a slope in block units:
          nbelev /= (float) (WORLD_REGION_BLOCKS);
          // truncate:
          if (nbelev < -1.5) {
            nbelev = -1.5;
          } else if (nbelev > 1.5) {
            nbelev = 1.5;
          }
          nbelev = (1.5 + nbelev) * CL_WIND_ELEVATION_FORCING;
          if (wr->topography.terrain_height.z < TR_HEIGHT_SEA_LEVEL) {
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
      (
        wr->topography.terrain_height.z
      - (TR_SCALE_HILLS + TR_SCALE_RIDGES + TR_SCALE_MOUNDS)
      )
    > body->level
    ) {
      // This is a pure land region: do nothing
      return SRESULT_IGNORE;
    } else if (wr->topography.terrain_height.z < body->level) {
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
  } else {
#ifdef DEBUG
    printf("Unknown search/fill step: %d\n", step);
#endif
    return SRESULT_ABORT;
  }
}

void release_river(world_region *wr) {
  river *r;
  geopt rseed;
  ptrdiff_t salt;
  float dnh;
  global_pos glpos;
  size_t ridx;
  for (ridx = 0; ridx < WM_MAX_RIVERS; ++ridx) {
    if (wr->climate.water.rivers[ridx] == NULL) {
      break;
    }
  }
  if (ridx == WM_MAX_RIVERS) {
    // There's no room to create another river here
    return;
  }

  salt = (wr->seed * 71414) + 112;

  // Seed a new river here:
  r = create_river();
  rseed = inner_pt(wr, &salt);
  salt = prng(salt);
  l_append_element(r->widths, (void*) 1); // widths are 1 before flow analysis
  l_append_element(r->depths, (void*) 1); // same with depths
  l_append_element(r->path, rseed); // A random starting point
  rseed = (geopt) l_get_item(r->path, 0);
  geopt__glpos(wr->world, &rseed, &glpos);
  dnh = mani_downhill(&(wr->topography.terrain_height)); // flow downhill
  // TODO: Add a random element here?
  glpos.x += CL_RIVER_SEED_CPT_DIST * CL_RIVER_SEP_BASE * cosf(dnh);
  glpos.y += CL_RIVER_SEED_CPT_DIST * CL_RIVER_SEP_BASE * sinf(dnh);
  if (glpos.x < 0) { glpos.x = 0; } // snap in control point if necessary
  if (glpos.y < 0) { glpos.y = 0; }
  if (glpos.x > wr->world->width * WORLD_REGION_BLOCKS) {
    glpos.x = wr->world->width * WORLD_REGION_BLOCKS;
  }
  if (glpos.y > wr->world->height * WORLD_REGION_BLOCKS) {
    glpos.y = wr->world->height * WORLD_REGION_BLOCKS;
  }
  glpos__geopt(wr->world, &glpos, &rseed); // recycle rseed here
  l_append_element(r->control_points, rseed);
  rseed = (geopt) l_get_item(r->control_points, 0);
  wr->climate.water.rivers[ridx] = r;
  l_append_element(wr->world->all_rivers, r);
}

void generate_hydrology(world_map *wm) {
  world_map_pos xy, nbxy;
  world_region *wr, *nb;
  body_of_water *next_water;
  list *lake_sites = create_list();
  ptrdiff_t lakes_seed = wm->seed + 8177342;
  size_t i;
  uint8_t changed_something;

  next_water = create_body_of_water(
    WM_HS_OCEAN,
    TR_HEIGHT_SEA_LEVEL,
    WM_SL_SALINE
  );

  // First generate world oceans and take note of potential lake sites:
  printf("    ...filling oceans...\n");
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
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
        next_water = create_body_of_water(
          WM_HS_OCEAN,
          TR_HEIGHT_SEA_LEVEL,
          WM_SL_SALINE
        );
      }
      if (wr->climate.water.body == NULL && wr->topography.downhill == NULL) {
        l_append_element(lake_sites, wr);
      }
    }
  }
  // Clean up the extra body of water:
  cleanup_body_of_water(next_water);

  // Find river headwaters (using soil block type as a temporary buffer):
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      if (wr->topography.flow_potential > CL_HEADWATERS_FLOW_THRESHOLD) {
        wr->climate.soil.base_soil.main_block_type = 1;
      } else {
        wr->climate.soil.base_soil.main_block_type = 0;
      }
    }
  }

  // Erode the base dirt buffer we're using upwards:
  changed_something = 1;
  while (changed_something) {
    changed_something = 0;
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = 0; xy.y < wm->height; ++xy.y) {
        wr = get_world_region(wm, &xy); // no need to worry about NULL here
        if (wr->climate.soil.base_soil.main_block_type == 1) {
          for (nbxy.x = xy.x - 1; nbxy.x <= xy.x + 1; ++nbxy.x) {
            for (nbxy.y = xy.y - 1; nbxy.y <= xy.y + 1; ++nbxy.y) {
              nb = get_world_region(wm, &nbxy);
              if (nb == NULL || nb->climate.soil.base_soil.main_block_type==0) {
                continue;
              }
              if (
                nb->topography.flow_potential > CL_HEADWATERS_FLOW_THRESHOLD
              && (
                nb->topography.terrain_height.z
              > wr->topography.terrain_height.z
                )
              ) {
                wr->climate.soil.base_soil.main_block_type = 0;
                changed_something = 1;
                break;
              }
            }
          }
          if (wr->climate.soil.base_soil.main_block_type == 0) {
            break;
          }
        }
      }
    }
  }

  // Run major rivers downwards from headwaters:
  printf("    ...generating rivers...\n");
  // Seed rivers (and reset soil array that we've been using as a temp):
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      if (
        wr->climate.soil.base_soil.main_block_type == 1
     && wr->climate.water.state == WM_HS_LAND
      ) {
        release_river(wr);
      }
      wr->climate.soil.base_soil.main_block_type = 0;
    }
  }
  // DEBUG: REMOVE
  //exit(1);
  // Grow rivers:
  for (i = 0; i < CL_RIVER_GROWTH_ITERATIONS; ++i) {
    // Grow each river once
    printf(
      "      ...%zu / %d river growth iterations completed...\r",
      i,
      CL_RIVER_GROWTH_ITERATIONS
    );
    l_witheach(wm->all_rivers, (void*) wm, &_iter_grow_rivers);
  }
  printf(
    "      ...%d / %d river growth iterations completed...\n",
    CL_RIVER_GROWTH_ITERATIONS,
    CL_RIVER_GROWTH_ITERATIONS
  );

  // Now probabilistically fill every valley with a lake:
  printf("    ...processing %zu lake sites...\n", l_get_length(lake_sites));
  l_witheach(lake_sites, (void*) &lakes_seed, &_iter_fill_lake_site);
  cleanup_list(lake_sites);
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
      elev = elevation(wr->topography.terrain_height.z);

      // Winds:
      // ------
      get_standard_distortion(
        wr->anchor.x, wr->anchor.y, &salt,
        CL_WIND_CELL_DISTORTION_SCALE,
        CL_WIND_CELL_DISTORTION_STRENGTH,
        &dst_x, &dst_y
      );
      trig_component(
        &winds_base,
        wr->anchor.x + dst_x.z, wr->anchor.y + dst_y.z,
        1 + dst_x.dx, dst_x.dy,
        dst_y.dx, 1 + dst_y.dy,
        CL_WIND_CELL_SCALE,
        &salt
      );
      mani_offset_const(&winds_base, 1);
      mani_scale_const(&winds_base, 0.5);

      // Put slopes at around a comparable magnitude with the actual terrain:
      mani_scale_const(
        &winds_base,
        (1.0 / (CL_WIND_CELL_SCALE))
      );
      mani_scale_const(&winds_base, CL_WIND_BASE_STRENGTH);

      // Compute wind strength and direction:
      r = mani_slope(&winds_base);
      theta = mani_contour(&winds_base);
      if (wr->topography.terrain_height.z > TR_HEIGHT_SEA_LEVEL) {
        r2 = mani_slope(
          &(wr->topography.terrain_height)
        ) * CL_WIND_LAND_INFLUENCE;
        theta2 = mani_contour(&(wr->topography.terrain_height));
        // Pick the terrain contour angle that's closest to the wind angle:
        if (angle_between(theta, theta2) > M_PI_2) {
          theta2 += M_PI;
          norm_angle(&theta2);
        }
        // Note we're not changing magnitude here:
        theta = mix_angles(theta, theta2, r / (r + r2));
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
        wr->climate.atmosphere.rainfall[i] = CL_MEAN_AVG_PRECIPITATION;
        wr->climate.atmosphere.temp_low[i] = 16;
        wr->climate.atmosphere.temp_mean[i] = 24;
        wr->climate.atmosphere.temp_high[i] = 32;
      }
    }
  }
}
