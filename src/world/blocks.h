#ifndef BLOCKS_H
#define BLOCKS_H

// blocks.h
// Defines block types.

#include <stdint.h>

#include "util.h"

/************
 * Typedefs *
 ************/

typedef uint16_t block;
typedef uint8_t block_id;
typedef uint16_t block_count;
typedef uint16_t block_range;
typedef uint8_t block_flag;
typedef uint16_t block_data;
typedef uint16_t block_limit;

/*************
 * Constants *
 *************/

static block_count const BLOCK_TYPE_COUNT = 256;

/**********
 * Ranges *
 **********/

static block_range const  BR_FLAGS = 0x00c0;
static block_range const   BR_DATA = 0x003f;
static block_range const     BR_ID = 0xff00;

/*********
 * Flags *
 *********/

// Flag shifts:
// ------------

// The ability to turn a flag read into a 1 or 0 (instead of an N or 0) is
// useful sometimes. Flag shifts tell you how many bits to shift (and
// simultaneously are used to define the flag itself). Note that dynamic block
// flags are stored separately from normal block data/ids.

#define  BFS_EXPOSED_ABOVE_SHIFT 0
#define  BFS_EXPOSED_BELOW_SHIFT 1
#define  BFS_EXPOSED_NORTH_SHIFT 2
#define  BFS_EXPOSED_SOUTH_SHIFT 3
#define   BFS_EXPOSED_EAST_SHIFT 4
#define   BFS_EXPOSED_WEST_SHIFT 5

#define     BFS_ORIENTABLE_SHIFT 6
#define     BFS_HAS_ENTITY_SHIFT 7

// Dynamic flags:
// --------------

// Block exposure:
static block_flag const  BF_EXPOSED_ABOVE = 1 << BFS_EXPOSED_ABOVE_SHIFT;
static block_flag const  BF_EXPOSED_BELOW = 1 << BFS_EXPOSED_BELOW_SHIFT;
static block_flag const  BF_EXPOSED_NORTH = 1 << BFS_EXPOSED_NORTH_SHIFT;
static block_flag const  BF_EXPOSED_SOUTH = 1 << BFS_EXPOSED_SOUTH_SHIFT;
static block_flag const   BF_EXPOSED_EAST = 1 << BFS_EXPOSED_EAST_SHIFT;
static block_flag const   BF_EXPOSED_WEST = 1 << BFS_EXPOSED_WEST_SHIFT;

static block_flag const BF_EXPOSED_ANY = 
  (1 << BFS_EXPOSED_ABOVE_SHIFT) |
  (1 << BFS_EXPOSED_BELOW_SHIFT) |
  (1 << BFS_EXPOSED_NORTH_SHIFT) |
  (1 << BFS_EXPOSED_SOUTH_SHIFT) |
  (1 << BFS_EXPOSED_EAST_SHIFT) |
  (1 << BFS_EXPOSED_WEST_SHIFT);

static block_flag const BF_ALL_FLAGS = umaxof(block_flag);

// Static flags:
// -------------

// Is this block orientable?
static block_flag const  BF_ORIENTABLE = 1 << BFS_ORIENTABLE_SHIFT;
// Does this block have an entity associated with it?
static block_flag const  BF_HAS_ENTITY = 1 << BFS_HAS_ENTITY_SHIFT;

/********
 * Data *
 ********/

// Note: interpretation of block data depends on the block type, hence the
// multiple overlapping values.

// Orientations:
static block_data const        BD_ORI_UP = 0x0000;
static block_data const      BD_ORI_DOWN = 0x0001;
static block_data const     BD_ORI_NORTH = 0x0002;
static block_data const     BD_ORI_SOUTH = 0x0003;
static block_data const      BD_ORI_EAST = 0x0004;
static block_data const      BD_ORI_WEST = 0x0005;
                            
static block_data const        BD_ORI_IN = 0x0006;
static block_data const       BD_ORI_OUT = 0x0007;

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
static block_data const      BD_FACE_TOP = 0x0000;
static block_data const      BD_FACE_BOT = 0x0001;
                          
static block_data const    BD_FACE_FRONT = 0x0002;
static block_data const     BD_FACE_BACK = 0x0003;
                          
static block_data const    BD_FACE_RIGHT = 0x0004;
static block_data const     BD_FACE_LEFT = 0x0005;
                          
static block_data const   BD_FACE_INSIDE = 0x0006;
static block_data const  BD_FACE_OUTSIDE = 0x0007;

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

static block_data const ROTATE_FACE[8][8] = {
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
static block_limit const    BL_MAX_INVISIBLE =  0x02ff;
// The last translucent liquid block:
static block_limit const     BL_MAX_T_LIQUID =  0x04ff;

// Normal non-solid blocks go here.

// The first opaque liquid block:
static block_limit const     BL_MIN_O_LIQUID =  0x3d00;

// Solid limits:
// -------------

// The first solid block:
static block_limit const        BL_MIN_SOLID =  0x4000;

// Normal solid blocks go here.

// The first (partially) transparent block:
static block_limit const  BL_MIN_TRANSPARENT =  0xfd00;

/**********
 * Blocks *
 **********/

// Non-solid:
// ----------

// Invisible blocks:
static block const        B_VOID = 0x0000; // used for invalid/missing blocks
static block const         B_AIR = 0x0100;
static block const       B_ETHER = 0x0200;

// Translucent liquid blocks:
static block const       B_WATER = 0x0300;
static block const  B_WATER_FLOW = 0x0400;

// Visible non-solid blocks:
static block const      B_MIASMA = 0x0500;

// Opaque liquid blocks:
static block const        B_LAVA = 0x3d00;
static block const   B_LAVA_FLOW = 0x3e00;
static block const   B_QUICKSAND = 0x3f00;

// Solid:
// ------

// Opaque solid blocks:
static block const    B_BOUNDARY = 0x4000;
static block const       B_STONE = 0x4100;
static block const        B_DIRT = 0x4200;
static block const       B_GRASS = 0x4300;
static block const        B_SAND = 0x4400;
static block const       B_TRUNK = 0x4500;

// Translucent solid blocks:
static block const    B_BRANCHES = 0xfd00;
static block const      B_LEAVES = 0xfe00;
static block const       B_GLASS = 0xff00;

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

// Comparisons:

static inline block block_is(block b, block c) {
  return just_id(b) == just_id(c);
}

static inline block shares_translucency(block b, block c) {
  return (
    (just_id(b) | 1) == (just_id(c) | 1)
  &&
    !block_is(b, B_LEAVES)
  );
}

// Type checks:

static inline block is_void(block b) {
  return block_is(b, B_VOID);
}

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
  return is_translucent_liquid(b);
}

static inline block is_transparent(block b) {
  return b >= BL_MIN_TRANSPARENT;
}

static inline block is_opaque(block b) {
  return (
    (b >= BL_MIN_O_LIQUID)
  &&
    (b < BL_MIN_TRANSPARENT)
  );
}

#endif // ifndef BLOCKS_H
