// world.c
// Structures and functions for representing the world.

#include <stdint.h>
#include <stdlib.h>
// DEBUG
#include <stdio.h>

#include "blocks.h"
#include "world.h"

#include "datatypes/octree.h"
#include "data/data.h"

/**********
 * Macros *
 **********/

// Macros for defining a get/paste_cell functions at different scales:
#define CA_CELL_DEF(BITS) \
  CA_CELL_SIG(CA_CELL_FN(BITS)) { \
    return &(ca->data->d ## BITS.cells[ \
      ((idx->x & CH_MASK) >> BITS) + \
      (((idx->y & CH_MASK) >> BITS) << (CHUNK_BITS - BITS)) + \
      (((idx->z & CH_MASK) >> BITS) << ((CHUNK_BITS - BITS)*2)) \
    ]); \
  }

#define CA_PASTE_CELL_DEF(BITS) \
  CA_PASTE_CELL_SIG(CA_PASTE_CELL_FN(BITS)) { \
    cell *dst = CA_CELL_FN(BITS)(ca, idx); \
    copy_cell(cl, dst); \
  }

/******************************
 * Constructors & Destructors *
 ******************************/

chunk * create_chunk(region_chunk_pos const * const rcpos) {
  chunk *c = (chunk *) malloc(sizeof(chunk));
  c->type = CA_TYPE_CHUNK;
  c->rcpos.x = rcpos->x;
  c->rcpos.y = rcpos->y;
  c->rcpos.z = rcpos->z;
  c->chunk_flags = 0;
  layer ly;
  for (ly = 0; ly < N_LAYERS; ++ly) {
    setup_vertex_buffer(&(c->layers[ly]));
  }
  c->cell_entities = create_list();
  return c;
}

void cleanup_chunk(chunk *c) {
#ifdef DEBUG
  if (c->type != CA_TYPE_CHUNK) {
    fprintf(stderr, "Error: bad chunk in cleanup_chunk.\n");
    exit(1);
  }
#endif
  layer ly;
  for (ly = 0; ly < N_LAYERS; ++ly) {
    cleanup_vertex_buffer(&(c->layers[ly]));
  }
  destroy_list(c->cell_entities);
  free(c);
}

chunk_approximation * create_chunk_approximation(
  region_chunk_pos *rcpos,
  lod detail
) {
  chunk_approximation *ca = (chunk_approximation *) malloc(
    sizeof(chunk_approximation)
  );
  ca->type = CA_TYPE_APPROXIMATION;
  ca->rcpos.x = rcpos->x;
  ca->rcpos.y = rcpos->y;
  ca->rcpos.z = rcpos->z;
  ca->chunk_flags = 0;
  layer ly;
  for (ly = 0; ly < N_LAYERS; ++ly) {
    setup_vertex_buffer(&(ca->layers[ly]));
  }
  ca->detail = detail;
  if (detail == LOD_HALF) {
    ca->data = (approx_data *) malloc(sizeof(APPROX_DATA_STRUCT(1)));
  } else if (detail == LOD_QUARTER) {
    ca->data = (approx_data *) malloc(sizeof(APPROX_DATA_STRUCT(2)));
  } else if (detail == LOD_EIGHTH) {
    ca->data = (approx_data *) malloc(sizeof(APPROX_DATA_STRUCT(3)));
  } else if (detail == LOD_SIXTEENTH) {
    ca->data = (approx_data *) malloc(sizeof(APPROX_DATA_STRUCT(4)));
  }
  return ca;
}

void cleanup_chunk_approximation(chunk_approximation *ca) {
#ifdef DEBUG
  if (ca->type != CA_TYPE_APPROXIMATION) {
    fprintf(stderr, "Error: bad approx in cleanup_chunk_approximation.\n");
    exit(1);
  }
#endif
  layer ly;
  free(ca->data);
  for (ly = 0; ly < N_LAYERS; ++ly) {
    cleanup_vertex_buffer(&(ca->layers[ly]));
  }
  free(ca);
}

/*************
 * Functions *
 *************/

// Macro-based definitions of the various approximate cell manipulation
// routines:
CA_CELL_DEF(1)
CA_CELL_DEF(2)
CA_CELL_DEF(3)
CA_CELL_DEF(4)

CA_PASTE_CELL_DEF(1)
CA_PASTE_CELL_DEF(2)
CA_PASTE_CELL_DEF(3)
CA_PASTE_CELL_DEF(4)

// Macro-based function table definitions for the above functions:
DECLARE_APPROX_FN_VARIANTS_TABLE(CA_CELL_SIG, CA_CELL_FN)
DECLARE_APPROX_FN_VARIANTS_TABLE(CA_PASTE_CELL_SIG, CA_PASTE_CELL_FN)


uint8_t CELL_AT_SALT = 0;
cell* cell_at(region_pos const * const rpos) {
  region_chunk_pos rcpos;
  static region_chunk_pos last_rcpos = { .x = 0, .y = 0, .z = 0 };
  chunk_or_approx coa;
  static chunk_or_approx last_coa = { .type=CA_TYPE_NOT_LOADED, .ptr=NULL };
  chunk_index cidx;
  static uint8_t last_salt = 1;

  rpos__rcpos(rpos, &rcpos);
  rpos__cidx(rpos, &cidx);
  if (
    last_salt == CELL_AT_SALT
  &&
    last_coa.type != CA_TYPE_NOT_LOADED
  &&
    last_rcpos.x == rcpos.x
  &&
    last_rcpos.y == rcpos.y
  &&
    last_rcpos.z == rcpos.z
  ) {
    // We can use the cached chunk pointer!
    coa.type = last_coa.type;
    coa.ptr = last_coa.ptr;
  } else {
    // We need to recompute our chunk pointer.
    get_best_data(&rcpos, &coa);
    last_coa.type = coa.type;
    last_coa.ptr = coa.ptr;
  }
  copy_rcpos(&rcpos, &last_rcpos);
  last_salt = CELL_AT_SALT;
  if (coa.type == CA_TYPE_CHUNK) {
    return c_cell((chunk *) (coa.ptr), cidx);
  } else if (coa.type == CA_TYPE_APPROXIMATION) {
    return ca_cell((chunk_approximation *) (coa.ptr), cidx);
  }
  return NULL;
}

size_t chunk_data_size(chunk *c) {
  return sizeof(cell) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
}

size_t chunk_overhead_size(chunk *c) {
  return sizeof(chunk) - chunk_data_size(c);
}

size_t chunk_gpu_size(chunk *c) {
  // TODO: HERE!
  return 0;
}

size_t chunk_approx_data_size(chunk_approximation *ca) {
  if (ca->detail == LOD_BASE) {
    fprintf(stderr, "Error: found approx with base LOD.\n");
    exit(1);
  } else if (ca->detail == LOD_HALF) {
    return sizeof(approx_data_1);
  } else if (ca->detail == LOD_QUARTER) {
    return sizeof(approx_data_2);
  } else if (ca->detail == LOD_EIGHTH) {
    return sizeof(approx_data_3);
  } else if (ca->detail == LOD_SIXTEENTH) {
    return sizeof(approx_data_4);
  } else {
    fprintf(
      stderr,
      "Error: found approx with bad detail level: %d.\n",
      ca->detail
    );
    exit(1);
  }
}

size_t chunk_approx_overhead_size(chunk_approximation *ca) {
  return sizeof(chunk_approximation);
}

size_t chunk_approx_gpu_size(chunk_approximation *ca) {
  // TODO: HERE!
  return 0;
}

