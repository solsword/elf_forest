#ifndef PMEM_H
#define PMEM_H

// pmem.h
// Profiling utilities for tracking memory.

#include <stddef.h>

/**************
 * Structures *
 **************/

struct mem_data_s;
typedef struct mem_data_s mem_data;

/***********
 * Globals *
 ***********/

// Track chunk cache data stored in RAM and on the GPU:
extern mem_data CHUNK_CACHE_RAM_USAGE;
extern mem_data CHUNK_CACHE_GPU_USAGE;

// Track the total size of texture data stored in RAM and on the GPU:
extern mem_data TEXTURE_RAM_USAGE;
extern mem_data TEXTURE_GPU_USAGE;

// Tracks data stored on disk that's been accessed by the program:
extern mem_data DISK_DATA_SEEN;

/*************************
 * Structure Definitions *
 *************************/

struct mem_data_s {
  size_t data, overhead; // Tracks data and overhead separately.
};

/*************
 * Functions *
 *************/

// Updates the data and overhead counts for the given mem_data struct, setting
// them to the given values.
void md_set_size(mem_data *md, size_t data, size_t overhead);

// Updates a mem_data struct as with update_set_size, but adds to/subtracts
// from the existing counts rather than overwriting them. When subtracting, if
// the result would be less than 0, it is set to 0 instead (and a warning is
// printed if compiled with DEBUG).
void md_add_size(mem_data *md, size_t data, size_t overhead);
void md_sub_size(mem_data *md, size_t data, size_t overhead);

// Computes the current memory in use by the chunk cache, storing the result in
// CHUNK_CACHE_RAM_USAGE and CHUNK_CACHE_GPU_USAGE.
void compute_chunk_cache_mem(void);

// TODO: Functions to compute other memory usage we're interested in.

#endif //ifndef PMEM_H
