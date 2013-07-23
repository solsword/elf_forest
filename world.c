// world.c
// Structures and functions for representing the world.

#include <stdint.h>
#include <stdlib.h>
// DEBUG
#include <stdio.h>

#include "blocks.h"
#include "world.h"
#include "octree.h"

/***********
 * Globals *
 ***********/

frame MAIN_FRAME;

/*************
 * Functions *
 *************/

void compute_exposure(frame *f, frame_chunk_index idx) {
  frame_pos base, pos;
  // Get frame coords from chunk coords:
  fcidx__fpos(&idx, &base);
  for (pos.x = base.x; pos.x < base.x + CHUNK_SIZE; ++pos.x) {
    for (pos.y = base.y; pos.y < base.y + CHUNK_SIZE; ++pos.y) {
      for (pos.z = base.z; pos.z < base.z + CHUNK_SIZE; ++pos.z) {
        block b = block_at(f, pos);
        if (is_invisible(b)) {
          //printf("invis: 0x%04x\n", b);
          set_block(f, pos, set_exposed(b));
        } else {
          //printf("vis: 0x%04x\n", b);
          block ba = block_above(f, pos);
          block bb = block_below(f, pos);
          block bn = block_north(f, pos);
          block bs = block_south(f, pos);
          block be = block_east(f, pos);
          block bw = block_west(f, pos);
          if (is_translucent(b)) {
            //printf("TL!\n");
            if (
              !(is_opaque(ba) || shares_translucency(b, ba))
              || !(is_opaque(bb) || shares_translucency(b, bb))
              || !(is_opaque(bn) || shares_translucency(b, bn))
              || !(is_opaque(bs) || shares_translucency(b, bs))
              || !(is_opaque(be) || shares_translucency(b, be))
              || !(is_opaque(bw) || shares_translucency(b, bw))
            ) {
              //printf("exposed\n");
              set_block(f, pos, set_exposed(b));
            }
          } else {
            if (
              !is_opaque(ba)
              || !is_opaque(bb)
              || !is_opaque(bn)
              || !is_opaque(bs)
              || !is_opaque(be)
              || !is_opaque(bw)
            ) {
              //printf("exposed\n");
              set_block(f, pos, set_exposed(b));
            }
          }
        }
      }
    }
  }
}

void setup_frame(frame *f) {
  frame_chunk_index idx;
  for (idx.x = 0; idx.x < FRAME_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < FRAME_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < FRAME_SIZE; ++idx.z) {
        setup_chunk(chunk_at(f, idx));
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

void setup_chunk(chunk *c) {
  c->block_entities = create_list();
}

void cleanup_chunk(chunk *c) {
  cleanup_vertex_buffer(&(c->opaque_vertices));
  cleanup_vertex_buffer(&(c->translucent_vertices));
  destroy_list(c->block_entities);
}
