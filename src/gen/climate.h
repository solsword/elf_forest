#ifndef CLIMATE_H
#define CLIMATE_H

// climate.h
// Hydrology and climate.

#include <stdint.h>

#include "world/world_map.h"
#include "terrain.h"

/*************
 * Constants *
 *************/

// Ocean size limits:
#define CL_MIN_OCEAN_SIZE 20
#define CL_MAX_OCEAN_SIZE (-1)

// Lake parameters:
#define CL_MIN_LAKE_SIZE 2
#define CL_MAX_LAKE_SIZE 320

#define CL_LAKE_PROBABILITY 0.15

#define CL_MIN_LAKE_DEPTH 18.0
#define CL_MAX_LAKE_DEPTH 250.0
#define CL_LAKE_DEPTH_DIST_SQUASH 2.5

#define CL_LAKE_SALINITY_THRESHOLD_BRACKISH 0.03
#define CL_LAKE_SALINITY_THRESHOLD_SALINE 0.025
#define CL_LAKE_SALINITY_THRESHOLD_BRINY 0.02

// River parameters:
#define CL_RIVER_SEED_PROB_FLOOR 0.007
#define CL_RIVER_SEED_PROB_CLIMB 1.4
#define CL_RIVER_SEED_PROB_CEILING 0.45

#define CL_RIVER_SEED_CPT_DIST 0.8

#define CL_RIVER_BASE_WIDTH 8.0
#define CL_RIVER_WIDTH_VAR 63.0

#define CL_RIVER_BRANCH_PROB_BASE 0.0001
#define CL_RIVER_BRANCH_PROB_CLIMB 1.3
#define CL_RIVER_BRANCH_PROB_MAX 0.001

//#define CL_RIVER_SEP_BASE 28000.0
#define CL_RIVER_SEP_BASE 28.0
#define CL_RIVER_SEP_VAR 54.0
#define CL_RIVER_SEP_MIN 18.0

// TODO: A better mechanism here!
#define CL_RIVER_SHRINK_PROB 0.0001

#define CL_RIVER_GROWTH_ITERATIONS 16384
//#define CL_RIVER_GROWTH_ITERATIONS 100

// Elevation remapping:
#define CL_ELEV_REMAP_MID 0.4
#define CL_ELEV_REMAP_POWER 1.6
#define CL_ELEV_REMAP_TO 0.7

// Wind parameters:
#define CL_WIND_CELL_DISTORTION_SCALE (3.8 / (100.0 * WORLD_REGION_BLOCKS))
#define CL_WIND_CELL_DISTORTION_STRENGTH (0.17 * (100.0 * WORLD_REGION_BLOCKS))
#define CL_WIND_CELL_SCALE (2.2 / (100.0 * WORLD_REGION_BLOCKS))

// should be roughly the median wind strength
#define CL_WIND_BASE_STRENGTH 3.0

#define CL_WIND_UPPER_STRENGTH 5.0

// how much land slopes affect wind direction
#define CL_WIND_LAND_INFLUENCE 43.0

// Temperature parameters:
#define CL_GLOBAL_TEMP_DISTORTION_SCALE 4.5
#define CL_GLOBAL_TEMP_DISTORTION_STRENGTH 0.15

#define CL_ELEVATION_TEMP_ADJUST (-38.0)

#define CL_ARCTIC_BASE_TEMP (-20.0)
#define CL_EQUATOR_BASE_TEMP 30.0

#define CL_TEMP_HIGH 37.0
#define CL_TEMP_LOW (-30.0)

// Cloud and rainfall parameters:
#define CL_LAND_PRECIPITATION_QUOTIENT 0.085
#define CL_WATER_PRECIPITATION_QUOTIENT 0.085

#define CL_ELEVATION_PRECIPITATION_QUOTIENT 0.45

#define CL_BASE_WATER_CLOUD_POTENTIAL 1500.0
#define CL_BASE_LAND_CLOUD_POTENTIAL 600.0

#define CL_EVAPORATION_NOISE_SCALE 2.4
#define CL_EVAPORATION_NOISE_BASE 1900

#define CL_EVAPORATION_TEMP_SCALING (1.3/30.0)
#define CL_EVAPORATION_TEMP_INFLUENCE 0.6

#define CL_HUGE_CLOUD_POTENTIAL (\
  CL_TEMP_HIGH *\
  CL_EVAPORATION_TEMP_SCALING *\
  (CL_BASE_WATER_CLOUD_POTENTIAL + CL_EVAPORATION_NOISE_BASE)\
)

//#define CL_CLOUD_RECHARGE_RATE 0.03
#define CL_CLOUD_RECHARGE_RATE 0.07

#define CL_WIND_FOCUS_EXP 2.8
#define CL_WIND_FOCUS 9.5

#define CL_CALM_CLOUD_DIFFUSION_RATE 2.0

#define CL_EDGE_CLOUD_POTENTIAL 400.0

#define CL_WIND_ELEVATION_FORCING 2.0


// Water cycle simulation parameters:
#define CL_WATER_CYCLE_SIM_STEPS 64
#define CL_WATER_CYCLE_FINISH_STEPS 1

#define CL_WATER_CYCLE_AVG_ADJ 14.0

// Some rainfall numbers in mm/year:
//
// Regions:
//  25 - Sahara Desert
//  80 - "Most areas of Egypt"
//  240 - Nevada
//  250 - the cutoff for being considered a desert
//  310 - Utah (second-driest in America)
//  345 - Arizona
//  1050 - New York (median wettest state)
//  1600 - Hawaii
//  2500-4000 - Meghalaya, India
//
// Spots:
//  0.75 - Arica, Chile
//  2.5 - Wadi Halfa, Sudan
//  5 - Iquique, Chile
//  12 - Aoulef, Algeria
//  45 - Calexico recently
//  60 - Death Valley
//
//  11900 - Mawsynram, India
//  12900 - Puerto Lopez de Micay, Columbia
//
//  Extremes:
//   0 - some years it just doesn't rain at all in very dry places
//   6200 - driest year at Puerto Lopez de Micay
//   23900 - wettest year at Puerto Lopez de Micay

// Min/max rainfall numbers (millimeters/year):
#define MIN_AVG_PRECIPITATION 10
#define MEAN_AVG_PRECIPITATION 1000
#define MAX_AVG_PRECIPITATION 1800

// Some temperature numbers in degrees Celsius:
//
// Regions:
//  -3 - Alaska
//  11 - Indiana (median American state)
//  21.5 - Florida
//
//  9.5 -- 17 (coastal Egyptian winter)
//  23 -- 31 (coastal Egyptian summer)
//
//  0 -- 18 (Egyptian desert winter)
//  7 -- 40 (Egyptian desert summer)
//
// Spots:
//  -58 - "Dome A", Antarctica
//  16 -- 21.5 -- 28 - Cairo, Egypt
//  34.5 - Dallol, Ethiopia
//
// Extremes:
//  -89.2 - Coldest recorded temperature (Antarctica)
//  56.7 - Hottest recorded temperature (Death Valley)
//
// 30-year average global temperature: 14

// Standard temperature numbers (degrees Celsius):
// TODO: THESE NUMBERS


/********************
 * Inline Functions *
 ********************/

// Takes a height in blocks and returns an "elevation" number in [-1, 1].
// Heights above TR_MAX_HEIGHT or below 0 are truncated.
static inline float elevation(float height) {
  float result = 0;
  if (height > TR_HEIGHT_SEA_LEVEL) {
    result = (
      (height - TR_HEIGHT_SEA_LEVEL)
    /
      (TR_MAX_HEIGHT - TR_HEIGHT_SEA_LEVEL)
    );
  } else {
    result = (
      (height - TR_HEIGHT_SEA_LEVEL)
    /
      TR_HEIGHT_SEA_LEVEL
    );
  }
  if (result < -1) {
    result = -1;
  } else if (result > 1) {
    result = 1;
  } else if (result < 0) {
    result = result;
    result = smooth(
      result,
      0.7,
      (
        (TR_HEIGHT_CONTINENTAL_SHELF - TR_HEIGHT_SEA_LEVEL)
      /
        ((float) TR_HEIGHT_SEA_LEVEL)
      )
    );
  } else if (result < CL_ELEV_REMAP_MID) {
    result /= CL_ELEV_REMAP_MID;
    result = pow(result, CL_ELEV_REMAP_POWER);
    result *= CL_ELEV_REMAP_TO;
  } else {
    result = (result - CL_ELEV_REMAP_MID) / (1.0 - CL_ELEV_REMAP_MID);
    result = (1.0 - CL_ELEV_REMAP_TO) * pow(result, -CL_ELEV_REMAP_POWER);
    result += CL_ELEV_REMAP_TO;
  }
  return result;
}

// Temperature influence on evaporation.
static inline float temp_evap_influence(float temp) {
  temp *= CL_EVAPORATION_TEMP_SCALING;
  temp = 0.5 + CL_EVAPORATION_TEMP_INFLUENCE * temp;
  if (temp < 0) { temp = 0; }
  return temp;
}

// Computes base evaporation for the given world region.
static inline float evaporation(world_region *wr) {
  float result, temp, elev;
  float lat, lon;
  lon = wr->pos.x / ((float) (wr->world->width));
  lat = wr->pos.y / ((float) (wr->world->height));
  // Temperature scaling:
  temp = temp_evap_influence(wr->climate.atmosphere.mean_temp);
  // Elevation scaling:
  elev = elevation(wr->topography.terrain_height.z);
  if (elev < 0) { elev = 0; }
  // The noise component:
  result = sxnoise_2d(
    lat * CL_EVAPORATION_NOISE_SCALE,
    lon * CL_EVAPORATION_NOISE_SCALE,
    prng(wr->world->seed + 71182)
  );
  result = (1 + result) / 2.0;
  result *= CL_EVAPORATION_NOISE_BASE;
  result = result * (1 + temp) * 0.5;
  // The base component:
  if (wr->climate.water.body != NULL) {
    result += CL_BASE_WATER_CLOUD_POTENTIAL * temp * (1 - elev);
  } else {
    result += CL_BASE_LAND_CLOUD_POTENTIAL * temp * (1 - elev);
  }
  return result;
}

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new body of water.
body_of_water* create_body_of_water(float level, salinity salt);

// Frees the memory associated with the given body of water.
void cleanup_body_of_water(body_of_water *body);

// Allocates and returns a new river.
river* create_river(void);

// Frees the memory associated with the given river.
void cleanup_river(river *r);

/*************
 * Functions *
 *************/

// Generates hydrology (rivers, lakes, and the oceans) for the given world.
void generate_hydrology(world_map *wm);

// Generates climate for the given world.
void generate_climate(world_map *wm);

// Once base climate generation is complete, this will (crudely) simulate the
// water cycle, populating precipitation information.
void simulate_water_cycle(world_map *wm);

// A fill step function which takes a body of water and fills ares of the given
// world map with it. If it succeeds, the regions filled will have their
// hydrology info set to point to the given body of water.
step_result fill_water(
  search_step step,
  world_region *wr,
  void* v_body
);

// A fill step function which iterates over shore regions and randomly adds
// river seeds to the world_map's all_rivers list.
step_result seed_rivers(
  search_step step,
  world_region *wr,
  void* ignored
);

#endif // ifndef CLIMATE_H
