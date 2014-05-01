// blocks.c
// Defines block types.

#include "blocks.h"

/**********************
 * Constants and Data *
 **********************/

// Bits are:
//  1-2: Visibility (visible, invisible, transparent, translucent)
//  3-4: Substance (solid, liquid, empty, obstructed)
//  5-8: Geometry (full, half, liquid, grass, etc.)
//  9: Anisotropic (different textures for different faces)
//  10: Orientable (can be placed facing multiple directions)

block_info const BLOCK_INFO[TOTAL_BLOCK_TYPES] = {
//  VOID:             BOUNDARY:         AIR:              ETHER:
    0x00000009,       0x00000000,       0x00000009,       0x00000009,
//  WATER:            WATER_FLOW:       SLIME:            SLIME_FLOW:
    0x00000117,       0x00000117,       0x00000117,       0x00000117,
//  ACID:             ACID_FLOW:        ____:             ____:
    0x00000117,       0x00000117,       0x00000117,       0x00000117,
//  ____:             ____:             ____:             ____:
    0x00000117,       0x00000117,       0x00000117,       0x00000117, // 0x00f
//  ____:             ____:             ____:             ____:
    0x00000117,       0x00000117,       0x00000117,       0x00000117,
//  QUICKSAND:        ____:             LAVA:             LAVA_FLOW:
    0x00000115,       0x00000115,       0x00000115,       0x00000115,
//  ____:             ____:             ____:             ____:
    0x00000115,       0x00000115,       0x00000115,       0x00000115,
//  ____:             ____:             ____:             ____:
    0x00000115,       0x00000115,       0x00000115,       0x00000115, // 0x01f
//  SMOKE:            MIASMA:           ____:             ____:
    0x0000000b,       0x0000000b,       0x0000000b,       0x0000000b,
//  ____:             ____:             ____:             ____:
    0x0000000b,       0x0000000b,       0x0000000b,       0x0000000b,
//  BLACK_SMOKE:      ____:             ____:             ____:
    0x00000008,       0x00000008,       0x00000008,       0x00000008,
//  ____:             ____:             ____:             ____:
    0x00000008,       0x00000008,       0x00000008,       0x00000008, // 0x02f
//  DIRT:             MUD:              CLAY:             SAND:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  GRAVEL:           SCREE:            STONE:            METAL_ORE:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  NATIVE_METAL:     ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x03f
//  MUSHROOM:         MUSHROOM_STALK:   MUSHROOM_CAP:     MOSS:
    0x0000004a,       0x00000300,       0x00000300,       0x0000005a,
//  GRASS:            GRASS_ROOTS:      VINE:             VINE_FRUIT:
    0x0000003a,       0x0000006a,       0x0000005e,       0x0000005e,
//  HERB:             HERB_ROOTS:       BUSH_BRANCHES:    BUSH_LEAVES:
    0x0000004a,       0x0000006a,       0x0000000e,       0x0000000e,
//  BUSH_FRUIT:       BUSH_ROOTS:       SHRUB_BRANCHES:   SHRUB_LEAVES:
    0x0000000e,       0x0000006e,       0x00000006,       0x0000000e, // 0x04f
//  SHRUB_FRUIT:      SHRUB_ROOTS:      TREE_BRANCHES:    TREE_INT_BRANCHES:
    0x0000000e,       0x0000006e,       0x00000006,       0x0000000e,
//  TREE_LEAVES:      TREE_FRUIT:       TREE_TRUNK:       TREE_ROOTS:
    0x0000000e,       0x0000000e,       0x00000300,       0x0000006e,
//  TREE_HEART_ROOTS: AQUATIC_GRASS:    AQ_PLANT_LEAVES:  AQ_PLANT_STEMS:
    0x00000006,       0x0000003a,       0x0000000e,       0x0000000e,
//  AQ_PLANT_ANCHORS: CORAL_FROND:      CORAL_BODY:       ____:
    0x00000000,       0x0000000e,       0x00000000,       0x00000000, // 0x05f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x06f
//  SMOOTHED_ROCK:    HEWN_ROCK_STEPS:  SM_ROCK_STEPS:    HEWN_ROCK_GRATE:
    0x00000000,       0x00000270,       0x00000270,       0x000002b2,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x07f
//  BALE:             THATCH:           WATTLE:           WOODEN_PLANK:
    0x00000300,       0x00000300,       0x00000282,       0x00000000,
//  WOODEN_BEAM:      WOODEN_PANEL:     WOODEN_PILLAR:    CORDWOOD:
    0x00000092,       0x000002b0,       0x000003c0,       0x00000300,
//  COB:              RAMMED_EARTH:     STACKED_STONE:    FITTED_STONE:
    0x00000000,       0x00000100,       0x00000100,       0x00000000,
//  MORTARED_STONE:   METAL_BARS:       STONE_POST:       STONE_PILLAR:
    0x00000000,       0x000002b2,       0x00000092,       0x000003c0, // 0x08f
//  MUD_BRICK:        CLAY_BRICK:       STONE_BRICK:      STONE_TILE:
    0x00000000,       0x00000000,       0x00000000,       0x000002b0,
//  CERAMIC_TILE:     WOODEN_SHINGLE:   WOODEN_GRATE:     STONE_GRATE:
    0x000002b0,       0x000002b0,       0x000002b2,       0x000002b2,
//  METAL_GRATE:      GLASS_BLOCK:      GLASS_PANE:       FRAMED_GLASS:
    0x000002b2,       0x00000003,       0x000002b3,       0x000002b3,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x09f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0af
//  WOODEN_GATE:      METAL_GATE:       WD_PLANK_DOOR:    WOODEN_PANEL_DOOR:
    0x000002a2,       0x000002a2,       0x000003a2,       0x000003a2,
//  STONE_DOOR:       METAL_DOOR:       ____:             ____:
    0x000003a0,       0x000003a0,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0bf
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0cf
//  PLASTER:          STUCCO:           PAINT:            BANNER:
    0x00000028,       0x00000028,       0x00000028,       0x000002ba,
//  TAPESTRY:         PAINTING:         ENGRAVING:        CARPET:
    0x000002ba,       0x000002b0,       0x0000030a,       0x000002ba,
//  RUG:              CLOTH_MAT:        STEM_MAT:         ____:
    0x000002ba,       0x000002ba,       0x000002ba,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0df
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0ef
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0ff
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x10f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x11f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x12f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x13f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x14f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x15f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x16f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x17f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x18f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x19f
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x1af
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x1bf
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x1cf
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x1df
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x1ef
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000 // 0x1ff
};

char const * const BLOCK_NAMES[TOTAL_BLOCK_TYPES] = {
"void"             ,"boundary"         ,"air"              ,"ether"            ,
"water"            ,"water_flow"       ,"slime"            ,"SLIME_FLOW"       ,
"acid"             ,"acid_flow"        ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x00f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"quicksand"        ,"____"             ,"lava"             ,"lava_flow"        ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x01f                                                   
"smoke"            ,"miasma"           ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"black_smoke"      ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x02f                                                   
"dirt"             ,"mud"              ,"clay"             ,"sand"             ,
"gravel"           ,"scree"            ,"stone"            ,"metal_ore"        ,
"native_metal"     ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x03f                                                   
"mushroom"         ,"mushroom_stalk"   ,"mushroom_cap"     ,"moss"             ,
"grass"            ,"grass_roots"      ,"vine"             ,"vine_fruit"       ,
"herb"             ,"herb_roots"       ,"bush_branches"    ,"bush_leaves"      ,
"bush_fruit"       ,"bush_roots"       ,"shrub_branches"   ,"shrub_leaves"     ,
// 0x04f                                                   
"shrub_fruit"      ,"shrub_roots"      ,"tree_branches"    ,"tree_int_branches",
"tree_leaves"      ,"tree_fruit"       ,"tree_trunk"       ,"tree_roots"       ,
"tree_ht_roots"    ,"aquatic_grass"    ,"aq_p_leaves"      ,"aq_p_stems"       ,
"aq_p_anchors"     ,"coral_frond"      ,"coral_body"       ,"____"             ,
// 0x05f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x06f                                                   
"smoothed_rock"    ,"hewn_rock_steps"  ,"sm_rock_steps"    ,"hewn_rock_grate"  ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x07f                                                   
"bale"             ,"thatch"           ,"wattle"           ,"wood_plank"       ,
"wood_beam"        ,"wood_pane"        ,"wood_pillar"      ,"cordwood"         ,
"cob"              ,"rammed_earth"     ,"stacked_stone"    ,"fitted_stone"     ,
"mortared_stone"   ,"metal_bars"       ,"stone_post"       ,"stone_pillar"     ,
// 0x08f                                                   
"mud_brick"        ,"clay_brick"       ,"stone_brick"      ,"stone_tile"       ,
"ceramic_tile"     ,"wood_shingle"     ,"wood_grate"       ,"stone_grate"      ,
"metal_grate"      ,"glass_block"      ,"glass_pane"       ,"framed_glass"     ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x09f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x0af                                                   
"wood_gate"        ,"metal_gate"       ,"wood_plank_door"  ,"wood_panel_door"  ,
"stone_door"       ,"metal_door"       ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x0bf                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x0cf                                                   
"plaster"          ,"stucco"           ,"paint"            ,"banner"           ,
"tapestry"         ,"painting"         ,"engraving"        ,"carpet"           ,
"rug"              ,"cloth_mat"        ,"stem_mat"         ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x0df                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x0ef                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x0ff                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x10f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x11f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x12f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x13f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x14f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x15f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x16f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x17f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x18f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x19f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x1af                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x1bf                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x1cf                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x1df                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x1ef                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"
// 0x1ff
};
