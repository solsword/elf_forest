#ifndef PERSIST_H
#define PERSIST_H

// persist.h
// Loading from and saving to disk.

#include <stdint.h>
#include <stdio.h>

#include "world/blocks.h"
#include "world/world.h"

#include "datatypes/string.h"

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

// A block of chunk data. Contains file info for reading from/writing to disk.
struct ps_block_s;
typedef struct ps_block_s ps_block;

/*************
 * Constants *
 *************/

// 16*16*16 chunks/block * ~256 kb/chunk = ~1 GB/block
// 32*32*32 chunks/block * ~256 kb/chunk = ~8 GB/block <-
#define PS_BLOCK_BITS 5
#define PS_BLOCK_SIZE (1 << PS_BLOCK_BITS)
#define PS_BLOCK_MASK (PS_BLOCK_SIZE - 1)
#define PS_BLOCK_TOTAL_CHUNKS (PS_BLOCK_SIZE * PS_BLOCK_SIZE * PS_BLOCK_SIZE)

// This should be smaller than FOPEN_MAX, but should probably be as large as
// possible. On the other hand, as long as the loading distance is less than
// PS_BLOCK_SIZE/2 a cache size of 12 should be sufficient in most situations.
// The minimum viable cache size is 8, no matter what the loading distance is
// since when a player is near the corner of a block, they'll be loading chunks
// from eight adjacent blocks.
#define PS_BLOCK_CACHE_SIZE 12

extern char const * const BLOCKS_DIRECTORY;
extern char const * const DEFAULT_WORLD_DIRECTORY;

/*************************
 * Structure Definitions *
 *************************/

struct ps_block_pos_s {
  ps_bpos_t x, y, z;
};

struct ps_chunk_pos_s {
  ps_cpos_t x, y, z;
};

struct ps_block_s {
  uint8_t age;
  ps_block_pos pos;
  string* filename;
  FILE* file;
  uint64_t file_end;
  uint64_t indices[PS_BLOCK_TOTAL_CHUNKS];
};

/***********
 * Globals *
 ***********/

// The world directory
extern string* WORLD_DIRECTORY;

// The directory separator for this OS
extern string* DIRSEP;

// The full prefix for block filenames
extern string* PS_BLOCK_DIR_PREFIX;

// The persist block cache
extern ps_block PS_BLOCK_CACHE[];

// A bunch of zeroes used to copy into files for their indices headers
extern uint64_t EMPTY_INDICES[];

/********************
 * Inline Functions *
 ********************/

static inline void glcpos__psbpos(
  global_chunk_pos const * const glcpos,
  ps_block_pos *psbpos
) {
  psbpos->x = (ps_bpos_t) (glcpos->x >> PS_BLOCK_BITS);
  psbpos->y = (ps_bpos_t) (glcpos->y >> PS_BLOCK_BITS);
  psbpos->z = (ps_bpos_t) (glcpos->z >> PS_BLOCK_BITS);
}

static inline void glcpos__pscpos(
  global_chunk_pos const * const glcpos,
  ps_chunk_pos *pscpos
) {
  pscpos->x = (ps_cpos_t) (glcpos->x & PS_BLOCK_MASK);
  pscpos->y = (ps_cpos_t) (glcpos->y & PS_BLOCK_MASK);
  pscpos->z = (ps_cpos_t) (glcpos->z & PS_BLOCK_MASK);
}

static inline void pspos__glcpos(
  ps_block_pos const * const psbpos,
  ps_chunk_pos const * const pscpos,
  global_chunk_pos *glcpos
) {
  glcpos->x = (((gl_cpos_t) psbpos->x) << PS_BLOCK_BITS) + pscpos->x;
  glcpos->y = (((gl_cpos_t) psbpos->y) << PS_BLOCK_BITS) + pscpos->y;
  glcpos->z = (((gl_cpos_t) psbpos->z) << PS_BLOCK_BITS) + pscpos->z;
}

static inline void copy_psbpos(
  ps_block_pos const * const source,
  ps_block_pos *destination
) {
  destination->x = source->x;
  destination->y = source->y;
  destination->z = source->z;
}

static inline int psbpos_equals(
  ps_block_pos const * const a,
  ps_block_pos const * const b
) {
  return (a->x == b->x && a->y == b->y && a->z == b->z);
}

static inline uint64_t* block_chunk_index(ps_block* b, ps_chunk_pos* pscpos){
  return &(b->indices[
    pscpos->z * PS_BLOCK_SIZE * PS_BLOCK_SIZE
  + pscpos->y * PS_BLOCK_SIZE
  + pscpos->x
  ]);
}

static inline size_t block_chunk_index_offset(ps_block* b,ps_chunk_pos* pscpos){
  return (
    pscpos->z * PS_BLOCK_SIZE * PS_BLOCK_SIZE
  + pscpos->y * PS_BLOCK_SIZE
  + pscpos->x
  ) * sizeof(uint64_t);
}

/*************
 * Functions *
 *************/

// Sets up the persistence module, binding the given world directory.
void setup_persist(char const * const world_directory);

// Initializes the given block struct.
void init_block(ps_block* block);

// Creates and returns a new string holding the filename for the block at the
// given position.
string* block_filename(ps_block_pos* pos);

// Sets the given block to read from the file for the given block position,
// closing the old file if necessary and opening the new one.
void select_block(ps_block* block, ps_block_pos* pos);

// Swaps the block at the given position into the block cache, booting out the
// least-recently-used block if necessary. Returns the block cache index of the
// newly swapped-in block.
size_t block_cache_swap(ps_block_pos* bpos);

// Looks up the correct block for the given chunk and loads the chunk data into
// the chunk. This may result in a block cache swap. Returns 1 on success and 0
// on failure (i.e., if the chunk data isn't stored on disk).
int load_chunk_data(chunk* chunk);

// Loads a chunk from a particular position in a particular block. Returns 1 on
// success and 0 on failure.
int load_chunk_from_block(ps_block* block, ps_chunk_pos* cpos, chunk* chunk);

// Writes the data from the given chunk out to file. Might force a block cache
// swap.
void persist_chunk(chunk* chunk);

// Writes the chunk data to the file connected to the given block.
void persist_chunk_in_block(ps_block* block, ps_chunk_pos* cpos, chunk* chunk);

#endif // ifndef PERSIST_H
