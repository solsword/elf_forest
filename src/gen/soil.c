// soil.c
// Soil generation.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include "world/world_map.h"
#include "world/species.h"

#include "elements.h"
#include "terrain.h"
#include "climate.h"
#include "ecology.h"

#include "soil.h"

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
    || ( owater->state == WM_HS_OCEAN
      && hwater->state != WM_HS_OCEAN )
    || ( hwater->state == WM_HS_OCEAN
      && owater->state != WM_HS_OCEAN )
    || ( owater->state == WM_HS_LAKE
      && hwater->state != WM_HS_LAKE )
    || ( hwater->state == WM_HS_LAKE
      && owater->state != WM_HS_LAKE )
      // Salinity changes:
    || ( owater->salt != hwater->salt )
      // Temperature changes:
    || ( fabs(oweather->mean_temp - hweather->mean_temp) > SL_CP_MEAN_TEMP)
      // Precipitation changes:
    || ( fabs(oweather->total_precipitation - hweather->total_precipitation)
       > SL_CP_PRECIPITATION )
      // There's already soil data here:
    || (bm_check_bit(has_data, wr->pos.x + wr->pos.y * wr->world->width))
    ) {
      // If any change threshold is crossed, this region will get a different
      // soil type:
      return SRESULT_IGNORE;
    }
    // In all other cases, copy over soil information from our origin, note the
    // presence of soil information in our data bitmap, and continue iteration:
    copy_soil(origin, wr);
    bm_set_bits(has_data, wr->pos.x + wr->pos.y * wr->world->width, 1);
    return SRESULT_CONTINUE;
  } else {
#ifdef DEBUG
    printf("Unknown search/fill step: %d\n", step);
#endif
    return SRESULT_ABORT;
  }
}


/*********************
 * Private Constants *
 *********************/

static rngtable LOAMY_DIRT_ALTS = {
  .size = 7,
  .values = (void*[]) {
    (void*) B_DIRT,
    (void*) B_MUD,
    (void*) B_CLAY,
    (void*) B_SAND,
    (void*) B_GRAVEL,
    (void*) B_SCREE,
    (void*) B_STONE
  },
  .weights = (float[]) {
    200, // dirt
     30, // mud
     20, // clay
     50, // sand
     15, // gravel
     10, // scree
      5 // stone
  }
};

static rngtable SANDY_DIRT_ALTS = {
  .size = 7,
  .values = (void*[]) {
    (void*) B_DIRT,
    (void*) B_MUD,
    (void*) B_CLAY,
    (void*) B_SAND,
    (void*) B_GRAVEL,
    (void*) B_SCREE,
    (void*) B_STONE
  },
  .weights = (float[]) {
     80, // dirt
     10, // mud
     20, // clay
    100, // sand
     40, // gravel
     15, // scree
     10 // stone
  }
};

static rngtable POOR_DIRT_ALTS = {
  .size = 7,
  .values = (void*[]) {
    (void*) B_DIRT,
    (void*) B_MUD,
    (void*) B_CLAY,
    (void*) B_SAND,
    (void*) B_GRAVEL,
    (void*) B_SCREE,
    (void*) B_STONE
  },
  .weights = (float[]) {
    100, // dirt
     10, // mud
     10, // clay
    100, // sand
     50, // gravel
     20, // scree
     15 // stone
  }
};

static rngtable ROCKY_DIRT_ALTS = {
  .size = 7,
  .values = (void*[]) {
    (void*) B_DIRT,
    (void*) B_MUD,
    (void*) B_CLAY,
    (void*) B_SAND,
    (void*) B_GRAVEL,
    (void*) B_SCREE,
    (void*) B_STONE
  },
  .weights = (float[]) {
    100, // dirt
      5, // mud
     15, // clay
     50, // sand
     60, // gravel
     40, // scree
     20 // stone
  }
};

static rngtable VERY_ROCKY_DIRT_ALTS = {
  .size = 7,
  .values = (void*[]) {
    (void*) B_DIRT,
    (void*) B_MUD,
    (void*) B_CLAY,
    (void*) B_SAND,
    (void*) B_GRAVEL,
    (void*) B_SCREE,
    (void*) B_STONE
  },
  .weights = (float[]) {
     50, // dirt
      5, // mud
     15, // clay
     80, // sand
    100, // gravel
     60, // scree
     40 // stone
  }
};

static rngtable DRY_SAND_ALTS = {
  .size = 6,
  .values = (void*[]) {
    (void*) B_DIRT,
    // No mud
    (void*) B_CLAY,
    (void*) B_SAND,
    (void*) B_GRAVEL,
    (void*) B_SCREE,
    (void*) B_STONE
  },
  .weights = (float[]) {
     50, // dirt
      5, // clay
    200, // sand
     60, // gravel
     30, // scree
     20 // stone
  }
};

static rngtable WET_SAND_ALTS = {
  .size = 7,
  .values = (void*[]) {
    (void*) B_DIRT,
    (void*) B_MUD,
    (void*) B_CLAY,
    (void*) B_SAND,
    (void*) B_GRAVEL,
    (void*) B_SCREE,
    (void*) B_STONE
  },
  .weights = (float[]) {
     40, // dirt
     80, // mud
     30, // clay
    100, // sand
     40, // gravel
     20, // scree
      5 // stone
  }
};

// TODO: Use this
static rngtable MUD_ALTS = {
  .size = 7,
  .values = (void*[]) {
    (void*) B_DIRT,
    (void*) B_MUD,
    (void*) B_CLAY,
    (void*) B_SAND,
    (void*) B_GRAVEL,
    (void*) B_SCREE,
    (void*) B_STONE
  },
  .weights = (float[]) {
     90, // dirt
    200, // mud
     70, // clay
     50, // sand
     10, // gravel
      5, // scree
      5 // stone
  }
};

static rngtable OCEAN_DEPTHS_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.05,
    0.55,
    0.10,
    0.20,
    0.10
  }
};

static rngtable CONTINENTAL_SHELF_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.05,
    0.60,
    0.10,
    0.20,
    0.05
  }
};

static rngtable ARCTIC_MOUNTAINS_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.02,
    0.08,
    0.30,
    0.30,
    0.30
  }
};

static rngtable ARCTIC_PLAINS_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.02,
    0.20,
    0.29,
    0.29,
    0.20
  }
};

static rngtable DRY_TUNDRA_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.20,
    0.20,
    0.45,
    0.10,
    0.05
  }
};

static rngtable WET_TUNDRA_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.25,
    0.20,
    0.40,
    0.10,
    0.05
  }
};

static rngtable DESERT_MOUNTAINS_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.05,
    0.40,
    0.20,
    0.20,
    0.15
  }
};

static rngtable DESERT_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.05,
    0.60,
    0.20,
    0.10,
    0.05
  }
};

static rngtable ARID_MOUNTAINS_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.05,
    0.25,
    0.35,
    0.20,
    0.15
  }
};

static rngtable MOUNTAINS_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.20,
    0.10,
    0.30,
    0.25,
    0.15
  }
};

static rngtable WET_MOUNTAINS_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.40,
    0.05,
    0.20,
    0.25,
    0.10
  }
};

static rngtable DRY_HIGHLANDS_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.25,
    0.20,
    0.20,
    0.20,
    0.15
  }
};

static rngtable HIGHLANDS_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.30,
    0.10,
    0.25,
    0.20,
    0.15
  }
};

static rngtable WET_HIGHLANDS_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.40,
    0.05,
    0.25,
    0.20,
    0.10
  }
};

static rngtable ARID_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.05,
    0.30,
    0.50,
    0.10,
    0.05
  }
};

static rngtable COLD_DRY_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.50,
    0.15,
    0.20,
    0.10,
    0.05
  }
};

static rngtable WARM_DRY_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.55,
    0.20,
    0.15,
    0.05,
    0.05
  }
};

static rngtable COLD_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.50,
    0.05,
    0.30,
    0.10,
    0.05
  }
};

static rngtable WARM_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.60,
    0.05,
    0.25,
    0.05,
    0.05
  }
};

static rngtable COLD_WET_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.55,
    0.05,
    0.20,
    0.15,
    0.05
  }
};

static rngtable WARM_WET_ALTS_ALTS = {
  .size = 5,
  .values = (void*[]) {
    (void*) &LOAMY_DIRT_ALTS,
    (void*) &SANDY_DIRT_ALTS,
    (void*) &POOR_DIRT_ALTS,
    (void*) &ROCKY_DIRT_ALTS,
    (void*) &VERY_ROCKY_DIRT_ALTS,
  },
  .weights = (float[]) {
    0.70,
    0.05,
    0.10,
    0.10,
    0.05
  }
};

/*************
 * Functions *
 *************/

void generate_soil(world_map *wm) {
  world_map_pos xy;
  world_region *wr;
  size_t count, choice;
  ptrdiff_t index;
  ptrdiff_t seed = prng(wm->seed + 9181811);
  // First, ensure that soil data is empty:
  for (xy.x = 0; xy.x < wm->width; ++xy.x) {
    for (xy.y = 0; xy.y < wm->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      erase_soil(&(wr->climate.soil));
    }
  }
  // Now that soil data has been emptied, start randomly picking world regions,
  // generating endemic soil for them, and spreading that soil around, until
  // all world regions have soil data:
  bitmap *has_data = create_bitmap(wm->width * wm->height);
  count = bm_size(has_data);
  choice = posmod(seed, count);
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
    choice = posmod(seed, count);
    seed = prng(seed);
    index = bm_select_open(has_data, choice);
  }

  // Finally, work on the soil composition of rivers and lakes by spreading
  // soil types downstream:
  // TODO: This?
  // TODO: Any other postprocessing?

  cleanup_bitmap(has_data);
}

void create_appropriate_soil(world_region *wr) {
  // Things that are taken into account:
  //   Altitude
  //   Annual precipitation
  //   Temperature (mean, seasonal, and min)
  //   Salinity
  //   Bedrock chemistry

  ptrdiff_t seed = prng(wr->seed + 443355818);

  // TODO: Finer distinctions?

  create_local_dirt(wr, &(wr->climate.soil.base_soil));
  if (ptrf(seed) < 0.5) {
    copy_soil_type(&(wr->climate.soil.base_soil), &(wr->climate.soil.top_soil));
  } else {
    create_topsoil_variant(
      wr,
      &(wr->climate.soil.base_soil),
      &(wr->climate.soil.top_soil)
    );
  }
  seed = prng(seed);

  create_local_silt(wr, &(wr->climate.soil.river_banks));
  if (ptrf(seed) < 0.5) {
    copy_soil_type(
      &(wr->climate.soil.river_banks),
      &(wr->climate.soil.river_bank_topsoil)
    );
  } else {
    create_topsoil_variant(
      wr,
      &(wr->climate.soil.river_banks),
      &(wr->climate.soil.river_bank_topsoil)
    );
  }
  seed = prng(seed);

  create_local_mud(wr, &(wr->climate.soil.river_bottoms));
  if (ptrf(seed) < 0.5) {
    copy_soil_type(
      &(wr->climate.soil.river_bottoms),
      &(wr->climate.soil.river_bottom_topsoil)
    );
  } else {
    create_topsoil_variant(
      wr,
      &(wr->climate.soil.river_bottoms),
      &(wr->climate.soil.river_bottom_topsoil)
    );
  }
  seed = prng(seed);

  create_local_sand(wr, &(wr->climate.soil.lake_shores));
  if (ptrf(seed) < 0.5) {
    copy_soil_type(
      &(wr->climate.soil.lake_shores),
      &(wr->climate.soil.lake_shore_topsoil)
    );
  } else {
    create_topsoil_variant(
      wr,
      &(wr->climate.soil.lake_shores),
      &(wr->climate.soil.lake_shore_topsoil)
    );
  }
  seed = prng(seed);

  create_local_sand(wr, &(wr->climate.soil.lake_bottoms));
  if (ptrf(seed) < 0.5) {
    copy_soil_type(
      &(wr->climate.soil.lake_bottoms),
      &(wr->climate.soil.lake_bottom_topsoil)
    );
  } else {
    create_topsoil_variant(
      wr,
      &(wr->climate.soil.lake_bottoms),
      &(wr->climate.soil.lake_bottom_topsoil)
    );
  }
  seed = prng(seed);

  create_local_sand(wr, &(wr->climate.soil.beaches));
  if (ptrf(seed) < 0.5) {
    copy_soil_type(
      &(wr->climate.soil.beaches),
      &(wr->climate.soil.beach_topsoil)
    );
  } else {
    create_topsoil_variant(
      wr,
      &(wr->climate.soil.beaches),
      &(wr->climate.soil.beach_topsoil)
    );
  }
  seed = prng(seed);

  create_local_mud(wr, &(wr->climate.soil.ocean_floor));
  if (ptrf(seed) < 0.5) {
    copy_soil_type(
      &(wr->climate.soil.ocean_floor),
      &(wr->climate.soil.ocean_floor_topsoil)
    );
  } else {
    create_topsoil_variant(
      wr,
      &(wr->climate.soil.ocean_floor),
      &(wr->climate.soil.ocean_floor_topsoil)
    );
  }
  seed = prng(seed);
}

void create_local_dirt(
  world_region *wr,
  soil_type *soil
) {
  size_t i;

  rngtable *alt_table;
  ptrdiff_t seed = prng(wr->seed + 5464221);

  alt_table = pick_alt_dirt_table(wr, seed);
  seed = prng(seed);

  soil->main_block_type = B_DIRT;
  soil->main_species = create_dirt_species();

  // Fill out the main dirt species:
  fill_dirt_species(get_dirt_species(soil->main_species), wr, seed);
  seed = prng(seed);

  for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    soil->alt_block_types[i] = (block) rt_pick_result(alt_table, seed);
#pragma GCC diagnostic warning "-Wpointer-to-int-cast"
    seed = prng(seed);
    switch(soil->alt_block_types[i]) {
      default:
      case B_DIRT:
        soil->alt_species[i] = create_dirt_species();
        fill_dirt_variant(
          get_dirt_species(soil->main_species),
          get_dirt_species(soil->alt_species[i]),
          wr
        );
        soil->alt_strengths[i] = randf(seed, 0, 1.0);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -0.3, 0.3);
        seed = prng(seed);
        break;
      case B_MUD:
        // TODO: Possibly copy another alt here?
        if (ptrf(seed) < 0.7) {
          soil->alt_species[i] = soil->main_species;
        } else {
          soil->alt_species[i] = create_dirt_species();
          fill_dirt_variant(
            get_dirt_species(soil->main_species),
            get_dirt_species(soil->alt_species[i]),
            wr
          );
        }
        seed = prng(seed);
        soil->alt_strengths[i] = randf_pnorm(seed, 0, 0.9);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -0.7, 0.2);
        seed = prng(seed);
        break;
      case B_CLAY:
        // TODO: Something more complex here?
        soil->alt_species[i] = create_clay_species();
        fill_clay_species(
          get_clay_species(soil->alt_species[i]),
          wr,
          seed
        );
        // DEBUG:
        // TODO: Enable clays!
        soil->alt_strengths[i] = 0;
        soil->alt_hdeps[i] = 0;
        /*
        soil->alt_strengths[i] = expdist(randf_pnorm(seed, 0, 0.8), 2);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -0.5, 0.5);
        seed = prng(seed);
        */
        break;
      case B_SAND:
        // TODO: Something more complex here.
        soil->alt_species[i] = get_bedrock(wr);
        soil->alt_strengths[i] = randf_pnorm(seed, 0, 1);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -0.3, 0.8);
        seed = prng(seed);
        break;
      case B_GRAVEL:
      case B_SCREE:
        // TODO: Something more complex here.
        soil->alt_species[i] = get_bedrock(wr);
        soil->alt_strengths[i] = expdist(ptrf(seed), 2);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, 0.2, 1.0);
        seed = prng(seed);
        break;
      case B_STONE:
        // TODO: Something more complex here.
        soil->alt_species[i] = get_bedrock(wr);
        soil->alt_strengths[i] = expdist(ptrf(seed), 2.5);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -1.0, -0.4);
        seed = prng(seed);
        break;
    }
  }
}

void create_local_silt(
  world_region *wr,
  soil_type *soil
) {
  // TODO: Better here!
  create_local_dirt(wr, soil);
}

void create_local_mud(
  world_region *wr,
  soil_type *soil
) {
  // TODO: Better here!
  create_local_dirt(wr, soil);
}

void create_local_sand(
  world_region *wr,
  soil_type *soil
) {
  size_t i;

  rngtable *alt_table;
  ptrdiff_t seed = prng(wr->seed + 324334);

  alt_table = pick_alt_sand_table(wr, seed);
  seed = prng(seed);

  soil->main_block_type = B_SAND;
  // TODO: create a variant species here?
  soil->main_species = get_bedrock(wr);

  for (i = 0; i < WM_MAX_SOIL_ALTS; ++i) {
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
    soil->alt_block_types[i] = (block) rt_pick_result(alt_table, seed);
#pragma GCC diagnostic warning "-Wpointer-to-int-cast"
    seed = prng(seed);
    switch(soil->alt_block_types[i]) {
      default:
      case B_DIRT:
        soil->alt_species[i] = create_dirt_species();
        fill_dirt_species(
          get_dirt_species(soil->alt_species[i]),
          wr,
          seed
        );
        seed = prng(seed);
        soil->alt_strengths[i] = randf(seed, 0, 0.8);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -0.5, 0.4);
        seed = prng(seed);
        break;
      case B_MUD:
        // TODO: Possibly copy another alt here?
        soil->alt_species[i] = create_dirt_species();
        fill_dirt_species(
          get_dirt_species(soil->alt_species[i]),
          wr,
          seed
        );
        seed = prng(seed);
        soil->alt_strengths[i] = randf_pnorm(seed, 0, 0.5);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -1.0, -0.2);
        seed = prng(seed);
        break;
      case B_CLAY:
        // TODO: Something more complex here?
        soil->alt_species[i] = create_clay_species();
        fill_clay_species(
          get_clay_species(soil->alt_species[i]),
          wr,
          seed
        );
        // DEBUG:
        // TODO: Enable clays!
        soil->alt_strengths[i] = 0;
        soil->alt_hdeps[i] = 0;
        /*
        soil->alt_strengths[i] = expdist(randf_pnorm(seed, 0, 0.8), 2);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -0.5, 0.5);
        seed = prng(seed);
        */
        break;
      case B_SAND:
        // TODO: An actual variant here!
        soil->alt_species[i] = get_bedrock(wr);
        soil->alt_strengths[i] = randf_pnorm(seed, 0, 1);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -0.3, 0.3);
        seed = prng(seed);
        break;
      case B_GRAVEL:
      case B_SCREE:
        // TODO: Something more complex here.
        soil->alt_species[i] = get_bedrock(wr);
        soil->alt_strengths[i] = expdist(ptrf(seed), 1.3);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -1.0, -0.2);
        seed = prng(seed);
        break;
      case B_STONE:
        // TODO: Something more complex here.
        soil->alt_species[i] = get_bedrock(wr);
        soil->alt_strengths[i] = expdist(ptrf(seed), 1.8);
        seed = prng(seed);
        soil->alt_hdeps[i] = randf(seed, -1.0, -0.4);
        seed = prng(seed);
        break;
    }
  }
}

void create_topsoil_variant(
  world_region *wr,
  soil_type *original,
  soil_type *result
) {
  copy_soil_type(original, result);
  // TODO: More HERE!
  switch (original->main_block_type) {
    default:
    case B_DIRT:
    case B_MUD:
      result->main_species = create_dirt_species();
      fill_dirt_variant(
        get_dirt_species(original->main_species),
        get_dirt_species(result->main_species),
        wr
      );
      break;
    case B_CLAY:
      // TODO: HERE
      break;
    case B_SAND:
      // TODO: HERE
      break;
    case B_GRAVEL:
    case B_SCREE:
    case B_STONE:
      // TODO: HERE
      break;
  }
  // TODO: Variants of non-main blocks!
}

rngtable* pick_alt_dirt_table(world_region *wr, ptrdiff_t seed) {
  if (wr->s_altitude == WM_AC_OCEAN_DEPTHS) {
    // precipitation and temperature are irrelevant
    return (rngtable*) rt_pick_result(&OCEAN_DEPTHS_ALTS_ALTS, seed);
  } else if (wr->s_altitude == WM_AC_CONT_SHELF) {
    // precipitation and temperature are irrelevant
    return (rngtable*) rt_pick_result(&CONTINENTAL_SHELF_ALTS_ALTS, seed);

  // Above sea level...
  } else if (wr->s_temperature == WM_TC_ARCTIC) {
    // altitude and precipitation are largely irrelevant
    if (
       wr->s_altitude == WM_AC_MOUNTAIN_SLOPES
    || wr->s_altitude == WM_AC_MOUNTAIN_PEAKS
    ) {
      return (rngtable*) rt_pick_result(&ARCTIC_MOUNTAINS_ALTS_ALTS, seed);
    } else {
      return (rngtable*) rt_pick_result(&ARCTIC_PLAINS_ALTS_ALTS, seed);
    }
  } else if (wr->s_temperature == WM_TC_TUNDRA) {
    // altitude and precipitation are mostly irrelevant
    if (
       wr->s_altitude == WM_AC_MOUNTAIN_SLOPES
    || wr->s_altitude == WM_AC_MOUNTAIN_PEAKS
    ) {
      return (rngtable*) rt_pick_result(&ARCTIC_MOUNTAINS_ALTS_ALTS, seed);
    } else if (
       wr->s_precipitation == WM_PC_DESERT
    || wr->s_precipitation == WM_PC_ARID
    || wr->s_precipitation == WM_PC_DRY
    ) {
      return (rngtable*) rt_pick_result(&DRY_TUNDRA_ALTS_ALTS, seed);
    } else {
      return (rngtable*) rt_pick_result(&WET_TUNDRA_ALTS_ALTS, seed);
    }

  // Neither arctic nor tundra...
  } else if (wr->s_precipitation == WM_PC_DESERT) {
    // altitude and temperature are largely irrelevant
    if (
       wr->s_altitude == WM_AC_MOUNTAIN_SLOPES
    || wr->s_altitude == WM_AC_MOUNTAIN_PEAKS
    ) {
      return (rngtable*) rt_pick_result(&DESERT_MOUNTAINS_ALTS_ALTS, seed);
    } else {
      return (rngtable*) rt_pick_result(&DESERT_ALTS_ALTS, seed);
    }
  } else if (wr->s_precipitation == WM_PC_ARID) {
    // altitude and temperature are largely irrelevant
    if (
       wr->s_altitude == WM_AC_MOUNTAIN_SLOPES
    || wr->s_altitude == WM_AC_MOUNTAIN_PEAKS
    ) {
      return (rngtable*) rt_pick_result(&ARID_MOUNTAINS_ALTS_ALTS, seed);
    } else {
      return (rngtable*) rt_pick_result(&ARID_ALTS_ALTS, seed);
    }

  // Neither desert nor arid...
  } else if (wr->s_precipitation == WM_PC_DRY) {
    if (
       wr->s_altitude == WM_AC_MOUNTAIN_SLOPES
    || wr->s_altitude == WM_AC_MOUNTAIN_PEAKS
    ) {
      // don't care about temperature
      return (rngtable*) rt_pick_result(&ARID_MOUNTAINS_ALTS_ALTS, seed);
    } else if (wr->s_altitude == WM_AC_HIGHLANDS) {
      // don't care about temperature
      return (rngtable*) rt_pick_result(&DRY_HIGHLANDS_ALTS_ALTS, seed);
    } else {
      if (
         wr->s_temperature == WM_TC_COLD_FROST
      || wr->s_temperature == WM_TC_COLD_RARE_FROST
      || wr->s_temperature == WM_TC_MILD_FROST
      || wr->s_temperature == WM_TC_MILD_RARE_FROST
      ) {
        return (rngtable*) rt_pick_result(&COLD_DRY_ALTS_ALTS, seed);
      } else if (
         wr->s_temperature == WM_TC_WARM_FROST
      || wr->s_temperature == WM_TC_WARM_NO_FROST
      || wr->s_temperature == WM_TC_HOT
      || wr->s_temperature == WM_TC_TROPICAL
      ) {
        return (rngtable*) rt_pick_result(&WARM_DRY_ALTS_ALTS, seed);
      }
    }
  } else if (
     wr->s_precipitation == WM_PC_NORMAL
  || wr->s_precipitation == WM_PC_SEASONAL
  || wr->s_precipitation == WM_PC_WET
  ) {
    if (
       wr->s_altitude == WM_AC_MOUNTAIN_SLOPES
    || wr->s_altitude == WM_AC_MOUNTAIN_PEAKS
    ) {
      return (rngtable*) rt_pick_result(&MOUNTAINS_ALTS_ALTS, seed);
    } else if (wr->s_altitude == WM_AC_HIGHLANDS) {
      return (rngtable*) rt_pick_result(&HIGHLANDS_ALTS_ALTS, seed);
    } else {
      if (
         wr->s_temperature == WM_TC_COLD_FROST
      || wr->s_temperature == WM_TC_COLD_RARE_FROST
      || wr->s_temperature == WM_TC_MILD_FROST
      || wr->s_temperature == WM_TC_MILD_RARE_FROST
      ) {
        return (rngtable*) rt_pick_result(&COLD_ALTS_ALTS, seed);
      } else if (
         wr->s_temperature == WM_TC_WARM_FROST
      || wr->s_temperature == WM_TC_WARM_NO_FROST
      || wr->s_temperature == WM_TC_HOT
      || wr->s_temperature == WM_TC_TROPICAL
      ) {
        return (rngtable*) rt_pick_result(&WARM_ALTS_ALTS, seed);
      }
    }
  } else if (
     wr->s_precipitation == WM_PC_SOAKING
  || wr->s_precipitation == WM_PC_FLOODED
  ) {
    if (
       wr->s_altitude == WM_AC_MOUNTAIN_SLOPES
    || wr->s_altitude == WM_AC_MOUNTAIN_PEAKS
    ) {
      if (
         wr->s_temperature == WM_TC_COLD_FROST
      || wr->s_temperature == WM_TC_COLD_RARE_FROST
      ) {
        return (rngtable*) rt_pick_result(&MOUNTAINS_ALTS_ALTS, seed);
      } else if (
         wr->s_temperature == WM_TC_MILD_FROST
      || wr->s_temperature == WM_TC_MILD_RARE_FROST
      || wr->s_temperature == WM_TC_WARM_FROST
      || wr->s_temperature == WM_TC_WARM_NO_FROST
      || wr->s_temperature == WM_TC_HOT
      || wr->s_temperature == WM_TC_TROPICAL
      ) {
        return (rngtable*) rt_pick_result(&WET_MOUNTAINS_ALTS_ALTS, seed);
      }
    } else if (wr->s_altitude == WM_AC_HIGHLANDS) {
      if (
         wr->s_temperature == WM_TC_COLD_FROST
      || wr->s_temperature == WM_TC_COLD_RARE_FROST
      ) {
        return (rngtable*) rt_pick_result(&HIGHLANDS_ALTS_ALTS, seed);
      } else if (
         wr->s_temperature == WM_TC_MILD_FROST
      || wr->s_temperature == WM_TC_MILD_RARE_FROST
      || wr->s_temperature == WM_TC_WARM_FROST
      || wr->s_temperature == WM_TC_WARM_NO_FROST
      || wr->s_temperature == WM_TC_HOT
      || wr->s_temperature == WM_TC_TROPICAL
      ) {
        return (rngtable*) rt_pick_result(&WET_HIGHLANDS_ALTS_ALTS, seed);
      }
    } else {
      if (
         wr->s_temperature == WM_TC_COLD_FROST
      || wr->s_temperature == WM_TC_COLD_RARE_FROST
      || wr->s_temperature == WM_TC_MILD_FROST
      || wr->s_temperature == WM_TC_MILD_RARE_FROST
      ) {
        return (rngtable*) rt_pick_result(&COLD_WET_ALTS_ALTS, seed);
      } else if (
         wr->s_temperature == WM_TC_WARM_FROST
      || wr->s_temperature == WM_TC_WARM_NO_FROST
      || wr->s_temperature == WM_TC_HOT
      || wr->s_temperature == WM_TC_TROPICAL
      ) {
        return (rngtable*) rt_pick_result(&WARM_WET_ALTS_ALTS, seed);
      }
    }
#ifdef DEBUG
  } else { // shouldn't be possible
    printf("ERROR: impossible else case in pick_alt_dirt_table!");
    exit(1);
#endif
  }
#ifdef DEBUG
  // shouldn't be possible
  printf("ERROR: fell out of if/else in pick_alt_dirt_table!");
  exit(1);
#else
  // last-resort default
  return &LOAMY_DIRT_ALTS;
#endif
}

rngtable* pick_alt_sand_table(world_region *wr, ptrdiff_t seed) {
  return &DRY_SAND_ALTS;
  return &WET_SAND_ALTS;
}

void fill_dirt_species(
  dirt_species *dsp,
  world_region *wr,
  ptrdiff_t seed
) {
  size_t i, limit;
  float richness;
  float sand, clay;
  float organics;
  float tmp;
  seed = prng(seed + 881721);
  list *all_elements = wr->world->all_elements;
  list *nutrients = wr->world->all_nutrients;
  element_species *esp;
  list *trace_candidates;

  // variable with broad effects:
  richness = ptrf(seed);
  seed = prng(seed);

  // base sand and clay percentages
  sand = expdist(randf(seed, 0, SL_COMP_MAX_SAND), 2);
  seed = prng(seed);
  clay = randf(seed, 0, SL_COMP_MAX_CLAY);

  // modify richness and sand/clay percentages based on region summary info:
  switch (wr->s_altitude) {
    case WM_AC_OCEAN_DEPTHS:
      richness = 0.5 * richness + 0.5 * randf_pnorm(seed, 0.7, 1.0);
      sand = 0.7 * sand + 0.3 * SL_COMP_MAX_SAND;
      clay = 0.8 * clay + 0.2 * SL_COMP_MAX_CLAY;
      break;
    case WM_AC_CONT_SHELF:
      richness = 0.5 * richness + 0.5 * randf_pnorm(seed, 0.3, 1.0);
      sand = 0.5 * sand + 0.5 * SL_COMP_MAX_SAND;
      clay = 0.8 * clay;
      break;
    case WM_AC_COASTAL_PLAINS:
      richness = 0.6 * richness + 0.4 * randf_pnorm(seed, 0.5, 1.0);
      sand = 0.7 * sand;
      clay = 0.8 * clay;
      break;
    default:
    case WM_AC_INLAND_HILLS:
      richness = 0.4 * richness + 0.6 * randf_pnorm(seed, 0.2, 1.0);
      sand = 0.9 * sand;
      clay = 0.8 * clay + 0.2 * SL_COMP_MAX_CLAY;
      break;
    case WM_AC_HIGHLANDS:
      richness = 0.5 * richness + 0.5 * randf_pnorm(seed, 0.0, 0.8);
      sand = 0.9 * sand + 0.1 * SL_COMP_MAX_SAND;
      clay = 0.9 * clay;
      break;
    case WM_AC_MOUNTAIN_SLOPES:
      richness = 0.4 * richness + 0.6 * randf_pnorm(seed, 0.0, 0.6);
      sand = 0.8 * sand + 0.2 * SL_COMP_MAX_SAND;
      clay = 0.7 * clay + 0.3 * SL_COMP_MAX_CLAY;
      break;
    case WM_AC_MOUNTAIN_PEAKS:
      richness = 0.3 * richness + 0.7 * randf_pnorm(seed, 0.0, 0.4);
      sand = 0.6 * sand + 0.4 * SL_COMP_MAX_SAND;
      clay = 0.5 * clay + 0.5 * SL_COMP_MAX_CLAY;
      break;
  }

  switch (wr->s_precipitation) {
    case WM_PC_DESERT:
      richness = 0.3 * richness + 0.7 * expdist(randf(seed, 0, 0.4), 3);
      sand = 0.2 * sand + 0.8 * SL_COMP_MAX_SAND;
      clay = 0.2 * clay;
      break;
    case WM_PC_ARID:
      richness = 0.5 * richness + 0.5 * randf(seed, 0, 0.6);
      sand = 0.4 * sand + 0.6 * SL_COMP_MAX_SAND;
      clay = 0.6 * clay;
      break;
    case WM_PC_DRY:
      richness = 0.6 * richness + 0.4 * randf(seed, 0, 0.8);
      sand = 0.7 * sand + 0.3 * SL_COMP_MAX_SAND;
      clay = 0.8 * clay;
      break;
    case WM_PC_SEASONAL:
      richness = 0.6 * richness + 0.4 * randf(seed, 0.2, 1.0);
      sand = 0.7 * sand;
      clay = 0.7 * clay + 0.3 * SL_COMP_MAX_CLAY;
      break;
    default:
    case WM_PC_NORMAL:
      richness = 0.8 * richness + 0.2 * randf(seed, 0.1, 1.0);
      sand = 0.9 * sand;
      clay = 0.9 * clay + 0.1 * SL_COMP_MAX_CLAY;
      break;
    case WM_PC_WET:
      richness = 0.6 * richness + 0.4 * randf(seed, 0.3, 1.0);
      sand = 0.9 * sand;
      clay = 0.9 * clay;
      break;
    case WM_PC_SOAKING:
      richness = 0.7 * richness + 0.3 * randf(seed, 0.1, 1.0);
      sand = 0.9 * sand;
      clay = 0.8 * clay;
      break;
    case WM_PC_FLOODED:
      richness = 0.6 * richness + 0.4 * randf(seed, 0.0, 0.7);
      sand = 0.8 * sand;
      clay = 0.6 * clay;
      break;
  }

  switch (wr->s_temperature) {
    case WM_TC_ARCTIC:
      richness = 0.1 * richness + 0.9 * expdist(randf(seed, 0, 0.2), 3);
      sand = 0.4 * sand + 0.6 * SL_COMP_MAX_SAND;
      clay = 0.4 * clay;
      break;
    case WM_TC_TUNDRA:
      richness = 0.2 * richness + 0.8 * randf(seed, 0, 0.4);
      sand = 0.8 * sand;
      clay = 0.7 * clay;
      break;
    case WM_TC_COLD_FROST:
      richness = 0.6 * richness + 0.4 * randf(seed, 0, 0.8);
      sand = 0.9 * sand;
      clay = 0.9 * clay;
      break;
    case WM_TC_COLD_RARE_FROST:
      richness = 0.8 * richness + 0.2 * randf(seed, 0, 0.9);
      sand = 0.9 * sand + 0.1 * SL_COMP_MAX_SAND;
      clay = 0.9 * clay + 0.1 * SL_COMP_MAX_CLAY;
      break;
    case WM_TC_MILD_FROST:
      richness = 0.9 * richness + 0.1 * randf(seed, 0.2, 1.0);
      sand = 0.9 * sand;
      // no effect on clay
      break;
    case WM_TC_MILD_RARE_FROST:
      richness = 0.8 * richness + 0.2 * randf(seed, 0.2, 1.0);
      sand = 0.9 * sand;
      clay = 0.9 * clay + 0.1 * SL_COMP_MAX_CLAY;
      break;
    default:
    case WM_TC_WARM_FROST:
      richness = 0.7 * richness + 0.3 * randf(seed, 0.3, 1.0);
      sand = 0.9 * sand;
      clay = 0.8 * clay + 0.2 * SL_COMP_MAX_CLAY;
      break;
    case WM_TC_WARM_NO_FROST:
      richness = 0.5 * richness + 0.5 * randf(seed, 0.5, 1.0);
      sand = 0.8 * sand;
      clay = 0.7 * clay + 0.3 * SL_COMP_MAX_CLAY;
      break;
    case WM_TC_HOT:
      richness = 0.7 * richness + 0.3 * randf(seed, 0.4, 1.0);
      sand = 0.9 * sand;
      clay = 0.6 * clay + 0.4 * SL_COMP_MAX_CLAY;
      break;
    case WM_TC_TROPICAL:
      richness = 0.6 * richness + 0.4 * randf(seed, 0.2, 1.0);
      sand = 0.9 * sand;
      clay = 0.7 * clay + 0.3 * SL_COMP_MAX_CLAY;
      break;
  }

  // now modify sand/clay based on richness:
  sand = 0.5 * sand + 0.5 * (1 - richness) * SL_COMP_MAX_SAND;
  clay = 0.9 * clay + 0.1 * richness * SL_COMP_MAX_CLAY;

  tmp = sand + clay;
  if (tmp > 0.9) {
    sand = (sand / tmp) - 0.05;
    clay = (clay / tmp) - 0.05;
  }
  if (sand < 0) { sand = 0; }
  if (clay < 0) { clay = 0; }

  // clay/sand/silt values:
  dsp->sand_percent = sand;
  dsp->clay_percent = clay;
  dsp->silt_percent = 1.0 - (dsp->sand_percent + dsp->clay_percent);

  // organic content
  organics = expdist(randf(seed, 0, SL_RICH_ORGANICS), 4);
  seed = prng(seed);
  organics += randf(seed, 0, SL_POOR_ORGANICS);
  seed = prng(seed);
  tmp = expdist(richness, 7);
  organics = (
    0.5 * organics
  + 0.5 * SL_RICH_ORGANICS * (
      pow(tmp + 0.5, 3) - (0.25 * (0.5 - tmp)) // 0 -- 1 -- 3.5
    )
  );
  dsp->organic_content = (uint8_t) fastfloor(organics);

  // base pH
  dsp->base_ph = randf_pnorm(seed, 5.5, 8.3);
  seed = prng(seed);
  if (dsp->base_ph < 7.0) {
    dsp->base_ph = (
      0.4 * dsp->base_ph
    + 0.6 * (4.5 + 2.8 * richness)
    );
  } else {
    dsp->base_ph = (
      0.4 * dsp->base_ph
    + 0.6 * (6.7 + 1.5 * (1 - richness))
    );
  }
  // bring richer soils closer to neutral pH:
  tmp = 0.8 * richness;
  dsp->base_ph = (1 - tmp) * dsp->base_ph + tmp * 7.0;

  // nutrients
  for (i = 0; i < l_get_length(nutrients); ++i) {
    esp = (element_species*) l_get_item(nutrients, i);
    tmp = richness + ptrf(seed);
    seed = prng(seed);

    // base concentrations from soil richness:
    switch (esp->plant_nutrition) {
      case NT_CAT_CRITICAL:
      case NT_CAT_CRITICAL_CAN_OVERDOSE:
        if (tmp < 0.05) {
          tmp = ptrf(seed);
          seed = prng(seed);
          if (
             esp->plant_nutrition == NT_CAT_CRITICAL_CAN_OVERDOSE
          && tmp < 0.3
          ) {
            dsp->nutrients[i] = randi(
              seed,
              MN_NT_CRIT_LEVEL_ABUNDANT,
              MN_NT_CRIT_LEVEL_CHOKING
            );
            seed = prng(seed);
          } else {
            dsp->nutrients[i] = randi(seed, 0, MN_NT_CRIT_LEVEL_LACKING);
            seed = prng(seed);
          }
        } else if (tmp < 0.3) {
          tmp = ptrf(seed);
          seed = prng(seed);
          if (
             esp->plant_nutrition == NT_CAT_CRITICAL_CAN_OVERDOSE
          && tmp < 0.3
          ) {
            dsp->nutrients[i] = randi(
              seed,
              MN_NT_CRIT_LEVEL_ABUNDANT,
              MN_NT_CRIT_LEVEL_OVERABUNDANT
            );
          } else {
            dsp->nutrients[i] = randi(
              seed,
              MN_NT_CRIT_LEVEL_STARVED,
              MN_NT_CRIT_LEVEL_SUFFICIENT
            );
            seed = prng(seed);
          }
        } else if (tmp < 0.7) {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_CRIT_LEVEL_LACKING,
            (MN_NT_CRIT_LEVEL_SUFFICIENT + MN_NT_CRIT_LEVEL_INCREASED) / 2
          );
          seed = prng(seed);
        } else if (tmp < 1.0) {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_CRIT_LEVEL_LACKING,
            MN_NT_CRIT_LEVEL_INCREASED
          );
          seed = prng(seed);
        } else if (tmp < 1.5) {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_CRIT_LEVEL_LACKING,
            (MN_NT_CRIT_LEVEL_INCREASED + MN_NT_CRIT_LEVEL_ABUNDANT) / 2
          );
          seed = prng(seed);
        } else {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_CRIT_LEVEL_LACKING,
            (MN_NT_CRIT_LEVEL_ABUNDANT + MN_NT_CRIT_LEVEL_OVERABUNDANT) / 2
          );
          seed = prng(seed);
        }
        break;
      case NT_CAT_BENEFICIAL:
      case NT_CAT_BENEFICIAL_CAN_OVERDOSE:
        if (tmp < 0.1) {
          tmp = ptrf(seed);
          seed = prng(seed);
          if (
             esp->plant_nutrition == NT_CAT_BENEFICIAL_CAN_OVERDOSE
          && tmp < 0.3
          ) {
            dsp->nutrients[i] = randi(
              seed,
              MN_NT_BFCL_LEVEL_ABUNDANT,
              MN_NT_BFCL_LEVEL_CHOKING
            );
            seed = prng(seed);
          } else {
            dsp->nutrients[i] = randi(seed, 0, MN_NT_BFCL_LEVEL_LACKING);
            seed = prng(seed);
          }
        } else if (tmp < 0.3) {
          dsp->nutrients[i] = randi(
            seed,
            0,
            MN_NT_BFCL_LEVEL_MINIMAL
          );
          seed = prng(seed);
        } else if (tmp < 0.7) {
          dsp->nutrients[i] = randi(
            seed,
            0,
            (MN_NT_BFCL_LEVEL_PRESENT + MN_NT_BFCL_LEVEL_ABUNDANT) / 2
          );
          seed = prng(seed);
        } else if (tmp < 1.2) {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_BFCL_LEVEL_LACKING,
            (MN_NT_BFCL_LEVEL_PRESENT + MN_NT_BFCL_LEVEL_ABUNDANT) / 2
          );
          seed = prng(seed);
        } else {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_BFCL_LEVEL_LACKING,
            MN_NT_BFCL_LEVEL_ABUNDANT
          );
          seed = prng(seed);
        }
        break;
      case NT_CAT_DETRIMENTAL:
        if (tmp < 0.05) {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_DTML_LEVEL_PRESENT,
            MN_NT_DTML_LEVEL_DEADLY
          );
          seed = prng(seed);
        } else if (tmp < 0.2) {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_DTML_LEVEL_ABSENT,
            MN_NT_DTML_LEVEL_ABUNDANT
          );
          seed = prng(seed);
        } else if (tmp < 0.35) {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_DTML_LEVEL_ABSENT,
            (MN_NT_DTML_LEVEL_PRESENT + MN_NT_DTML_LEVEL_ABUNDANT) / 2
          );
          seed = prng(seed);
        } else if (tmp < 0.4) {
          dsp->nutrients[i] = randi(
            seed,
            0,
            (MN_NT_DTML_LEVEL_PRESENT + MN_NT_DTML_LEVEL_ABUNDANT) / 2
          );
          seed = prng(seed);
        } else if (tmp < 0.5) {
          dsp->nutrients[i] = randi(seed, 0, MN_NT_DTML_LEVEL_PRESENT);
          seed = prng(seed);
        } else if (tmp < 0.6) {
          dsp->nutrients[i] = randi(
            seed,
            0,
            (MN_NT_DTML_LEVEL_ABSENT + MN_NT_DTML_LEVEL_PRESENT) / 2
          );
          seed = prng(seed);
        } else if (tmp < 0.7) {
          dsp->nutrients[i] = randi(seed, 0, MN_NT_DTML_LEVEL_ABSENT);
          seed = prng(seed);
        } else {
          dsp->nutrients[i] = 0;
        }
        break;
      case NT_CAT_POISONOUS:
        if (tmp < 0.03) {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_PSNS_LEVEL_SAFE,
            MN_NT_PSNS_LEVEL_DEADLY
          );
          seed = prng(seed);
        } else if (tmp < 0.8) {
          dsp->nutrients[i] = randi(
            seed,
            MN_NT_PSNS_LEVEL_SAFE,
            MN_NT_PSNS_LEVEL_DEADLY
          );
          seed = prng(seed);
        } else if (tmp < 0.15) {
          dsp->nutrients[i] = randi(seed, 0, MN_NT_PSNS_LEVEL_DEADLY);
          seed = prng(seed);
        } else if (tmp < 0.3) {
          dsp->nutrients[i] = randi(seed, 0, MN_NT_PSNS_LEVEL_HARMFUL);
          seed = prng(seed);
        } else if (tmp < 0.5) {
          dsp->nutrients[i] = randi(seed, 0, MN_NT_PSNS_LEVEL_SAFE);
          seed = prng(seed);
        } else {
          dsp->nutrients[i] = 0;
        }
        break;
      default:
      case NT_CAT_NONE: // this is an animal nutrient only
        switch (esp->animal_nutrition) {
          case NT_CAT_CRITICAL:
          case NT_CAT_CRITICAL_CAN_OVERDOSE:
            if (tmp < 0.4) {
              dsp->nutrients[i] = randi(seed, 0, MN_NT_CRIT_LEVEL_INCREASED);
              seed = prng(seed);
            } else {
              dsp->nutrients[i] = randi(
                seed,
                MN_NT_CRIT_LEVEL_LACKING,
                MN_NT_CRIT_LEVEL_ABUNDANT
              );
              seed = prng(seed);
            }
            break;
          case NT_CAT_BENEFICIAL:
          case NT_CAT_BENEFICIAL_CAN_OVERDOSE:
            if (tmp < 0.5) {
              dsp->nutrients[i] = randi(seed, 0, MN_NT_BFCL_LEVEL_PRESENT);
            } else if (tmp < 0.9) {
              dsp->nutrients[i] = randi(
                seed,
                0,
                (MN_NT_BFCL_LEVEL_PRESENT + MN_NT_BFCL_LEVEL_ABUNDANT) / 2
              );
              seed = prng(seed);
            } else {
              dsp->nutrients[i] = randi(
                seed,
                MN_NT_BFCL_LEVEL_LACKING,
                MN_NT_BFCL_LEVEL_ABUNDANT
              );
              seed = prng(seed);
            }
            break;
          case NT_CAT_DETRIMENTAL:
            if (tmp < 0.05) {
              dsp->nutrients[i] = randi(
                seed,
                MN_NT_DTML_LEVEL_ABSENT,
                MN_NT_DTML_LEVEL_DEADLY
              );
              seed = prng(seed);
            } else if (tmp < 0.1) {
              dsp->nutrients[i] = randi(
                seed,
                0,
                (MN_NT_DTML_LEVEL_PRESENT + MN_NT_DTML_LEVEL_ABUNDANT) / 2
              );
              seed = prng(seed);
            } else if (tmp < 0.3) {
              dsp->nutrients[i] = randi(seed, 0, MN_NT_DTML_LEVEL_PRESENT);
              seed = prng(seed);
            } else if (tmp < 0.8) {
              dsp->nutrients[i] = randi(seed, 0, MN_NT_DTML_LEVEL_ABSENT);
              seed = prng(seed);
            } else {
              dsp->nutrients[i] = 0;
            }
            break;
          case NT_CAT_POISONOUS:
            if (tmp < 0.03) {
              dsp->nutrients[i] = randi(
                seed,
                MN_NT_PSNS_LEVEL_SAFE,
                MN_NT_PSNS_LEVEL_DEADLY
              );
              seed = prng(seed);
            } else if (tmp < 0.08) {
              dsp->nutrients[i] = randi(seed, 0, MN_NT_PSNS_LEVEL_DANGEROUS);
              seed = prng(seed);
            } else if (tmp < 0.15) {
              dsp->nutrients[i] = randi(
                seed,
                0,
                (MN_NT_PSNS_LEVEL_HARMFUL + MN_NT_PSNS_LEVEL_DANGEROUS) / 2
              );
              seed = prng(seed);
            } else if (tmp < 0.25) {
              dsp->nutrients[i] = randi(seed, 0, MN_NT_PSNS_LEVEL_HARMFUL);
              seed = prng(seed);
            } else if (tmp < 0.4) {
              dsp->nutrients[i] = randi(seed, 0, MN_NT_PSNS_LEVEL_SAFE);
              seed = prng(seed);
            } else {
              dsp->nutrients[i] = 0;
            }
            break;
          default:
          case NT_CAT_NONE: // this shouldn't be possible
#ifdef DEBUG
            printf(
              "ERROR: element %d in all_nutrients list is not a nutrient!\n",
              esp->id
            );
            exit(1);
#endif
            break;
        }
        break;
    }

    // adjustments based on bedrock element content
    if (stone_contains_element(get_stone_species(get_bedrock(wr)), esp->id)) {
      switch (esp->solubility) {
        case SOLUBILITY_SOLUBLE:
          dsp->nutrients[i] += randi(
            seed,
            MN_NT_SMALL_ADJUST,
            MN_NT_LARGE_ADJUST
          );
          seed = prng(seed);
          break;
        default:
        case SOLUBILITY_SLIGHTLY_SOLUBLE:
          dsp->nutrients[i] += randi(seed, 0, MN_NT_LARGE_ADJUST);
          seed = prng(seed);
          break;
        case SOLUBILITY_INSOLUBLE:
          dsp->nutrients[i] += randi(seed, 0, MN_NT_SMALL_ADJUST);
          seed = prng(seed);
          break;
      }
    } else if (
      stone_contains_trace_element(
        get_stone_species(get_bedrock(wr)),
        esp->id
      )
    ) {
      switch (esp->solubility) {
        case SOLUBILITY_SOLUBLE:
          dsp->nutrients[i] += randi(seed, 0, MN_NT_SMALL_ADJUST);
          seed = prng(seed);
          break;
        default:
        case SOLUBILITY_SLIGHTLY_SOLUBLE:
          dsp->nutrients[i] += randi(seed, 0, MN_NT_TINY_ADJUST);
          seed = prng(seed);
          break;
        case SOLUBILITY_INSOLUBLE:
          // no effect
          break;
      }
    }
  }
  for (; i < MAX_TOTAL_NUTRIENTS; ++i) { // set extra nutrients to 0
    dsp->nutrients[i] = 0;
  }

  // trace elements
  trace_candidates = create_list();
  for (i = 0; i < l_get_length(all_elements); ++i) {
    esp = (element_species*) l_get_item(all_elements, i);
    if (
       esp->plant_nutrition == NT_CAT_NONE
    && esp->animal_nutrition == NT_CAT_NONE
    ) {
      l_append_element(trace_candidates, (void*) esp);
    }
  }
  l_shuffle(trace_candidates, seed);
  seed = prng(seed);
  limit = randi(seed, 0, MN_MAX_TRACE_CONSTITUENTS);
  seed = prng(seed);
  for (i = 0; i < limit; ++i) {
    dsp->trace_minerals[i] = ((element_species*) l_get_item(
      trace_candidates,
      i
    ))->id;
  }
  for (; i < MN_MAX_TRACE_CONSTITUENTS; ++i) { // set extras to 0 (invalid)
    dsp->trace_minerals[i] = 0;
  }

  // Finally, create a material and appearance for this dirt:
  determine_new_dirt_appearance(dsp, seed);
  determine_new_dirt_material(dsp, seed);

  // cleanup
  cleanup_list(trace_candidates);
}

void fill_dirt_variant(
  dirt_species *model,
  dirt_species *dsp,
  world_region *wr
) {
  size_t i, limit;
  ptrdiff_t seed = prng(wr->seed + 7494844);
  float tmp;
  float sand, clay;
  element_species *esp;

  // clay/sand/silt composition:
  sand = model->sand_percent + randf(seed, -0.08, 0.08);
  seed = prng(seed);
  if (sand < 0) { sand = 0; }
  if (sand > SL_COMP_MAX_SAND) { sand = SL_COMP_MAX_SAND; }
  clay = model->clay_percent + randf(seed, -0.08, 0.08);
  seed = prng(seed);
  if (clay < 0) { clay = 0; }
  if (clay > SL_COMP_MAX_CLAY) { clay = SL_COMP_MAX_CLAY; }

  tmp = sand + clay;
  if (tmp > 0.9) {
    sand = (sand / tmp) - 0.05;
    clay = (clay / tmp) - 0.05;
  }
  if (sand < 0) { sand = 0; }
  if (clay < 0) { clay = 0; }

  // clay/sand/silt values:
  dsp->sand_percent = sand;
  dsp->clay_percent = clay;
  dsp->silt_percent = 1.0 - (dsp->sand_percent + dsp->clay_percent);

  // organic content
  if (model->organic_content > 12) {
    dsp->organic_content = model->organic_content + randi(seed, -5, 5);
    seed = prng(seed);
  } else if (model->organic_content > 3) {
    dsp->organic_content = model->organic_content + randi(seed, -2, 2);
    seed = prng(seed);
  } else if (model->organic_content > 0) {
    dsp->organic_content = model->organic_content + randi(seed, -1, 1);
    seed = prng(seed);
  } else {
    dsp->organic_content = model->organic_content + randi(seed, 0, 1);
    seed = prng(seed);
  }

  // base pH
  if (model->base_ph < 7.0) {
    dsp->base_ph = model->base_ph + randf(seed, -0.5, 0.3);
    seed = prng(seed);
  } else {
    dsp->base_ph = model->base_ph + randf(seed, -0.3, 0.4);
    seed = prng(seed);
  }

  // nutrients
  limit = l_get_length(wr->world->all_nutrients);
  for (i = 0; i < limit; ++i) {
    esp = (element_species*) l_get_item(
      wr->world->all_nutrients,
      i
    );
    dsp->nutrients[i] = model->nutrients[i];
    if (ptrf(seed) < 0.4 && dsp->nutrients[i] != 0) {
      seed = prng(seed);
      switch (esp->plant_nutrition) {
        case NT_CAT_CRITICAL:
        case NT_CAT_CRITICAL_CAN_OVERDOSE:
        case NT_CAT_DETRIMENTAL:
        case NT_CAT_POISONOUS:
          if (dsp->nutrients[i] > MN_NT_LARGE_ADJUST) {
            dsp->nutrients[i] += randi(
              seed,
              -MN_NT_TINY_ADJUST,
              MN_NT_TINY_ADJUST
            );
            seed = prng(seed);
          }
          break;
        case NT_CAT_BENEFICIAL:
        case NT_CAT_BENEFICIAL_CAN_OVERDOSE:
          if (dsp->nutrients[i] > MN_NT_LARGE_ADJUST) {
            dsp->nutrients[i] += randi(
              seed,
              -MN_NT_SMALL_ADJUST,
              MN_NT_SMALL_ADJUST
            );
            seed = prng(seed);
          } else if (dsp->nutrients[i] > MN_NT_SMALL_ADJUST) {
            dsp->nutrients[i] += randi(
              seed,
              -MN_NT_TINY_ADJUST,
              MN_NT_TINY_ADJUST
            );
            seed = prng(seed);
          }
          break;
        default:
        case NT_CAT_NONE:
          switch (esp->animal_nutrition) {
            case NT_CAT_CRITICAL:
            case NT_CAT_CRITICAL_CAN_OVERDOSE:
            case NT_CAT_DETRIMENTAL:
            case NT_CAT_POISONOUS:
              if (dsp->nutrients[i] > MN_NT_LARGE_ADJUST) {
                dsp->nutrients[i] += randi(
                  seed,
                  -MN_NT_TINY_ADJUST,
                  MN_NT_TINY_ADJUST
                );
                seed = prng(seed);
              }
              break;
            case NT_CAT_BENEFICIAL:
            case NT_CAT_BENEFICIAL_CAN_OVERDOSE:
              if (dsp->nutrients[i] > MN_NT_LARGE_ADJUST) {
                dsp->nutrients[i] += randi(
                  seed,
                  -MN_NT_SMALL_ADJUST,
                  MN_NT_SMALL_ADJUST
                );
                seed = prng(seed);
              } else if (dsp->nutrients[i] > MN_NT_SMALL_ADJUST) {
                dsp->nutrients[i] += randi(
                  seed,
                  -MN_NT_TINY_ADJUST,
                  MN_NT_TINY_ADJUST
                );
                seed = prng(seed);
              }
              break;
            default:
            case NT_CAT_NONE: // this shouldn't be possible
#ifdef DEBUG
              printf(
                "ERROR: element %d in all_nutrients list is not a nutrient!\n",
                esp->id
              );
              exit(1);
#endif
              break;
          }
          break;
      }
    }
    seed = prng(seed);
  }
  for(; i < N_TOTAL_NUTRIENTS; ++i) {
    dsp->nutrients[i] = 0;
  }

  // TODO: Mutate trace minerals!
  for (i = 0; i < MN_MAX_TRACE_CONSTITUENTS; ++i) {
    dsp->trace_minerals[i] = model->trace_minerals[i];
  }

  // Copy and modify appearance:
  // TODO: Worry about changed trace elements here?
  mutate_mineral_appearance(&(model->appearance), &(dsp->appearance), seed);
  seed = prng(seed);

  // Copy and modify material:
  mutate_material(&(model->material), &(dsp->material), seed);
  seed = prng(seed);
}

void fill_clay_species(
  clay_species *dsp,
  world_region *wr,
  ptrdiff_t seed
) {
  // TODO: HERE
}

void fill_clay_variant(
  clay_species *model,
  clay_species *dsp,
  world_region* wr
) {
  ptrdiff_t seed = prng(wr->seed + 7139933);
  // TODO: HERE

  // Copy and modify appearance:
  // TODO: Worry about changed trace elements here?
  mutate_mineral_appearance(&(model->appearance), &(dsp->appearance), seed);
  seed = prng(seed);

  // Copy and modify material:
  mutate_material(&(model->material), &(dsp->material), seed);
  seed = prng(seed);
}

void determine_new_dirt_appearance(
  dirt_species *species,
  ptrdiff_t seed
) {
  float tmp;
  float organic_influence;
  mineral_filter_args *target = &(species->appearance);
  seed = prng(seed + 389918);
  target->seed = seed;
  seed = prng(seed);

  // A mapping from organic content to a (roughly) 0-1 scale:
  tmp = (float) (species->organic_content);
  if (tmp < 12) {
    organic_influence = 0.8 * pow(tmp / 12.0, 0.5);
  } else {
    organic_influence = 0.8 + 0.2 * ((tmp / 255.0) - (12.0 / 255.0));
  }

  // Dirt generally has smaller scales:
  target->scale = 1.0 / (7.0 + 5.0*ptrf(seed));
  seed = prng(seed);

  // Dirt is usually gritty; composition influences this.
  target->gritty = (
    0.6
  + 0.25 * ptrf(seed)
  + 0.4 * species->sand_percent
  - 0.3 * species->clay_percent
  + 0.2 * species->silt_percent
  + 0.2 * organic_influence
  );
  if (target->gritty < 0.3) { target->gritty = 0.3; }
  seed = prng(seed);

  // Dirt tends not to be very contoured.
  target->contoured = (
    0.15
  + 0.25 * ptrf(seed)
  - 0.3 * species->sand_percent
  + 0.4 * species->clay_percent
  - 0.2 * species->silt_percent
  );
  if (target->contoured < 0.15) { target->contoured = 0.15; }
  seed = prng(seed);

  // Dirt can be somewhat porous.
  target->porous = (
    0.3
  + 0.3 * ptrf(seed)
  + 0.4 * species->sand_percent
  + 0.2 * species->clay_percent
  - 0.2 * species->silt_percent
  + 0.3 * organic_influence
  );
  seed = prng(seed);

  // Dirt is usually quite bumpy.
  target->bumpy = (
    0.4
  + 0.6 * ptrf(seed)
  - 0.3 * species->sand_percent
  + 0.2 * species->clay_percent
  + 0.3 * species->silt_percent
  + 0.1 * organic_influence
  );
  seed = prng(seed);

  // Dirt can exhibit strong layering, but this is rare.
  target->layered = (
    expdist(randf_pnorm(seed, 0, 1.0), 3)
  - 0.1 * species->sand_percent
  + 0.1 * species->clay_percent
  + 0.2 * species->silt_percent
  + 0.2 * organic_influence
  );
  if (target->layered > 1.0) { target->layered = 1.0; }
  seed = prng(seed);
  target->layerscale = 1.0 / (3.0 + 6.0*ptrf(seed) + 2.0*species->sand_percent);
  seed = prng(seed);

  // Dirt is usually pretty straight.
  target->layerwaves = (
    0.5
  + 4.0 * ptrf(seed)
  + 1.5 * species->sand_percent
  + 1.5 * species->clay_percent
  );
  seed = prng(seed);
  target->wavescale = 1.0 / (2.5 + 2.5*ptrf(seed) + 1.0*species->sand_percent);
  seed = prng(seed);

  // Dirt sometimes has inclusions.
  tmp = (
    randf_pnorm(seed, 0.3, 1)
  - 0.2 * species->sand_percent
  - 0.3 * species->clay_percent
  + 0.2 * species->silt_percent
  - 0.2 * organic_influence
  );
  if (tmp < 0) { tmp = 0; }
  target->inclusions = pow(tmp, 1.8);
  seed = prng(seed);

  // The distortion scale is within 30% of the base scale.
  target->dscale = target->scale * (
    randf(seed, 0.8, 1.2)
  + 0.1 * species->sand_percent
  - 0.1 * species->clay_percent
  - 0.1 * species->silt_percent
  );
  seed = prng(seed);

  // Dirt can have little to medium distortion.
  target->distortion = (
    4.5 * pow(randf_pnorm(seed, 0, 1), 1.4)
  - 2.0 * species->sand_percent
  + 3.0 * species->clay_percent
  );
  seed = prng(seed);

  // Dirt is often squashed horizontally.
  target->squash = randf_pnorm(seed, 0.5, 1.1);
  seed = prng(seed);
  target->squash /= randf_pnorm(seed, 0.8, 1.4);
  seed = prng(seed);

  // Dirt tends to be somewhat desaturated.
  target->desaturate = (
    randf(seed, 0.3, 0.8)
  + 0.1 * species->sand_percent
  + 0.1 * species->clay_percent
  - 0.3 * organic_influence
  );
  seed = prng(seed);

  // Dirt tends to have medium saturation noise.
  target->sat_noise = (
    randf(seed, 0.1, 0.7)
  + 0.2 * species->sand_percent
  - 0.2 * species->clay_percent
  + 0.3 * organic_influence
  );
  if (target->sat_noise > 1.0) { target->sat_noise = 1.0; }
  seed = prng(seed);

  // We define our color in L*c*h*, and convert to RGB later, potentially
  // clipping some values to keep them in-gamut.
  precise_color color;
  compute_combined_dirt_color(species, &color, seed);
  seed = prng(seed);

  // lightness
  // a bit of lightness variance:
  color.x *= (
    randf_pnorm(seed, 0.6, 1.0)
  + 0.3 * species->sand_percent
  + 0.2 * species->clay_percent
  - 0.4 * organic_influence
  );

  // saturation
  // a bit of saturation variance:
  color.y *= (
    randf_pnorm(seed, 0.9, 1.1)
  - 0.1 * species->sand_percent
  - 0.1 * species->clay_percent
  + 0.2 * organic_influence
  );
  // attenuate according to how far our hue is from dirt brown
  color.y *= (
    0.3
  + 0.7 * (1 - (angle_between(color.z, SL_COLOR_DIRT_BROWN) / M_PI))
  );

  // hue
  // organics make things brown
  color.z = (
    (1 - organic_influence) * color.z
  + (
      organic_influence
    * M_PI
    * randf(seed, SL_COLOR_REDDISH_BROWN, SL_COLOR_GREENISH_BROWN)
    )
  );

  // Construct the base color:
  lch__lab(&color);
  lab__xyz(&color);

  target->base_color = xyz__rgb(&color);

  // Dirt inclusions use the same color as the base, but are often darker or
  // lighter and have little saturation.
  xyz__lab(&color);
  lab__lch(&color);

  color.x *= randf(seed, 0.4, 1.4); // lightness
  color.y *= randf_pnorm(seed, 0.0, 0.4); // saturation
  seed = prng(seed);

  lch__lab(&color);
  lab__xyz(&color);

  target->alt_color = xyz__rgb(&color);

  // Dirt is generally dark.
  target->brightness = (
    randf_pnorm(seed, -0.2, 0.2)
  + 0.1 * species->sand_percent
  + 0.1 * species->clay_percent
  - 0.2 * organic_influence
  );
}

void determine_new_dirt_material(dirt_species *species, ptrdiff_t seed) {
  // TODO: HERE!
}

void compute_combined_dirt_color(
  dirt_species *species,
  precise_color *color,
  ptrdiff_t seed
) {
  size_t i;
  element_species *element;
  float denom = 0.0;
  float avg;
  float weight;

  color->x = 0;
  color->y = 0;
  color->z = 0;

  // Hue
  // TODO: Tint vs. oxide colors?
  for (i = 0; i < MN_MAX_TRACE_CONSTITUENTS; ++i) {
    if (species->trace_minerals[i] != 0) {
      element = get_element_species(species->trace_minerals[i]);
      weight = expdist(ptrf(seed), 2);
      seed = prng(seed);
      weight = 0.5 * weight + 0.5 * randf_pnorm(seed, 0.4, 1.0);
      seed = prng(seed);
      // TODO: Better circular averaging technique here?
      color->z += weight * element->stone_oxide_chroma;
      denom += weight;
    }
  }
  if (denom != 0) {
    color->z /= denom;
  }

  // Saturation
  color->y = expdist(ptrf(seed), 3) * 60.0;
  seed = prng(seed);

  // Lightness
  AVERAGE_ELEMENT_PROPERTY(
    species->trace_minerals,
    MN_MAX_TRACE_CONSTITUENTS,
    i,
    element,
    denom,
    stone_light_dark_tendency,
    avg
  );
  color->x = 100.0 * (ptrf(seed) + ptrf(prng(seed + 18291))) / 2.0;
  seed = prng(seed);
  color->x = 0.4 * color->x + 0.6 * avg;

  // The modified color value is the result; no need to return anything.
}
