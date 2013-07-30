// data.c
// Loading from and saving to disk.

#include <stdint.h>

#include "blocks.h"
#include "list.h"
#include "world.h"
#include "display.h"
#include "terrain.h"
#include "data.h"

/*************
 * Constants *
 *************/

// TODO: adjust this; dynamic load capping
const int LOAD_CAP = 2;

/***********
 * Globals *
 ***********/

list *DIRTY_CHUNKS;

/*************
 * Functions *
 *************/

void setup_data(void) {
  DIRTY_CHUNKS = create_list();
}

void cleanup_data(void) {
  cleanup_list(DIRTY_CHUNKS);
}

void mark_for_reload(chunk *c) {
  append_element(DIRTY_CHUNKS, (void *) c);
}

void tick_load(void) {
  int n = 0;
  chunk *c;
  if (get_length(DIRTY_CHUNKS) > 0) {
    do {
      c = (chunk *) pop_element(DIRTY_CHUNKS);
      load_chunk(c);
      n += 1;
    } while (n < LOAD_CAP && c != NULL);
  }
}

void compute_exposure(chunk *c) {
  chunk_index idx;
  block ba = 0, bb = 0, bn = 0, bs = 0, be = 0, bw = 0;
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        block b = c_get_block(c, idx);
        if (is_invisible(b)) {
          //printf("invis: 0x%04x\n", b);
          c_put_block(c, idx, set_exposed(b));
        } else {
          //printf("vis: 0x%04x\n", b);
          // get neighboring blocks:
          c_get_neighbors(c, idx, &ba, &bb, &bn, &bs, &be, &bw);
          // translucent blocks:
          if (is_translucent(b)) {
            c_put_block(c, idx, set_exposed(b));
            //printf("TL!\n");
            /*
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
            */
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
              c_put_block(c, idx, set_exposed(b));
            }
          }
        }
      }
    }
  }
}

void load_chunk(chunk *c) {
  // TODO: Diff data?
  // TODO: Block entities!
  chunk_index idx;
  region_pos rpos;
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        cidx__rpos(c, &idx, &rpos);
        c_put_block(c, idx, terrain_block(rpos));
      }
    }
  }
  compute_exposure(c);
  compile_chunk(c);
}
