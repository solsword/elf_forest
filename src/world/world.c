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
//#include "gen/terrain.h"

/***********
 * Globals *
 ***********/

// TODO: Get rid of this
//frame MAIN_FRAME;

/**********
 * Macros *
 **********/

// Macros for defining a get/put_a_block/flag functions at different scales:
#define CA_GET_BLOCK_DEF(BITS) \
  CA_GET_BLOCK_SIG(CA_GET_BLOCK_FN(BITS)) { \
    return ca->data->d ## BITS.blocks[ \
      ((idx->x & CH_MASK) >> BITS) + \
      (((idx->y & CH_MASK) >> BITS) << (CHUNK_BITS - BITS)) + \
      (((idx->z & CH_MASK) >> BITS) << ((CHUNK_BITS - BITS)*2)) \
    ]; \
  }

#define CA_PUT_BLOCK_DEF(BITS) \
  CA_PUT_BLOCK_SIG(CA_PUT_BLOCK_FN(BITS)) { \
    ca->data->d ## BITS.blocks[ \
      ((idx->x & CH_MASK) >> BITS) + \
      (((idx->y & CH_MASK) >> BITS) << (CHUNK_BITS - BITS)) + \
      (((idx->z & CH_MASK) >> BITS) << ((CHUNK_BITS - BITS)*2)) \
    ] = b; \
  }

#define CA_GET_FLAGS_DEF(BITS) \
  CA_GET_FLAGS_SIG(CA_GET_FLAGS_FN(BITS)) { \
    return ca->data->d ## BITS.block_flags[ \
      ((idx->x & CH_MASK) >> BITS) + \
      (((idx->y & CH_MASK) >> BITS) << (CHUNK_BITS - BITS)) + \
      (((idx->z & CH_MASK) >> BITS) << ((CHUNK_BITS - BITS)*2)) \
    ]; \
  }

#define CA_PUT_FLAGS_DEF(BITS) \
  CA_PUT_FLAGS_SIG(CA_PUT_FLAGS_FN(BITS)) { \
    ca->data->d ## BITS.block_flags[ \
      ((idx->x & CH_MASK) >> BITS) + \
      (((idx->y & CH_MASK) >> BITS) << (CHUNK_BITS - BITS)) + \
      (((idx->z & CH_MASK) >> BITS) << ((CHUNK_BITS - BITS)*2)) \
    ] = flags; \
  }

#define CA_SET_FLAGS_DEF(BITS) \
  CA_SET_FLAGS_SIG(CA_SET_FLAGS_FN(BITS)) { \
    ca->data->d ## BITS.block_flags[ \
      ((idx->x & CH_MASK) >> BITS) + \
      (((idx->y & CH_MASK) >> BITS) << (CHUNK_BITS - BITS)) + \
      (((idx->z & CH_MASK) >> BITS) << ((CHUNK_BITS - BITS)*2)) \
    ] |= flags; \
  }

#define CA_CLEAR_FLAGS_DEF(BITS) \
  CA_CLEAR_FLAGS_SIG(CA_CLEAR_FLAGS_FN(BITS)) { \
    ca->data->d ## BITS.block_flags[ \
      ((idx->x & CH_MASK) >> BITS) + \
      (((idx->y & CH_MASK) >> BITS) << (CHUNK_BITS - BITS)) + \
      (((idx->z & CH_MASK) >> BITS) << ((CHUNK_BITS - BITS)*2)) \
    ] &= ~flags; \
  }


/******************************
 * Constructors & Destructors *
 ******************************/

/* TODO: Get rid of this
void setup_frame(frame *f, region_chunk_pos *roff) {
  frame_chunk_index idx;
  region_chunk_pos rpos;
  f->chunk_offset.x = 0;
  f->chunk_offset.y = 0;
  f->chunk_offset.z = 0;
  f->region_offset.x = roff->x;
  f->region_offset.y = roff->y;
  f->region_offset.z = roff->z;
  for (idx.x = 0; idx.x < FRAME_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < FRAME_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < FRAME_SIZE; ++idx.z) {
        fcidx__rcpos(&idx, f, &rpos);
        setup_chunk(chunk_at(f, idx), &rpos);
      }
    }
  }
  f->entities = create_list();
  f->oct = setup_octree(FULL_FRAME);
}

void cleanup_frame(frame *f) {
  frame_chunk_index idx;
  for (idx.x = 0; idx.x < FRAME_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < FRAME_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < FRAME_SIZE; ++idx.z) {
        cleanup_chunk(chunk_at(f, idx));
      }
    }
  }
  destroy_list(f->entities);
  cleanup_octree(f->oct);
}
*/

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
  c->block_entities = create_list();
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
  destroy_list(c->block_entities);
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

// Macro-based definitions of the various approximate block manipulation
// routines:
CA_GET_BLOCK_DEF(1)
CA_GET_BLOCK_DEF(2)
CA_GET_BLOCK_DEF(3)
CA_GET_BLOCK_DEF(4)

CA_PUT_BLOCK_DEF(1)
CA_PUT_BLOCK_DEF(2)
CA_PUT_BLOCK_DEF(3)
CA_PUT_BLOCK_DEF(4)

CA_GET_FLAGS_DEF(1)
CA_GET_FLAGS_DEF(2)
CA_GET_FLAGS_DEF(3)
CA_GET_FLAGS_DEF(4)

CA_PUT_FLAGS_DEF(1)
CA_PUT_FLAGS_DEF(2)
CA_PUT_FLAGS_DEF(3)
CA_PUT_FLAGS_DEF(4)

CA_SET_FLAGS_DEF(1)
CA_SET_FLAGS_DEF(2)
CA_SET_FLAGS_DEF(3)
CA_SET_FLAGS_DEF(4)

CA_CLEAR_FLAGS_DEF(1)
CA_CLEAR_FLAGS_DEF(2)
CA_CLEAR_FLAGS_DEF(3)
CA_CLEAR_FLAGS_DEF(4)

// Macro-based function table definitions for the above functions:
DECLARE_APPROX_FN_VARIANTS_TABLE(CA_GET_BLOCK_SIG, CA_GET_BLOCK_FN)
DECLARE_APPROX_FN_VARIANTS_TABLE(CA_PUT_BLOCK_SIG, CA_PUT_BLOCK_FN)

DECLARE_APPROX_FN_VARIANTS_TABLE(CA_GET_FLAGS_SIG, CA_GET_FLAGS_FN)
DECLARE_APPROX_FN_VARIANTS_TABLE(CA_PUT_FLAGS_SIG, CA_PUT_FLAGS_FN)
DECLARE_APPROX_FN_VARIANTS_TABLE(CA_SET_FLAGS_SIG, CA_SET_FLAGS_FN)
DECLARE_APPROX_FN_VARIANTS_TABLE(CA_CLEAR_FLAGS_SIG, CA_CLEAR_FLAGS_FN)


uint8_t BLOCK_AT_SALT = 0;
block block_at(region_pos const * const rpos) {
  region_chunk_pos rcpos;
  static region_chunk_pos last_rcpos = { .x = 0, .y = 0, .z = 0 };
  chunk_or_approx coa;
  static chunk_or_approx last_coa = { .type=CA_TYPE_NOT_LOADED, .ptr=NULL };
  chunk_index cidx;
  static uint8_t last_salt = 1;

  rpos__rcpos(rpos, &rcpos);
  rpos__cidx(rpos, &cidx);
  if (
    last_salt == BLOCK_AT_SALT
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
  last_salt = BLOCK_AT_SALT;
  if (coa.type == CA_TYPE_CHUNK) {
    return c_get_block((chunk *) (coa.ptr), cidx);
  } else if (coa.type == CA_TYPE_APPROXIMATION) {
    return ca_get_block((chunk_approximation *) (coa.ptr), cidx);
  }
  return B_VOID;
}
