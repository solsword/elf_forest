// blocks.c
// Defines block types.

#include "blocks.h"

#include "species.h"

/**********************
 * Constants and Data *
 **********************/

// Bits are:
//  0-1: Visibility (visible, invisible, transparent, translucent)
//  2-3: Substance (solid, liquid, empty, obstructed)
//
//  4-7: Geometry (full, half, liquid, grass, etc.)
//
//  8: Anisotropic (different textures for different faces)
//  9: Orientable (can be placed facing multiple directions)
//
//  12: Grows (will be processed by the ecology subsystem)
//  13: Growth site (grows by grammar transformations)
//  14: Growth core (grows using growth particles)
//  15: Aquatic (grows in water instead of air)
//
//  16: Seed (generally more transient)
//
//  19-23: Species type (see species_type_e in species.h)

block_info BLOCK_INFO[TOTAL_BLOCK_TYPES];
//  B_MOSS_SHOOTS:      B_MOSS_FLOWERS:     B_MOSS_FRUIT:       B_____:
    0x0000305a,       0x0000305a,       0x0000305a,       0x00000000, // 0x04f
//  B_GRASS_SEEDS:      B_GRASS_ROOTS:      B_GRASS_SHOOTS:     B_GRASS:
    0x0001306a,       0x0000306e,       0x0000303a,       0x0000303a,
//  B_GRASS_BUDS:       B_GRASS_FLOWERS:    B_GRASS_FRUIT:      B_____:
    0x0000303a,       0x0000303a,       0x0000303a,       0x00000000,
//  B_VINE_SEEDS:       B_VINE_CORE:        B_VINE_ROOTS:       B_VINE_SHOOTS:
    0x0001306a,       0x0000506e,       0x0000106e,       0x0000125e,
//  B_SPROUTING_VINE:   B_VINE:             B_VINE_BUDS:        B_VINE_FLOWERS:
    0x0000125e,       0x0000125e,       0x0000125e,       0x0000125e, // 0x05f
//  B_VINE_FRUIT:       B_SHEDDING_VINE:    B_DORMANT_VINE:     B_____:
    0x0000125e,       0x0000125e,       0x0000125e,       0x00000000,
//  B_HERB_SEEDS:       B_HERB_CORE:        B_HERB_ROOTS:       B_HERB_SHOOTS:
    0x0001306a,       0x0000506e,       0x0000106e,       0x0000104e,
//  B_HERB:             B_HERB_BUDS:        B_HERB_FLOWERS:     B_HERB_FRUIT:
    0x0000104e,       0x0000104e,       0x0000104e,       0x0000104e,
//  B_BUSH_SEEDS:       B_BUSH_CORE:        B_BUSH_ROOTS:       B_BUSH_SHOOTS:
    0x0001306a,       0x0000506e,       0x0000106e,       0x0000104e, // 0x06f
//  B_SPR_BUSH_BRNCHES: B_BUSH_BRANCHES:    B_BUD_BUSH_BRNCHES: B_FLR_BUSH_BRNCHES:
    0x0000100e,       0x0000100e,       0x0000100e,       0x0000100e,
//  B_FRT_BUSH_BRNCHES: B_SHD_BUSH_BRNCHES: B_DMT_BUSH_BRNCHES: B_SPR_BUSH_LEAVES:
    0x0000100e,       0x0000100e,       0x0000100e,       0x0000100e,
//  B_BUSH_LEAVES:      B_BUD_BUSH_LEAVES:  B_FLR_BUSH_LEAVES:  B_FRT_BUSH_LEAVES:
    0x0000100e,       0x0000100e,       0x0000100e,       0x0000100e,
//  B_SHD_BUSH_LEAVES:  B_DMT_BUSH_LEAVES:  B_____:             B_____:
    0x0000100e,       0x0000100e,       0x00000000,       0x00000000, // 0x07f
//  B_SHRUB_SEEDS:      B_SHRUB_CORE:       B_SHRUB_ROOTS:      B_SHRUB_THCK_ROOTS:
    0x0001306a,       0x0000506e,       0x0000106e,       0x0000106e,
//  B_SHRUB_SHOOTS:     B_SPR_SHRUB_BRNCHS: B_SHRUB_BRANCHES:   B_BUD_SHRUB_BRNCHS:
    0x0000104e,       0x0000100e,       0x0000100e,       0x0000100e,
//  B_FLR_SHRUB_BRNCHS: B_FRT_SHRUB_BRNCHS: B_SHD_SHRUB_BRNCHS: B_DMT_SHRUB_BRNCHS:
    0x0000100e,       0x0000100e,       0x0000100e,       0x0000100e,
//  B_SPR_SHRUB_LEAVES: B_SHRUB_LEAVES:     B_BUD_SHRUB_LEAVES: B_FLR_SHRUB_LEAVES:
    0x0000100e,       0x0000100e,       0x0000100e,       0x0000100e, // 0x08f
//  B_FRT_SHRUB_LEAVES: B_SHD_SHRUB_LEAVES: B_DMT_SHRUB_LEAVES: B_____:
    0x0000100e,       0x0000100e,       0x0000100e,       0x00000000,
//  B_TREE_SEEDS:       B_TREE_CORE:        B_TREE_THICK_CORE:  B_TREE_ROOTS:
    0x0001306a,       0x0000506e,       0x00005000,       0x0000106e,
//  B_TREE_THICK_ROOTS: B_TREE_HEART_ROOTS: B_TREE_SHOOTS:      B_TREE_TRUNK:
    0x0000106e,       0x00001000,       0x0000104e,       0x00001300,
//  B_TREE_BARE_BRNCHS: B_SPR_TREE_BRNCHES: B_TREE_BRANCHES:    B_BUD_TREE_BRNCHS:
    0x0000100e,       0x0000100e,       0x0000100e,       0x0000100e, // 0x09f
//  B_FLR_TREE_BRNCHS:  B_FRT_TREE_BRNCHS:  B_SHD_TREE_BRNCHS:  B_DMT_TREE_BRNCHS:
    0x0000100e,       0x0000100e,       0x0000100e,       0x0000100e,
//  B_SPR_TREE_LEAVES:  B_TREE_LEAVES:      B_BUD_TREE_LEAVES:  B_FLR_TREE_LEAVES:
    0x0000100e,       0x0000100e,       0x0000100e,       0x0000100e,
//  B_FRT_TREE_LEAVES:  B_SHD_TREE_LEAVES:  B_DMT_TREE_LEAVES:  B_____:
    0x0000100e,       0x0000100e,       0x0000100e,       0x00000000,
//  B_AQ_GRASS_SEEDS:   B_AQ_GRASS_ROOTS:   B_AQ_GRASS_SHOOTS:  B_AQ_GRASS:
    0x0001b06a,       0x0000b06e,       0x0000b04e,       0x0000b04e, // 0x0af
//  B_AQ_GRASS_FLOWERS: B_AQ_GRASS_FRUIT:   B_____:             B_____:
    0x0000b04e,       0x0000b04e,       0x00000000,       0x00000000,
//  B_AQ_PLANT_SEEDS:   B_AQ_PLANT_CORE:    B_AQ_PLANT_ANCHORS: B_AQ_PLANT_SHOOTS:
    0x0001b06a,       0x0000d06e,       0x0000906e,       0x0000904e,
//  B_AQ_PLANT_STEMS:   B_AQ_PLANT_LEAVES:  B_AQ_PLANT_FLOWERS: B_AQ_PLANT_FRUIT:
    0x0000907e,       0x0000907e,       0x0000907e,       0x0000907e,
//  B_YOUNG_CORAL:      B_CORAL_CORE:       B_CORAL_BODY:       B_CORAL_FROND:
    0x0001b04e,       0x0000d000,       0x00009000,       0x0000904e, // 0x0bf
//  B_SMOOTHED_ROCK:    B_HEWN_ROCK_GRATE:  B_____:             B_____:
    0x00000000,       0x000002c2,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0cf
//  B_BALE:             B_THATCH:           B_WATTLE:           B_WOODEN_PLANK:
    0x00000300,       0x00000300,       0x00000292,       0x00000000,
//  B_WOODEN_BEAM:      B_WOODEN_PANEL:     B_WOODEN_PILLAR:    B_CORDWOOD:
    0x000000a2,       0x000002c0,       0x000003d0,       0x00000300,
//  B_COB:              B_RAMMED_EARTH:     B_STACKED_STONE:    B_FITTED_STONE:
    0x00000000,       0x00000100,       0x00000100,       0x00000000,
//  B_MORTARED_STONE:   B_METAL_BARS:       B_STONE_POST:       B_STONE_PILLAR:
    0x00000000,       0x000002c2,       0x000000a2,       0x000003d0, // 0x0df
//  B_MUD_BRICK:        B_CLAY_BRICK:       B_STONE_BRICK:      B_STONE_TILE:
    0x00000000,       0x00000000,       0x00000000,       0x000002c0,
//  B_CERAMIC_TILE:     B_WOODEN_SHINGLE:   B_WOODEN_GRATE:     B_STONE_GRATE:
    0x000002c0,       0x000002c0,       0x000002c2,       0x000002c2,
//  B_METAL_GRATE:      B_GLASS_BLOCK:      B_GLASS_PANE:       B_FRAMED_GLASS:
    0x000002c2,       0x00000003,       0x000002c3,       0x000002c3,
//  B_METAL_BLOCK:      B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0ef
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x0ff
//  B_WOODEN_GATE:      B_METAL_GATE:       B_WD_PLANK_DOOR:    B_WOODEN_PANEL_DOOR:
    0x000002b2,       0x000002b2,       0x000003b2,       0x000003b2,
//  B_STONE_DOOR:       B_METAL_DOOR:       B_____:             B_____:
    0x000003b0,       0x000003b0,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x10f
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x11f
//  B_PLASTER:          B_STUCCO:           B_PAINT:            B_BANNER:
    0x00000028,       0x00000028,       0x00000028,       0x000002ca,
//  B_TAPESTRY:         B_PAINTING:         B_ENGRAVING:        B_CARPET:
    0x000002ca,       0x000002c0,       0x0000030a,       0x000002ca,
//  B_RUG:              B_CLOTH_MAT:        B_STEM_MAT:         B_____:
    0x000002ca,       0x000002ca,       0x000002ca,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x12f
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x13f
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x14f
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x15f
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x16f
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x17f
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x18f
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x19f
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x1af
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x1bf
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x1cf
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x1df
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000, // 0x1ef
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000,
//  B_____:             B_____:             B_____:             B_____:
    0x00000000,       0x00000000,       0x00000000,       0x00000000 // 0x1ff
};

char* BLOCK_NAMES[TOTAL_BLOCK_TYPES] = {
"void"             ,"boundary"         ,"air"              ,"ether"            ,
"fumes"            ,"____"             ,"____"             ,"____"             ,
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
"mushroom_spores"  ,"mushroom_shoots"  ,"mushroom"         ,"____"             ,
"g_mushroom_spores","g_mushroom_core"  ,"giant_mycelium"   ,"g_mushroom_sprout",
"g_mushroom_stalk" ,"g_mushroom_cap"   ,"moss_spores"      ,"moss_shoots"      ,
"moss"             ,"flowering_moss"   , "fruiting_moss"    ,"____"            ,
// 0x04f
"grass_seeds"      ,"grass_roots"      ,"grass_shoots"     ,"grass"            ,
"budding_grass"    ,"flowering_grass"  ,"fruiting_grass"   ,"____"             ,
"vine_seeds"       ,"vine_core"        ,"vine_roots"       ,"vine_shoots"      ,
"sprouting_vine"   ,"vine"             ,"budding_vine"     ,"flowering_vine"   ,
// 0x05f                                                   
"fruiting_vine"    ,"shedding_vine"    ,"dormant_vine"     ,"____"             ,
"herb_seeds"       ,"herb_core"        ,"herb_roots"       ,"herb_shoots"      ,
"herb"             ,"budding_herb"     ,"flowering_herb"   ,"fruiting_herb"    ,
"bush_seeds"       ,"bush_core"        ,"bush_roots"       ,"bush_shoots"      ,
// 0x06f                                                   
"spr_bush_branches","bush_branches"    ,"bud_bush_branches","flr_bush_branches",
"frt_bush_branches","shd_bush_branches","dmt_bush_branches","spr_bush_leaves"  ,
"bush_leaves"      ,"bud_bush_leaves"  ,"flr_bush_leaves"  ,"frt_bush_leaves"  ,
"shd_bush_leaves"  ,"dmt_bush_leaves"  ,"____"             ,"____"             ,
// 0x07f                                                   
"shrub_seeds"      ,"shrub_core"       ,"shrub_roots"      ,"thick_shrub_roots",
"shrub_shoots"     ,"spr_shrub_brnches","shrub_branches"   ,"bud_shrub_brnches",
"flr_shrub_brnches","frt_shrub_brnches","shd_shrub_branchs","dmt_shrub_brnches",
"spr_shrub_leaves" ,"shrub_leaves"     ,"bud_shrub_leaves" ,"flr_shrub_leaves" ,
// 0x08f                                                   
"frt_shrub_leaves" ,"shd_shrub_leaves" ,"dmt_shrub_leaves" ,"____"             ,
"tree_seeds"       ,"tree_core"        ,"tree_thick_core"  ,"tree_roots"       ,
"thick_tree_roots" ,"tree_heart_roots" ,"tree_shoots"      ,"tree_trunk"       ,
"tree_bare_brnches","spr_tree_branches","tree_branches"    ,"bud_tree_branches",
// 0x09f                                                   
"flr_tree_branches","frt_tree_branches","shd_tree_branches","dmt_tree_branches",
"spr_tree_leaves"  ,"tree_leaves"      ,"bud_tree_leaves"  ,"flr_tree_leaves"  ,
"frt_tree_leaves"  ,"shd_tree_leaves"  ,"dmt_tree_leaves"  ,"____"             ,
"aq_grass_seeds"   ,"aq_grass_roots"   ,"aq_grass_shoots"  ,"aquatic_grass"    ,
// 0x0af                                                   
"flr_aq_grass"     ,"frt_aq_grass"     ,"____"             ,"____"             ,
"aq_plant_seeds"   ,"aq_plant_core"    ,"aq_plant_anchors" ,"aq_plant_shoots"  ,
"aq_plant_stems"   ,"aq_plant_leaves"  ,"flr_aq_plant_lvs" ,"frt_aq_plant_lvs" ,
"young_coral_body" ,"coral_core"       ,"coral_body"       ,"coral_frond"      ,
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


/*********************
 * Private Functions *
 *********************/

void _bdef(
  block id,
  char* name,
  block_info vis,
  block_info sbst,
  block_info geom,
  species_type spt,
  block_info flags
) {
  BLOCK_NAMES[id] = name;
  BLOCK_INFO[id] = (
    (vis << BIMS_VISIBILITY)
  & (sbst << BIMS_SUBSTANCE)
  & (geom << BIMS_GEOMETRY)
  & (spt << BIMS_SPECIES_TYPE)
  & flags
  );
}

/*************
 * Functions *
 *************/

void init_blocks(void) {
// All flags:
// TODO: Keep this up-to-date
//  BIF_ANISOTROPIC
//  BIF_ORIENTABLE
//  BIF_GROWS
//  BIF_GROWTH_SITE
//  BIF_GROWTH_CORE
//  BIF_AQUATIC
//  BIF_SEED

  _bdef( B_VOID, "void",
    BI_VIS_INVISIBLE,    BI_SBST_EMPTY,       BI_GEOM_EMPTY,   SPT_NO_SPECIES,
    0);

  _bdef( B_BOUNDARY, "boundary",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_NO_SPECIES,
    0);

  _bdef( B_AIR, "air",
    BI_VIS_INVISIBLE,    BI_SBST_EMPTY,       BI_GEOM_EMPTY,   SPT_NO_SPECIES,
    0);
  _bdef( B_ETHER, "ether",
    BI_VIS_INVISIBLE,    BI_SBST_EMPTY,       BI_GEOM_EMPTY,   SPT_NO_SPECIES,
    0);

  _bdef( B_FUMES, "fumes",
    BI_VIS_INVISIBLE,    BI_SBST_EMPTY,       BI_GEOM_EMPTY,   SPT_GAS,
    0);

  _bdef( B_WATER, "water",
    BI_VIS_TRANSLUCENT,  BI_SBST_LIQUID,      BI_GEOM_LIQUID,  SPT_NO_SPECIES,
    BIF_ANISOTROPIC);
  _bdef( B_WATER_FLOW, "water_flow",
    BI_VIS_TRANSLUCENT,  BI_SBST_LIQUID,      BI_GEOM_LIQUID,  SPT_NO_SPECIES,
    BIF_ANISOTROPIC | BIF_ORIENTABLE);

  _bdef( B_SLIME, "slime",
    BI_VIS_TRANSLUCENT,  BI_SBST_LIQUID,      BI_GEOM_LIQUID,  SPT_NO_SPECIES,
    BIF_ANISOTROPIC);
  _bdef( B_SLIME_FLOW, "slime_flow",
    BI_VIS_TRANSLUCENT,  BI_SBST_LIQUID,      BI_GEOM_LIQUID,  SPT_NO_SPECIES,
    BIF_ANISOTROPIC | BIF_ORIENTABLE);

  _bdef( B_ACID, "acid",
    BI_VIS_TRANSLUCENT,  BI_SBST_LIQUID,      BI_GEOM_LIQUID,  SPT_NO_SPECIES,
    BIF_ANISOTROPIC);
  _bdef( B_ACID_FLOW, "acid_flow",
    BI_VIS_TRANSLUCENT,  BI_SBST_LIQUID,      BI_GEOM_LIQUID,  SPT_NO_SPECIES,
    BIF_ANISOTROPIC | BIF_ORIENTABLE);

  _bdef( B_QUICKSAND, "quicksand",
    BI_VIS_OPAQUE,       BI_SBST_LIQUID,      BI_GEOM_LIQUID,  SPT_STONE,
    BIF_ANISOTROPIC);

  _bdef( B_LAVA, "lava",
    BI_VIS_OPAQUE,       BI_SBST_LIQUID,      BI_GEOM_LIQUID,  SPT_STONE,
    BIF_ANISOTROPIC);
  _bdef( B_LAVA_FLOW, "lava_flow",
    BI_VIS_OPAQUE,       BI_SBST_LIQUID,      BI_GEOM_LIQUID,  SPT_STONE,
    BIF_ANISOTROPIC | BIF_ORIENTABLE);

  _bdef( B_SMOKE, "smoke",
    BI_VIS_TRANSLUCENT,  BI_SBST_EMPTY,       BI_GEOM_SOLID,   SPT_GAS,
    0);
  _bdef( B_MIASMA, "miasma",
    BI_VIS_TRANSLUCENT,  BI_SBST_EMPTY,       BI_GEOM_SOLID,   SPT_GAS,
    0);

  _bdef( B_THICK_SMOKE, "thick_smoke",
    BI_VIS_OPAQUE,       BI_SBST_EMPTY,       BI_GEOM_SOLID,   SPT_GAS,
    0);

  _bdef( B_DIRT, "dirt",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_DIRT,
    0);
  _bdef( B_MUD, "mud",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_DIRT,
    0);
  _bdef( B_CLAY, "clay",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_CLAY,
    0);
  _bdef( B_SAND, "sand",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_STONE,
    0);
  _bdef( B_GRAVEL, "gravel",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_STONE,
    0);
  _bdef( B_SCREE, "scree",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_STONE,
    0);
  _bdef( B_STONE, "stone",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_STONE,
    0);
  _bdef( B_NATIVE_METAL, "native_metal",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_METAL,
    0);

  _bdef( B_MUSHROOM_SPORES, "mushroom_spores",
    BI_VIS_INVISIBLE,    BI_SBST_EMPTY,       BI_GEOM_EMPTY,   SPT_FUNGUS,
    BIF_GROWS | BIF_GROWTH_SITE | BIF_SEED);
  _bdef( B_MUSHROOM_SHOOTS, "mushroom_shoots",
    BI_VIS_TRANSPARENT,  BI_SBST_EMPTY,       BI_GEOM_HERB,    SPT_FUNGUS,
    BIF_GROWS | BIF_GROWTH_SITE);
  _bdef( B_MUSHROOM_GROWN, "mushroom",
    BI_VIS_TRANSPARENT,  BI_SBST_EMPTY,       BI_GEOM_HERB,    SPT_FUNGUS,
    BIF_GROWS | BIF_GROWTH_SITE);

  _bdef( B_GIANT_MUSHROOM_SPORES, "giant_mushroom_spores",
    BI_VIS_INVISIBLE,    BI_SBST_EMPTY,       BI_GEOM_EMPTY,   SPT_FUNGUS,
    BIF_GROWS | BIF_GROWTH_SITE | BIF_SEED);
  _bdef( B_GIANT_MUSHROOM_CORE, "giant_mushroom_core",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_ROOT,    SPT_FUNGUS,
    BIF_GROWS | BIF_GROWTH_CORE);
  _bdef( B_GIANT_MUSHROOM_MYCELIUM, "giant_mushroom_mycelium",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_ROOT,    SPT_FUNGUS,
    BIF_GROWS);
  _bdef( B_GIANT_MUSHROOM_SPROUT, "giant_mushroom_sprout",
    BI_VIS_TRANSPARENT,  BI_SBST_EMPTY,       BI_GEOM_HERB,    SPT_FUNGUS,
    BIF_GROWS);
  _bdef( B_GIANT_MUSHROOM_STALK, "giant_mushroom_stalk",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_FUNGUS,
    BIF_GROWS);
  _bdef( B_GIANT_MUSHROOM_CAP, "giant_mushroom_cap",
    BI_VIS_OPAQUE,       BI_SBST_SOLID,       BI_GEOM_SOLID,   SPT_FUNGUS,
    BIF_GROWS);

  _bdef( B_MOSS_SPORES, "moss_spores",
    BI_VIS_INVISIBLE,    BI_SBST_EMPTY,       BI_GEOM_EMPTY,   SPT_MOSS,
    BIF_GROWS | BIF_GROWTH_SITE | BIF_SEED);
  _bdef( B_MOSS_SHOOTS, "moss_shoots",
    BI_VIS_TRANSPARENT,  BI_SBST_EMPTY,       BI_GEOM_VINE,    SPT_MOSS,
    BIF_GROWS | BIF_GROWTH_SITE);
  _bdef( B_MOSS_GROWN, "moss",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_VINE,    SPT_MOSS,
    BIF_GROWS | BIF_GROWTH_SITE);
  _bdef( B_MOSS_FLOWERING, "flowering_moss",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_VINE,    SPT_MOSS,
    BIF_GROWS | BIF_GROWTH_SITE);
  _bdef( B_MOSS_FRUITING, "fruiting_moss",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_VINE,    SPT_MOSS,
    BIF_GROWS | BIF_GROWTH_SITE);

  _bdef( B_GRASS_SEEDS, "grass_seeds",
    BI_VIS_INVISIBLE,    BI_SBST_EMPTY,       BI_GEOM_EMPTY,   SPT_GRASS,
    BIF_GROWS | BIF_GROWTH_SITE | BIF_SEED);
  _bdef( B_GRASS_ROOTS, "grass_roots",
    BI_VIS_TRANSPARENT,  BI_SBST_EMPTY,       BI_GEOM_ROOT,    SPT_GRASS,
    BIF_GROWS | BIF_GROWTH_SITE);
  _bdef( B_GRASS_SHOOTS, "grass_shoots",
    BI_VIS_TRANSPARENT,  BI_SBST_EMPTY,       BI_GEOM_GRASS,   SPT_GRASS,
    BIF_GROWS | BIF_GROWTH_SITE);
  _bdef( B_GRASS_GROWN, "grass",
    BI_VIS_TRANSPARENT,  BI_SBST_EMPTY,       BI_GEOM_GRASS,   SPT_GRASS,
    BIF_GROWS | BIF_GROWTH_SITE);
  _bdef( B_GRASS_FLOWERING, "flowering_grass",
    BI_VIS_TRANSPARENT,  BI_SBST_EMPTY,       BI_GEOM_GRASS,   SPT_GRASS,
    BIF_GROWS | BIF_GROWTH_SITE);
  _bdef( B_GRASS_FRUITING, "fruiting_grass",
    BI_VIS_TRANSPARENT,  BI_SBST_EMPTY,       BI_GEOM_GRASS,   SPT_GRASS,
    BIF_GROWS | BIF_GROWTH_SITE);

  _bdef( B_VINE_SEEDS, "vine_seeds",
    BI_VIS_INVISIBLE,    BI_SBST_EMPTY,       BI_GEOM_EMPTY,   SPT_VINE,
    BIF_GROWS | BIF_GROWTH_SITE | BIF_SEED);
  _bdef( B_VINE_CORE, "vine_core",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_ROOT,    SPT_VINE,
    BIF_GROWS | BIF_GROWTH_CORE);
  _bdef( B_VINE_ROOTS, "vine_roots",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_ROOT,    SPT_VINE,
    BIF_GROWS);
  _bdef( B_VINE_SHOOTS, "vine_shoots",
    BI_VIS_TRANSPARENT,  BI_SBST_EMPTY,       BI_GEOM_VINE,   SPT_VINE,
    BIF_GROWS);
  _bdef( B_VINE_SPROUTING, "vine_sprouting",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_VINE,   SPT_VINE,
    BIF_GROWS);
  _bdef( B_VINE_GROWN, "vine",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_VINE,   SPT_VINE,
    BIF_GROWS);
  _bdef( B_VINE_BUDDING, "budding_vine",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_VINE,   SPT_VINE,
    BIF_GROWS);
  _bdef( B_VINE_FLOWERING, "flowering_vine",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_VINE,   SPT_VINE,
    BIF_GROWS);
  _bdef( B_VINE_FRUITING, "fruiting_vine",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_VINE,   SPT_VINE,
    BIF_GROWS);
  _bdef( B_VINE_SHEDDING, "shedding_vine",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_VINE,   SPT_VINE,
    BIF_GROWS);
  _bdef( B_VINE_DORMANT, "dormant_vine",
    BI_VIS_TRANSPARENT,  BI_SBST_OBSTRUCTED,  BI_GEOM_VINE,   SPT_VINE,
    BIF_GROWS);

#define                        B_HERB_SEEDS 0x064
#define                         B_HERB_CORE 0x065
#define                        B_HERB_ROOTS 0x066
#define                       B_HERB_SHOOTS 0x067
#define                        B_HERB_GROWN 0x068
#define                      B_HERB_BUDDING 0x069
#define                    B_HERB_FLOWERING 0x06a
#define                     B_HERB_FRUITING 0x06b

#define                        B_BUSH_SEEDS 0x06c
#define                         B_BUSH_CORE 0x06d
#define                        B_BUSH_ROOTS 0x06e
#define                       B_BUSH_SHOOTS 0x06f
#define           B_BUSH_BRANCHES_SPROUTING 0x070
#define               B_BUSH_BRANCHES_GROWN 0x071
#define             B_BUSH_BRANCHES_BUDDING 0x072
#define           B_BUSH_BRANCHES_FLOWERING 0x073
#define            B_BUSH_BRANCHES_FRUITING 0x074
#define            B_BUSH_BRANCHES_SHEDDING 0x075
#define             B_BUSH_BRANCHES_DORMANT 0x076
#define             B_BUSH_LEAVES_SPROUTING 0x077
#define                 B_BUSH_LEAVES_GROWN 0x078
#define               B_BUSH_LEAVES_BUDDING 0x079
#define             B_BUSH_LEAVES_FLOWERING 0x07a
#define              B_BUSH_LEAVES_FRUITING 0x07b
#define              B_BUSH_LEAVES_SHEDDING 0x07c
#define               B_BUSH_LEAVES_DORMANT 0x07d

#define                       B_SHRUB_SEEDS 0x080
#define                        B_SHRUB_CORE 0x081
#define                       B_SHRUB_ROOTS 0x082
#define                 B_SHRUB_THICK_ROOTS 0x083
#define                      B_SHRUB_SHOOTS 0x084
#define          B_SHRUB_BRANCHES_SPROUTING 0x085
#define              B_SHRUB_BRANCHES_GROWN 0x086
#define            B_SHRUB_BRANCHES_BUDDING 0x087
#define          B_SHRUB_BRANCHES_FLOWERING 0x088
#define           B_SHRUB_BRANCHES_FRUITING 0x089
#define           B_SHRUB_BRANCHES_SHEDDING 0x08a
#define            B_SHRUB_BRANCHES_DORMANT 0x08b
#define            B_SHRUB_LEAVES_SPROUTING 0x08c
#define                B_SHRUB_LEAVES_GROWN 0x08d
#define              B_SHRUB_LEAVES_BUDDING 0x08e
#define            B_SHRUB_LEAVES_FLOWERING 0x08f
#define             B_SHRUB_LEAVES_FRUITING 0x090
#define             B_SHRUB_LEAVES_SHEDDING 0x091
#define              B_SHRUB_LEAVES_DORMANT 0x092

#define                        B_TREE_SEEDS 0x094
#define                         B_TREE_CORE 0x095
#define                   B_TREE_THICK_CORE 0x096
#define                        B_TREE_ROOTS 0x097
#define                  B_TREE_THICK_ROOTS 0x098
#define                  B_TREE_HEART_ROOTS 0x099
#define                       B_TREE_SHOOTS 0x09a
#define                        B_TREE_TRUNK 0x09b
#define                B_TREE_BARE_BRANCHES 0x09c
#define           B_TREE_BRANCHES_SPROUTING 0x09d
#define               B_TREE_BRANCHES_GROWN 0x09e
#define             B_TREE_BRANCHES_BUDDING 0x09f
#define           B_TREE_BRANCHES_FLOWERING 0x0a0
#define            B_TREE_BRANCHES_FRUITING 0x0a1
#define            B_TREE_BRANCHES_SHEDDING 0x0a2
#define             B_TREE_BRANCHES_DORMANT 0x0a3
#define             B_TREE_LEAVES_SPROUTING 0x0a4
#define                 B_TREE_LEAVES_GROWN 0x0a5
#define               B_TREE_LEAVES_BUDDING 0x0a6
#define             B_TREE_LEAVES_FLOWERING 0x0a7
#define              B_TREE_LEAVES_FRUITING 0x0a8
#define              B_TREE_LEAVES_SHEDDING 0x0a9
#define               B_TREE_LEAVES_DORMANT 0x0aa

#define               B_AQUATIC_GRASS_SEEDS 0x0ac
#define               B_AQUATIC_GRASS_ROOTS 0x0ad
#define              B_AQUATIC_GRASS_SHOOTS 0x0ae
#define               B_AQUATIC_GRASS_GROWN 0x0af
#define           B_AQUATIC_GRASS_FLOWERING 0x0b0
#define            B_AQUATIC_GRASS_FRUITING 0x0b1

#define               B_AQUATIC_PLANT_SEEDS 0x0b4
#define                B_AQUATIC_PLANT_CORE 0x0b5
#define             B_AQUATIC_PLANT_ANCHORS 0x0b6
#define              B_AQUATIC_PLANT_SHOOTS 0x0b7
#define               B_AQUATIC_PLANT_STEMS 0x0b8
#define        B_AQUATIC_PLANT_LEAVES_GROWN 0x0b9
#define    B_AQUATIC_PLANT_LEAVES_FLOWERING 0x0ba
#define     B_AQUATIC_PLANT_LEAVES_FRUITING 0x0bb

#define                       B_YOUNG_CORAL 0x0bc
#define                        B_CORAL_CORE 0x0bd
#define                        B_CORAL_BODY 0x0be
#define                       B_CORAL_FROND 0x0bf

// Hewn Blocks:
#define                     B_SMOOTHED_ROCK 0x0c0
#define                   B_HEWN_ROCK_GRATE 0x0c1

// Construction Materials:
#define                              B_BALE 0x0d0
#define                            B_THATCH 0x0d1
#define                            B_WATTLE 0x0d2

#define                      B_WOODEN_PLANK 0x0d3
#define                       B_WOODEN_BEAM 0x0d4
#define                      B_WOODEN_PANEL 0x0d5
#define                     B_WOODEN_PILLAR 0x0d6

#define                          B_CORDWOOD 0x0d7
#define                               B_COB 0x0d8
#define                      B_RAMMED_EARTH 0x0d9
#define                     B_STACKED_STONE 0x0da
#define                      B_FITTED_STONE 0x0db
#define                    B_MORTARED_STONE 0x0dc
#define                        B_METAL_BARS 0x0dd

#define                        B_STONE_POST 0x0de
#define                      B_STONE_PILLAR 0x0df

#define                         B_MUD_BRICK 0x0e0
#define                        B_CLAY_BRICK 0x0e1
#define                       B_STONE_BRICK 0x0e2

#define                        B_STONE_TILE 0x0e3
#define                      B_CERAMIC_TILE 0x0e4
#define                    B_WOODEN_SHINGLE 0x0e5

#define                      B_WOODEN_GRATE 0x0e6
#define                       B_STONE_GRATE 0x0e7
#define                       B_METAL_GRATE 0x0e8

#define                       B_GLASS_BLOCK 0x0e9
#define                        B_GLASS_PANE 0x0ea
#define                      B_FRAMED_GLASS 0x0eb

#define                       B_METAL_BLOCK 0x0ec

// Interactive blocks:
#define                       B_WOODEN_GATE 0x100
#define                        B_METAL_GATE 0x101
#define                 B_WOODEN_PLANK_DOOR 0x102
#define                 B_WOODEN_PANEL_DOOR 0x103
#define                        B_STONE_DOOR 0x104
#define                        B_METAL_DOOR 0x105

// Decorative blocks:
#define                           B_PLASTER 0x120
#define                            B_STUCCO 0x121
#define                             B_PAINT 0x122

#define                            B_BANNER 0x123
#define                          B_TAPESTRY 0x124
#define                          B_PAINTING 0x125
#define                         B_ENGRAVING 0x126

#define                            B_CARPET 0x127
#define                               B_RUG 0x128
#define                         B_CLOTH_MAT 0x129
#define                          B_STEM_MAT 0x12a
}
