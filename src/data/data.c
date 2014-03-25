// data.c
// Loading from and saving to disk.

#include <stdint.h>
#include <stdio.h>

#include "data.h"

#include "datatypes/queue.h"
//#include "datatypes/map.h"
#include "graphics/display.h"
#include "gen/terrain.h"
#include "world/blocks.h"
#include "world/world.h"
#include "world/chunk_data.h"

/*************
 * Constants *
 *************/

// TODO: dynamic capping?
int const LOAD_CAP = 16;
int const COMPILE_CAP = 1024;

/***********
 * Globals *
 ***********/

queue *CHUNKS_TO_RELOAD;
queue *CHUNKS_TO_RECOMPILE;

/*************
 * Functions *
 *************/

void setup_data(void) {
  CHUNKS_TO_RELOAD = create_queue();
  CHUNKS_TO_RECOMPILE = create_queue();
}

void cleanup_data(void) {
  destroy_queue(CHUNKS_TO_RELOAD);
  destroy_queue(CHUNKS_TO_RECOMPILE);
}

void mark_for_reload(frame *f, frame_chunk_index fcidx) {
  if (chunk_at(f, fcidx)->chunk_flags & CF_NEEDS_RELOAD) { return; }
  chunk_neighborhood *cnb = get_neighborhood(f, fcidx);
  cnb->c->chunk_flags |= CF_NEEDS_RELOAD;
  q_push_element(CHUNKS_TO_RELOAD, (void *) cnb);
}

void mark_for_recompile(frame *f, frame_chunk_index fcidx) {
  if (chunk_at(f, fcidx)->chunk_flags & CF_NEEDS_RECOMIPLE) { return; }
  chunk_neighborhood *cnb = get_neighborhood(f, fcidx);
  cnb->c->chunk_flags |= CF_NEEDS_RECOMIPLE;
  q_push_element(CHUNKS_TO_RECOMPILE, (void *) cnb);
}

void tick_data(void) {
  int n = 0;
  int skipped = 0;
  chunk_neighborhood *cnb = NULL;
  while (n < LOAD_CAP && q_get_length(CHUNKS_TO_RELOAD) > 0) {
    cnb = (chunk_neighborhood *) q_pop_element(CHUNKS_TO_RELOAD);
    load_chunk(cnb);
    cnb->c->chunk_flags &= ~CF_NEEDS_RELOAD;
    free(cnb);
    n += 1;
  }
  n = 0;
  skipped = 0;
  while (n < COMPILE_CAP && (q_get_length(CHUNKS_TO_RECOMPILE) - skipped) > 0) {
    cnb = (chunk_neighborhood *) q_pop_element(CHUNKS_TO_RECOMPILE);
    if (is_fully_loaded(cnb)) {
      compute_exposure(cnb);
      compile_chunk(cnb->c);
      cnb->c->chunk_flags &= ~CF_NEEDS_RECOMIPLE;
      free(cnb);
      n += 1;
    } else {
      q_push_element(CHUNKS_TO_RECOMPILE, (void *) cnb);
      skipped += 1;
    }
  }
}

void load_chunk(chunk_neighborhood *cnb) {
  // TODO: Data from disk!
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
}
