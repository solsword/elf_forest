// soil.c
// Soil generation.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include "world/world_map.h"
#include "world/species.h"

#include "ecology.h"

/*********************
 * Private Functions *
 *********************/

step_result _iter_spread_soil(
  search_step step,
  world_region *wr,
  void* v_bitmap
) {
  static world_region *origin;
  bitmap *has_data = (bitmap*) v_bitmap;
  if (step == SSTEP_INIT) {
    // Note our origin region:
    origin = wr;
    return SRESULT_CONTINUE;
  } else if (step == SSTEP_CLEANUP) {
    // Nothing to do here
    return SRESULT_CONTINUE;
  } else if (step == SSTEP_FINISH) {
    // Nothing to do here
    return SRESULT_CONTINUE;
  } else if (step == SSTEP_PROCESS) {
    // Check for edge conditions that require a new soil type:
    float oalt = origin->topography.terrain_height.z;
    float halt = wr->topography.terrain_height.z;
    hydrology *owater = &(origin->climate.water);
    hydrology *hwater = &(wr->climate.water);
    weather *oweather = &(origin->climate.atmosphere);
    weather *hweather = &(wr->climate.atmosphere);
    if (
      // Altitude changes:
       ( fabs(oalt - halt) > SL_CP_ALTITUDE )
      // Transitions onto or off of fully submerged regions:
    || ( owater->state == HYDRO_WATER
      && hwater->state != HYDRO_WATER )
    || ( hwater->state == HYDRO_WATER
      && owater->state != HYDRO_WATER )
      // Salinity changes:
    || ( owater->salinity != hwater->salinity )
      // Temperature changes:
    || ( fabs(oweather->mean_temp - hweather->mean_temp) > SL_CP_MEAN_TEMP)
      // Precipitation changes:
    || ( fabs(oweather->total_precipitation - hweather->total_precipitation)
       > SL_CP_PRECIPITATION )
      // There's already soil data here:
    || (bm_check_bit(has_data, wr->pos.x + wr->pos.y * wr->world.width))
    ) {
      // If any change threshold is crossed, this region will get a different
      // soil type:
      return SRESULT_IGNORE;
    }
    // In all other cases, copy over soil information from our origin, note the
    // presence of soil information in our data bitmap, and continue iteration:
    copy_soil(origin, wr);
    bm_set_bits(has_data, wr->pos.x + wr->pos.y * wr->world.width, 1);
    return SRESULT_CONTINUE;
#ifdef DEBUG
  } else {
    printf("Unknown search/fill step: %d\n", step);
    return SRESULT_ABORT;
#endif
  }
}


/*************
 * Functions *
 *************/

void generate_soil(world_map *wm) {
  world_map_pos xy;
  world_region *wr;
  size_t i;
  size_t count, choice;
  ptrdiff_t index;
  ptrdiff_t seed = prng(wm->seed + 9181811);
  // First, ensure that soil data is empty:
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      wr->climate.soil.base_soil.main_block_type = B_VOID;
      wr->climate.soil.base_soil.topsoil_block_type = B_VOID;
      wr->climate.soil.base_soil.main_species = 0;
      wr->climate.soil.base_soil.topsoil_species = 0;
      for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
        wr->climate.soil.base_soil.alt_block_types[i] = B_VOID;
        wr->climate.soil.base_soil.alt_topsoil_block_types[i] = B_VOID;
        wr->climate.soil.base_soil.alt_species[i] = 0;
        wr->climate.soil.base_soil.alt_topsoil_species[i] = 0;
        wr->climate.soil.base_soil.alt_strengths[i] = 0.0;
        wr->climate.soil.base_soil.alt_hdeps[i] = 0.0;
      }

      wr->climate.soil.river_banks.main_block_type = B_VOID;
      wr->climate.soil.river_banks.topsoil_block_type = B_VOID;
      wr->climate.soil.river_banks.main_species = 0;
      wr->climate.soil.river_banks.topsoil_species = 0;
      for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
        wr->climate.soil.river_banks.alt_block_types[i] = B_VOID;
        wr->climate.soil.river_banks.alt_topsoil_block_types[i] = B_VOID;
        wr->climate.soil.river_banks.alt_species[i] = 0;
        wr->climate.soil.river_banks.alt_topsoil_species[i] = 0;
        wr->climate.soil.river_banks.alt_strengths[i] = 0.0;
        wr->climate.soil.river_banks.alt_hdeps[i] = 0.0;
      }

      wr->climate.soil.river_bottoms.main_block_type = B_VOID;
      wr->climate.soil.river_bottoms.topsoil_block_type = B_VOID;
      wr->climate.soil.river_bottoms.main_species = 0;
      wr->climate.soil.river_bottoms.topsoil_species = 0;
      for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
        wr->climate.soil.river_bottoms.alt_block_types[i] = B_VOID;
        wr->climate.soil.river_bottoms.alt_topsoil_block_types[i] = B_VOID;
        wr->climate.soil.river_bottoms.alt_species[i] = 0;
        wr->climate.soil.river_bottoms.alt_topsoil_species[i] = 0;
        wr->climate.soil.river_bottoms.alt_strengths[i] = 0.0;
        wr->climate.soil.river_bottoms.alt_hdeps[i] = 0.0;
      }

      wr->climate.soil.lake_shores.main_block_type = B_VOID;
      wr->climate.soil.lake_shores.topsoil_block_type = B_VOID;
      wr->climate.soil.lake_shores.main_species = 0;
      wr->climate.soil.lake_shores.topsoil_species = 0;
      for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
        wr->climate.soil.lake_shores.alt_block_types[i] = B_VOID;
        wr->climate.soil.lake_shores.alt_topsoil_block_types[i] = B_VOID;
        wr->climate.soil.lake_shores.alt_species[i] = 0;
        wr->climate.soil.lake_shores.alt_topsoil_species[i] = 0;
        wr->climate.soil.lake_shores.alt_strengths[i] = 0.0;
        wr->climate.soil.lake_shores.alt_hdeps[i] = 0.0;
      }

      wr->climate.soil.lake_bottoms.main_block_type = B_VOID;
      wr->climate.soil.lake_bottoms.topsoil_block_type = B_VOID;
      wr->climate.soil.lake_bottoms.main_species = 0;
      wr->climate.soil.lake_bottoms.topsoil_species = 0;
      for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
        wr->climate.soil.lake_bottoms.alt_block_types[i] = B_VOID;
        wr->climate.soil.lake_bottoms.alt_topsoil_block_types[i] = B_VOID;
        wr->climate.soil.lake_bottoms.alt_species[i] = 0;
        wr->climate.soil.lake_bottoms.alt_topsoil_species[i] = 0;
        wr->climate.soil.lake_bottoms.alt_strengths[i] = 0.0;
        wr->climate.soil.lake_bottoms.alt_hdeps[i] = 0.0;
      }

      wr->climate.soil.beaches.main_block_type = B_VOID;
      wr->climate.soil.beaches.topsoil_block_type = B_VOID;
      wr->climate.soil.beaches.main_species = 0;
      wr->climate.soil.beaches.topsoil_species = 0;
      for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
        wr->climate.soil.beaches.alt_block_types[i] = B_VOID;
        wr->climate.soil.beaches.alt_topsoil_block_types[i] = B_VOID;
        wr->climate.soil.beaches.alt_species[i] = 0;
        wr->climate.soil.beaches.alt_topsoil_species[i] = 0;
        wr->climate.soil.beaches.alt_strengths[i] = 0.0;
        wr->climate.soil.beaches.alt_hdeps[i] = 0.0;
      }

      wr->climate.soil.ocean_floor.main_block_type = B_VOID;
      wr->climate.soil.ocean_floor.topsoil_block_type = B_VOID;
      wr->climate.soil.ocean_floor.main_species = 0;
      wr->climate.soil.ocean_floor.topsoil_species = 0;
      for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
        wr->climate.soil.ocean_floor.alt_block_types[i] = B_VOID;
        wr->climate.soil.ocean_floor.alt_topsoil_block_types[i] = B_VOID;
        wr->climate.soil.ocean_floor.alt_species[i] = 0;
        wr->climate.soil.ocean_floor.alt_topsoil_species[i] = 0;
        wr->climate.soil.ocean_floor.alt_strengths[i] = 0.0;
        wr->climate.soil.ocean_floor.alt_hdeps[i] = 0.0;
      }
    }
  }
  // Now that soil data has been emptied, start randomly picking world regions,
  // generating endemic soil for them, and spreading that soil around, until
  // all world regions have soil data:
  bitmap *has_data = create_bitmap(wm->width * wm->height);
  count = bm_size(has_data) - bm_popcount(has_data);
  choice = seed % count;
  seed = prng(seed);
  index = bm_select_open(has_data, choice);
  while (index != -1) {
    xy.x = index % wm->width;
    xy.y = index / wm->width;
    wr = get_world_region(wm, &xy); // we shouldn't have to worry about NULL
#ifdef DEBUG
    if (wr == NULL) {
      fprintf(stderr, "ERROR: soil selection selected region not in map!\n");
      exit(1);
    }
#endif
    create_appropriate_soil(wr);
    breadth_first_iter(
      wm, &xy,
      0, SL_REGION_MAX_SIZE,
      (void*) has_data,
      &_iter_spread_soil
    );
    count = bm_size(has_data) - bm_popcount(has_data);
    if (count == 0) { break; }
    choice = seed % count;
    seed = prng(seed);
    index = bm_select_open(has_data, choice);
  }

  // Finally, work on the soil composition of rivers and lakes by spreading
  // soil types downstream:
  // TODO: This?
  // TODO: Any other postprocessing?
}

void create_appropriate_soil(world_region *wr) {
  // TODO: Things to take into account:
  //   Altitude
  //   Annual precipitation
  //   Mean temperature
  //   Seasonal temperature variability
  //   Min temperature (frost or not?)
  //   Salinity
  //   Bedrock chemistry

  // TODO: HERE!
}
