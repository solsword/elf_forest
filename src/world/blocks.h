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
// 14 bits of block variant.
// 3 bits of block orientation.
// 6 bits of block exposure.
typedef uint32_t block;

// Extra static block data and flags stored in the BLOCK_INFO table.
typedef uint32_t block_info;

// Extra variable block data. Meaning depends on block ID.
typedef uint16_t block_data;

struct cell_s;
typedef struct cell_s cell;

struct cell_s {
  block primary;
  block secondary;
  block_data p_data;
  block_data s_data;
};

/**********************
 * Constants and Data *
 **********************/

#define BLOCK_ID_BITS 9;
#define BLOCK_VAR_BITS 14;
#define BLOCK_ORI_BITS 3;
#define BLOCK_EXP_BITS 6;

#define TOTAL_BLOCK_TYPES = (1 << BLOCK_ID_BITS)
#define TOTAL_BLOCK_VARIANTS = (1 << (BLOCK_ID_BITS + BLOCK_VAR_BITS))
#define VARIANTS_PER_BLOCK = (1 << BLOCK_VAR_BITS)

extern block_info const BLOCK_INFO[TOTAL_BLOCK_TYPES];

/*******************
 * Flags and Masks *
 *******************/

// The ability to turn a read into a 1 or 0 (instead of an N or 0) is useful
// sometimes. Shifts tell you how many bits to shift (and simultaneously are
// used to define flags).

// Exposure Flags:
// ---------------

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

// Masks:
// ------

static block const    BM_EXPOSURE = 0x3f;
static block const BM_ORIENTATION = 0x7 << BLOCK_EXP_BITS;
static block const     BM_VARIANT = 0x3fff << (BLOCK_EXP_BITS + BLOCK_ORI_BITS);

// Info Masks:
// -----------

#define   BIMS_VISIBILITY 0
#define    BIMS_SUBSTANCE 2
#define     BIMS_GEOMETRY 4

static block_info const  BIM_VISIBILITY = 0x03 << BIMS_VISIBILITY;
static block_info const   BIM_SUBSTANCE = 0x03 << BIMS_SUBSTANCE;
static block_info const    BIM_GEOMETRY = 0x0f << BIMS_SUBSTANCE;

// Info Flags:
// -----------

#define  BIFS_DIRECTIONAL 8
#define   BIFS_ORIENTABLE 9

static block_info const  BIF_DIRECTIONAL = 1 << BIFS_DIRECTIONAL;
static block_info const   BIF_ORIENTABLE = 1 << BIFS_ORIENTABLE;

/********
 * Info *
 ********/

// Note: these are defined unshifted.

// Visibility:
static block_info const       BI_VIS_VISIBLE = 0x00;
static block_info const     BI_VIS_INVISIBLE = 0x01;
static block_info const   BI_VIS_TRANSPARENT = 0x02;
static block_info const   BI_VIS_TRANSLUCENT = 0x03;

// Substance:
static block_info const        BI_SBST_SOLID = 0x00;
static block_info const       BI_SBST_LIQUID = 0x01;
static block_info const        BI_SBST_EMPTY = 0x02;
static block_info const   BI_SBST_OBSTRUCTED = 0x03;

// Geometry:
static block_info const   BI_GEOM_FULL_BLOCK = 0x00;
static block_info const   BI_GEOM_HALF_BLOCK = 0x01;
static block_info const       BI_GEOM_LIQUID = 0x02;
static block_info const       BI_GEOM_STAIRS = 0x03;
static block_info const        BI_GEOM_FENCE = 0x04;
static block_info const         BI_GEOM_DOOR = 0x05;
static block_info const         BI_GEOM_PANE = 0x06;
static block_info const        BI_GEOM_GRASS = 0x07;
static block_info const         BI_GEOM_VINE = 0x08;
static block_info const       BI_GEOM_COLUMN = 0x09;

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
static block const                    B_VOID = 0x000; // for invalid/missing
static block const                B_BOUNDARY = 0x001; // for sealing areas 

// Invisible blocks:
static block const                     B_AIR = 0x002;
static block const                   B_ETHER = 0x003;

// Translucent liquid blocks:
static block const                   B_WATER = 0x004;
static block const              B_WATER_FLOW = 0x005;

static block const                   B_SLIME = 0x006;
static block const              B_SLIME_FLOW = 0x007;

static block const                    B_ACID = 0x008;
static block const               B_ACID_FLOW = 0x009;

// Opaque liquid blocks:
static block const               B_QUICKSAND = 0x014;

static block const                    B_LAVA = 0x016;
static block const               B_LAVA_FLOW = 0x017;

// Translucent non-solid blocks:
static block const                   B_SMOKE = 0x021;
static block const                  B_MIASMA = 0x022;

// Opaque non-solid blocks:
static block const             B_BLACK_SMOKE = 0x028;

// Mineral blocks:
static block const                    B_DIRT = 0x030;
static block const                     B_MUD = 0x031;
static block const                    B_CLAY = 0x032;
static block const                    B_SAND = 0x033;
static block const                  B_GRAVEL = 0x034;
static block const                   B_SCREE = 0x035;
static block const                   B_STONE = 0x036;
static block const               B_METAL_ORE = 0x037;
static block const            B_NATIVE_METAL = 0x038;

// Plant blocks:
static block const                B_MUSHROOM = 0x040;
static block const          B_MUSHROOM_STALK = 0x041;
static block const            B_MUSHROOM_CAP = 0x042;

static block const                    B_MOSS = 0x043;

static block const                   B_GRASS = 0x044;
static block const             B_GRASS_ROOTS = 0x045;

static block const                    B_VINE = 0x046;
static block const              B_VINE_FRUIT = 0x047;

static block const                    B_HERB = 0x048;
static block const              B_HERB_ROOTS = 0x049;

static block const           B_BUSH_BRANCHES = 0x04a;
static block const             B_BUSH_LEAVES = 0x04b;
static block const              B_BUSH_FRUIT = 0x04c;
static block const              B_BUSH_ROOTS = 0x04d;

static block const          B_SHRUB_BRANCHES = 0x04e;
static block const            B_SHRUB_LEAVES = 0x04f;
static block const             B_SHRUB_FRUIT = 0x050;
static block const             B_SHRUB_ROOTS = 0x051;

static block const           B_TREE_BRANCHES = 0x052;
static block const             B_TREE_LEAVES = 0x053;
static block const              B_TREE_FRUIT = 0x054;
static block const              B_TREE_TRUNK = 0x055;
static block const              B_TREE_ROOTS = 0x056;
static block const        B_TREE_HEART_ROOTS = 0x057;

static block const           B_AQUATIC_GRASS = 0x058;
static block const     B_AQUATIC_GRASS_ROOTS = 0x059;

static block const    B_AQUATIC_PLANT_LEAVES = 0x05a;
static block const     B_AQUATIC_PLANT_STEMS = 0x05b;
static block const     B_AQUATIC_PLANT_ROOTS = 0x05c;

static block const                   B_CORAL = 0x05d;

// Hewn Blocks:
static block const           B_SMOOTHED_ROCK = 0x070;
static block const         B_HEWN_ROCK_STEPS = 0x072;
static block const     B_SMOOTHED_ROCK_STEPS = 0x073;
static block const         B_HEWN_ROCK_GRATE = 0x074;

// Construction Materials:
static block const                    B_BALE = 0x080;
static block const                  B_THATCH = 0x081;
static block const                  B_WATTLE = 0x082;

static block const            B_WOODEN_PLANK = 0x083;
static block const             B_WOODEN_BEAM = 0x084;
static block const            B_WOODEN_PANEL = 0x085;
static block const           B_WOODEN_PILLAR = 0x086;

static block const                B_CORDWOOD = 0x087;
static block const                     B_COB = 0x088;
static block const            B_RAMMED_EARTH = 0x089;
static block const           B_STACKED_STONE = 0x08a;
static block const            B_FITTED_STONE = 0x08b;
static block const          B_MORTARED_STONE = 0x08c;
static block const              B_METAL_BARS = 0x08d;

static block const              B_STONE_POST = 0x08e;
static block const            B_STONE_PILLAR = 0x08f;

static block const               B_MUD_BRICK = 0x090;
static block const              B_CLAY_BRICK = 0x091;
static block const             B_STONE_BRICK = 0x092;

static block const              B_STONE_TILE = 0x093;
static block const            B_CERAMIC_TILE = 0x094;
static block const          B_WOODEN_SHINGLE = 0x095;

static block const            B_WOODEN_GRATE = 0x096;
static block const             B_STONE_GRATE = 0x097;
static block const             B_METAL_GRATE = 0x098;

static block const             B_GLASS_BLOCK = 0x099;
static block const              B_GLASS_PANE = 0x09a;
static block const            B_FRAMED_GLASS = 0x09b;

// Interactive blocks:

static block const             B_WOODEN_GATE = 0x0b0;
static block const              B_METAL_GATE = 0x0b1;
static block const       B_WOODEN_PLANK_DOOR = 0x0b2;
static block const       B_WOODEN_PANEL_DOOR = 0x0b3;
static block const              B_STONE_DOOR = 0x0b4;
static block const              B_METAL_DOOR = 0x0b5;

// Decorative blocks:
static block const                 B_PLASTER = 0x0d0;
static block const                  B_STUCCO = 0x0d1;
static block const                   B_PAINT = 0x0d2;

static block const                  B_BANNER = 0x0d3;
static block const                B_TAPESTRY = 0x0d4;
static block const                B_PAINTING = 0x0d5;
static block const               B_ENGRAVING = 0x0d6;

static block const                  B_CARPET = 0x0d7;
static block const                     B_RUG = 0x0d8;
static block const               B_CLOTH_MAT = 0x0d9;
static block const                B_STEM_MAT = 0x0da;

/********************
 * Inline Functions *
 ********************/

// Conversions:

static inline block_id b_id(block b) {
  return (block_id) (b >> 8);
}

static inline block b_full(block_id id) {
  return ((block) id) << 8;
}

static inline block_data b_orientation(block b) {
  return b & BR_ORIENTATION;
}

// Comparisons:

static inline block b_is(block b, block c) {
  return b_id(b) == b_id(c);
}

static inline block b_shares_translucency(block b, block c) {
  return (
    (b_id(b) | 1) == (b_id(c) | 1)
  &&
    !b_is(b, B_LEAVES)
  );
}

// Type checks:

static inline block b_is_void(block b) {
  return b_is(b, B_VOID);
}

// TODO: Implement static block-properties table!
static inline block b_is_orientable(block b) {
  return b & BF_ORIENTABLE;
}

static inline block b_is_omnidirectional(block b) {
  return b & BF_OMNIDIRECTIONAL;
}

static inline block b_is_translucent_liquid(block b) {
  return (
    (b > BL_MAX_INVISIBLE)
  &&
    (b <= BL_MAX_T_LIQUID)
  );
}

static inline block b_is_opaque_liquid(block b) {
  return (
    (b >= BL_MIN_O_LIQUID)
  &&
    (b < BL_MIN_SOLID)
  );
}

static inline block b_is_liquid(block b) {
  return b_is_translucent_liquid(b) || b_is_opaque_liquid(b);
}

static inline block b_is_solid(block b) {
  return b >= BL_MIN_SOLID;
}

static inline block b_is_invisible(block b) {
  return b <= BL_MAX_INVISIBLE;
}

static inline block b_is_translucent(block b) {
  return b_is_translucent_liquid(b);
}

static inline block b_is_transparent(block b) {
  return b >= BL_MIN_TRANSPARENT;
}

static inline block b_is_opaque(block b) {
  return (
    (b >= BL_MIN_O_LIQUID)
  &&
    (b < BL_MIN_TRANSPARENT)
  );
}

// Utilities:

static inline block next_block(block b) {
  return b_full((b_id(b) + 1) % BLOCK_TYPE_COUNT);
}

#endif // ifndef BLOCKS_H
