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
// For flowing liquids:
//   2 bits of level.
//   3 bits of flow direction.
//   3 bits of flow rate.
// For organic blocks:
//   2 bits of vitality (healthy, sick, dying, dead)
//   3 bits of growth stage
//     (dormant, sprouting, budding, flowering, fruit-bearing, normal, autumnal)
//   3 bits of growth direction.
typedef uint8_t block_data;

// Extra static block data and flags stored in the BLOCK_INFO table.
typedef uint32_t block_info;

// A cell holds two blocks and their block data:
struct cell_s;
typedef struct cell_s cell;

/*************************
 * Structure Definitions *
 *************************/

struct cell_s {
  block primary;
  block secondary;
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

extern block_info const BLOCK_INFO[TOTAL_BLOCK_TYPES];

#define MAX_BLOCK_NAME_LENGTH 17

extern char const * const BLOCK_NAMES[TOTAL_BLOCK_TYPES];

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

// Info Masks:
// -----------

#define   BIMS_VISIBILITY 0
#define    BIMS_SUBSTANCE 2
#define     BIMS_GEOMETRY 4

static block_info const  BIM_VISIBILITY = 0x03 << BIMS_VISIBILITY;
static block_info const   BIM_SUBSTANCE = 0x03 << BIMS_SUBSTANCE;
static block_info const    BIM_GEOMETRY = 0x0f << BIMS_GEOMETRY;

// Info Flags:
// -----------

#define  BIFS_ANISOTROPIC 8
#define   BIFS_ORIENTABLE 9

static block_info const  BIF_ANISOTROPIC = 1 << BIFS_ANISOTROPIC;
static block_info const   BIF_ORIENTABLE = 1 << BIFS_ORIENTABLE;

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
#define                    B_VOID 0x000 // for invalid/missing blocks
#define                B_BOUNDARY 0x001 // for sealing areas 

// Invisible blocks:
#define                     B_AIR 0x002
#define                   B_ETHER 0x003
#define               B_BLACKDAMP 0x004
#define               B_WHITEDAMP 0x005
#define                B_FIREDAMP 0x006
#define               B_STINKDAMP 0x007

// Translucent liquid blocks:
#define                   B_WATER 0x00a
#define              B_WATER_FLOW 0x00b

#define                   B_SLIME 0x00c
#define              B_SLIME_FLOW 0x00d

#define                    B_ACID 0x00e
#define               B_ACID_FLOW 0x00f

// Opaque liquid blocks:
#define               B_QUICKSAND 0x014

#define                    B_LAVA 0x016
#define               B_LAVA_FLOW 0x017

// Translucent non-solid blocks:
#define                   B_SMOKE 0x021
#define                  B_MIASMA 0x022

// Opaque non-solid blocks:
#define             B_BLACK_SMOKE 0x028

// Mineral blocks:
#define                    B_DIRT 0x030
#define                     B_MUD 0x031
#define                    B_CLAY 0x032
#define                    B_SAND 0x033
#define                  B_GRAVEL 0x034
#define                   B_SCREE 0x035
#define                   B_STONE 0x036
#define            B_NATIVE_METAL 0x037

// Plant blocks:
#define         B_MUSHROOM_SPORES 0x040
#define                B_MUSHROOM 0x041
#define          B_MUSHROOM_STALK 0x042
#define            B_MUSHROOM_CAP 0x043

#define             B_MOSS_SPORES 0x044
#define                    B_MOSS 0x045
#define            B_MOSS_FLOWERS 0x046
#define              B_MOSS_FRUIT 0x047

#define             B_GRASS_SEEDS 0x048
#define             B_GRASS_ROOTS 0x049
#define                   B_GRASS 0x04a
#define           B_GRASS_FLOWERS 0x04b
#define             B_GRASS_FRUIT 0x04c

#define              B_VINE_SEEDS 0x04e
#define              B_VINE_ROOTS 0x04f
#define                    B_VINE 0x050
#define            B_VINE_FLOWERS 0x051
#define              B_VINE_FRUIT 0x052

#define              B_HERB_SEEDS 0x054
#define              B_HERB_ROOTS 0x055
#define                    B_HERB 0x056
#define            B_HERB_FLOWERS 0x057
#define              B_HERB_FRUIT 0x058

#define              B_BUSH_SEEDS 0x05a
#define              B_BUSH_ROOTS 0x05b
#define           B_BUSH_BRANCHES 0x05c
#define             B_BUSH_LEAVES 0x05d
#define            B_BUSH_FLOWERS 0x05e
#define              B_BUSH_FRUIT 0x05f

#define             B_SHRUB_SEEDS 0x060
#define             B_SHRUB_ROOTS 0x061
#define          B_SHRUB_BRANCHES 0x062
#define            B_SHRUB_LEAVES 0x063
#define           B_SHRUB_FLOWERS 0x064
#define             B_SHRUB_FRUIT 0x065

#define              B_TREE_SEEDS 0x066
#define        B_TREE_HEART_ROOTS 0x067
#define              B_TREE_ROOTS 0x068
#define              B_TREE_TRUNK 0x069
#define  B_TREE_INTERIOR_BRANCHES 0x06a
#define           B_TREE_BRANCHES 0x06b
#define             B_TREE_LEAVES 0x06c
#define            B_TREE_FLOWERS 0x06d
#define              B_TREE_FRUIT 0x06e

#define     B_AQUATIC_GRASS_SEEDS 0x070
#define     B_AQUATIC_GRASS_ROOTS 0x071
#define           B_AQUATIC_GRASS 0x072
#define   B_AQUATIC_GRASS_FLOWERS 0x073
#define     B_AQUATIC_GRASS_FRUIT 0x074

#define     B_AQUATIC_PLANT_SEEDS 0x076
#define   B_AQUATIC_PLANT_ANCHORS 0x077
#define     B_AQUATIC_PLANT_STEMS 0x078
#define    B_AQUATIC_PLANT_LEAVES 0x079
#define   B_AQUATIC_PLANT_FLOWERS 0x07a
#define     B_AQUATIC_PLANT_FRUIT 0x07b

#define             B_CORAL_FROND 0x07c
#define              B_CORAL_BODY 0x07d

// Construction Materials:
#define                    B_BALE 0x080
#define                  B_THATCH 0x081
#define                  B_WATTLE 0x082

#define            B_WOODEN_PLANK 0x083
#define             B_WOODEN_BEAM 0x084
#define            B_WOODEN_PANEL 0x085
#define           B_WOODEN_PILLAR 0x086

#define                B_CORDWOOD 0x087
#define                     B_COB 0x088
#define            B_RAMMED_EARTH 0x089
#define           B_STACKED_STONE 0x08a
#define            B_FITTED_STONE 0x08b
#define          B_MORTARED_STONE 0x08c
#define              B_METAL_BARS 0x08d

#define              B_STONE_POST 0x08e
#define            B_STONE_PILLAR 0x08f

#define               B_MUD_BRICK 0x090
#define              B_CLAY_BRICK 0x091
#define             B_STONE_BRICK 0x092

#define              B_STONE_TILE 0x093
#define            B_CERAMIC_TILE 0x094
#define          B_WOODEN_SHINGLE 0x095

#define            B_WOODEN_GRATE 0x096
#define             B_STONE_GRATE 0x097
#define             B_METAL_GRATE 0x098

#define             B_GLASS_BLOCK 0x099
#define              B_GLASS_PANE 0x09a
#define            B_FRAMED_GLASS 0x09b

#define             B_METAL_BLOCK 0x09c


// Hewn Blocks:
#define           B_SMOOTHED_ROCK 0x0a0
#define         B_HEWN_ROCK_GRATE 0x0a1

// Interactive blocks:
#define             B_WOODEN_GATE 0x0b0
#define              B_METAL_GATE 0x0b1
#define       B_WOODEN_PLANK_DOOR 0x0b2
#define       B_WOODEN_PANEL_DOOR 0x0b3
#define              B_STONE_DOOR 0x0b4
#define              B_METAL_DOOR 0x0b5

// Decorative blocks:
#define                 B_PLASTER 0x0d0
#define                  B_STUCCO 0x0d1
#define                   B_PAINT 0x0d2

#define                  B_BANNER 0x0d3
#define                B_TAPESTRY 0x0d4
#define                B_PAINTING 0x0d5
#define               B_ENGRAVING 0x0d6

#define                  B_CARPET 0x0d7
#define                     B_RUG 0x0d8
#define               B_CLOTH_MAT 0x0d9
#define                B_STEM_MAT 0x0da

/********************
 * Inline Functions *
 ********************/

// Properties:

// Per-block properties:
static inline block b_id(block b) { return b >> BS_ID; }
static inline species b_species(block b) { return (b & BM_SPECIES) >> BS_SPC; }
static inline block b_idspc(block b) { return b >> BS_SPC; }
static inline block b_ori(block b) { return (b & BM_ORIENTATION) >> BS_ORI; }

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

// Constructors:

// Turn an ID into a block with default species and orientation:
static inline block b_make_block(block id) {
  return id << BS_ID;
}

// Combine an id and species into a block with default orientation:
static inline block b_make_species(block id, species sp) {
  return (id << BS_ID) + (((block) sp) << BS_SPC);
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

// Other flags can be checked manually...

// Utilities:

static inline block next_block(block b) {
  return b_make_block((b_id(b) + 1) % TOTAL_BLOCK_TYPES);
}

static inline block next_species(block b) {
  return (((b >> BS_SPC) + 1) % TOTAL_BLOCK_SPECIES) << BS_SPC;
}

static inline void copy_cell(cell const * const src, cell * dst) {
  dst->primary = src->primary;
  dst->secondary = src->secondary;
}

#endif // ifndef BLOCKS_H
