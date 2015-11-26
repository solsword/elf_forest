#ifndef ECOLOGY_H
#define ECOLOGY_H

// ecology.h
// Generation of ecology.

#include "world/world_map.h"
#include "world/species.h"

/**************
 * Structures *
 **************/

struct biome_info_s;
typedef struct biome_info_s biome_info;

/*************
 * Constants *
 *************/

#define EC_BIOME_TINY_SIZE 5
#define EC_BIOME_SMALL_SIZE 22
#define EC_BIOME_MEDIUM_SIZE 60
#define EC_BIOME_LARGE_SIZE 140
#define EC_BIOME_HUGE_SIZE 300
#define EC_BIOME_GIGANTIC_SIZE 410

extern biome_info const BIOME_INFO[];

/*************************
 * Structure Definitions *
 *************************/

struct biome_info_s { // Note: int better have at least 9 bits...
  size_t max_size;
  int hydro_state_compatibility;
  int salinity_compatibility;
  int altitude_compatibility;
  int precipitation_compatibility;
  int temperature_compatibility;
};

/*************
 * Functions *
 *************/

// Generates ecological information for the given world map.
void generate_ecology(world_map *wm);

// Used with breadth_first_iter to spread biomes over regions.
step_result fill_with_biome(search_step step, world_region *wr, void* v_biome);

// Functions to initialize single biomes of various types appropriate to a
// given world region.
void init_ocean_biome(world_region *wr, biome *b, ptrdiff_t seed);
void init_beach_biome(world_region *wr, biome *b, ptrdiff_t seed);
void init_lake_biome(world_region *wr, biome *b, ptrdiff_t seed);
void init_river_biome(world_region *wr, biome *b, ptrdiff_t seed);
void init_alpine_biome(world_region *wr, biome *b, ptrdiff_t seed);
void init_terrestrial_biome(world_region *wr, biome *b, ptrdiff_t seed);

// Fills out any kind of biome (just delegates to one of functions below).
void fill_any_biome(biome *b, world_region *wr);

void fill_sea_ice_biome(biome *b, world_region *wr);
void fill_ocean_vents_biome(biome *b, world_region *wr);
void fill_deep_aquatic_biome(biome *b, world_region *wr);
void fill_pelagic_biome(biome *b, world_region *wr);
void fill_offshore_biome(biome *b, world_region *wr);
void fill_aquatic_grassland_biome(biome *b, world_region *wr);
void fill_aquatic_forest_biome(biome *b, world_region *wr);
void fill_cold_reef_biome(biome *b, world_region *wr);
void fill_warm_reef_biome(biome *b, world_region *wr);

void fill_frozen_beach_biome(biome *b, world_region *wr);
void fill_cold_beach_biome(biome *b, world_region *wr);
void fill_warm_beach_biome(biome *b, world_region *wr);
void fill_tropical_beach_biome(biome *b, world_region *wr);

void fill_frozen_lake_biome(biome *b, world_region *wr);
void fill_cold_lake_biome(biome *b, world_region *wr);
void fill_temperate_lake_biome(biome *b, world_region *wr);
void fill_warm_lake_biome(biome *b, world_region *wr);
void fill_tropical_lake_biome(biome *b, world_region *wr);
void fill_salt_lake_biome(biome *b, world_region *wr);

void fill_cold_river_biome(biome *b, world_region *wr);
void fill_temperate_river_biome(biome *b, world_region *wr);
void fill_warm_river_biome(biome *b, world_region *wr);
void fill_tropical_river_biome(biome *b, world_region *wr);

void fill_frozen_alpine_biome(biome *b, world_region *wr);
void fill_cold_alpine_biome(biome *b, world_region *wr);
void fill_temperate_wet_alpine_biome(biome *b, world_region *wr);
void fill_temperate_dry_alpine_biome(biome *b, world_region *wr);
void fill_warm_wet_alpine_biome(biome *b, world_region *wr);
void fill_warm_dry_alpine_biome(biome *b, world_region *wr);
void fill_tropical_wet_alpine_biome(biome *b, world_region *wr);
void fill_tropical_dry_alpine_biome(biome *b, world_region *wr);

void fill_frozen_desert_biome(biome *b, world_region *wr);
void fill_cold_desert_biome(biome *b, world_region *wr);
void fill_temperate_desert_biome(biome *b, world_region *wr);
void fill_warm_desert_biome(biome *b, world_region *wr);
void fill_hot_desert_biome(biome *b, world_region *wr);

void fill_cold_grassland_biome(biome *b, world_region *wr);
void fill_temperate_grassland_biome(biome *b, world_region *wr);
void fill_warm_grassland_biome(biome *b, world_region *wr);
void fill_tropical_grassland_biome(biome *b, world_region *wr);

void fill_cold_shrubland_biome(biome *b, world_region *wr);
void fill_temperate_shrubland_biome(biome *b, world_region *wr);
void fill_warm_shrubland_biome(biome *b, world_region *wr);
void fill_tropical_shrubland_biome(biome *b, world_region *wr);

void fill_temperate_savanna_biome(biome *b, world_region *wr);
void fill_warm_savanna_biome(biome *b, world_region *wr);
void fill_tropical_savanna_biome(biome *b, world_region *wr);

void fill_cold_coniferous_forest_biome(biome *b, world_region *wr);
void fill_temperate_coniferous_forest_biome(biome *b, world_region *wr);
void fill_warm_coniferous_forest_biome(biome *b, world_region *wr);
void fill_tropical_coniferous_forest_biome(biome *b, world_region *wr);

void fill_temperate_broadleaf_forest_biome(biome *b, world_region *wr);
void fill_warm_wet_broadleaf_forest_biome(biome *b, world_region *wr);
void fill_warm_dry_broadleaf_forest_biome(biome *b, world_region *wr);
void fill_tropical_wet_broadleaf_forest_biome(biome *b, world_region *wr);
void fill_tropical_dry_broadleaf_forest_biome(biome *b, world_region *wr);

void fill_tundra_biome(biome *b, world_region *wr);
void fill_cold_freshwater_wetland_biome(biome *b, world_region *wr);
void fill_cold_saltwater_wetland_biome(biome *b, world_region *wr);
void fill_temperate_freshwater_wetland_biome(biome *b, world_region *wr);
void fill_temperate_saltwater_wetland_biome(biome *b, world_region *wr);
void fill_warm_freshwater_wetland_biome(biome *b, world_region *wr);
void fill_warm_freshwater_forested_wetland_biome(biome *b, world_region *wr);
void fill_warm_saltwater_wetland_biome(biome *b, world_region *wr);
void fill_tropical_freshwater_wetland_biome(biome *b, world_region *wr);
void fill_tropical_freshwater_forested_wetland_biome(biome *b,world_region *wr);
void fill_tropical_saltwater_wetland_biome(biome *b, world_region *wr);
void fill_tropical_saltwater_forested_wetland_biome(biome *b, world_region *wr);

#endif // ifndef ECOLOGY_H
