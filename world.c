// world.c
// Structures and functions for representing the world.

#include <stdint.h>
#include <stdlib.h>
// DEBUG
#include <stdio.h>

#include "blocks.h"
#include "terrain.h"
#include "octree.h"
#include "world.h"

/***********
 * Globals *
 ***********/

frame MAIN_FRAME;

/*************
 * Functions *
 *************/

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

void setup_chunk(chunk *c, region_chunk_pos *rpos) {
  c->block_entities = create_list();
  c->rpos.x = rpos->x;
  c->rpos.y = rpos->y;
  c->rpos.z = rpos->z;
  c->flags = 0;
}

void cleanup_chunk(chunk *c) {
  cleanup_vertex_buffer(&(c->opaque_vertices));
  cleanup_vertex_buffer(&(c->translucent_vertices));
  destroy_list(c->block_entities);
}

void tick_blocks(frame *f) {
  // TODO: HERE
}
