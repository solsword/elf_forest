// pmem.c
// Profiling utilities for tracking memory.

#include <stdlib.h>
#include <stdio.h>

#include "pmem.h"

#include "world/world.h"
#include "data/data.h"

/***********
 * Globals *
 ***********/

mem_data CHUNK_CACHE_RAM_USAGE;
mem_data CHUNK_CACHE_GPU_USAGE;

mem_data TEXTURE_RAM_USAGE;
mem_data TEXTURE_GPU_USAGE;

mem_data DISK_DATA_SEEN;

/*********************
 * Private Functions *
 *********************/

// Used for counting chunk/approx sizes in a list or map, given a data
// destination as the second argument:
void count_chunk_size(void *chunk_handle, void *data_handle) {
  chunk *c = (chunk *) chunk_handle;
  mem_data *md = (mem_data *) data_handle;
  md_add_size(md, chunk_data_size(c), chunk_overhead_size(c));
}
void count_chunk_approx_size(void *chunk_approx_handle, void *data_handle) {
  chunk_approximation *ca = (chunk_approximation *) chunk_approx_handle;
  mem_data *md = (mem_data *) data_handle;
  md_add_size(
    md,
    chunk_approx_data_size(ca),
    chunk_approx_overhead_size(ca)
  );
}
void count_chunk_gpu_size(void *chunk_handle, void *data_handle) {
  chunk *c = (chunk *) chunk_handle;
  mem_data *md = (mem_data *) data_handle;
  md_add_size(md, chunk_gpu_size(c), 0);
}
void count_chunk_approx_gpu_size(void *chunk_approx_handle, void *data_handle) {
  chunk_approximation *ca = (chunk_approximation *) chunk_approx_handle;
  mem_data *md = (mem_data *) data_handle;
  md_add_size(md, chunk_approx_gpu_size(ca), 0);
}

/*************
 * Functions *
 *************/

void md_set_size(mem_data *md, size_t data, size_t overhead) {
  md->data = data;
  md->overhead = overhead;
}

void md_add_size(mem_data *md, size_t data, size_t overhead) {
  md->data += data;
  md->overhead += overhead;
}

void md_sub_size(mem_data *md, size_t data, size_t overhead) {
  if (data > md->data) {
    md->data = 0;
#ifdef DEBUG
    fprintf(
      stderr,
      "Warning: attempt to subtract more data than recorded.\n"
    );
#endif
  } else {
    md->data -= data;
  }
  if (overhead > md->overhead) {
    md->overhead = 0;
#ifdef DEBUG
    fprintf(
      stderr,
      "Warning: attempt to subtract more overhead than recorded.\n"
    );
#endif
  } else {
    md->overhead -= overhead;
  }
}

void compute_chunk_cache_mem(void) {
  lod i;
  md_set_size(&CHUNK_CACHE_RAM_USAGE, 0, 0);
  md_set_size(&CHUNK_CACHE_GPU_USAGE, 0, 0);
  md_add_size(&CHUNK_CACHE_RAM_USAGE, 0, sizeof(chunk_queue_set)*2);
  md_add_size(&CHUNK_CACHE_RAM_USAGE, 0, sizeof(chunk_cache));
  for (i = LOD_BASE; i < N_LODS; ++i) {
    queue *lq = LOAD_QUEUES->levels[i];
    map *lm = LOAD_QUEUES->maps[i];
    queue *cq = COMPILE_QUEUES->levels[i];
    map *cm = LOAD_QUEUES->maps[i];
    map *ccm = CHUNK_CACHE->levels[i];
    md_add_size(
      &CHUNK_CACHE_RAM_USAGE,
      0,
      q_data_size(lq) + q_overhead_size(lq) + m_overhead_size(lm) +\
      q_data_size(cq) + q_overhead_size(cq) + m_overhead_size(cm)
    );
    md_add_size(
      &CHUNK_CACHE_RAM_USAGE,
      0,
      m_data_size(ccm) + m_overhead_size(ccm)
    );
    if (i == LOD_BASE) {
      q_witheach(lq, &CHUNK_CACHE_RAM_USAGE, count_chunk_size);
      q_witheach(cq, &CHUNK_CACHE_RAM_USAGE, count_chunk_size);
      m_witheach(ccm, &CHUNK_CACHE_RAM_USAGE, count_chunk_size);
      m_witheach(ccm, &CHUNK_CACHE_GPU_USAGE, count_chunk_gpu_size);
    } else {
      q_witheach(lq, &CHUNK_CACHE_RAM_USAGE, count_chunk_approx_size);
      q_witheach(cq, &CHUNK_CACHE_RAM_USAGE, count_chunk_approx_size);
      m_witheach(ccm, &CHUNK_CACHE_RAM_USAGE, count_chunk_approx_size);
      m_witheach(ccm, &CHUNK_CACHE_GPU_USAGE, count_chunk_approx_gpu_size);
    }
  }
}
