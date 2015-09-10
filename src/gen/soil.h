#ifndef SOIL_H
#define SOIL_H

// soil.h
// Soil generation.

#include "world/world_map.h"
#include "world/species.h"

/*************
 * Constants *
 *************/

// Layout of block_data for soils:
// All soils:
#define              SL_BS_HYDRATION 0

// Fertile soils:
#define               SL_BS_PH_SHIFT 2
#define         SL_BS_NITROGEN_LEVEL 4
#define   SL_BS_PHOSPHORUS_DEFICIENT 6
#define    SL_BS_POTASSIUM_DEFICIENT 7

// TODO: Any way to buy more space for this stuff?

// Post-shift masks:
#define              SL_BM_HYDRATION 0x3
#define               SL_BM_PH_SHIFT 0x3
#define         SL_BM_NITROGEN_LEVEL 0x3
#define   SL_BM_PHOSPHORUS_DEFICIENT 0x1
#define    SL_BM_POTASSIUM_DEFICIENT 0x1

// Values:
#define         SL_HYDRATION_DROUGHT 0
#define             SL_HYDRATION_DRY 1
#define             SL_HYDRATION_WET 2
#define         SL_HYDRATION_FLOODED 3

#define      SL_PH_SHIFT_VERY_ACIDIC 0
#define           SL_PH_SHIFT_ACIDIC 1
#define          SL_PH_SHIFT_NEUTRAL 2
#define            SL_PH_SHIFT_BASIC 3

#define   SL_NITROGEN_LEVEL_VERY_LOW 0
#define        SL_NITROGEN_LEVEL_LOW 1
#define     SL_NITROGEN_LEVEL_NORMAL 2
#define       SL_NITROGEN_LEVEL_HIGH 3



/*************
 * Functions *
 *************/

// Generates soil information for the given world map.
void generate_soil(world_map *wm);

#endif // ifndef SOIL_H
