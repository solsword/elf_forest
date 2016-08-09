#ifndef ECOLOGY_H
#define ECOLOGY_H

// ecology.h
// Generation of ecology.

#include "world/world_map.h"
#include "world/species.h"

/**************
 * Structures *
 **************/

struct eco_info_s;
typedef struct eco_info_s eco_info;

/*************
 * Constants *
 *************/

#define EC_BIOME_TINY_SIZE 5
#define EC_BIOME_SMALL_SIZE 22
#define EC_BIOME_MEDIUM_SIZE 60
#define EC_BIOME_LARGE_SIZE 140
#define EC_BIOME_HUGE_SIZE 300
#define EC_BIOME_GIGANTIC_SIZE 410

/*************************
 * Structure Definitions *
 *************************/

struct eco_info_s { // Note: int better have at least 9 bits...
  size_t max_size;
  int hydro_state_compatibility;
  int salinity_compatibility;
  int altitude_compatibility;
  int precipitation_compatibility;
  int temperature_compatibility;
};

// must be declared after the structure is concrete...
extern eco_info const ECO_INFO[];

/********************
 * Inline Functions *
 ********************/

static inline int biome_is_compatible(biome_category bc, world_region *wr) {
  if (!(wr->climate.water.state & ECO_INFO[bc].hydro_state_compatibility)) {
    return 0;
  };
  if (!(wr->climate.water.salt & ECO_INFO[bc].salinity_compatibility)) {
    return 0;
  };
  if (!(wr->s_altitude & ECO_INFO[bc].altitude_compatibility)) {
    return 0;
  };
  if (!(wr->s_precipitation & ECO_INFO[bc].precipitation_compatibility)) {
    return 0;
  };
  if (!(wr->s_temperature & ECO_INFO[bc].temperature_compatibility)) {
    return 0;
  };
  return 1;
}

static inline int has_biome_in_category(world_region *wr, biome_category bc) {
  size_t i;
  for (i = 0; i < WM_MAX_BIOME_OVERLAP; ++i) {
    if (
      wr->ecology.biomes[i] != NULL
   && wr->ecology.biomes[i]->category == bc
    ) {
      return 1;
    }
  }
  return 0;
}

/*************
 * Functions *
 *************/

// Generates ecological information for the given world map.
void generate_ecology(world_map *wm);

// Used with breadth_first_iter to spread biomes over regions.
step_result fill_with_biome(search_step step, world_region *wr, void* v_biome);

// Fills out any kind of biome (just delegates to one of functions below).
void init_any_biome(biome *b, world_region *wr);

void init_deep_aquatic_biome(biome *b, world_region *wr);
void init_ocean_vents_biome(biome *b, world_region *wr);

void init_sea_ice_biome(biome *b, world_region *wr);
void init_temperate_pelagic_biome(biome *b, world_region *wr);
void init_tropical_pelagic_biome(biome *b, world_region *wr);

void init_temperate_offshore_biome(biome *b, world_region *wr);
void init_tropical_offshore_biome(biome *b, world_region *wr);
void init_temperate_aquatic_grassland_biome(biome *b, world_region *wr);
void init_tropical_aquatic_grassland_biome(biome *b, world_region *wr);
void init_temperate_aquatic_forest_biome(biome *b, world_region *wr);
void init_tropical_aquatic_forest_biome(biome *b, world_region *wr);
void init_cold_reef_biome(biome *b, world_region *wr);
void init_warm_reef_biome(biome *b, world_region *wr);

void init_frozen_beach_biome(biome *b, world_region *wr);
void init_cold_beach_biome(biome *b, world_region *wr);
void init_warm_beach_biome(biome *b, world_region *wr);
void init_tropical_beach_biome(biome *b, world_region *wr);

void init_frozen_lake_biome(biome *b, world_region *wr);
void init_cold_lake_biome(biome *b, world_region *wr);
void init_temperate_lake_biome(biome *b, world_region *wr);
void init_warm_lake_biome(biome *b, world_region *wr);
void init_tropical_lake_biome(biome *b, world_region *wr);
void init_salt_lake_biome(biome *b, world_region *wr);

void init_cold_river_biome(biome *b, world_region *wr);
void init_temperate_river_biome(biome *b, world_region *wr);
void init_warm_river_biome(biome *b, world_region *wr);
void init_tropical_river_biome(biome *b, world_region *wr);

void init_frozen_alpine_biome(biome *b, world_region *wr);
void init_cold_alpine_biome(biome *b, world_region *wr);
void init_temperate_wet_alpine_biome(biome *b, world_region *wr);
void init_temperate_dry_alpine_biome(biome *b, world_region *wr);
void init_warm_wet_alpine_biome(biome *b, world_region *wr);
void init_warm_dry_alpine_biome(biome *b, world_region *wr);
void init_tropical_wet_alpine_biome(biome *b, world_region *wr);
void init_tropical_dry_alpine_biome(biome *b, world_region *wr);

void init_frozen_desert_biome(biome *b, world_region *wr);
void init_cold_desert_biome(biome *b, world_region *wr);
void init_temperate_desert_biome(biome *b, world_region *wr);
void init_warm_desert_biome(biome *b, world_region *wr);
void init_hot_desert_biome(biome *b, world_region *wr);

void init_cold_grassland_biome(biome *b, world_region *wr);
void init_temperate_grassland_biome(biome *b, world_region *wr);
void init_warm_grassland_biome(biome *b, world_region *wr);
void init_tropical_grassland_biome(biome *b, world_region *wr);

void init_cold_shrubland_biome(biome *b, world_region *wr);
void init_temperate_shrubland_biome(biome *b, world_region *wr);
void init_warm_shrubland_biome(biome *b, world_region *wr);
void init_tropical_shrubland_biome(biome *b, world_region *wr);

void init_temperate_savanna_biome(biome *b, world_region *wr);
void init_warm_savanna_biome(biome *b, world_region *wr);
void init_tropical_savanna_biome(biome *b, world_region *wr);

void init_cold_coniferous_forest_biome(biome *b, world_region *wr);
void init_temperate_coniferous_forest_biome(biome *b, world_region *wr);
void init_warm_coniferous_forest_biome(biome *b, world_region *wr);
void init_tropical_coniferous_forest_biome(biome *b, world_region *wr);

void init_temperate_broadleaf_forest_biome(biome *b, world_region *wr);
void init_warm_wet_broadleaf_forest_biome(biome *b, world_region *wr);
void init_warm_dry_broadleaf_forest_biome(biome *b, world_region *wr);
void init_tropical_wet_broadleaf_forest_biome(biome *b, world_region *wr);
void init_tropical_dry_broadleaf_forest_biome(biome *b, world_region *wr);

void init_tundra_biome(biome *b, world_region *wr);
void init_cold_freshwater_wetland_biome(biome *b, world_region *wr);
void init_cold_saltwater_wetland_biome(biome *b, world_region *wr);
void init_temperate_freshwater_wetland_biome(biome *b, world_region *wr);
void init_temperate_saltwater_wetland_biome(biome *b, world_region *wr);
void init_warm_freshwater_wetland_biome(biome *b, world_region *wr);
void init_warm_freshwater_forested_wetland_biome(biome *b, world_region *wr);
void init_warm_saltwater_wetland_biome(biome *b, world_region *wr);
void init_tropical_freshwater_wetland_biome(biome *b, world_region *wr);
void init_tropical_freshwater_forested_wetland_biome(biome *b,world_region *wr);
void init_tropical_saltwater_wetland_biome(biome *b, world_region *wr);
void init_tropical_saltwater_forested_wetland_biome(biome *b, world_region *wr);

#endif // ifndef ECOLOGY_H
