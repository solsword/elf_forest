#ifndef PERSIST_H
#define PERSIST_H

// persist.h
// Loading from and saving to disk.

#include <stdint.h>
#include <stdio.h>

#include "world/blocks.h"
#include "world/world.h"
#include "world/chunk_data.h"

/************************
 * Types and Structures *
 ************************/

// Type for persist block global coordinates.
typedef uint16_t ps_bpos_t;

// Type for within-persist-block coordinates.
typedef uint8_t ps_cpos_t;

// Position within the world in terms of persist blocks.
struct ps_block_pos_s;
typedef struct ps_block_pos_s ps_block_pos;

// Position of a chunk within a persist block.
struct ps_chunk_pos_s;
typedef struct ps_chunk_pos_s ps_chunk_pos;

/*************
 * Constants *
 *************/

#define PS_BLOCK_BITS 5
#define PS_BLOCK_SIZE (1 << PS_BLOCK_BITS)
#define PS_BLOCK_MASK (PS_BLOCK_SIZE - 1)

/***********
 * Globals *
 ***********/

/*************************
 * Structure Definitions *
 *************************/

struct ps_block_pos_s {
  ps_bpos_t x, y, z;
}

struct ps_chunk_pos_s {
  ps_cpos_t x, y, z;
}

/********************
 * Inline Functions *
 ********************/

static inline glcpos__psbpos(
  global_chunk_pos const * const glcpos,
  ps_block_pos *psbpos
) {
  psbpos->x = (ps_pos_t) (glcpos->x >> PS_BLOCK_BITS);
  psbpos->y = (ps_pos_t) (glcpos->y >> PS_BLOCK_BITS);
  psbpos->z = (ps_pos_t) (glcpos->z >> PS_BLOCK_BITS);
}

static inline glcpos__pscpos(
  global_chunk_pos const * const glcpos,
  ps_chunk_pos *pscpos
) {
  pscpos->x = (ps_cpos_t) (glcpos->x & PS_BLOCK_MASK);
  pscpos->y = (ps_cpos_t) (glcpos->y & PS_BLOCK_MASK);
  pscpos->z = (ps_cpos_t) (glcpos->z & PS_BLOCK_MASK);
}

static inline pspos__glcpos(
  ps_block_pos const * const psbpos,
  ps_chunk_pos const * const pscpos,
  global_chunk_pos *glcpos
) {
  glcpos->x = (((gl_cpos_t) psbpos->x) << PS_BLOCK_BITS) + pscpos->x;
  glcpos->y = (((gl_cpos_t) psbpos->y) << PS_BLOCK_BITS) + pscpos->y;
  glcpos->z = (((gl_cpos_t) psbpos->z) << PS_BLOCK_BITS) + pscpos->z;
}

/*************
 * Functions *
 *************/

#endif // ifndef PERSIST_H
