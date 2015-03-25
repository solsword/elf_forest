// persist.c
// Loading from and saving to disk.


#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "util.h"

#include "datatypes/string.h"
#include "world/blocks.h"
#include "world/world.h"

#include "persist.h"

/*************
 * Constants *
 *************/

char const * const BLOCKS_DIRECTORY = "blocks";
char const * const DEFAULT_WORLD_DIRECTORY = "world";

/***********
 * Globals *
 ***********/

string* WORLD_DIRECTORY;

string* DIRSEP;

string* PS_BLOCK_DIR_PREFIX;

ps_block PS_BLOCK_CACHE[PS_BLOCK_CACHE_SIZE];

uint64_t EMPTY_INDICES[PS_TOTAL_CHUNKS];

/*************
 * Functions *
 *************/

void setup_persist(char const * const world_directory) {
  size_t i;
  // Convert some char*s to strings and set up the PS_BLOCK_DIR_PREFIX string:
  WORLD_DIRECTORY = create_string_from_ntchars(world_directory);
  if (s_contains_nul(WORLD_DIRECTORY)) {
    perror("World directory string contains NUL!");
    exit(-1);
  }
  DIRSEP = create_string_from_ntchars("/"); // TODO: OS-specific branches
  string* rdir = create_string_from_ntchars(BLOCKS_DIRECTORY);
  PS_BLOCK_DIR_PREFIX = s_join(DIRSEP, WORLD_DIRECTORY, rdir, NULL);
  cleanup_string(rdir);
  s_append(PS_BLOCK_DIR_PREFIX, DIRSEP);

  // Initialize the block arrays:
  for (i = 0; i < PS_BLOCK_CACHE_SIZE; ++i) {
    init_block(&(PS_BLOCK_CACHE[i]));
  }

  // Initialize the empty indices array:
  memset((void*) EMPTY_INDICES, 0, sizeof(uint64_t)*PS_TOTAL_CHUNKS);
}

void init_block(ps_block* block) {
  block->filename = NULL;
  block->file = NULL;
  block->age = 0;
  block->file_end = 0;
  memset((void*) &(block->indices), 0, sizeof(uint64_t)*PS_TOTAL_CHUNKS);
}

string* block_filename(ps_block_pos* pos) {
  string* result;
  string* s = s_sprintf("b%u-%u-%u.efb", pos->x, pos->y, pos->z);
  result = s_concat(PS_BLOCK_DIR_PREFIX, s);
  cleanup_string(s);
  return result;
}

void select_block(ps_block* block, ps_block_pos* pos) {
  char* encoded_filename;
  size_t i;

  // Cleanup old resources if they were already allocated:
  if (block->filename != NULL) {
    cleanup_string(block->filename);
  }
  if (block->file != NULL) {
    fclose(block->file);
    block->file = NULL;
  }

  // Copy in new filename and position information & reset the age:
  copy_psbpos(pos, &(block->pos));
  block->filename = block_filename(pos);
  block->age = 0;

  // Encode our filename into the user's encoding & open the file:
  encoded_filename = s_encode_nt(block->filename); 
  if (encoded_filename == NULL) {
    perror("Failed to encode block filename.");
    exit(errno);
  }
  if (0 == access(encoded_filename, F_OK)) {
    block->file = fopen(encoded_filename, "r+b");

    // Load the block index information:
    fread(
      (void*) (&(block->indices)),
      sizeof(uint64_t),
      PS_TOTAL_CHUNKS,
      block->file
    );

    // Convert endianness only if we need to (this is a bit expensive):
    if (IS_LITTLE_ENDIAN) {
      for (i = 0; i < PS_TOTAL_CHUNKS; ++i) {
        block->indices[i] = ntoh64(block->indices[i]);
      }
    }
  } else {
    block->file = fopen(encoded_filename, "w+b");

    // Create the indices table & put zeroes in our indices as well:
    fwrite((void*)EMPTY_INDICES,sizeof(uint64_t), PS_TOTAL_CHUNKS, block->file);
    memset((void*) &(block->indices), 0, sizeof(uint64_t)*PS_TOTAL_CHUNKS);
  }

  // Free the encoded filename:
  free(encoded_filename);

  // Store the end-of-file offset:
  fseek(block->file, 0, SEEK_END);
  block->file_end = ftell(block->file);
  fseek(block->file, 0, SEEK_SET);
}

size_t block_cache_swap(ps_block_pos* bpos) {
  size_t i;
  size_t max_age = 0;
  size_t max_index = 0;
  for (i = 0; i < PS_BLOCK_CACHE_SIZE; ++i) {
    if (PS_BLOCK_CACHE[i].file == NULL) {
      // unallocated cache slots will always be the oldest
      PS_BLOCK_CACHE[i].age += 2*PS_BLOCK_CACHE_SIZE;
    } else {
      PS_BLOCK_CACHE[i].age += 1;
    }
    if (PS_BLOCK_CACHE[i].age > max_age) {
      max_age = PS_BLOCK_CACHE[i].age;
      max_index = i;
    }
  }
  select_block(&(PS_BLOCK_CACHE[max_index]), bpos);
  return max_index;
}

int load_chunk_data(chunk* chunk) {
  ps_block_pos bpos;
  ps_chunk_pos cpos;
  size_t i;
  glcpos__psbpos(&(chunk->glcpos), &bpos);
  glcpos__pscpos(&(chunk->glcpos), &cpos);
  for (i = 0; i < PS_BLOCK_CACHE_SIZE; ++i) {
    if (psbpos_equals(&bpos, &(PS_BLOCK_CACHE[i].pos))) {
      return load_chunk_from_block(&(PS_BLOCK_CACHE[i]), &cpos, chunk);
    }
  }
  i = block_cache_swap(&bpos);
  return load_chunk_from_block(&(PS_BLOCK_CACHE[i]), &cpos, chunk);
}

int load_chunk_from_block(ps_block* block, ps_chunk_pos* cpos, chunk* chunk) {
  // TODO: optimize pure-air/pure-water chunks?
  uint64_t* offset = block_chunk_index(block, cpos);
  if (*offset == 0) { // we have no data for this chunk!
    return 0;
  }
  fseek(block->file, *offset, SEEK_SET);
  fread(
    (void*) (&(chunk->cells)),
    sizeof(cell),
    TOTAL_CHUNK_CELLS,
    block->file
  );
  // TODO: How to load entities?!?
  return 1;
}

void persist_chunk(chunk* chunk) {
  ps_block_pos bpos;
  ps_chunk_pos cpos;
  size_t i;
  glcpos__psbpos(&(chunk->glcpos), &bpos);
  glcpos__pscpos(&(chunk->glcpos), &cpos);
  for (i = 0; i < PS_BLOCK_CACHE_SIZE; ++i) {
    if (psbpos_equals(&bpos, &(PS_BLOCK_CACHE[i].pos))) {
      return persist_chunk_in_block(&(PS_BLOCK_CACHE[i]), &cpos, chunk);
    }
  }
  i = block_cache_swap(&bpos);
  return persist_chunk_in_block(&(PS_BLOCK_CACHE[i]), &cpos, chunk);
}

void persist_chunk_in_block(ps_block* block, ps_chunk_pos* cpos, chunk* chunk) {
  // TODO: optimize pure-air/pure-water chunks?
  uint64_t* offset = block_chunk_index(block, cpos);
  if (*offset == 0) { // we had no data for this chunk: create some
    // Set our offset in memory to point to the current end of the file:
    *offset = block->file_end;
    // Seek to the index entry in the file and write the new index entry:
    fseek(block->file, block_chunk_index_offset(block, cpos), SEEK_SET);
    *offset = hton64(*offset); // swizzle if necessary before copying into file
    fwrite((void*) offset, sizeof(uint64_t), 1, block->file);
    *offset = ntoh64(*offset); // swizzle back
    // Add to our file end pointer:
    block->file_end += sizeof(cell) * TOTAL_CHUNK_CELLS;
    // Seek to the end of the file
    fseek(block->file, 0, SEEK_END);
  } else {
    // Seek to offset for this chunk
    fseek(block->file, *offset, SEEK_SET);
  }
  // Write out our cell data:
  fwrite(
    (void*) (&(chunk->cells)),
    sizeof(cell),
    TOTAL_CHUNK_CELLS,
    block->file
  );
  // TODO: How to store entities?!?
}