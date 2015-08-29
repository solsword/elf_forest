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

void grow_block(chunk_neighborhood *nbh, block_index idx, ptrdiff_t t) {
  global_pos cell_position;
  chunk *c = nbh->members[NBH_CENTER];
  block *b = c_block(c, idx);
  block_data sprout_timer = 0;
  cidx__glpos(c, &idx, &cell_position);
  ptrdiff_t growth_rate = get_growth_rate(*b);
  ptrdiff_t growth_offset = posmod(cell_hash(&cell_position), growth_rate);
  if (
     (gri_vitality(*b) != GR_VITALITY_DEAD)
  && ((t - growth_offset) % growth_rate == 0)
  ) {
    if (bi_gcore(*b)) {
      grow_from_core(&nbh, idx);
    } else if (bi_gsite(*b)) {
      grow_at_site(&nbh, idx);
    }
  }
}

void grow_at_site(
  chunk_neighborhood *nbh,
  block_index idx
) {
  global_pos cell_position;
  chunk *c = nbh->members[NBH_CENTER];
  block *b = c_block(c, idx);
  block_data sprout_timer = 0;
  cidx__glpos(c, &idx, &cell_position);
  ptrdiff_t hash = cell_hash(&cell_position);
  if (bi_seed(*b)) {
    // This growth site is a seed: check the sprout timer.
    sprout_timer = gri_sprout_timer(*b);
    if (sprout_timer > 0) {
      // This seed isn't ready to sprout yet: just tick down the sprout timer
      // TODO: Implement sprout timer ticking conditions here!
      gri_set_sprout_timer(sprout_timer - 1);
      return;
    }
  }
  // TODO: HERE
  // 1. get grammar
  // 2. check/apply grammar
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
  block bid;

  fill_chunk_neighborhood(&(c->glcpos), &nbh);
  if (nbh.members[0] == NULL) {
    return 0; // failure: insufficient data
  }

  // TODO: A find-into-list, then iterate-over-list approach might be more
  // efficient? Probably worse for lots of low-t updates, but better for a few
  // high-t updates.

  // loop forward in time growing things as necessary:
  for (t = c->growth_counter; t < c->growth_counter + cycles; ++t) {
    for (idx.xyz.x = 0; idx.xyz.x < CHUNK_SIZE; ++idx.xyz.x) {
      for (idx.xyz.y = 0; idx.xyz.y < CHUNK_SIZE; ++idx.xyz.y) {
        for (idx.xyz.z = 0; idx.xyz.z < CHUNK_SIZE; ++idx.xyz.z) {
          cl = c_cell(c, idx);

          idx.xyz.w = 0;
          b = &(cl->blocks[0]);
          if (bi_grws(*b)) {
            update_growth(b);
            grow_block(&nbh, idx, t);
          }

          idx.xyz.w = 1;
          b = &(cl->blocks[1]);
          if (bi_grws(*b)) {
            update_growth(b);
            grow_block(&nbh, idx, t);
          }
        }
      }
    }
  }
  // Update this chunk's growth counter...
  c->growth_counter += cycles;
  return 1; // success
}
