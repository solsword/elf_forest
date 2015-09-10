#ifndef BLOCKS_H
#define BLOCKS_H

// blocks.h
// Defines block types.

#include <stdint.h>

#include "util.h"

/************
 * Typedefs *
 ************/

// 9 bits of block ID.
// 12 bits of block species.
// 8 bits of block data.
// 3 bits of block orientation.
typedef uint32_t block;

// Used for storing species info separately.
typedef uint16_t species;

// Used for storing block data separately. Meaning depends on block ID.
// Layouts for various block IDs can be found in the following places:
//   Growing blocks   -   ecology/grow.h
//   Soil blocks      -   gen/soil.h
// For flowing liquids:
//   2 bits of level.
//   3 bits of flow direction.
//   3 bits of flow rate.
typedef uint8_t block_data;

// Extra static block data and flags stored in the BLOCK_INFO table.
typedef uint32_t block_info;

// A cell holds two blocks:
struct cell_s;
typedef struct cell_s cell;

/*************************
 * Structure Definitions *
 *************************/

struct cell_s {
  block blocks[2];
};

/**********************
 * Constants and Data *
 **********************/

#define BLOCK_ID_BITS 9
#define BLOCK_SPC_BITS 12
#define BLOCK_DAT_BITS 8
#define BLOCK_ORI_BITS 3

#define BS_ID (BLOCK_SPC_BITS + BLOCK_DAT_BITS + BLOCK_ORI_BITS)
#define BS_SPC (BLOCK_DAT_BITS + BLOCK_ORI_BITS)
#define BS_DAT BLOCK_ORI_BITS
#define BS_ORI 0

#define TOTAL_BLOCK_TYPES (1 << BLOCK_ID_BITS)
#define TOTAL_BLOCK_SPECIES (1 << (BLOCK_ID_BITS + BLOCK_SPC_BITS))
#define SPECIES_PER_BLOCK (1 << BLOCK_SPC_BITS)

extern block_info BLOCK_INFO[TOTAL_BLOCK_TYPES];

#define MAX_BLOCK_NAME_LENGTH 128

extern char* BLOCK_NAMES[TOTAL_BLOCK_TYPES];

/*******************
 * Flags and Masks *
 *******************/

// The ability to turn a read into a 1 or 0 (instead of an N or 0) is useful
// sometimes. Shifts tell you how many bits to shift (and simultaneously are
// used to define flags).

// Exposure Flags:
// ---------------

// Note that block exposure isn't stored anywhere but is recomputed on the fly.

#define    BFS_EXPOSED_ABOVE_SHIFT 0
#define    BFS_EXPOSED_BELOW_SHIFT 1
#define    BFS_EXPOSED_NORTH_SHIFT 2
#define    BFS_EXPOSED_SOUTH_SHIFT 3
#define     BFS_EXPOSED_EAST_SHIFT 4
#define     BFS_EXPOSED_WEST_SHIFT 5

static block const  BF_EXPOSED_ABOVE = 1 << BFS_EXPOSED_ABOVE_SHIFT;
static block const  BF_EXPOSED_BELOW = 1 << BFS_EXPOSED_BELOW_SHIFT;
static block const  BF_EXPOSED_NORTH = 1 << BFS_EXPOSED_NORTH_SHIFT;
static block const  BF_EXPOSED_SOUTH = 1 << BFS_EXPOSED_SOUTH_SHIFT;
static block const   BF_EXPOSED_EAST = 1 << BFS_EXPOSED_EAST_SHIFT;
static block const   BF_EXPOSED_WEST = 1 << BFS_EXPOSED_WEST_SHIFT;

#define BF_EXPOSED_ANY (BF_EXPOSED_ABOVE | BF_EXPOSED_BELOW \
                      | BF_EXPOSED_NORTH | BF_EXPOSED_SOUTH \
                      | BF_EXPOSED_EAST | BF_EXPOSED_WEST)

// Masks:
// ------

static block const BM_ORIENTATION = ((1 << BLOCK_ORI_BITS) - 1) << BS_ORI;
static block const        BM_DATA = ((1 << BLOCK_DAT_BITS) - 1) << BS_DAT;
static block const     BM_SPECIES = ((1 << BLOCK_SPC_BITS) - 1) << BS_SPC;
static block const          BM_ID = ((1 << BLOCK_ID_BITS ) - 1) << BS_ID;
static block const  BM_FULL_BLOCK = umaxof(block);

// Info Masks:
// -----------

#define    BIMS_VISIBILITY 0x0
#define     BIMS_SUBSTANCE 0x2
#define      BIMS_GEOMETRY 0x4
#define  BIMS_SPECIES_TYPE 0x14

static block_info const    BIM_VISIBILITY = 0x03 << BIMS_VISIBILITY;
static block_info const     BIM_SUBSTANCE = 0x03 << BIMS_SUBSTANCE;
static block_info const      BIM_GEOMETRY = 0x0f << BIMS_GEOMETRY;
static block_info const  BIM_SPECIES_TYPE = 0x0f << BIMS_SPECIES_TYPE;

// Info Flags:
// -----------

#define   BIFS_ANISOTROPIC 0x8
#define    BIFS_ORIENTABLE 0x9
// 0xa and 0xb are free
#define         BIFS_GROWS 0xc
#define   BIFS_GROWTH_SITE 0xd
#define   BIFS_GROWTH_CORE 0xe
#define       BIFS_AQUATIC 0xf
#define          BIFS_SEED 0x10
// 0x11, 0x12, and 13 are free

static block_info const  BIF_ANISOTROPIC = 1 << BIFS_ANISOTROPIC;
static block_info const   BIF_ORIENTABLE = 1 << BIFS_ORIENTABLE;
static block_info const        BIF_GROWS = 1 << BIFS_GROWS;
static block_info const  BIF_GROWTH_SITE = 1 << BIFS_GROWTH_SITE;
static block_info const  BIF_GROWTH_CORE = 1 << BIFS_GROWTH_CORE;
static block_info const      BIF_AQUATIC = 1 << BIFS_AQUATIC;
static block_info const         BIF_SEED = 1 << BIFS_SEED;

/********
 * Info *
 ********/

// Note: these are defined unshifted.

// Visibility:
static block_info const        BI_VIS_OPAQUE = 0x0;
static block_info const     BI_VIS_INVISIBLE = 0x1;
static block_info const   BI_VIS_TRANSPARENT = 0x2;
static block_info const   BI_VIS_TRANSLUCENT = 0x3;

// Substance:
static block_info const        BI_SBST_SOLID = 0x0;
static block_info const       BI_SBST_LIQUID = 0x1;
static block_info const        BI_SBST_EMPTY = 0x2;
static block_info const   BI_SBST_OBSTRUCTED = 0x3;

// Geometry:
static block_info const        BI_GEOM_SOLID = 0x0;
static block_info const       BI_GEOM_LIQUID = 0x1;
static block_info const         BI_GEOM_FILM = 0x2;
static block_info const        BI_GEOM_GRASS = 0x3;
static block_info const         BI_GEOM_HERB = 0x4;
static block_info const         BI_GEOM_VINE = 0x5;
static block_info const         BI_GEOM_ROOT = 0x6;
static block_info const       BI_GEOM_TANGLE = 0x7;
static block_info const         BI_GEOM_RAMP = 0x8;
static block_info const        BI_GEOM_FENCE = 0x9;
static block_info const         BI_GEOM_BEAM = 0xa;
static block_info const         BI_GEOM_DOOR = 0xb;
static block_info const        BI_GEOM_PANEL = 0xc;
static block_info const       BI_GEOM_COLUMN = 0xd;
static block_info const        BI_GEOM_EMPTY = 0xf;

/********
 * Data *
 ********/

// Note: interpretation of block data depends on the block type, hence the
// multiple overlapping values.

// Orientations:
static block const     BD_ORI_NORTH = 0x00;
static block const     BD_ORI_SOUTH = 0x01;
static block const      BD_ORI_EAST = 0x02;
static block const      BD_ORI_WEST = 0x03;
static block const        BD_ORI_UP = 0x04;
static block const      BD_ORI_DOWN = 0x05;

static block const        BD_ORI_IN = 0x06;
static block const       BD_ORI_OUT = 0x07;

// Macro versions (for defining other constants):
#define  M_BD_ORI_NORTH 0x00
#define  M_BD_ORI_SOUTH 0x01
#define   M_BD_ORI_EAST 0x02
#define   M_BD_ORI_WEST 0x03
#define     M_BD_ORI_UP 0x04
#define   M_BD_ORI_DOWN 0x05
#define     M_BD_ORI_IN 0x06
#define    M_BD_ORI_OUT 0x07
                          
// Faces (same as orientations; front <-> north by default):
static block const    BD_FACE_FRONT = 0x00;
static block const     BD_FACE_BACK = 0x01;
static block const    BD_FACE_RIGHT = 0x02;
static block const     BD_FACE_LEFT = 0x03;
static block const      BD_FACE_TOP = 0x04;
static block const      BD_FACE_BOT = 0x05;
                          
static block const   BD_FACE_INSIDE = 0x06;
static block const  BD_FACE_OUTSIDE = 0x07;

// Macro versions (for defining other constants):
#define   M_BD_FACE_FRONT 0x00
#define    M_BD_FACE_BACK 0x01
#define   M_BD_FACE_RIGHT 0x02
#define    M_BD_FACE_LEFT 0x03
#define     M_BD_FACE_TOP 0x04
#define     M_BD_FACE_BOT 0x05
#define  M_BD_FACE_INSIDE 0x06
#define M_BD_FACE_OUTSIDE 0x07

// A table for combining orientations. Index the table by the block facing and
// then the face index. For example, if you want to know what face is on top
// of an orientable block facing down, you'd get ROTATE_FACE[BD_ORI_DOWN][
// BD_ORI_TOP] and get BD_ORI_BACK. Note that the extra "IN" and "OUT" faces
// don't rotate, and rotating to face them does nothing.

static block const ROTATE_FACE[8][8] = {
// Facing NORTH (the identity mapping):
//  FRONT             BACK              RIGHT              LEFT
  { M_BD_FACE_FRONT,  M_BD_FACE_BACK,   M_BD_FACE_RIGHT ,  M_BD_FACE_LEFT,
//  TOP               BOT               IN                 OUT
    M_BD_FACE_TOP  ,  M_BD_FACE_BOT ,   M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing SOUTH:
//  FRONT             BACK              RIGHT              LEFT
  { M_BD_FACE_BACK,   M_BD_FACE_FRONT,  M_BD_FACE_LEFT  ,  M_BD_FACE_RIGHT,
//  TOP               BOT               IN                 OUT
    M_BD_FACE_TOP ,   M_BD_FACE_BOT  ,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing EAST:
//  FRONT             BACK              RIGHT              LEFT
  { M_BD_FACE_LEFT,   M_BD_FACE_RIGHT,  M_BD_FACE_FRONT ,  M_BD_FACE_BACK,
//  TOP               BOT               IN                 OUT
    M_BD_FACE_TOP ,   M_BD_FACE_BOT  ,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing WEST:
//  FRONT             BACK              RIGHT              LEFT
  { M_BD_FACE_RIGHT,  M_BD_FACE_LEFT,   M_BD_FACE_BACK  ,  M_BD_FACE_FRONT,
//  TOP               BOT               IN                 OUT
    M_BD_FACE_TOP  ,  M_BD_FACE_BOT ,   M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing UP:
//  FRONT             BACK              RIGHT              LEFT
  { M_BD_FACE_BOT  ,  M_BD_FACE_TOP ,   M_BD_FACE_RIGHT ,  M_BD_FACE_LEFT,
//  TOP               BOT               IN                 OUT
    M_BD_FACE_FRONT,  M_BD_FACE_BACK,   M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing DOWN:
//  FRONT             BACK              RIGHT              LEFT
  { M_BD_FACE_TOP ,   M_BD_FACE_BOT  ,  M_BD_FACE_RIGHT ,  M_BD_FACE_LEFT,
//  TOP               BOT               IN                 OUT
    M_BD_FACE_BACK,   M_BD_FACE_FRONT,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing IN:
//  FRONT             BACK              RIGHT              LEFT
  { M_BD_FACE_FRONT,  M_BD_FACE_BACK,   M_BD_FACE_RIGHT ,  M_BD_FACE_LEFT,
//  TOP               BOT               IN                 OUT
    M_BD_FACE_TOP  ,  M_BD_FACE_BOT ,   M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing OUT:
//  FRONT             BACK              RIGHT              LEFT
  { M_BD_FACE_FRONT,  M_BD_FACE_BACK,   M_BD_FACE_RIGHT ,  M_BD_FACE_LEFT ,
//  TOP               BOT               IN                 OUT
    M_BD_FACE_TOP  ,  M_BD_FACE_BOT ,   M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },
};

/**********
 * Blocks *
 **********/

// Note that these are given as block IDs, ignoring the info bits.

// Special blocks:
#define                              B_VOID 0x000 // for invalid/missing blocks
#define                          B_BOUNDARY 0x001 // for sealing areas 

// Invisible blocks:
#define                               B_AIR 0x002
#define                             B_ETHER 0x003
#define                             B_FUMES 0x004

// Translucent liquid blocks:
#define                             B_WATER 0x00a
#define                        B_WATER_FLOW 0x00b

#define                             B_SLIME 0x00c
#define                        B_SLIME_FLOW 0x00d

#define                              B_ACID 0x00e
#define                         B_ACID_FLOW 0x00f

// Opaque liquid blocks:
#define                         B_QUICKSAND 0x014

#define                              B_LAVA 0x016
#define                         B_LAVA_FLOW 0x017

// Translucent non-solid blocks:
#define                             B_SMOKE 0x021
#define                            B_MIASMA 0x022

// Opaque non-solid blocks:
#define                       B_THICK_SMOKE 0x028

// Mineral blocks:
#define                              B_DIRT 0x030
#define                               B_MUD 0x031
#define                              B_CLAY 0x032
#define                              B_SAND 0x033
#define                            B_GRAVEL 0x034
#define                             B_SCREE 0x035
#define                             B_STONE 0x036
#define                      B_NATIVE_METAL 0x037

// Plant blocks:
#define                   B_MUSHROOM_SPORES 0x040
#define                   B_MUSHROOM_SHOOTS 0x041
#define                    B_MUSHROOM_GROWN 0x042

#define             B_GIANT_MUSHROOM_SPORES 0x044
#define               B_GIANT_MUSHROOM_CORE 0x045
#define           B_GIANT_MUSHROOM_MYCELIUM 0x046
#define             B_GIANT_MUSHROOM_SPROUT 0x047
#define              B_GIANT_MUSHROOM_STALK 0x048
#define                B_GIANT_MUSHROOM_CAP 0x049

#define                       B_MOSS_SPORES 0x04a
#define                       B_MOSS_SHOOTS 0x04b
#define                        B_MOSS_GROWN 0x04c
#define                    B_MOSS_FLOWERING 0x04d
#define                     B_MOSS_FRUITING 0x04e

#define                       B_GRASS_SEEDS 0x050
#define                       B_GRASS_ROOTS 0x051
#define                      B_GRASS_SHOOTS 0x052
#define                       B_GRASS_GROWN 0x053
#define                     B_GRASS_BUDDING 0x054
#define                   B_GRASS_FLOWERING 0x055
#define                    B_GRASS_FRUITING 0x056

#define                        B_VINE_SEEDS 0x058
#define                         B_VINE_CORE 0x059
#define                        B_VINE_ROOTS 0x05a
#define                       B_VINE_SHOOTS 0x05b
#define                    B_VINE_SPROUTING 0x05c
#define                        B_VINE_GROWN 0x05d
#define                      B_VINE_BUDDING 0x05e
#define                    B_VINE_FLOWERING 0x05f
#define                     B_VINE_FRUITING 0x060
#define                     B_VINE_SHEDDING 0x061
#define                      B_VINE_DORMANT 0x062

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
#define                   B_TREE_HEART_CORE 0x096
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

#define                    B_WOODEN_SHINGLE 0x0e3
#define                        B_STONE_TILE 0x0e4
#define                      B_CERAMIC_TILE 0x0e5

#define                      B_WOODEN_GRATE 0x0e6
#define                       B_STONE_GRATE 0x0e7
#define                       B_METAL_GRATE 0x0e8

#define                       B_GLASS_BLOCK 0x0e9
#define                        B_GLASS_PANE 0x0ea

#define                     B_WOODEN_EDGING 0x0ec
#define                      B_STONE_EDGING 0x0ed
#define                      B_METAL_EDGING 0x0ee

#define                      B_WOODEN_FRAME 0x0f0
#define                       B_STONE_FRAME 0x0f1
#define                       B_METAL_FRAME 0x0f2

#define                       B_METAL_BLOCK 0x0f4

// Interactive blocks:
#define                       B_WOODEN_GATE 0x100
#define                        B_STONE_GATE 0x101
#define                        B_METAL_GATE 0x102
#define                 B_WOODEN_PLANK_DOOR 0x103
#define                 B_WOODEN_PANEL_DOOR 0x104
#define                        B_STONE_DOOR 0x105
#define                        B_METAL_DOOR 0x106

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
#define                          B_STEM_MAT 0x129

/********************
 * Inline Functions *
 ********************/

// Properties:

// Per-block properties:
static inline block b_id(block b) { return b >> BS_ID; }
static inline species b_species(block b) { return (b & BM_SPECIES) >> BS_SPC; }
static inline block b_idspc(block b) { return b >> BS_SPC; }
static inline block b_data(block b) { return (b & BM_DATA) >> BS_DAT; }
static inline block b_ori(block b) { return (b & BM_ORIENTATION) >> BS_ORI; }
static inline block_info b_info(block b) { return BLOCK_INFO[b_id(b)]; }
static inline char* block_name(block b) { return BLOCK_NAMES[b_id(b)]; }

// Per-block-type properties:
static inline block_info bi_vis(block b) {
  return (BLOCK_INFO[b_id(b)] & BIM_VISIBILITY) >> BIMS_VISIBILITY;
}
static inline block_info bi_sbst(block b) {
  return (BLOCK_INFO[b_id(b)] & BIM_SUBSTANCE) >> BIMS_SUBSTANCE;
}
static inline block_info bi_geom(block b) {
  return (BLOCK_INFO[b_id(b)] & BIM_GEOMETRY) >> BIMS_GEOMETRY;
}
static inline block_info bi_anis(block b) {
  return (BLOCK_INFO[b_id(b)] & BIF_ANISOTROPIC) >> BIFS_ANISOTROPIC;
}
static inline block_info bi_oabl(block b) {
  return (BLOCK_INFO[b_id(b)] & BIF_ORIENTABLE) >> BIFS_ORIENTABLE;
}
static inline block_info bi_grws(block b) {
  return (BLOCK_INFO[b_id(b)] & BIF_GROWS) >> BIFS_GROWS;
}
static inline block_info bi_gsite(block b) {
  return (BLOCK_INFO[b_id(b)] & BIF_GROWTH_SITE) >> BIFS_GROWTH_SITE;
}
static inline block_info bi_gcore(block b) {
  return (BLOCK_INFO[b_id(b)] & BIF_GROWTH_CORE) >> BIFS_GROWTH_CORE;
}
static inline block_info bi_gaqua(block b) {
  return (BLOCK_INFO[b_id(b)] & BIF_AQUATIC) >> BIFS_AQUATIC;
}
static inline block_info bi_seed(block b) {
  return (BLOCK_INFO[b_id(b)] & BIF_SEED) >> BIFS_SEED;
}
static inline block_info bi_species_type(block b) {
  return (BLOCK_INFO[b_id(b)] & BIM_SPECIES_TYPE) >> BIMS_SPECIES_TYPE;
}

// Constructors:

// Turn an ID into a block with default species and orientation:
static inline block b_make_block(block id) {
  return id << BS_ID;
}

// Combine an id and species into a block with default orientation:
static inline block b_make_species(block id, species sp) {
  return (id << BS_ID) + (((block) sp) << BS_SPC);
}

// Edit functions:
static inline void b_set_data(block *b, block_data data) {
  *b &= ~BM_DATA; // zero out the data
  *b |= ((block) data) << BS_DAT;
}

// Comparisons:

// Compare block IDs and species:
static inline block b_is(block b, block c) {
  return b >> BS_SPC == c >> BS_SPC;
}

// Compare just block IDs:
static inline block b_is_same_type(block b, block c) {
  return b_id(b) == b_id(c);
}

// Compare all but the lowest bit of IDs:
static inline block b_same_liquid(block b, block c) {
  return (b_id(b) | 1) == (b_id(c) | 1);
}

// Type checks:

static inline block b_is_void(block b) { return b_is(b, B_VOID); }
static inline block b_is_boundary(block b) { return b_is(b, B_BOUNDARY); }

static inline block b_is_opaque(block b) { return bi_vis(b) == BI_VIS_OPAQUE; }
static inline block b_is_invisible(block b) {
  return bi_vis(b) == BI_VIS_INVISIBLE;
}
static inline block b_is_transparent(block b) {
  return bi_vis(b) == BI_VIS_TRANSPARENT;
}
static inline block b_is_translucent(block b) {
  return bi_vis(b) == BI_VIS_TRANSLUCENT;
}

static inline block b_is_solid(block b) { return bi_sbst(b) == BI_SBST_SOLID; }
static inline block b_is_liquid(block b) { return bi_sbst(b) == BI_SBST_LIQUID;}
static inline block b_is_empty(block b) { return bi_sbst(b) == BI_SBST_EMPTY; }
static inline block b_is_obstructed(block b) {
  return bi_sbst(b) == BI_SBST_OBSTRUCTED;
}

// Synthetic categories:

static inline block b_is_natural_terrain(block b) {
  return b_id(b) >= B_DIRT && b_id(b) < B_MUSHROOM_SPORES;
}

// Other flags can be checked manually...


// Utilities:

static inline block next_block(block b) {
  return b_make_block((b_id(b) + 1) % TOTAL_BLOCK_TYPES);
}

static inline block next_species(block b) {
  return (((b >> BS_SPC) + 1) % TOTAL_BLOCK_SPECIES) << BS_SPC;
}

static inline void copy_cell(cell const * const src, cell * dst) {
  dst->blocks[0] = src->blocks[0];
  dst->blocks[1] = src->blocks[1];
}

/*************
 * Functions *
 *************/

// Fills in the block info table.
void init_blocks(void);

#endif // ifndef BLOCKS_H
