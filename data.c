// data.c
// Loading from and saving to disk.

#include <stdint.h>
#include <assert.h>

#include "blocks.h"
#include "list.h"
#include "world.h"
#include "display.h"
#include "terrain.h"
#include "data.h"

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

static inline int opaque_occludes_face(neighbor) {
  return (
    block_is(neighbor, OUT_OF_RANGE)
  ||
    is_opaque(neighbor)
  );
}

static inline int translucent_occludes_face(block neighbor, block b) {
  return (
    block_is(neighbor, OUT_OF_RANGE)
  ||
    is_opaque(neighbor)
  ||
    shares_translucency(b, neighbor)
  );
}

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
        b = c_get_block(cnb->c, idx);
        flags = 0;
        c_clear_flags(cnb->c, idx, BF_ALL_FLAGS);
        // get neighboring blocks:
        c_get_neighbors(cnb->c, idx, &ba, &bb, &bn, &bs, &be, &bw);
        // invisible blocks:
        if (is_invisible(b)) {
          // above
          if (block_is(ba, OUT_OF_RANGE) && cnb->above) {
            assert(idx.z == CHUNK_SIZE - 1);
            idx.z = 0;
            c_set_flags(cnb->above, idx, BF_EXPOSED_BELOW);
            mark_for_recompile(cnb->above);
            idx.z = CHUNK_SIZE - 1;
          }
          // below
          if (block_is(bb, OUT_OF_RANGE) && cnb->below) {
            assert(idx.z == 0);
            idx.z = CHUNK_SIZE - 1;
            c_set_flags(cnb->below, idx, BF_EXPOSED_ABOVE);
            mark_for_recompile(cnb->below);
            idx.z = 0;
          }
          // north
          if (block_is(bn, OUT_OF_RANGE) && cnb->north) {
            assert(idx.y == CHUNK_SIZE - 1);
            idx.y = 0;
            c_set_flags(cnb->north, idx, BF_EXPOSED_SOUTH);
            mark_for_recompile(cnb->north);
            idx.y = CHUNK_SIZE - 1;
          }
          // south
          if (block_is(bs, OUT_OF_RANGE) && cnb->south) {
            assert(idx.y == 0);
            idx.y = CHUNK_SIZE - 1;
            c_set_flags(cnb->south, idx, BF_EXPOSED_NORTH);
            mark_for_recompile(cnb->south);
            idx.y = 0;
          }
          // east
          if (block_is(be, OUT_OF_RANGE) && cnb->east) {
            assert(idx.x == CHUNK_SIZE - 1);
            idx.x = 0;
            c_set_flags(cnb->east, idx, BF_EXPOSED_WEST);
            mark_for_recompile(cnb->east);
            idx.x = CHUNK_SIZE - 1;
          }
          // west
          if (block_is(bw, OUT_OF_RANGE) && cnb->west) {
            assert(idx.x == 0);
            idx.x = CHUNK_SIZE - 1;
            c_set_flags(cnb->west, idx, BF_EXPOSED_EAST);
            mark_for_recompile(cnb->west);
            idx.x = 0;
          }
        // translucent blocks:
        } else if (is_translucent(b)) {
          // above
          if (block_is(ba, OUT_OF_RANGE) && cnb->above) {
            assert(idx.z == CHUNK_SIZE - 1);
            idx.z = 0;
            ba = c_get_block(cnb->above, idx);
            if (!translucent_occludes_face(b, ba)) {
              c_set_flags(cnb->above, idx, BF_EXPOSED_BELOW);
              mark_for_recompile(cnb->above);
            }
            idx.z = CHUNK_SIZE - 1;
          }
          if (!translucent_occludes_face(ba, b)) {
            flags |= BF_EXPOSED_ABOVE;
          }
          // below
          if (block_is(bb, OUT_OF_RANGE) && cnb->below) {
            assert(idx.z == 0);
            idx.z = CHUNK_SIZE - 1;
            bb = c_get_block(cnb->below, idx);
            if (!translucent_occludes_face(b, bb)) {
              c_set_flags(cnb->below, idx, BF_EXPOSED_ABOVE);
              mark_for_recompile(cnb->below);
            }
            idx.z = 0;
          }
          if (!translucent_occludes_face(bb, b)) {
            flags |= BF_EXPOSED_BELOW;
          }
          // north
          if (block_is(bn, OUT_OF_RANGE) && cnb->north) {
            assert(idx.y == CHUNK_SIZE - 1);
            idx.y = 0;
            bn = c_get_block(cnb->north, idx);
            if (!translucent_occludes_face(b, bn)) {
              c_set_flags(cnb->north, idx, BF_EXPOSED_SOUTH);
              mark_for_recompile(cnb->north);
            }
            idx.y = CHUNK_SIZE - 1;
          }
          if (!translucent_occludes_face(bn, b)) {
            flags |= BF_EXPOSED_NORTH;
          }
          // south
          if (block_is(bs, OUT_OF_RANGE) && cnb->south) {
            assert(idx.y == 0);
            idx.y = CHUNK_SIZE - 1;
            bs = c_get_block(cnb->south, idx);
            if (!translucent_occludes_face(b, bs)) {
              c_set_flags(cnb->south, idx, BF_EXPOSED_NORTH);
              mark_for_recompile(cnb->south);
            }
            idx.y = 0;
          }
          if (!translucent_occludes_face(bs, b)) {
            flags |= BF_EXPOSED_SOUTH;
          }
          // east
          if (block_is(be, OUT_OF_RANGE) && cnb->east) {
            assert(idx.x == CHUNK_SIZE - 1);
            idx.x = 0;
            be = c_get_block(cnb->east, idx);
            if (!translucent_occludes_face(b, be)) {
              c_set_flags(cnb->east, idx, BF_EXPOSED_WEST);
              mark_for_recompile(cnb->east);
            }
            idx.x = CHUNK_SIZE - 1;
          }
          if (!translucent_occludes_face(be, b)) {
            flags |= BF_EXPOSED_EAST;
          }
          // west
          if (block_is(bw, OUT_OF_RANGE) && cnb->west) {
            assert(idx.x == 0);
            idx.x = CHUNK_SIZE - 1;
            bw = c_get_block(cnb->west, idx);
            if (!translucent_occludes_face(b, bw)) {
              c_set_flags(cnb->west, idx, BF_EXPOSED_EAST);
              mark_for_recompile(cnb->west);
            }
            idx.x = 0;
          }
          if (!translucent_occludes_face(bw, b)) {
            flags |= BF_EXPOSED_WEST;
          }
        // all other blocks:
        } else {
          // above
          if (block_is(ba, OUT_OF_RANGE) && cnb->above) {
            assert(idx.z == CHUNK_SIZE - 1);
            idx.z = 0;
            ba = c_get_block(cnb->above, idx);
            idx.z = CHUNK_SIZE - 1;
          }
          if (!opaque_occludes_face(ba)) {
            flags |= BF_EXPOSED_ABOVE;
          }
          // below
          if (block_is(bb, OUT_OF_RANGE) && cnb->below) {
            assert(idx.z == 0);
            idx.z = CHUNK_SIZE - 1;
            bb = c_get_block(cnb->below, idx);
            idx.z = 0;
          }
          if (!opaque_occludes_face(bb)) {
            flags |= BF_EXPOSED_BELOW;
          }
          // north
          if (block_is(bn, OUT_OF_RANGE) && cnb->north) {
            assert(idx.y == CHUNK_SIZE - 1);
            idx.y = 0;
            bn = c_get_block(cnb->north, idx);
            idx.y = CHUNK_SIZE - 1;
          }
          if (!opaque_occludes_face(bn)) {
            flags |= BF_EXPOSED_NORTH;
          }
          // south
          if (block_is(bs, OUT_OF_RANGE) && cnb->south) {
            assert(idx.y == 0);
            idx.y = CHUNK_SIZE - 1;
            bs = c_get_block(cnb->south, idx);
            idx.y = 0;
          }
          if (!opaque_occludes_face(bs)) {
            flags |= BF_EXPOSED_SOUTH;
          }
          // east
          if (block_is(be, OUT_OF_RANGE) && cnb->east) {
            assert(idx.x == CHUNK_SIZE - 1);
            idx.x = 0;
            be = c_get_block(cnb->east, idx);
            idx.x = CHUNK_SIZE - 1;
          }
          if (!opaque_occludes_face(be)) {
            flags |= BF_EXPOSED_EAST;
          }
          // west
          if (block_is(bw, OUT_OF_RANGE) && cnb->west) {
            assert(idx.x == 0);
            idx.x = CHUNK_SIZE - 1;
            bw = c_get_block(cnb->west, idx);
            idx.x = 0;
          }
          if (!opaque_occludes_face(bw)) {
            flags |= BF_EXPOSED_WEST;
          }
        }
        // Set the computed exposure flags:
        c_set_flags(cnb->c, idx, flags);
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
