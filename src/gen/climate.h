#ifndef CLIMATE_H
#define CLIMATE_H

// climate.h
// Hydrology and climate.

#include <stdint.h>

/************************
 * Types and Structures *
 ************************/

struct body_of_water_s;
typedef struct body_of_water_s body_of_water;

struct hydrology_s;
typedef struct hydrology_s hydrology;

struct soil_composition_s;
typedef struct soil_composition_s soil_composition;

struct weather_s;
typedef struct weather_s weather;

/*********
 * Enums *
 *********/

enum hydro_state_e {
  HYDRO_LAND = 0,
  HYDRO_WATER = 1,
  HYDRO_SHORE = 2,
};
typedef enum hydro_state_e hydro_state;

enum salinity_e {
  SALINITY_FRESH = 0,
  SALINITY_BRACKISH = 1,
  SALINITY_SALINE = 2,
  SALINITY_BRINY = 3,
};
typedef enum salinity_e salinity;

/*************
 * Constants *
 *************/

// Ocean size limits:
#define MIN_OCEAN_SIZE 20
#define MAX_OCEAN_SIZE (-1)

// Lake parameters:
#define MIN_LAKE_SIZE 2
#define MAX_LAKE_SIZE 320

#define LAKE_PROBABILITY 0.4

#define MIN_LAKE_DEPTH 18.0
#define MAX_LAKE_DEPTH 250.0
#define LAKE_DEPTH_DIST_SQUASH 4.0

#define LAKE_SALINITY_THRESHOLD_BRACKISH 0.03
#define LAKE_SALINITY_THRESHOLD_SALINE 0.025
#define LAKE_SALINITY_THRESHOLD_BRINY 0.02

// Number of seasons in the year
#define N_SEASONS 5

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

// Maximum alternate dirt/sand types:
#define MAX_ALT_DIRTS 5
#define MAX_ALT_SANDS 3


/***********
 * Globals *
 ***********/

/*************************
 * Structure Definitions *
 *************************/

struct body_of_water_s {
  float level;
  salinity salt;
};

struct hydrology_s {
  hydro_state state; // what kind of region this is
  body_of_water *body; // what body of water this region belongs to
  r_pos_t water_table; // how high the water table is, in blocks
  salinity salt; // the salinity of local groundwater
};

struct soil_composition_s {
  species base_dirt; // dirt (/sand/mud) species for normal soil
  block alt_dirt_blocks[MAX_ALT_DIRTS];
  species alt_dirt_species[MAX_ALT_DIRTS];
  float alt_dirt_strengths[MAX_ALT_DIRTS];
  float alt_dirt_hdeps[MAX_ALT_DIRTS]; // height-dependence
  species base_sand; // sand species (for beaches, rivers, & oceans)
  block alt_sand_blocks[MAX_ALT_SANDS];
  species alt_sand_species[MAX_ALT_SANDS];
  float alt_sand_strengths[MAX_ALT_SANDS];
  float alt_sand_hdeps[MAX_ALT_SANDS]; // height-dependence
};

struct weather_s {
  // TODO: wind chaos?
  float wind_strength, wind_direction; // wind strength & direction
  float rainfall[N_SEASONS]; // rainfall per season
  float temp_low[N_SEASONS]; // temperature low, mean and high throughout the
  float temp_mean[N_SEASONS]; // day, in each season
  float temp_high[N_SEASONS];
};

/******************************
 * Constructors & Destructors *
 ******************************/

// Allocates and returns a new body of water.
body_of_water* create_body_of_water(float level, salinity salt);

// Frees the memory associated with the given body of water.
void cleanup_body_of_water(body_of_water *body);

/*************
 * Functions *
 *************/

#endif // ifndef CLIMATE_H
