// ecology.c
// Generation of ecology.

#ifdef DEBUG
#include <stdio.h>
#endif

#include "world/world_map.h"
#include "world/species.h"
#include "datatypes/rngtable.h"

#include "ecology.h"

/*************
 * Constants *
 *************/

eco_info const ECO_INFO[] = {
  { // BIOME_CAT_UNKNOWN,
    .max_size = EC_BIOME_TINY_SIZE,
    .hydro_state_compatibility = 0,
    .salinity_compatibility = 0,
    .altitude_compatibility = 0,
    .precipitation_compatibility = 0,
    .temperature_compatibility = 0
  },

  { // BIOME_CAT_DEEP_AQUATIC,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_OCEAN_DEPTHS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },
  { // BIOME_CAT_OCEAN_VENTS,
    .max_size = EC_BIOME_TINY_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_OCEAN_DEPTHS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },

  { // BIOME_CAT_SEA_ICE,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = (WM_AC_CONTINENTAL_SHELF | WM_AC_OCEAN_DEPTHS),
    .precipitation_compatibility = -1,
    .temperature_compatibility = (WM_TC_ARCTIC | WM_TC_TUNDRA)
  },
  { // BIOME_CAT_TEMPERATE_PELAGIC,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_OCEAN_DEPTHS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    )
  },
  { // BIOME_CAT_TROPICAL_PELAGIC,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_OCEAN_DEPTHS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },

  { // BIOME_CAT_TEMPERATE_OFFSHORE,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONTINENTAL_SHELF,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },
  { // BIOME_CAT_TROPICAL_OFFSHORE,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONTINENTAL_SHELF,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },
  { // BIOME_CAT_AQUATIC_GRASSLAND,
    .max_size = EC_BIOME_SMALL_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN | WM_HS_OCEAN_SHORE,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONTINENTAL_SHELF | WM_AC_COASTAL_PLAINS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = ~(
      WM_TC_ARCTIC
    | WM_TC_TUNDRA
    | WM_TC_COLD_FROST
    )
  },
  { // BIOME_CAT_AQUATIC_FOREST,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONTINENTAL_SHELF,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },
  { // BIOME_CAT_COLD_REEF,
    .max_size = EC_BIOME_SMALL_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN | WM_HS_OCEAN_SHORE,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONTINENTAL_SHELF | WM_AC_COASTAL_PLAINS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_TUNDRA
    | WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    )
  },
  { // BIOME_CAT_WARM_REEF,
    .max_size = EC_BIOME_SMALL_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN | WM_HS_OCEAN_SHORE,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONTINENTAL_SHELF | WM_AC_COASTAL_PLAINS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },

  { // BIOME_CAT_FROZEN_BEACH,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN_SHORE,
    .salinity_compatibility = -1,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = WM_TC_ARCTIC | WM_TC_TUNDRA
  },
  { // BIOME_CAT_COLD_BEACH,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN_SHORE,
    .salinity_compatibility = -1,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    )
  },
  { // BIOME_CAT_WARM_BEACH,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN_SHORE,
    .salinity_compatibility = -1,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    )
  },
  { // BIOME_CAT_TROPICAL_BEACH,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN_SHORE,
    .salinity_compatibility = -1,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },

  { // BIOME_CAT_FROZEN_LAKE,
    .max_size = EC_BIOME_GIGANTIC_SIZE, // limited by lake size
    .hydro_state_compatibility = WM_HS_LAKE | WM_HS_LAKE_SHORE,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = WM_TC_ARCTIC | WM_TC_TUNDRA
  },
  { // BIOME_CAT_COLD_LAKE,
    .max_size = EC_BIOME_GIGANTIC_SIZE, // limited by lake size
    .hydro_state_compatibility = WM_HS_LAKE | WM_HS_LAKE_SHORE,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    )
  },
  { // BIOME_CAT_TEMPERATE_LAKE,
    .max_size = EC_BIOME_GIGANTIC_SIZE, // limited by lake size
    .hydro_state_compatibility = WM_HS_LAKE | WM_HS_LAKE_SHORE,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    )
  },
  { // BIOME_CAT_WARM_LAKE,
    .max_size = EC_BIOME_GIGANTIC_SIZE, // limited by lake size
    .hydro_state_compatibility = WM_HS_LAKE | WM_HS_LAKE_SHORE,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    )
  },
  { // BIOME_CAT_TROPICAL_LAKE,
    .max_size = EC_BIOME_GIGANTIC_SIZE, // limited by lake size
    .hydro_state_compatibility = WM_HS_LAKE | WM_HS_LAKE_SHORE,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },
  { // BIOME_CAT_SALT_LAKE,
    .max_size = EC_BIOME_GIGANTIC_SIZE, // limited by lake size
    .hydro_state_compatibility = WM_HS_LAKE | WM_HS_LAKE_SHORE,
    .salinity_compatibility = WM_SL_BRACKISH | WM_SL_SALINE | WM_SL_BRINY,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },

  { // BIOME_CAT_FROZEN_RIVER,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = WM_HS_LAKE_SHORE | WM_HS_RIVER,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = WM_TC_TUNDRA
  },
  { // BIOME_CAT_COLD_RIVER,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = WM_HS_LAKE_SHORE | WM_HS_RIVER,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    )
  },
  { // BIOME_CAT_TEMPERATE_RIVER,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = WM_HS_LAKE_SHORE | WM_HS_RIVER,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    )
  },
  { // BIOME_CAT_WARM_RIVER,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = WM_HS_LAKE_SHORE | WM_HS_RIVER,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    )
  },
  { // BIOME_CAT_TROPICAL_RIVER,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = WM_HS_LAKE_SHORE | WM_HS_RIVER,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },

  { // BIOME_CAT_FROZEN_ALPINE,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = WM_AC_MOUNTAIN_SLOPES | WM_AC_MOUNTAIN_PEAKS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_ARCTIC
    | WM_TC_TUNDRA
    )
  },
  { // BIOME_CAT_COLD_ALPINE,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = WM_AC_MOUNTAIN_SLOPES | WM_AC_MOUNTAIN_PEAKS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    )
  },
  { // BIOME_CAT_TEMPERATE_WET_ALPINE,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = WM_AC_MOUNTAIN_SLOPES | WM_AC_MOUNTAIN_PEAKS,
    .precipitation_compatibility = (
      WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    )
  },
  { // BIOME_CAT_TEMPERATE_DRY_ALPINE,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = WM_AC_MOUNTAIN_SLOPES | WM_AC_MOUNTAIN_PEAKS,
    .precipitation_compatibility = (
      WM_PC_DESERT
    | WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    ),
    .temperature_compatibility = (
      WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    )
  },
  { // BIOME_CAT_WARM_WET_ALPINE,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = WM_AC_MOUNTAIN_SLOPES | WM_AC_MOUNTAIN_PEAKS,
    .precipitation_compatibility = (
      WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    )
  },
  { // BIOME_CAT_WARM_DRY_ALPINE,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = WM_AC_MOUNTAIN_SLOPES | WM_AC_MOUNTAIN_PEAKS,
    .precipitation_compatibility = (
      WM_PC_DESERT
    | WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    ),
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    )
  },
  { // BIOME_CAT_TROPICAL_WET_ALPINE,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = WM_AC_MOUNTAIN_SLOPES | WM_AC_MOUNTAIN_PEAKS,
    .precipitation_compatibility = (
      WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },
  { // BIOME_CAT_TROPICAL_DRY_ALPINE,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = WM_AC_MOUNTAIN_SLOPES | WM_AC_MOUNTAIN_PEAKS,
    .precipitation_compatibility = (
      WM_PC_DESERT
    | WM_PC_ARID
    | WM_PC_DRY
    ),
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },

  { // BIOME_CAT_FROZEN_DESERT,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = WM_PC_DESERT | WM_PC_ARID,
    .temperature_compatibility = WM_TC_ARCTIC | WM_TC_TUNDRA
  },
  { // BIOME_CAT_COLD_DESERT,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = WM_PC_DESERT | WM_PC_ARID,
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    )
  },
  { // BIOME_CAT_TEMPERATE_DESERT,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = WM_PC_DESERT | WM_PC_ARID,
    .temperature_compatibility = (
      WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    )
  },
  { // BIOME_CAT_WARM_DESERT,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = WM_PC_DESERT | WM_PC_ARID,
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    )
  },
  { // BIOME_CAT_HOT_DESERT,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = WM_PC_DESERT | WM_PC_ARID,
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },

  { // BIOME_CAT_COLD_GRASSLAND,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    ),
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    )
  },
  { // BIOME_CAT_TEMPERATE_GRASSLAND,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    ),
    .temperature_compatibility = (
      WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    )
  },
  { // BIOME_CAT_WARM_GRASSLAND,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    ),
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    )
  },
  { // BIOME_CAT_TROPICAL_GRASSLAND,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    ),
    .temperature_compatibility = WM_TC_HOT | WM_TC_TROPICAL
  },

  { // BIOME_CAT_COLD_SHRUBLAND,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    ),
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    )
  },
  { // BIOME_CAT_TEMPERATE_SHRUBLAND,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    ),
    .temperature_compatibility = (
      WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    )
  },
  { // BIOME_CAT_WARM_SHRUBLAND,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
      | WM_AC_COASTAL_PLAINS
      | WM_AC_INLAND_HILLS
      | WM_AC_HIGHLANDS
      | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    ),
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    )
  },
  { // BIOME_CAT_TROPICAL_SHRUBLAND,
    .max_size = EC_BIOME_SMALL_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    ),
    .temperature_compatibility = WM_TC_HOT | WM_TC_TROPICAL
  },

  { // BIOME_CAT_TEMPERATE_SAVANNA,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    ),
    .temperature_compatibility = (
      WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    )
  },
  { // BIOME_CAT_WARM_SAVANNA,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    ),
    .temperature_compatibility = (
      WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    )
  },
  { // BIOME_CAT_TROPICAL_SAVANNA,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_ARID
    | WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    ),
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },

  { // BIOME_CAT_COLD_CONIFEROUS_FOREST,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    )
  },
  { // BIOME_CAT_TEMPERATE_CONIFEROUS_FOREST,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    )
  },
  { // BIOME_CAT_WARM_CONIFEROUS_FOREST,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    )
  },
  { // BIOME_CAT_TROPICAL_CONIFEROUS_FOREST,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_DRY
    | WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },

  { // BIOME_CAT_TEMPERATE_BROADLEAF_FOREST,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_SEASONAL
    | WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    ),
    .temperature_compatibility = (
      WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    )
  },
  { // BIOME_CAT_WARM_WET_BROADLEAF_FOREST,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    )
  },
  { // BIOME_CAT_WARM_DRY_BROADLEAF_FOREST,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_DRY
    | WM_PC_SEASONAL
    ),
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    )
  },
  { // BIOME_CAT_TROPICAL_WET_BROADLEAF_FOREST,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },
  { // BIOME_CAT_TROPICAL_DRY_BROADLEAF_FOREST,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = (
      WM_PC_DRY
    | WM_PC_SEASONAL
    ),
    .temperature_compatibility = (
      WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },

  { // BIOME_CAT_TUNDRA,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = -1,
    .temperature_compatibility = WM_TC_TUNDRA
  },
  { // BIOME_CAT_COLD_FRESHWATER_WETLAND,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    )
  },
  { // BIOME_CAT_COLD_SALTWATER_WETLAND,
    .max_size = EC_BIOME_SMALL_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = (
      WM_SL_BRACKISH
    | WM_SL_SALINE
    ),
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    )
  },
  { // BIOME_CAT_TEMPERATE_FRESHWATER_WETLAND,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    )
  },
  { // BIOME_CAT_TEMPERATE_SALTWATER_WETLAND,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = (
      WM_SL_BRACKISH
    | WM_SL_SALINE
    ),
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    )
  },
  { // BIOME_CAT_WARM_FRESHWATER_WETLAND,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    )
  },
  { // BIOME_CAT_WARM_FRESHWATER_FORESTED_WETLAND,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    )
  },
  { // BIOME_CAT_WARM_SALTWATER_WETLAND,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = (
      WM_SL_BRACKISH
    | WM_SL_SALINE
    ),
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_MILD_RARE_FROST
    | WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    )
  },
  { // BIOME_CAT_TROPICAL_FRESHWATER_WETLAND,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },
  { // BIOME_CAT_TROPICAL_FRESHWATER_FORESTED_WETLAND,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },
  { // BIOME_CAT_TROPICAL_SALTWATER_WETLAND,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = (
      WM_SL_BRACKISH
    | WM_SL_SALINE
    ),
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    ),
    .precipitation_compatibility = (
      WM_PC_NORMAL
    | WM_PC_WET
    | WM_PC_SOAKING
    | WM_PC_FLOODED
    ),
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },
  { // BIOME_CAT_TROPICAL_SALTWATER_FORESTED_WETLAND
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = (
      WM_SL_BRACKISH
    | WM_SL_SALINE
    ),
    .altitude_compatibility = (
      WM_AC_CONTINENTAL_SHELF
    | WM_AC_COASTAL_PLAINS
    ),
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },
};

static rngtable DEEP_OCEAN_BIOME_CATEGORIES = {
  .size = 2,
  .values = (void*[]) {
    (void*) BIOME_CAT_DEEP_AQUATIC,
    (void*) BIOME_CAT_OCEAN_VENTS
  },
  .weights = (float[]) { 0.9, 0.1 }
};

static rngtable PELAGIC_BIOME_CATEGORIES = {
  .size = 3,
  .values = (void*[]) {
    (void*) BIOME_CAT_SEA_ICE,
    (void*) BIOME_CAT_TEMPERATE_PELAGIC,
    (void*) BIOME_CAT_TROPICAL_PELAGIC
  },
  .weights = (float[]) { 1, 1, 1 }
};

static rngtable OFFSHORE_BIOME_CATEGORIES = {
  .size = 8,
  .values = (void*[]) {
    (void*) BIOME_CAT_TEMPERATE_OFFSHORE,
    (void*) BIOME_CAT_TROPICAL_OFFSHORE,
    (void*) BIOME_CAT_TEMPERATE_AQUATIC_GRASSLAND,
    (void*) BIOME_CAT_TROPICAL_AQUATIC_GRASSLAND,
    (void*) BIOME_CAT_TEMPERATE_AQUATIC_FOREST,
    (void*) BIOME_CAT_TROPICAL_AQUATIC_FOREST,
    (void*) BIOME_CAT_COLD_REEF,
    (void*) BIOME_CAT_WARM_REEF
  },
  .weights = (float[]) { 1, 0.8, 0.3, 0.3, 0.4, 0.3, 0.4, 0.2 }
};

static rngtable BEACH_BIOME_CATEGORIES = {
  .size = 4,
  .values = (void*[]) {
    (void*) BIOME_CAT_FROZEN_BEACH,
    (void*) BIOME_CAT_COLD_BEACH,
    (void*) BIOME_CAT_WARM_BEACH,
    (void*) BIOME_CAT_TROPICAL_BEACH
  },
  .weights = (float[]) { 1, 1.2, 1.2, 1 }
};

static rngtable LAKE_BIOME_CATEGORIES = {
  .size = 6,
  .values = (void*[]) {
    (void*) BIOME_CAT_FROZEN_LAKE,
    (void*) BIOME_CAT_COLD_LAKE,
    (void*) BIOME_CAT_TEMPERATE_LAKE,
    (void*) BIOME_CAT_WARM_LAKE,
    (void*) BIOME_CAT_TROPICAL_LAKE,
    (void*) BIOME_CAT_SALT_LAKE
  },
  .weights = (float[]) { 0.8, 1, 1, 1, 1, 0.05 }
};

static rngtable RIVER_BIOME_CATEGORIES = {
  .size = 5,
  .values = (void*[]) {
    (void*) BIOME_CAT_FROZEN_RIVER,
    (void*) BIOME_CAT_COLD_RIVER,
    (void*) BIOME_CAT_TEMPERATE_RIVER,
    (void*) BIOME_CAT_WARM_RIVER,
    (void*) BIOME_CAT_TROPICAL_RIVER
  },
  .weights = (float[]) { 0.8, 1, 1, 1, 1 }
};

static rngtable ALPINE_BIOME_CATEGORIES = {
  .size = 8,
  .values = (void*[]) {
    (void*) BIOME_CAT_FROZEN_ALPINE,
    (void*) BIOME_CAT_COLD_ALPINE,
    (void*) BIOME_CAT_TEMPERATE_WET_ALPINE,
    (void*) BIOME_CAT_TEMPERATE_DRY_ALPINE,
    (void*) BIOME_CAT_WARM_WET_ALPINE,
    (void*) BIOME_CAT_WARM_DRY_ALPINE,
    (void*) BIOME_CAT_TROPICAL_WET_ALPINE,
    (void*) BIOME_CAT_TROPICAL_DRY_ALPINE
  },
  .weights = (float[]) { 0.8, 1, 1, 0.9, 1, 0.9, 1, 0.9 }
};

static rngtable TERRESTRIAL_BIOME_CATEGORIES = {
  .size = 37,
  .values = (void*[]) {
    (void*) BIOME_CAT_FROZEN_DESERT,
    (void*) BIOME_CAT_COLD_DESERT,
    (void*) BIOME_CAT_TEMPERATE_DESERT,
    (void*) BIOME_CAT_WARM_DESERT,
    (void*) BIOME_CAT_HOT_DESERT,

    (void*) BIOME_CAT_COLD_GRASSLAND,
    (void*) BIOME_CAT_TEMPERATE_GRASSLAND,
    (void*) BIOME_CAT_WARM_GRASSLAND,
    (void*) BIOME_CAT_TROPICAL_GRASSLAND,

    (void*) BIOME_CAT_COLD_SHRUBLAND,
    (void*) BIOME_CAT_TEMPERATE_SHRUBLAND,
    (void*) BIOME_CAT_WARM_SHRUBLAND,
    (void*) BIOME_CAT_TROPICAL_SHRUBLAND,

    (void*) BIOME_CAT_TEMPERATE_SAVANNA,
    (void*) BIOME_CAT_WARM_SAVANNA,
    (void*) BIOME_CAT_TROPICAL_SAVANNA,

    (void*) BIOME_CAT_COLD_CONIFEROUS_FOREST,
    (void*) BIOME_CAT_TEMPERATE_CONIFEROUS_FOREST,
    (void*) BIOME_CAT_WARM_CONIFEROUS_FOREST,
    (void*) BIOME_CAT_TROPICAL_CONIFEROUS_FOREST,

    (void*) BIOME_CAT_TEMPERATE_BROADLEAF_FOREST,
    (void*) BIOME_CAT_WARM_WET_BROADLEAF_FOREST,
    (void*) BIOME_CAT_WARM_DRY_BROADLEAF_FOREST,
    (void*) BIOME_CAT_TROPICAL_WET_BROADLEAF_FOREST,
    (void*) BIOME_CAT_TROPICAL_DRY_BROADLEAF_FOREST,

    (void*) BIOME_CAT_TUNDRA,
    (void*) BIOME_CAT_COLD_FRESHWATER_WETLAND,
    (void*) BIOME_CAT_COLD_SALTWATER_WETLAND,
    (void*) BIOME_CAT_TEMPERATE_FRESHWATER_WETLAND,
    (void*) BIOME_CAT_TEMPERATE_SALTWATER_WETLAND,
    (void*) BIOME_CAT_WARM_FRESHWATER_WETLAND,
    (void*) BIOME_CAT_WARM_FRESHWATER_FORESTED_WETLAND,
    (void*) BIOME_CAT_WARM_SALTWATER_WETLAND,
    (void*) BIOME_CAT_TROPICAL_FRESHWATER_WETLAND,
    (void*) BIOME_CAT_TROPICAL_FRESHWATER_FORESTED_WETLAND,
    (void*) BIOME_CAT_TROPICAL_SALTWATER_WETLAND,
    (void*) BIOME_CAT_TROPICAL_SALTWATER_FORESTED_WETLAND
  },
  .weights = (float[]) {
    0.2, // BIOME_CAT_FROZEN_DESERT,
    0.2, // BIOME_CAT_COLD_DESERT,
    0.2, // BIOME_CAT_TEMPERATE_DESERT,
    0.25, // BIOME_CAT_WARM_DESERT,
    0.25, // BIOME_CAT_HOT_DESERT,

    0.8, // BIOME_CAT_COLD_GRASSLAND,
    0.9, // BIOME_CAT_TEMPERATE_GRASSLAND,
    0.9, // BIOME_CAT_WARM_GRASSLAND,
    0.7, // BIOME_CAT_TROPICAL_GRASSLAND,

    0.4, // BIOME_CAT_COLD_SHRUBLAND,
    0.5, // BIOME_CAT_TEMPERATE_SHRUBLAND,
    0.5, // BIOME_CAT_WARM_SHRUBLAND,
    0.4, // BIOME_CAT_TROPICAL_SHRUBLAND,

    0.4, // BIOME_CAT_TEMPERATE_SAVANNA,
    0.3, // BIOME_CAT_WARM_SAVANNA,
    0.3, // BIOME_CAT_TROPICAL_SAVANNA,

    0.8, // BIOME_CAT_COLD_CONIFEROUS_FOREST,
    0.8, // BIOME_CAT_TEMPERATE_CONIFEROUS_FOREST,
    0.4, // BIOME_CAT_WARM_CONIFEROUS_FOREST,
    0.4, // BIOME_CAT_TROPICAL_CONIFEROUS_FOREST,

    1.0, // BIOME_CAT_TEMPERATE_BROADLEAF_FOREST,
    0.9, // BIOME_CAT_WARM_WET_BROADLEAF_FOREST,
    0.8, // BIOME_CAT_WARM_DRY_BROADLEAF_FOREST,
    1.0, // BIOME_CAT_TROPICAL_WET_BROADLEAF_FOREST,
    0.8, // BIOME_CAT_TROPICAL_DRY_BROADLEAF_FOREST,

    0.3,  // BIOME_CAT_TUNDRA, (often forced)
    0.15,  // BIOME_CAT_COLD_FRESHWATER_WETLAND,
    0.05, // BIOME_CAT_COLD_SALTWATER_WETLAND,
    0.15,  // BIOME_CAT_TEMPERATE_FRESHWATER_WETLAND,
    0.05, // BIOME_CAT_TEMPERATE_SALTWATER_WETLAND,
    0.15,  // BIOME_CAT_WARM_FRESHWATER_WETLAND,
    0.05, // BIOME_CAT_WARM_FRESHWATER_FORESTED_WETLAND,
    0.15,  // BIOME_CAT_WARM_SALTWATER_WETLAND,
    0.15,  // BIOME_CAT_TROPICAL_FRESHWATER_WETLAND,
    0.15,  // BIOME_CAT_TROPICAL_FRESHWATER_FORESTED_WETLAND,
    0.05, // BIOME_CAT_TROPICAL_SALTWATER_WETLAND,
    0.05  // BIOME_CAT_TROPICAL_SALTWATER_FORESTED_WETLAND
  }
};

/*********************
 * Private Functions *
 *********************/

int _rtfilter_biome_category_fits_region(
  void* v_category,
  void* v_region
) {
  return biome_is_compatible(
    (biome_category) v_category,
    (world_region*) v_region
  );
}

int needs_biome_in_table(world_region *wr, void *v_table, ptrdiff_t seed) {
  rngtable *rt = (rngtable*) v_table;
  size_t i;
  biome_category bc;
  int some_biome_is_compatible = 0;
  // If this region is full then it's not available for adding biomes to...
  if (wr->ecology.biome_count >= WM_MAX_BIOME_OVERLAP) {
    return 0;
  }
  for (i = 0; i < rt->size; ++i) {
    bc = (biome_category) (rt->values[i]);
    if (has_biome_in_category(wr, bc)) {
      // check if this region already has this kind of biome
      return 0;
    } else if (biome_is_compatible(bc, wr)) {
      // check whether this kind of biome would be compatible with this region
      some_biome_is_compatible = 1;
    }
  }
  return some_biome_is_compatible;
}

int _fill_biome_from_table(world_region *wr, void *v_table, ptrdiff_t seed) {
  rngtable *rt = (rngtable*) v_table;
  biome_category bc;
  eco_info ei;
  biome *b;

  bc = (biome_category) rt_pick_filtered_result(
    rt,
    (void*) wr,
    &_rtfilter_biome_category_fits_region,
    seed ^ (wr->seed)
  );
#ifdef DEBUG
  if (bc == (biome_category) NULL) {
    printf("Error: no valid biomes for region during biome placement!\n");
    exit(1);
  }
#endif

  ei = ECO_INFO[bc];
  b = create_biome(bc);
  // TODO: Generate biome info for this biome!
  breadth_first_iter(
    wr->world,
    &(wr->pos),
    0,
    ei.max_size,
    (void*) b,
    &fill_with_biome
  );
  return 1;
}

/*************
 * Functions *
 *************/

void generate_ecology(world_map *wm) {
  ptrdiff_t seed = prng(wm->seed = 18182);

  fill_with_regions(
    wm,
    &DEEP_OCEAN_BIOME_CATEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions( // pelagic
    wm,
    &PELAGIC_BIOME_CATEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &OFFSHORE_BIOME_CATEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &BEACH_BIOME_CATEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &LAKE_BIOME_CATEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &RIVER_BIOME_CATEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &ALPINE_BIOME_CATEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &TERRESTRIAL_BIOME_CATEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);
}

step_result fill_with_biome(
  search_step step,
  world_region *wr,
  void* v_biome
) {
  static size_t size = 0;
  biome *b = (biome*) v_biome;
  switch (step) {
    case SSTEP_INIT:
      size = 0;
      return SRESULT_CONTINUE;
    default:
#ifdef DEBUG
      printf("Unknown search/fill step: %d\n", step);
      return SRESULT_ABORT;
#endif
    case SSTEP_CLEANUP:
    case SSTEP_FINISH:
      return SRESULT_CONTINUE;
    case SSTEP_PROCESS:
      if (
         size > ECO_INFO[b->category].max_size
      || !biome_is_compatible(b->category, wr)
      ) {
        return SRESULT_IGNORE;
      } else {
        add_biome(wr, b);
        size += 1;
        return SRESULT_CONTINUE;
      }
  }
}

void init_any_biome(biome *b, world_region *wr) {
  switch (b->category) {
    default:
    case BIOME_CAT_UNKNOWN:
      // No initialization
#ifdef DEBUG
      printf(
        "Error: Attempt to initialize a biome with an unknown category!\n"
      );
      exit(1);
#endif
      break;
    case BIOME_CAT_DEEP_AQUATIC:
      init_deep_aquatic_biome(b, wr);
      break;
    case BIOME_CAT_OCEAN_VENTS:
      init_ocean_vents_biome(b, wr);
      break;

    case BIOME_CAT_SEA_ICE:
      init_sea_ice_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_PELAGIC:
      init_temperate_pelagic_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_PELAGIC:
      init_tropical_pelagic_biome(b, wr);
      break;

    case BIOME_CAT_TEMPERATE_OFFSHORE:
      init_temperate_offshore_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_OFFSHORE:
      init_tropical_offshore_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_AQUATIC_GRASSLAND:
      init_temperate_aquatic_grassland_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_AQUATIC_GRASSLAND:
      init_tropical_aquatic_grassland_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_AQUATIC_FOREST:
      init_temperate_aquatic_forest_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_AQUATIC_FOREST:
      init_tropical_aquatic_forest_biome(b, wr);
      break;
    case BIOME_CAT_COLD_REEF:
      init_cold_reef_biome(b, wr);
      break;
    case BIOME_CAT_WARM_REEF:
      init_warm_reef_biome(b, wr);
      break;

    case BIOME_CAT_FROZEN_BEACH:
      init_frozen_beach_biome(b, wr);
      break;
    case BIOME_CAT_COLD_BEACH:
      init_cold_beach_biome(b, wr);
      break;
    case BIOME_CAT_WARM_BEACH:
      init_warm_beach_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_BEACH:
      init_tropical_beach_biome(b, wr);
      break;

    case BIOME_CAT_FROZEN_LAKE:
      init_frozen_lake_biome(b, wr);
      break;
    case BIOME_CAT_COLD_LAKE:
      init_cold_lake_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_LAKE:
      init_temperate_lake_biome(b, wr);
      break;
    case BIOME_CAT_WARM_LAKE:
      init_warm_lake_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_LAKE:
      init_tropical_lake_biome(b, wr);
      break;
    case BIOME_CAT_SALT_LAKE:
      init_salt_lake_biome(b, wr);
      break;

    case BIOME_CAT_COLD_RIVER:
      init_cold_river_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_RIVER:
      init_temperate_river_biome(b, wr);
      break;
    case BIOME_CAT_WARM_RIVER:
      init_warm_river_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_RIVER:
      init_tropical_river_biome(b, wr);
      break;

    case BIOME_CAT_FROZEN_ALPINE:
      init_frozen_alpine_biome(b, wr);
      break;
    case BIOME_CAT_COLD_ALPINE:
      init_cold_alpine_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_WET_ALPINE:
      init_temperate_wet_alpine_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_DRY_ALPINE:
      init_temperate_dry_alpine_biome(b, wr);
      break;
    case BIOME_CAT_WARM_WET_ALPINE:
      init_warm_wet_alpine_biome(b, wr);
      break;
    case BIOME_CAT_WARM_DRY_ALPINE:
      init_warm_dry_alpine_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_WET_ALPINE:
      init_tropical_wet_alpine_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_DRY_ALPINE:
      init_tropical_dry_alpine_biome(b, wr);
      break;

    case BIOME_CAT_FROZEN_DESERT:
      init_frozen_desert_biome(b, wr);
      break;
    case BIOME_CAT_COLD_DESERT:
      init_cold_desert_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_DESERT:
      init_temperate_desert_biome(b, wr);
      break;
    case BIOME_CAT_WARM_DESERT:
      init_warm_desert_biome(b, wr);
      break;
    case BIOME_CAT_HOT_DESERT:
      init_hot_desert_biome(b, wr);
      break;

    case BIOME_CAT_COLD_GRASSLAND:
      init_cold_grassland_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_GRASSLAND:
      init_temperate_grassland_biome(b, wr);
      break;
    case BIOME_CAT_WARM_GRASSLAND:
      init_warm_grassland_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_GRASSLAND:
      init_tropical_grassland_biome(b, wr);
      break;
    case BIOME_CAT_COLD_SHRUBLAND:
      init_temperate_shrubland_biome(b, wr);
      break;
    case BIOME_CAT_WARM_SHRUBLAND:
      init_warm_shrubland_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_SHRUBLAND:
      init_tropical_shrubland_biome(b, wr);
      break;

    case BIOME_CAT_TEMPERATE_SAVANNA:
      init_temperate_savanna_biome(b, wr);
      break;
    case BIOME_CAT_WARM_SAVANNA:
      init_warm_savanna_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_SAVANNA:
      init_tropical_savanna_biome(b, wr);
      break;

    case BIOME_CAT_COLD_CONIFEROUS_FOREST:
      init_cold_coniferous_forest_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_CONIFEROUS_FOREST:
      init_temperate_coniferous_forest_biome(b, wr);
      break;
    case BIOME_CAT_WARM_CONIFEROUS_FOREST:
      init_warm_coniferous_forest_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_CONIFEROUS_FOREST:
      init_tropical_coniferous_forest_biome(b, wr);
      break;

    case BIOME_CAT_TEMPERATE_BROADLEAF_FOREST:
      init_warm_wet_broadleaf_forest_biome(b, wr);
      break;
    case BIOME_CAT_WARM_DRY_BROADLEAF_FOREST:
      init_warm_dry_broadleaf_forest_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_WET_BROADLEAF_FOREST:
      init_tropical_wet_broadleaf_forest_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_DRY_BROADLEAF_FOREST:
      init_tropical_dry_broadleaf_forest_biome(b, wr);
      break;

    case BIOME_CAT_TUNDRA:
      init_tundra_biome(b, wr);
      break;
    case BIOME_CAT_COLD_FRESHWATER_WETLAND:
      init_cold_freshwater_wetland_biome(b, wr);
      break;
    case BIOME_CAT_COLD_SALTWATER_WETLAND:
      init_cold_saltwater_wetland_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_FRESHWATER_WETLAND:
      init_temperate_freshwater_wetland_biome(b, wr);
      break;
    case BIOME_CAT_TEMPERATE_SALTWATER_WETLAND:
      init_temperate_saltwater_wetland_biome(b, wr);
      break;
    case BIOME_CAT_WARM_FRESHWATER_WETLAND:
      init_warm_freshwater_wetland_biome(b, wr);
      break;
    case BIOME_CAT_WARM_FRESHWATER_FORESTED_WETLAND:
      init_warm_freshwater_forested_wetland_biome(b, wr);
      break;
    case BIOME_CAT_WARM_SALTWATER_WETLAND:
      init_warm_saltwater_wetland_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_FRESHWATER_WETLAND:
      init_tropical_freshwater_wetland_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_FRESHWATER_FORESTED_WETLAND:
      init_tropical_freshwater_forested_wetland_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_SALTWATER_WETLAND:
      init_tropical_saltwater_wetland_biome(b, wr);
      break;
    case BIOME_CAT_TROPICAL_SALTWATER_FORESTED_WETLAND:
      init_tropical_saltwater_forested_wetland_biome(b, wr);
      break;
  }
}

void init_deep_aquatic_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_ocean_vents_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_sea_ice_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_pelagic_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_pelagic_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_temperate_offshore_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_offshore_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_aquatic_grassland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_aquatic_grassland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_aquatic_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_aquatic_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_cold_reef_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_reef_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_frozen_beach_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_cold_beach_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_beach_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_beach_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_frozen_lake_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_cold_lake_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_lake_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_lake_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_lake_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_salt_lake_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_cold_river_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_river_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_river_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_river_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_frozen_alpine_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_cold_alpine_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_wet_alpine_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_dry_alpine_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_wet_alpine_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_dry_alpine_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_wet_alpine_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_dry_alpine_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_frozen_desert_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_cold_desert_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_desert_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_desert_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_hot_desert_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_cold_grassland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_temperate_grassland_biome(biome *b, world_region *wr) {
  //frequent_species fqsp;
  //frequent_species_set_frequency(&fqsp, frequency);
  //l_append_element(b->terrestrial_flora, fsp);
  // TODO: HERE
}

void init_warm_grassland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_grassland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_cold_shrubland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_shrubland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_shrubland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_shrubland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_temperate_savanna_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_savanna_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_savanna_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_cold_coniferous_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_coniferous_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_coniferous_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_coniferous_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_temperate_broadleaf_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_wet_broadleaf_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_dry_broadleaf_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_wet_broadleaf_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_dry_broadleaf_forest_biome(biome *b, world_region *wr) {
  // TODO: HERE
}


void init_tundra_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_cold_freshwater_wetland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_cold_saltwater_wetland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_freshwater_wetland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_temperate_saltwater_wetland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_freshwater_wetland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_freshwater_forested_wetland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_warm_saltwater_wetland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_freshwater_wetland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_freshwater_forested_wetland_biome(biome *b,world_region *wr) {
  // TODO: HERE
}

void init_tropical_saltwater_wetland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

void init_tropical_saltwater_forested_wetland_biome(biome *b, world_region *wr) {
  // TODO: HERE
}

