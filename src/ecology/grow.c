// grow.c
// Plant growth.

#include "grow.h"

#include "data/data.h"

/*************
 * Functions *
 *************/

void update_growth(block *b) {
  if (bi_grws(*b) && !bi_gcore(*b) && !gri_renewal(*b)) {
    if (gri_vitality(*b) == GR_VITALITY_HEALTHY) {
      gri_set_vitality(b, GR_VITALITY_DYING);
    } else if (
       gri_vitality(*b) == GR_VITALITY_SICK
    || gri_vitality(*b) == GR_VITALITY_DYING
    ) {
      gri_set_vitality(b, GR_VITALITY_DEAD);
    }
  }
  // TODO: decay of dead parts...
}

void grow_seed_block(chunk_neighborhood *nbh, block_index idx) {
  // TODO: HERE
}

void grow_from_core(
  chunk_neighborhood *nbh,
  block_index idx,
  ptrdiff_t t
) {
  global_pos cell_position;
  chunk *c = nbh->members[NBH_CENTER];
  block *b = c_block(c, idx);
  cidx__glpos(c, &idx, &cell_position);
  ptrdiff_t growth_rate = get_growth_rate(*b);
  ptrdiff_t growth_offset = posmod(cell_hash(&cell_position), growth_rate);
  if ((t - growth_offset) % growth_rate == 0) {
    // This block grows this tick:
    // TODO: Implement growth algorithm here!
  }
}

int grow_plants(chunk *c, ptrdiff_t cycles) {
  chunk_neighborhood nbh;
  block_index idx;
  ptrdiff_t t;
  cell* cl;
  block* b;

  fill_chunk_neighborhood(&(c->glcpos), &nbh);
  if (nbh.members[0] == NULL) {
    return 0; // failure: insufficient data
  }

  // TODO: A find-into-list, then iterate-over-list approach might be more
  // efficient? Probably worse for lots of low-t updates, but better for a few
  // high-t updates.

  // loop forward in time growing things as necessary:
  idx.xyz.w = 0;
  for (t = c->growth_counter; t < c->growth_counter + cycles; ++t) {
    for (idx.xyz.x = 0; idx.xyz.x < CHUNK_SIZE; ++idx.xyz.x) {
      for (idx.xyz.y = 0; idx.xyz.y < CHUNK_SIZE; ++idx.xyz.y) {
        for (idx.xyz.z = 0; idx.xyz.z < CHUNK_SIZE; ++idx.xyz.z) {
          cl = c_cell(c, idx);
          b = &(cl->blocks[0]);
          if (bi_grws(*b)) {
            update_growth(b);
            if (bi_gcore(*b)) {
              // TODO: process here!
            } else if (bi_seed(*b)) {
              // TODO: process here!
            }
          }
          b = &(cl->blocks[1]);
          if (bi_grws(*b)) {
            update_growth(b);
            if (bi_gcore(*b)) {
              // TODO: process here!
            } else if (bi_seed(*b)) {
              // TODO: process here!
            }
          }
        }
      }
    }
  }
  // Update this chunk's growth counter...
  c->growth_counter += cycles;
  return 1; // success
}
