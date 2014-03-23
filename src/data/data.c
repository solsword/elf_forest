// data.c
// Loading from and saving to disk.

#include <stdint.h>
#include <stdio.h>

#include "data.h"

#include "datatypes/list.h"
#include "graphics/display.h"
#include "gen/terrain.h"
#include "world/blocks.h"
#include "world/world.h"

/*************
 * Constants *
 *************/

// TODO: dynamic capping?
const int LOAD_CAP = 16;
const int COMPILE_CAP = 1024;

/***********
 * Globals *
 ***********/

list *CHUNKS_TO_RELOAD;
list *CHUNKS_TO_RECOMPILE;

/********************
 * Inline Functions *
 ********************/

static inline int occludes_face(block neighbor, block occluded) {
  return (
    block_is(neighbor, OUT_OF_RANGE)
  ||
    is_opaque(neighbor)
  ||
    (
      is_translucent(neighbor)
    &&
      is_translucent(occluded)
    &&
      shares_translucency(neighbor, occluded)
    )
  );
}

// Macro-expanded face-checking functions:
#define CHECK_ANY_FACE \
  static inline int FN_NAME( \
    chunk_index idx, \
    chunk *neighbor, \
    block here, block there \
  ) { \
    if (block_is(there, OUT_OF_RANGE) && neighbor) { \
      if (OOR_AXIS != OOR_RESET) { \
        fprintf(stderr, "Error: OUT_OF_RANGE at non-edge!\n");\
        fprintf(stderr, "  OOR_AXIS = %d, edge = %d\n", OOR_AXIS, OOR_RESET);\
        exit(-1); \
      } \
      OOR_AXIS = OOR_REPLACE; \
      there = c_get_block(neighbor, idx); \
      if (occludes_face(here, there)) { \
        if (c_get_flags(neighbor, idx) & OOR_EXPOSED) { \
          c_clear_flags(neighbor, idx, OOR_EXPOSED); \
          mark_for_recompile(neighbor); \
        } \
      } else { \
        if (!(c_get_flags(neighbor, idx) & OOR_EXPOSED)) { \
          c_set_flags(neighbor, idx, OOR_EXPOSED); \
          mark_for_recompile(neighbor); \
        } \
      } \
      OOR_AXIS = OOR_RESET; \
    } \
    return occludes_face(there, here); \
  } \

#define FN_NAME check_top_face
#define OOR_AXIS idx.z
#define OOR_REPLACE 0
#define OOR_RESET (CHUNK_SIZE - 1)
#define OOR_EXPOSED BF_EXPOSED_BELOW
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET
#undef OOR_EXPOSED

#define FN_NAME check_bot_face
#define OOR_AXIS idx.z
#define OOR_REPLACE (CHUNK_SIZE - 1)
#define OOR_RESET 0
#define OOR_EXPOSED BF_EXPOSED_ABOVE
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET
#undef OOR_EXPOSED

#define FN_NAME check_north_face
#define OOR_AXIS idx.y
#define OOR_REPLACE 0
#define OOR_RESET (CHUNK_SIZE - 1)
#define OOR_EXPOSED BF_EXPOSED_SOUTH
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET
#undef OOR_EXPOSED

#define FN_NAME check_south_face
#define OOR_AXIS idx.y
#define OOR_REPLACE (CHUNK_SIZE - 1)
#define OOR_RESET 0
#define OOR_EXPOSED BF_EXPOSED_NORTH
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET
#undef OOR_EXPOSED

#define FN_NAME check_east_face
#define OOR_AXIS idx.x
#define OOR_REPLACE 0
#define OOR_RESET (CHUNK_SIZE - 1)
#define OOR_EXPOSED BF_EXPOSED_WEST
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET
#undef OOR_EXPOSED

#define FN_NAME check_west_face
#define OOR_AXIS idx.x
#define OOR_REPLACE (CHUNK_SIZE - 1)
#define OOR_RESET 0
#define OOR_EXPOSED BF_EXPOSED_EAST
CHECK_ANY_FACE
#undef FN_NAME
#undef OOR_AXIS
#undef OOR_REPLACE
#undef OOR_RESET
#undef OOR_EXPOSED

/*************
 * Functions *
 *************/

chunk_neighborhood * get_neighborhood(frame *f, frame_chunk_index fcidx) {
  chunk_neighborhood *result =
    (chunk_neighborhood *) malloc(sizeof(chunk_neighborhood));
  result->c = chunk_at(f, fcidx);
  if (fcidx.z < FRAME_SIZE - 1) {
    fcidx.z += 1;
    result->above = chunk_at(f, fcidx);
    fcidx.z -= 1;
  } else {
    result->above = NULL;
  }
  if (fcidx.z > 0) {
    fcidx.z -= 1;
    result->below = chunk_at(f, fcidx);
    fcidx.z += 1;
  } else {
    result->below = NULL;
  }
  if (fcidx.y < FRAME_SIZE - 1) {
    fcidx.y += 1;
    result->north = chunk_at(f, fcidx);
    fcidx.y -= 1;
  } else {
    result->north = NULL;
  }
  if (fcidx.y > 0) {
    fcidx.y -= 1;
    result->south = chunk_at(f, fcidx);
    fcidx.y += 1;
  } else {
    result->south = NULL;
  }
  if (fcidx.x < FRAME_SIZE - 1) {
    fcidx.x += 1;
    result->east = chunk_at(f, fcidx);
    fcidx.x -= 1;
  } else {
    result->east = NULL;
  }
  if (fcidx.x > 0) {
    fcidx.x -= 1;
    result->west = chunk_at(f, fcidx);
    fcidx.x += 1;
  } else {
    result->west = NULL;
  }
  return result;
}

void setup_data(void) {
  CHUNKS_TO_RELOAD = create_list();
  CHUNKS_TO_RECOMPILE = create_list();
}

void cleanup_data(void) {
  cleanup_list(CHUNKS_TO_RELOAD);
  cleanup_list(CHUNKS_TO_RECOMPILE);
}

void mark_for_reload(frame *f, frame_chunk_index fcidx) {
  chunk_neighborhood *cnb = get_neighborhood(f, fcidx);
  if (cnb->c->chunk_flags & CF_NEEDS_RELOAD) { return; }
  cnb->c->chunk_flags |= CF_NEEDS_RELOAD;
  append_element(CHUNKS_TO_RELOAD, (void *) cnb);
}

void mark_for_recompile(chunk *c) {
  if (c->chunk_flags & CF_NEEDS_RECOMIPLE) { return; }
  c->chunk_flags |= CF_NEEDS_RECOMIPLE;
  append_element(CHUNKS_TO_RECOMPILE, (void *) c);
}

void tick_data(void) {
  int n = 0;
  chunk_neighborhood *cnb = NULL;
  chunk *c = NULL;
  while (n < LOAD_CAP && get_length(CHUNKS_TO_RELOAD) > 0) {
    cnb = (chunk_neighborhood *) pop_element(CHUNKS_TO_RELOAD);
    load_chunk(cnb);
    cnb->c->chunk_flags &= ~CF_NEEDS_RELOAD;
    free(cnb);
    n += 1;
  }
  n = 0;
  while (n < COMPILE_CAP && get_length(CHUNKS_TO_RECOMPILE) > 0) {
    c = (chunk *) pop_element(CHUNKS_TO_RECOMPILE);
    compile_chunk(c);
    c->chunk_flags &= ~CF_NEEDS_RECOMIPLE;
    n += 1;
  }
}

void compute_exposure(chunk_neighborhood *cnb) {
  chunk_index idx;
  block b = 0;
  block_flag flags = 0;
  block ba = 0, bb = 0, bn = 0, bs = 0, be = 0, bw = 0;
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        // get main block and neighbors:
        b = c_get_block(cnb->c, idx);
        c_get_neighbors(cnb->c, idx, &ba, &bb, &bn, &bs, &be, &bw);
        flags = 0;
        if (!check_top_face(idx, cnb->above, b, ba)) {
          flags |= BF_EXPOSED_ABOVE;
        }
        if (!check_bot_face(idx, cnb->below, b, bb)) {
          flags |= BF_EXPOSED_BELOW;
        }
        if (!check_north_face(idx, cnb->north, b, bn)) {
          flags |= BF_EXPOSED_NORTH;
        }
        if (!check_south_face(idx, cnb->south, b, bs)) {
          flags |= BF_EXPOSED_SOUTH;
        }
        if (!check_east_face(idx, cnb->east, b, be)) {
          flags |= BF_EXPOSED_EAST;
        }
        if (!check_west_face(idx, cnb->west, b, bw)) {
          flags |= BF_EXPOSED_WEST;
        }
        // Set the computed exposure flags:
        c_put_flags(cnb->c, idx, flags);
      }
    }
  }
}

void load_chunk(chunk_neighborhood *cnb) {
  // TODO: Diff data?
  // TODO: Block entities!
  chunk_index idx;
  region_pos rpos;
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        cidx__rpos(cnb->c, &idx, &rpos);
        c_put_block(cnb->c, idx, terrain_block(rpos));
      }
    }
  }
  compute_exposure(cnb);
  mark_for_recompile(cnb->c);
}
