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
//  11: Grows (will be processed by the ecology subsystem)

block_info const BLOCK_INFO[TOTAL_BLOCK_TYPES] = {
//  VOID:             BOUNDARY:         AIR:              ETHER:
    0x00000009,       0x00000000,       0x00000009,       0x00000009,
//  BLACKDAMP:        WHITEDAMP:        FIREDAMP:         STINKDAMP:
    0x00000009,       0x00000009,       0x00000009,       0x00000009,
//  ____:             ____:             WATER:            WATER_FLOW:
    0x00000117,       0x00000117,       0x00000117,       0x00000117,
//  SLIME:            SLIME_FLOW:       ACID:             ACID_FLOW:
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
//  GRAVEL:           SCREE:            STONE:            NATIVE_METAL:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x03f
//  MUSHROOM_SPORES:  MUSHROOM_SHOOTS:  MUSHROOM:         GIANT_MSH_MYCELIUM
    0x000004f9,       0x0000044a,       0x0000044a,       0x0000046e,
//  GIANT_MSH_SPROUT  GIANT_MSH_STALK:  GIANT_MSH_CAP:    ____:
    0x0000044e,       0x00000700,       0x00000700,       0x00000000,
//  MOSS_SPORES:      MOSS_SHOOTS:      MOSS:             MOSS_FLOWERS:
    0x000000f9,       0x0000005a,       0x0000005a,       0x0000005a,
//  MOSS_FRUIT:       ____:              ____:            ____:
    0x0000045a,       0x00000000,       0x00000000,       0x00000000, // 0x04f
//  GRASS_SEEDS:      GRASS_ROOTS:      GRASS_SHOOTS:     GRASS:
    0x0000046a,       0x0000046e,       0x0000043a,       0x0000043a,
//  GRASS_BUDS:       GRASS_FLOWERS:    GRASS_FRUIT:      ____:
    0x0000043a,       0x0000043a,       0x0000043a,       0x00000000,
//  VINE_SEEDS:       VINE_ROOTS:       VINE_SHOOTS:      SPROUTING_VINE:
    0x0000046a,       0x0000046e,       0x0000065e,       0x0000065e,
//  VINE:             VINE_BUDS:        VINE_FLOWERS:     VINE_FRUIT:
    0x0000065e,       0x0000065e,       0x0000065e,       0x0000065e, // 0x05f
//  SHEDDING_VINE:    DORMANT_VINE:     ____:             ____:
    0x0000065e,       0x0000065e,       0x00000000,       0x00000000,
//  HERB_SEEDS:       HERB_ROOTS:       HERB_SHOOTS:      HERB:
    0x0000046a,       0x0000046e,       0x0000044e,       0x0000044e,
//  HERB_BUDS:        HERB_FLOWERS:     HERB_FRUIT:       ____: 
    0x0000044e,       0x0000044e,       0x0000044e,       0x00000000,
//  BUSH_SEEDS:       BUSH_ROOTS:       BUSH_SHOOTS:      SPR_BUSH_BRANCHES:
    0x0000046a,       0x0000046e,       0x0000044e,       0x0000040e, // 0x06f
//  BUSH_BRANCHES:    BUD_BUSH_BRNCHES: FLR_BUSH_BRNCHES: FRT_BUSH_BRNCHES:
    0x0000040e,       0x0000040e,       0x0000040e,       0x0000040e,
//  SHD_BUSH_BRNCHES: DMT_BUSH_BRNCHES: SPR_BUSH_LEAVES:  BUSH_LEAVES:   
    0x0000040e,       0x0000040e,       0x0000040e,       0x0000040e,
//  BUD_BUSH_LEAVES:  FLR_BUSH_LEAVES:  FRT_BUSH_LEAVES:  SHD_BUSH_LEAVES:
    0x0000040e,       0x0000040e,       0x0000040e,       0x0000040e,
//  DMT_BUSH_LEAVES:  ____:             ____:             ____:
    0x0000040e,       0x00000000,       0x00000000,       0x00000000, // 0x07f
//  SHRUB_SEEDS:      SHRUB_ROOTS:      SHRUB_THCK_ROOTS: SHRUB_SHOOTS:
    0x0000046a,       0x0000046e,       0x0000046e,       0x0000044e,
//  SPR_SHRUB_BRNCHS: SHRUB_BRANCHES:   BUD_SHRUB_BRNCHS: FLR_SHRUB_BRNCHS:
    0x0000040e,       0x0000040e,       0x0000040e,       0x0000040e,
//  FRT_SHRUB_BRNCHS: SHD_SHRUB_BRNCHS: DMT_SHRUB_BRNCHS: SPR_SHRUB_LEAVES:
    0x0000040e,       0x0000040e,       0x0000040e,       0x0000040e,
//  SHRUB_LEAVES:     BUD_SHRUB_LEAVES: FLR_SHRUB_LEAVES: FRT_SHRUB_LEAVES:
    0x0000040e,       0x0000040e,       0x0000040e,       0x0000040e, // 0x08f
//  SHD_SHRUB_LEAVES: DMT_SHRUB_LEAVES: ____:              ____:
    0x0000040e,       0x0000040e,       0x00000000,       0x00000000,
//  TREE_SEEDS:       TREE_ROOTS:       TREE_THICK_ROOTS: TREE_HEART_ROOTS:
    0x0000046a,       0x0000046e,       0x0000046e,       0x00000400,
//  TREE_SHOOTS:      TREE_TRUNK:       TREE_BARE_BRNCHS: SPR_TREE_BRANCHES:
    0x0000044e,       0x00000700,       0x0000040e,       0x0000040e,
//  TREE_BRANCHES:    BUD_TREE_BRNCHS:  FLR_TREE_BRNCHS:  FRT_TREE_BRNCHS:
    0x0000040e,       0x0000040e,       0x0000040e,       0x0000040e, // 0x09f
//  SHD_TREE_BRNCHS:  DMT_TREE_BRNCHS:  SPR_TREE_LEAVES:  TREE_LEAVES:
    0x0000040e,       0x0000040e,       0x0000040e,       0x0000040e,
//  BUD_TREE_LEAVES:  FLR_TREE_LEAVES:  FRT_TREE_LEAVES:  SHD_TREE_LEAVES:
    0x0000040e,       0x0000040e,       0x0000040e,       0x0000040e,
//  DMT_TREE_LEAVES:  ____:             ____:             ____:
    0x0000040e,       0x00000000,       0x00000000,       0x00000000,
//  AQ_GRASS_SEEDS:   AQ_GRASS_ROOTS:   AQ_GRASS_SHOOTS:  AQ_GRASS:
    0x0000046a,       0x0000046e,       0x0000044e,       0x0000044e, // 0x0af
//  AQ_GRASS_FLOWERS: AQ_GRASS_FRUIT:   ____:             ____:
    0x0000044e,       0x0000044e,       0x00000000,       0x00000000,
//  AQ_PLANT_SEEDS:   AQ_PLANT_ANCHORS: AQ_PLANT_SHOOTS:  AQ_PLANT_STEMS:
    0x0000046a,       0x0000046e,       0x0000044e,       0x0000047e,
//  AQ_PLANT_LEAVES:  AQ_PLANT_FLOWERS: AQ_PLANT_FRUIT:   ____:
    0x0000047e,       0x0000047e,       0x0000047e,       0x00000000,
//  YOUNG_CORAL:      CORAL_BODY:       CORAL_FROND:      ____:
    0x0000044e,       0x00000400,       0x0000044e,       0x00000000, // 0x0bf
//  SMOOTHED_ROCK:    HEWN_ROCK_GRATE:  ____:             ____:
    0x00000000,       0x000002c2,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0cf
//  BALE:             THATCH:           WATTLE:           WOODEN_PLANK:
    0x00000300,       0x00000300,       0x00000292,       0x00000000,
//  WOODEN_BEAM:      WOODEN_PANEL:     WOODEN_PILLAR:    CORDWOOD:
    0x000000a2,       0x000002c0,       0x000003d0,       0x00000300,
//  COB:              RAMMED_EARTH:     STACKED_STONE:    FITTED_STONE:
    0x00000000,       0x00000100,       0x00000100,       0x00000000,
//  MORTARED_STONE:   METAL_BARS:       STONE_POST:       STONE_PILLAR:
    0x00000000,       0x000002c2,       0x000000a2,       0x000003d0, // 0x0df
//  MUD_BRICK:        CLAY_BRICK:       STONE_BRICK:      STONE_TILE:
    0x00000000,       0x00000000,       0x00000000,       0x000002c0,
//  CERAMIC_TILE:     WOODEN_SHINGLE:   WOODEN_GRATE:     STONE_GRATE:
    0x000002c0,       0x000002c0,       0x000002c2,       0x000002c2,
//  METAL_GRATE:      GLASS_BLOCK:      GLASS_PANE:       FRAMED_GLASS:
    0x000002c2,       0x00000003,       0x000002c3,       0x000002c3,
//  METAL_BLOCK:      ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0ef
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  ____:             ____:             ____:             ____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0ff
//  WOODEN_GATE:      METAL_GATE:       WD_PLANK_DOOR:    WOODEN_PANEL_DOOR:
    0x000002b2,       0x000002b2,       0x000003b2,       0x000003b2,
//  STONE_DOOR:       METAL_DOOR:       ____:             ____:
    0x000003b0,       0x000003b0,       0x00000000,       0x00000000,
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
//  PLASTER:          STUCCO:           PAINT:            BANNER:
    0x00000028,       0x00000028,       0x00000028,       0x000002ca,
//  TAPESTRY:         PAINTING:         ENGRAVING:        CARPET:
    0x000002ca,       0x000002c0,       0x0000030a,       0x000002ca,
//  RUG:              CLOTH_MAT:        STEM_MAT:         ____:
    0x000002ca,       0x000002ca,       0x000002ca,       0x00000000,
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
"blackdamp"        ,"whitedamp"        ,"firedamp"         ,"stinkdamp"        ,
"____"             ,"____"             ,"water"            ,"water_flow"       ,
"slime"            ,"slime_flow"       ,"acid"             ,"acid_flow"        ,
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
"gravel"           ,"scree"            ,"stone"            ,"native_metal"     ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x03f                                                   
"mushroom_spores"  ,"mushroom_shoots"  ,"mushroom"         ,"giant_mycelium"   ,
"g_mushroom_sprout","g_mushroom_stalk" ,"g_mushroom_cap"   ,"____"             ,
"moss_spores"      ,"moss_shoots"      ,"moss"             ,"flowering_moss"   ,
"fruiting_moss"    ,"____"             ,"____"             ,"____"             ,
// 0x04f
"grass_seeds"      ,"grass_roots"      ,"grass_shoots"     ,"grass"            ,
"budding_grass"    ,"flowering_grass"  ,"fruiting_grass"   ,"____"             ,
"vine_seeds"       ,"vine_roots"       ,"vine_shoots"      ,"sprouting_vine"   ,
"vine"             ,"budding_vine"     ,"flowering_vine"   ,"fruiting_vine"    ,
// 0x05f                                                   
"shedding_vine"    ,"dormant_vine"     ,"____"             ,"____"             ,
"herb_seeds"       ,"herb_roots"       ,"herb_shoots"      ,"herb"             ,
"budding_herb"     ,"flowering_herb"   ,"fruiting_herb"    ,"____"             ,
"bush_seeds"       ,"bush_roots"       ,"bush_shoots"      ,"spr_bush_branches",
// 0x06f                                                   
"bush_branches"    ,"bud_bush_branches","flr_bush_branches","frt_bush_branches",
"shd_bush_branches","dmt_bush_branches","spr_bush_leaves"  ,"bush_leaves"      ,
"bud_bush_leaves"  ,"flr_bush_leaves"  ,"frt_bush_leaves"  ,"shd_bush_leaves"  ,
"dmt_bush_leaves"  ,"____"             ,"____"             ,"____"             ,
// 0x07f                                                   
"shrub_seeds"      ,"shrub_roots"      ,"thick_shrub_roots","shrub_shoots"     ,
"spr_shrub_brnches","shrub_branches"   ,"bud_shrub_brnches","flr_shrub_brnches",
"frt_shrub_brnches","shd_shrub_branchs","dmt_shrub_brnches","spr_shrub_leaves" ,
"shrub_leaves"     ,"bud_shrub_leaves" ,"flr_shrub_leaves" ,"frt_shrub_leaves" ,
// 0x08f                                                   
"shd_shrub_leaves" ,"dmt_shrub_leaves" ,"____"             ,"____"             ,
"tree_seeds"       ,"tree_roots"       ,"thick_tree_roots" ,"tree_heart_roots" ,
"tree_shoots"      ,"tree_trunk"       ,"tree_bare_brnches","spr_tree_branches",
"tree_branches"    ,"bud_tree_branches","flr_tree_branches","frt_tree_branches",
// 0x09f                                                   
"shd_tree_branches","dmt_tree_branches","spr_tree_leaves"  ,"tree_leaves"      ,
"bud_tree_leaves"  ,"flr_tree_leaves"  ,"frt_tree_leaves"  ,"shd_tree_leaves"  ,
"dmt_tree_leaves"  ,"____"             ,"____"             ,"____"             ,
"aq_grass_seeds"   ,"aq_grass_roots"   ,"aq_grass_shoots"  ,"aquatic_grass"    ,
// 0x0af                                                   
"flr_aq_grass"     ,"frt_aq_grass"     ,"____"             ,"____"             ,
"aq_plant_seeds"   ,"aq_plant_anchors" ,"aq_plant_shoots"  ,"aq_plant_stems"   ,
"aq_plant_leaves"  ,"flr_aq_plant_lvs" ,"frt_aq_plant_lvs" ,"____"             ,
"young_coral_body" ,"coral_body"       ,"coral_frond"      ,"____"             ,
// 0x0bf                                                   
"smoothed_rock"    ,"hewn_rock_grate"  ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x0cf                                                   
"bale"             ,"thatch"           ,"wattle"           ,"wood_plank"       ,
"wood_beam"        ,"wood_pane"        ,"wood_pillar"      ,"cordwood"         ,
"cob"              ,"rammed_earth"     ,"stacked_stone"    ,"fitted_stone"     ,
"mortared_stone"   ,"metal_bars"       ,"stone_post"       ,"stone_pillar"     ,
// 0x0df                                                   
"mud_brick"        ,"clay_brick"       ,"stone_brick"      ,"stone_tile"       ,
"ceramic_tile"     ,"wood_shingle"     ,"wood_grate"       ,"stone_grate"      ,
"metal_grate"      ,"glass_block"      ,"glass_pane"       ,"framed_glass"     ,
"metal_block"      ,"____"             ,"____"             ,"____"             ,
// 0x0ef                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x0ff                                                   
"wood_gate"        ,"metal_gate"       ,"wood_plank_door"  ,"wood_panel_door"  ,
"stone_door"       ,"metal_door"       ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x10f                                                   
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
"____"             ,"____"             ,"____"             ,"____"             ,
// 0x11f                                                   
"plaster"          ,"stucco"           ,"paint"            ,"banner"           ,
"tapestry"         ,"painting"         ,"engraving"        ,"carpet"           ,
"rug"              ,"cloth_mat"        ,"stem_mat"         ,"____"             ,
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
