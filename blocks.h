#ifndef BLOCKS_H
#define BLOCKS_H

// blocks.h
// Defines block types.

#include <stdint.h>

/************
 * Typedefs *
 ************/

typedef uint16_t block;
typedef uint8_t block_id;
typedef uint16_t block_count;
typedef uint16_t block_range;
typedef uint16_t block_flag;
typedef uint16_t block_data;
typedef uint16_t block_limit;

/*************
 * Constants *
 *************/

static const block_count BLOCK_TYPE_COUNT = 256;

/**********
 * Ranges *
 **********/

static const block_range  BR_FLAGS = 0x00f8;
static const block_range   BR_DATA = 0x0007;
static const block_range     BR_ID = 0xff00;

/*********
 * Flags *
 *********/

// Flag shifts:
// ------------

// The ability to turn a flag read into a 1 or 0 (instead of an N or 0) is
// useful sometimes. Flag shifts tell you how many bits to shift (and
// simultaneously are used to define the flag itself).

#define     BFS_EXPOSED_SHIFT 7;
#define    BFS_RESERVED_SHIFT 6;
#define      BFS_ON_OFF_SHIFT 5;
#define  BFS_ORIENTABLE_SHIFT 4;
#define  BFS_HAS_ENTITY_SHIFT 3;

// Dynamic flags:
// --------------

// Is this block exposed?
static const block_flag     BF_EXPOSED = 1 << BFS_EXPOSED_SHIFT;
// Reserved for future use.
static const block_flag    BF_RESERVED = 1 << BFS_RESERVED_SHIFT;
// Generic on/off.
static const block_flag      BF_ON_OFF = 1 << BFS_ON_OFF_SHIFT;

// Static flags:
// -------------

// Is this block orientable?
static const block_flag  BF_ORIENTABLE = 1 << BFS_ORIENTABLE_SHIFT;
// Does this block have an entity associated with it?
static const block_flag  BF_HAS_ENTITY = 1 << BFS_HAS_ENTITY_SHIFT;

/********
 * Data *
 ********/

// Note: interpretation of block data depends on the block type, hence the
// multiple overlapping values.

// Orientations:
static const block_data        BD_ORI_UP = 0x0000;
static const block_data      BD_ORI_DOWN = 0x0001;
static const block_data     BD_ORI_NORTH = 0x0002;
static const block_data     BD_ORI_SOUTH = 0x0003;
static const block_data      BD_ORI_EAST = 0x0004;
static const block_data      BD_ORI_WEST = 0x0005;
                            
static const block_data        BD_ORI_IN = 0x0006;
static const block_data       BD_ORI_OUT = 0x0007;

// Macro versions (for defining other constants):
#define     M_BD_ORI_UP 0x0000
#define   M_BD_ORI_DOWN 0x0001
#define  M_BD_ORI_NORTH 0x0002
#define  M_BD_ORI_SOUTH 0x0003
#define   M_BD_ORI_EAST 0x0004
#define   M_BD_ORI_WEST 0x0005
#define     M_BD_ORI_IN 0x0006
#define    M_BD_ORI_OUT 0x0007
                          
// Faces (same as orientations; front <-> north by default):
static const block_data      BD_FACE_TOP = 0x0000;
static const block_data      BD_FACE_BOT = 0x0001;
                          
static const block_data    BD_FACE_FRONT = 0x0002;
static const block_data     BD_FACE_BACK = 0x0003;
                          
static const block_data    BD_FACE_RIGHT = 0x0004;
static const block_data     BD_FACE_LEFT = 0x0005;
                          
static const block_data   BD_FACE_INSIDE = 0x0006;
static const block_data  BD_FACE_OUTSIDE = 0x0007;

// Macro versions (for defining other constants):
#define     M_BD_FACE_TOP 0x0000
#define     M_BD_FACE_BOT 0x0001
#define   M_BD_FACE_FRONT 0x0002
#define    M_BD_FACE_BACK 0x0003
#define   M_BD_FACE_RIGHT 0x0004
#define    M_BD_FACE_LEFT 0x0005
#define  M_BD_FACE_INSIDE 0x0006
#define M_BD_FACE_OUTSIDE 0x0007

// A table for combining orientations. Index the table by the block facing and
// then the face index). For example, if you want to know what face is on top
// of an orientable block facing down, you'd get ROTATE_FACE[BD_ORI_DOWN,
// BD_ORI_TOP] and get BD_ORI_BACK. Note that the extra "IN" and "OUT" faces
// don't rotate, and rotating to face them does nothing.

static const block_data ROTATE_FACE[8][8] = {
// Facing UP:
//  TOP               BOT               FRONT              BACK
  { M_BD_FACE_FRONT,  M_BD_FACE_BACK ,  M_BD_FACE_BOT   ,  M_BD_FACE_TOP    ,
//  RIGHT             LEFT              IN                 OUT
    M_BD_FACE_RIGHT,  M_BD_FACE_LEFT ,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing DOWN:
//  TOP               BOT               FRONT              BACK
  { M_BD_FACE_BACK ,  M_BD_FACE_FRONT,  M_BD_FACE_TOP   ,  M_BD_FACE_BOT    ,
//  RIGHT             LEFT              IN                 OUT
    M_BD_FACE_RIGHT,  M_BD_FACE_LEFT ,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing NORTH (thidentity mapping):
//  TOP               BOT               FRONT              BACK
  { M_BD_FACE_TOP  ,  M_BD_FACE_BOT  ,  M_BD_FACE_FRONT ,  M_BD_FACE_BACK   ,
//  RIGHT             LEFT              IN                 OUT
    M_BD_FACE_RIGHT,  M_BD_FACE_LEFT ,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing SOUTH:
//  TOP               BOT               FRONT              BACK
  { M_BD_FACE_TOP  ,  M_BD_FACE_BOT  ,  M_BD_FACE_BACK  ,  M_BD_FACE_FRONT  ,
//  RIGHT             LEFT              IN                 OUT
    M_BD_FACE_LEFT ,  M_BD_FACE_RIGHT,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing EAST:
//  TOP               BOT               FRONT              BACK
  { M_BD_FACE_TOP  ,  M_BD_FACE_BOT  ,  M_BD_FACE_LEFT  ,  M_BD_FACE_RIGHT  ,
//  RIGHT             LEFT              IN                 OUT
    M_BD_FACE_FRONT,  M_BD_FACE_BACK ,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing WEST:
//  TOP               BOT               FRONT              BACK
  { M_BD_FACE_TOP  ,  M_BD_FACE_BOT  ,  M_BD_FACE_RIGHT ,  M_BD_FACE_LEFT   ,
//  RIGHT             LEFT              IN                 OUT
    M_BD_FACE_BACK ,  M_BD_FACE_FRONT,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing IN:
//  TOP               BOT               FRONT              BACK
  { M_BD_FACE_TOP  ,  M_BD_FACE_BOT  ,  M_BD_FACE_FRONT ,  M_BD_FACE_BACK   ,
//  RIGHT             LEFT              IN                 OUT
    M_BD_FACE_RIGHT,  M_BD_FACE_LEFT ,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },

// Facing OUT:
//  TOP               BOT               FRONT              BACK
  { M_BD_FACE_TOP  ,  M_BD_FACE_BOT  ,  M_BD_FACE_FRONT ,  M_BD_FACE_BACK   ,
//  RIGHT             LEFT              IN                 OUT
    M_BD_FACE_RIGHT,  M_BD_FACE_LEFT ,  M_BD_FACE_INSIDE,  M_BD_FACE_OUTSIDE },
};

/**********
 * Limits *
 **********/

// Non-solid limits:
// -----------------

// The last invisible block:
static const block_limit    BL_MAX_INVISIBLE =  0x02ff;
// The last translucent liquid block:
static const block_limit     BL_MAX_T_LIQUID =  0x04ff;

// Normal non-solid blocks go here.

// The first opaque liquid block:
static const block_limit     BL_MIN_O_LIQUID =  0x3d00;

// Solid limits:
// -------------

// The first solid block:
static const block_limit        BL_MIN_SOLID =  0x4000;

// Normal solid blocks go here.

// The first translucent block:
static const block_limit  BL_MIN_TRANSLUCENT =  0xff00;

/**********
 * Blocks *
 **********/

// Non-solid:
// ----------

// Invisible blocks:
static const block        B_VOID = 0x0000;
static const block         B_AIR = 0x0100;
static const block       B_ETHER = 0x0200;

// Translucent liquid blocks:
static const block       B_WATER = 0x0300;
static const block  B_WATER_FLOW = 0x0400;

// Visible non-solid blocks:
static const block      B_MIASMA = 0x0500;

// Opaque liquid blocks:
static const block        B_LAVA = 0x3d00;
static const block   B_LAVA_FLOW = 0x3e00;
static const block   B_QUICKSAND = 0x3f00;

// Solid:
// ------

// Opaque solid blocks:
static const block    B_BOUNDARY = 0x4000;
static const block       B_STONE = 0x4100;
static const block        B_DIRT = 0x4200;
static const block       B_GRASS = 0x4300;

// Translucent solid blocks:
static const block       B_GLASS = 0xff00;

/*************
 * Functions *
 *************/

// Conversions:

static inline block_id just_id(block b) {
  return (block_id) (b >> 8);
}

static inline block full_block(block_id id) {
  return ((block) id) << 8;
}

// Flag checks:

static inline block is_exposed(block b) {
  return b & BF_EXPOSED;
}

// Flag sets:

static inline block set_exposed(block b) {
  return b | BF_EXPOSED;
}

// Type checks:

static inline block is_translucent_liquid(block b) {
  return (
    (b > BL_MAX_INVISIBLE)
  &&
    (b <= BL_MAX_T_LIQUID)
  );
}

static inline block is_opaque_liquid(block b) {
  return (
    (b >= BL_MIN_O_LIQUID)
  &&
    (b < BL_MIN_SOLID)
  );
}

static inline block is_liquid(block b) {
  return is_translucent_liquid(b) || is_opaque_liquid(b);
}

static inline block is_solid(block b) {
  return b >= BL_MIN_SOLID;
}

static inline block is_invisible(block b) {
  return b <= BL_MAX_INVISIBLE;
}

static inline block is_translucent(block b) {
  return is_translucent_liquid(b) || b >= BL_MIN_TRANSLUCENT;
}

static inline block is_opaque(block b) {
  return (
    (b >= BL_MIN_O_LIQUID)
  &&
    (b < BL_MIN_TRANSLUCENT)
  );
}

// Comparisons:

static inline block shares_translucency(block b, block c) {
  return (b | 1) == (c | 1);
}

#endif // ifndef BLOCKS_H
