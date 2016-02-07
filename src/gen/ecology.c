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
  { // WM_BC_UNK,
    .max_size = EC_BIOME_TINY_SIZE,
    .hydro_state_compatibility = 0,
    .salinity_compatibility = 0,
    .altitude_compatibility = 0,
    .precipitation_compatibility = 0,
    .temperature_compatibility = 0
  },

  { // WM_BC_DEEP_AQ,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_OCEAN_DEPTHS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },
  { // WM_BC_OCN_VNTS,
    .max_size = EC_BIOME_TINY_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_OCEAN_DEPTHS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },

  { // WM_BC_SEA_ICE,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = (WM_AC_CONT_SHELF | WM_AC_OCEAN_DEPTHS),
    .precipitation_compatibility = -1,
    .temperature_compatibility = (WM_TC_ARCTIC | WM_TC_TUNDRA)
  },
  { // WM_BC_TMP_PEL,
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
  { // WM_BC_TRP_PEL,
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

  { // WM_BC_TMP_OFSH,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONT_SHELF,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },
  { // WM_BC_TRP_OFSH,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONT_SHELF,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },
  { // WM_BC_AQ_GSLD,
    .max_size = EC_BIOME_SMALL_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN | WM_HS_OCEAN_SHORE,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONT_SHELF | WM_AC_COASTAL_PLAINS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = ~(
      WM_TC_ARCTIC
    | WM_TC_TUNDRA
    | WM_TC_COLD_FROST
    )
  },
  { // WM_BC_AQ_FRST,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONT_SHELF,
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
  { // WM_BC_CLD_REEF,
    .max_size = EC_BIOME_SMALL_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN | WM_HS_OCEAN_SHORE,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONT_SHELF | WM_AC_COASTAL_PLAINS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_TUNDRA
    | WM_TC_COLD_FROST
    | WM_TC_COLD_RARE_FROST
    | WM_TC_MILD_FROST
    | WM_TC_MILD_RARE_FROST
    )
  },
  { // WM_BC_WRM_REEF,
    .max_size = EC_BIOME_SMALL_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN | WM_HS_OCEAN_SHORE,
    .salinity_compatibility = WM_SL_SALINE,
    .altitude_compatibility = WM_AC_CONT_SHELF | WM_AC_COASTAL_PLAINS,
    .precipitation_compatibility = -1,
    .temperature_compatibility = (
      WM_TC_WARM_FROST
    | WM_TC_WARM_NO_FROST
    | WM_TC_HOT
    | WM_TC_TROPICAL
    )
  },

  { // WM_BC_FRZ_SHOR,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = WM_HS_OCEAN_SHORE,
    .salinity_compatibility = -1,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = WM_TC_ARCTIC | WM_TC_TUNDRA
  },
  { // WM_BC_CLD_SHOR,
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
  { // WM_BC_WRM_SHOR,
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
  { // WM_BC_TRP_SHOR,
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

  { // WM_BC_FRZ_LAKE,
    .max_size = EC_BIOME_GIGANTIC_SIZE, // limited by lake size
    .hydro_state_compatibility = WM_HS_LAKE | WM_HS_LAKE_SHORE,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = WM_TC_ARCTIC | WM_TC_TUNDRA
  },
  { // WM_BC_CLD_LAKE,
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
  { // WM_BC_TMP_LAKE,
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
  { // WM_BC_WRM_LAKE,
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
  { // WM_BC_TRP_LAKE,
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
  { // WM_BC_SALT_LAKE,
    .max_size = EC_BIOME_GIGANTIC_SIZE, // limited by lake size
    .hydro_state_compatibility = WM_HS_LAKE | WM_HS_LAKE_SHORE,
    .salinity_compatibility = WM_SL_BRACKISH | WM_SL_SALINE | WM_SL_BRINY,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = -1
  },

  { // WM_BC_FRZ_RIVR,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = WM_HS_LAKE_SHORE | WM_HS_RIVER,
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = -1,
    .precipitation_compatibility = -1,
    .temperature_compatibility = WM_TC_TUNDRA
  },
  { // WM_BC_CLD_RIVR,
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
  { // WM_BC_TMP_RIVR,
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
  { // WM_BC_WRM_RIVR,
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
  { // WM_BC_TRP_RIVR,
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

  { // WM_BC_FRZ_ALPN,
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
  { // WM_BC_CLD_ALPN,
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
  { // WM_BC_TMP_WET_ALPN,
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
  { // WM_BC_TMP_DRY_ALPN,
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
  { // WM_BC_WRM_WET_ALPN,
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
  { // WM_BC_WRM_DRY_ALPN,
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
  { // WM_BC_TRP_WET_ALPN,
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
  { // WM_BC_TRP_DRY_ALPN,
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

  { // WM_BC_FRZ_DSRT,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = WM_PC_DESERT | WM_PC_ARID,
    .temperature_compatibility = WM_TC_ARCTIC | WM_TC_TUNDRA
  },
  { // WM_BC_CLD_DSRT,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TMP_DSRT,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_WRM_DSRT,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_HOT_DSRT,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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

  { // WM_BC_CLD_GSLD,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TMP_GSLD,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_WRM_GSLD,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TRP_GSLD,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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

  { // WM_BC_CLD_SBLD,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TMP_SBLD,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_WRM_SBLD,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TRP_SBLD,
    .max_size = EC_BIOME_SMALL_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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

  { // WM_BC_TMP_SVNA,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_WRM_SVNA,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TRP_SVNA,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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

  { // WM_BC_CLD_CNF_FRST,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TMP_CNF_FRST,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_WRM_CNF_FRST,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TRP_CNF_FRST,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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

  { // WM_BC_TMP_BDL_FRST,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_WRM_WET_BDL_FRST,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_WRM_DRY_BDL_FRST,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TRP_WET_BDL_FRST,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TRP_DRY_BDL_FRST,
    .max_size = EC_BIOME_HUGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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

  { // WM_BC_TND,
    .max_size = EC_BIOME_GIGANTIC_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = -1,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
    | WM_AC_COASTAL_PLAINS
    | WM_AC_INLAND_HILLS
    | WM_AC_HIGHLANDS
    | WM_AC_MOUNTAIN_SLOPES
    ),
    .precipitation_compatibility = -1,
    .temperature_compatibility = WM_TC_TUNDRA
  },
  { // WM_BC_CLD_FW_WTLD,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_CLD_SW_WTLD,
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
      WM_AC_CONT_SHELF
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
  { // WM_BC_TMP_FW_WTLD,
    .max_size = EC_BIOME_MEDIUM_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TMP_SW_WTLD,
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
      WM_AC_CONT_SHELF
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
  { // WM_BC_WRM_FW_WTLD,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_WRM_FW_FRST_WTLD,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_WRM_SW_WTLD,
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
      WM_AC_CONT_SHELF
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
  { // WM_BC_TRP_FW_WTLD,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TRP_FW_FRST_WTLD,
    .max_size = EC_BIOME_LARGE_SIZE,
    .hydro_state_compatibility = (
      WM_HS_LAND
    | WM_HS_LAKE_SHORE
    | WM_HS_OCEAN_SHORE
    ),
    .salinity_compatibility = WM_SL_FRESH,
    .altitude_compatibility = (
      WM_AC_CONT_SHELF
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
  { // WM_BC_TRP_SW_WTLD,
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
      WM_AC_CONT_SHELF
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
  { // WM_BC_TRP_SW_FRST_WTLD
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
      WM_AC_CONT_SHELF
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

static rngtable DEEP_OCN_WM_BCEGORIES = {
  .size = 2,
  .values = (void*[]) {
    (void*) WM_BC_DEEP_AQ,
    (void*) WM_BC_OCN_VNTS
  },
  .weights = (float[]) { 0.9, 0.1 }
};

static rngtable PEL_WM_BCEGORIES = {
  .size = 3,
  .values = (void*[]) {
    (void*) WM_BC_SEA_ICE,
    (void*) WM_BC_TMP_PEL,
    (void*) WM_BC_TRP_PEL
  },
  .weights = (float[]) { 1, 1, 1 }
};

static rngtable OFS_WM_BCEGORIES = {
  .size = 8,
  .values = (void*[]) {
    (void*) WM_BC_TMP_OFSH,
    (void*) WM_BC_TRP_OFSH,
    (void*) WM_BC_TMP_AQ_GSLD,
    (void*) WM_BC_TRP_AQ_GSLD,
    (void*) WM_BC_TMP_AQ_FRST,
    (void*) WM_BC_TRP_AQ_FRST,
    (void*) WM_BC_CLD_REEF,
    (void*) WM_BC_WRM_REEF
  },
  .weights = (float[]) { 1, 0.8, 0.3, 0.3, 0.4, 0.3, 0.4, 0.2 }
};

static rngtable SHOR_WM_BCEGORIES = {
  .size = 4,
  .values = (void*[]) {
    (void*) WM_BC_FRZ_SHOR,
    (void*) WM_BC_CLD_SHOR,
    (void*) WM_BC_WRM_SHOR,
    (void*) WM_BC_TRP_SHOR
  },
  .weights = (float[]) { 1, 1.2, 1.2, 1 }
};

static rngtable LAKE_WM_BCEGORIES = {
  .size = 6,
  .values = (void*[]) {
    (void*) WM_BC_FRZ_LAKE,
    (void*) WM_BC_CLD_LAKE,
    (void*) WM_BC_TMP_LAKE,
    (void*) WM_BC_WRM_LAKE,
    (void*) WM_BC_TRP_LAKE,
    (void*) WM_BC_SALT_LAKE
  },
  .weights = (float[]) { 0.8, 1, 1, 1, 1, 0.05 }
};

static rngtable RIVR_WM_BCEGORIES = {
  .size = 5,
  .values = (void*[]) {
    (void*) WM_BC_FRZ_RIVR,
    (void*) WM_BC_CLD_RIVR,
    (void*) WM_BC_TMP_RIVR,
    (void*) WM_BC_WRM_RIVR,
    (void*) WM_BC_TRP_RIVR
  },
  .weights = (float[]) { 0.8, 1, 1, 1, 1 }
};

static rngtable ALPN_WM_BCEGORIES = {
  .size = 8,
  .values = (void*[]) {
    (void*) WM_BC_FRZ_ALPN,
    (void*) WM_BC_CLD_ALPN,
    (void*) WM_BC_TMP_WET_ALPN,
    (void*) WM_BC_TMP_DRY_ALPN,
    (void*) WM_BC_WRM_WET_ALPN,
    (void*) WM_BC_WRM_DRY_ALPN,
    (void*) WM_BC_TRP_WET_ALPN,
    (void*) WM_BC_TRP_DRY_ALPN
  },
  .weights = (float[]) { 0.8, 1, 1, 0.9, 1, 0.9, 1, 0.9 }
};

static rngtable TERRESTRIAL_WM_BCEGORIES = {
  .size = 37,
  .values = (void*[]) {
    (void*) WM_BC_FRZ_DSRT,
    (void*) WM_BC_CLD_DSRT,
    (void*) WM_BC_TMP_DSRT,
    (void*) WM_BC_WRM_DSRT,
    (void*) WM_BC_HOT_DSRT,

    (void*) WM_BC_CLD_GSLD,
    (void*) WM_BC_TMP_GSLD,
    (void*) WM_BC_WRM_GSLD,
    (void*) WM_BC_TRP_GSLD,

    (void*) WM_BC_CLD_SBLD,
    (void*) WM_BC_TMP_SBLD,
    (void*) WM_BC_WRM_SBLD,
    (void*) WM_BC_TRP_SBLD,

    (void*) WM_BC_TMP_SVNA,
    (void*) WM_BC_WRM_SVNA,
    (void*) WM_BC_TRP_SVNA,

    (void*) WM_BC_CLD_CNF_FRST,
    (void*) WM_BC_TMP_CNF_FRST,
    (void*) WM_BC_WRM_CNF_FRST,
    (void*) WM_BC_TRP_CNF_FRST,

    (void*) WM_BC_TMP_BDL_FRST,
    (void*) WM_BC_WRM_WET_BDL_FRST,
    (void*) WM_BC_WRM_DRY_BDL_FRST,
    (void*) WM_BC_TRP_WET_BDL_FRST,
    (void*) WM_BC_TRP_DRY_BDL_FRST,

    (void*) WM_BC_TND,
    (void*) WM_BC_CLD_FW_WTLD,
    (void*) WM_BC_CLD_SW_WTLD,
    (void*) WM_BC_TMP_FW_WTLD,
    (void*) WM_BC_TMP_SW_WTLD,
    (void*) WM_BC_WRM_FW_WTLD,
    (void*) WM_BC_WRM_FW_FRST_WTLD,
    (void*) WM_BC_WRM_SW_WTLD,
    (void*) WM_BC_TRP_FW_WTLD,
    (void*) WM_BC_TRP_FW_FRST_WTLD,
    (void*) WM_BC_TRP_SW_WTLD,
    (void*) WM_BC_TRP_SW_FRST_WTLD
  },
  .weights = (float[]) {
    0.2, // WM_BC_FRZ_DSRT,
    0.2, // WM_BC_CLD_DSRT,
    0.2, // WM_BC_TMP_DSRT,
    0.25, // WM_BC_WRM_DSRT,
    0.25, // WM_BC_HOT_DSRT,

    0.8, // WM_BC_CLD_GSLD,
    0.9, // WM_BC_TMP_GSLD,
    0.9, // WM_BC_WRM_GSLD,
    0.7, // WM_BC_TRP_GSLD,

    0.4, // WM_BC_CLD_SBLD,
    0.5, // WM_BC_TMP_SBLD,
    0.5, // WM_BC_WRM_SBLD,
    0.4, // WM_BC_TRP_SBLD,

    0.4, // WM_BC_TMP_SVNA,
    0.3, // WM_BC_WRM_SVNA,
    0.3, // WM_BC_TRP_SVNA,

    0.8, // WM_BC_CLD_CNF_FRST,
    0.8, // WM_BC_TMP_CNF_FRST,
    0.4, // WM_BC_WRM_CNF_FRST,
    0.4, // WM_BC_TRP_CNF_FRST,

    1.0, // WM_BC_TMP_BDL_FRST,
    0.9, // WM_BC_WRM_WET_BDL_FRST,
    0.8, // WM_BC_WRM_DRY_BDL_FRST,
    1.0, // WM_BC_TRP_WET_BDL_FRST,
    0.8, // WM_BC_TRP_DRY_BDL_FRST,

    0.3,  // WM_BC_TND, (often forced)
    0.15,  // WM_BC_CLD_FW_WTLD,
    0.05, // WM_BC_CLD_SW_WTLD,
    0.15,  // WM_BC_TMP_FW_WTLD,
    0.05, // WM_BC_TMP_SW_WTLD,
    0.15,  // WM_BC_WRM_FW_WTLD,
    0.05, // WM_BC_WRM_FW_FRST_WTLD,
    0.15,  // WM_BC_WRM_SW_WTLD,
    0.15,  // WM_BC_TRP_FW_WTLD,
    0.15,  // WM_BC_TRP_FW_FRST_WTLD,
    0.05, // WM_BC_TRP_SW_WTLD,
    0.05  // WM_BC_TRP_SW_FRST_WTLD
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
    &DEEP_OCN_WM_BCEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions( // pelagic
    wm,
    &PEL_WM_BCEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &OFS_WM_BCEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &SHOR_WM_BCEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &LAKE_WM_BCEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &RIVR_WM_BCEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &ALPN_WM_BCEGORIES,
    &needs_biome_in_table,
    &_fill_biome_from_table,
    seed
  );
  seed = prng(seed);

  fill_with_regions(
    wm,
    &TERRESTRIAL_WM_BCEGORIES,
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
    case WM_BC_UNK:
      // No initialization
#ifdef DEBUG
      printf(
        "Error: Attempt to initialize a biome with an unknown category!\n"
      );
      exit(1);
#endif
      break;
    case WM_BC_DEEP_AQ:
      init_deep_aquatic_biome(b, wr);
      break;
    case WM_BC_OCN_VNTS:
      init_ocean_vents_biome(b, wr);
      break;

    case WM_BC_SEA_ICE:
      init_sea_ice_biome(b, wr);
      break;
    case WM_BC_TMP_PEL:
      init_temperate_pelagic_biome(b, wr);
      break;
    case WM_BC_TRP_PEL:
      init_tropical_pelagic_biome(b, wr);
      break;

    case WM_BC_TMP_OFSH:
      init_temperate_offshore_biome(b, wr);
      break;
    case WM_BC_TRP_OFSH:
      init_tropical_offshore_biome(b, wr);
      break;
    case WM_BC_TMP_AQ_GSLD:
      init_temperate_aquatic_grassland_biome(b, wr);
      break;
    case WM_BC_TRP_AQ_GSLD:
      init_tropical_aquatic_grassland_biome(b, wr);
      break;
    case WM_BC_TMP_AQ_FRST:
      init_temperate_aquatic_forest_biome(b, wr);
      break;
    case WM_BC_TRP_AQ_FRST:
      init_tropical_aquatic_forest_biome(b, wr);
      break;
    case WM_BC_CLD_REEF:
      init_cold_reef_biome(b, wr);
      break;
    case WM_BC_WRM_REEF:
      init_warm_reef_biome(b, wr);
      break;

    case WM_BC_FRZ_SHOR:
      init_frozen_beach_biome(b, wr);
      break;
    case WM_BC_CLD_SHOR:
      init_cold_beach_biome(b, wr);
      break;
    case WM_BC_WRM_SHOR:
      init_warm_beach_biome(b, wr);
      break;
    case WM_BC_TRP_SHOR:
      init_tropical_beach_biome(b, wr);
      break;

    case WM_BC_FRZ_LAKE:
      init_frozen_lake_biome(b, wr);
      break;
    case WM_BC_CLD_LAKE:
      init_cold_lake_biome(b, wr);
      break;
    case WM_BC_TMP_LAKE:
      init_temperate_lake_biome(b, wr);
      break;
    case WM_BC_WRM_LAKE:
      init_warm_lake_biome(b, wr);
      break;
    case WM_BC_TRP_LAKE:
      init_tropical_lake_biome(b, wr);
      break;
    case WM_BC_SALT_LAKE:
      init_salt_lake_biome(b, wr);
      break;

    case WM_BC_CLD_RIVR:
      init_cold_river_biome(b, wr);
      break;
    case WM_BC_TMP_RIVR:
      init_temperate_river_biome(b, wr);
      break;
    case WM_BC_WRM_RIVR:
      init_warm_river_biome(b, wr);
      break;
    case WM_BC_TRP_RIVR:
      init_tropical_river_biome(b, wr);
      break;

    case WM_BC_FRZ_ALPN:
      init_frozen_alpine_biome(b, wr);
      break;
    case WM_BC_CLD_ALPN:
      init_cold_alpine_biome(b, wr);
      break;
    case WM_BC_TMP_WET_ALPN:
      init_temperate_wet_alpine_biome(b, wr);
      break;
    case WM_BC_TMP_DRY_ALPN:
      init_temperate_dry_alpine_biome(b, wr);
      break;
    case WM_BC_WRM_WET_ALPN:
      init_warm_wet_alpine_biome(b, wr);
      break;
    case WM_BC_WRM_DRY_ALPN:
      init_warm_dry_alpine_biome(b, wr);
      break;
    case WM_BC_TRP_WET_ALPN:
      init_tropical_wet_alpine_biome(b, wr);
      break;
    case WM_BC_TRP_DRY_ALPN:
      init_tropical_dry_alpine_biome(b, wr);
      break;

    case WM_BC_FRZ_DSRT:
      init_frozen_desert_biome(b, wr);
      break;
    case WM_BC_CLD_DSRT:
      init_cold_desert_biome(b, wr);
      break;
    case WM_BC_TMP_DSRT:
      init_temperate_desert_biome(b, wr);
      break;
    case WM_BC_WRM_DSRT:
      init_warm_desert_biome(b, wr);
      break;
    case WM_BC_HOT_DSRT:
      init_hot_desert_biome(b, wr);
      break;

    case WM_BC_CLD_GSLD:
      init_cold_grassland_biome(b, wr);
      break;
    case WM_BC_TMP_GSLD:
      init_temperate_grassland_biome(b, wr);
      break;
    case WM_BC_WRM_GSLD:
      init_warm_grassland_biome(b, wr);
      break;
    case WM_BC_TRP_GSLD:
      init_tropical_grassland_biome(b, wr);
      break;
    case WM_BC_CLD_SBLD:
      init_temperate_shrubland_biome(b, wr);
      break;
    case WM_BC_WRM_SBLD:
      init_warm_shrubland_biome(b, wr);
      break;
    case WM_BC_TRP_SBLD:
      init_tropical_shrubland_biome(b, wr);
      break;

    case WM_BC_TMP_SVNA:
      init_temperate_savanna_biome(b, wr);
      break;
    case WM_BC_WRM_SVNA:
      init_warm_savanna_biome(b, wr);
      break;
    case WM_BC_TRP_SVNA:
      init_tropical_savanna_biome(b, wr);
      break;

    case WM_BC_CLD_CNF_FRST:
      init_cold_coniferous_forest_biome(b, wr);
      break;
    case WM_BC_TMP_CNF_FRST:
      init_temperate_coniferous_forest_biome(b, wr);
      break;
    case WM_BC_WRM_CNF_FRST:
      init_warm_coniferous_forest_biome(b, wr);
      break;
    case WM_BC_TRP_CNF_FRST:
      init_tropical_coniferous_forest_biome(b, wr);
      break;

    case WM_BC_TMP_BDL_FRST:
      init_warm_wet_broadleaf_forest_biome(b, wr);
      break;
    case WM_BC_WRM_DRY_BDL_FRST:
      init_warm_dry_broadleaf_forest_biome(b, wr);
      break;
    case WM_BC_TRP_WET_BDL_FRST:
      init_tropical_wet_broadleaf_forest_biome(b, wr);
      break;
    case WM_BC_TRP_DRY_BDL_FRST:
      init_tropical_dry_broadleaf_forest_biome(b, wr);
      break;

    case WM_BC_TND:
      init_tundra_biome(b, wr);
      break;
    case WM_BC_CLD_FW_WTLD:
      init_cold_freshwater_wetland_biome(b, wr);
      break;
    case WM_BC_CLD_SW_WTLD:
      init_cold_saltwater_wetland_biome(b, wr);
      break;
    case WM_BC_TMP_FW_WTLD:
      init_temperate_freshwater_wetland_biome(b, wr);
      break;
    case WM_BC_TMP_SW_WTLD:
      init_temperate_saltwater_wetland_biome(b, wr);
      break;
    case WM_BC_WRM_FW_WTLD:
      init_warm_freshwater_wetland_biome(b, wr);
      break;
    case WM_BC_WRM_FW_FRST_WTLD:
      init_warm_freshwater_forested_wetland_biome(b, wr);
      break;
    case WM_BC_WRM_SW_WTLD:
      init_warm_saltwater_wetland_biome(b, wr);
      break;
    case WM_BC_TRP_FW_WTLD:
      init_tropical_freshwater_wetland_biome(b, wr);
      break;
    case WM_BC_TRP_FW_FRST_WTLD:
      init_tropical_freshwater_forested_wetland_biome(b, wr);
      break;
    case WM_BC_TRP_SW_WTLD:
      init_tropical_saltwater_wetland_biome(b, wr);
      break;
    case WM_BC_TRP_SW_FRST_WTLD:
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

