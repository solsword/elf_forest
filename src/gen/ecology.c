// ecology.c
// Generation of ecology.

#ifdef DEBUG
#include <stdio.h>
#endif

#include "world/world_map.h"
#include "world/species.h"

#include "ecology.h"

/*************
 * Constants *
 *************/

biome_info const BIOME_INFO[] = {
  { // BIOME_CAT_OCEAN_VENTS,
    .max_size = EC_BIOME_TINY_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_OCEAN_DEPTHS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },
  { // BIOME_CAT_DEEP_AQUATIC,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_OCEAN_DEPTHS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },
  { // BIOME_CAT_PELAGIC,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_OCEAN_DEPTHS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },
  { // BIOME_CAT_OFFSHORE,
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
    );
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
    );
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
    );
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
    );
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
    | WM_TC_MILD_RARE_FROST
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
    | WM_TC_MILD_RARE_FROST
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
}

/*************
 * Functions *
 *************/

void generate_ecology(world_map *wm) {
  world_map_pos xy;
  world_region *wr;


    !breadth_first_iter(
      wr->world,
      &(wr->pos),
      CL_MIN_LAKE_SIZE,
      CL_MAX_LAKE_SIZE,
      (void*) water,
      &fill_water
    )
  // TODO: Something here!
}

step_result fill_with_biome(
  search_step step,
  world_region *wr,
  void* v_biome
) {
  biome *b = (biome*) v_biome;
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

void init_ocean_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

void init_beach_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

void init_lake_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

void init_river_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

void init_alpine_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

void init_terrestrial_biome(world_region *wr, biome *b, ptrdiff_t seed) {
}

