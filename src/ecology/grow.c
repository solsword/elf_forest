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

void grow_seed_block(cell **cell_neighborhood, int primary) {
  // TODO: HERE
}

void grow_from_core(
  chunk **chunk_neighborhood,
  chunk_index idx,
  int primary,
  ptrdiff_t t
) {
  global_pos cell_position;
  chunk *c = chunk_neighborhood[13];
  cell *cl = c_cell(c, idx);
  cidx__glpos(c, &idx, &cell_position);
  ptrdiff_t growth_rate = get_growth_rate(cl->primary);
  ptrdiff_t growth_offset = posmod(cell_hash(&cell_position), growth_rate);
  if ((t - growth_offset) % growth_rate == 0) {
    // This block grows this tick:
    // TODO: Implement growth algorithm here!
  }
}

int grow_plants(chunk *c, ptrdiff_t cycles) {
  chunk* neighborhood[27];
  chunk_index idx;
  int is_primary;
  ptrdiff_t t;
  size_t i;
  list* growth_primaries;
  list* growth_indices_x;
  list* growth_indices_y;
  list* growth_indices_z;
  cell* cl;
  block* b;

  get_exact_chunk_neighborhood(&(c->glcpos), neighborhood);
  if (neighborhood[0] == NULL) {
    return 0; // failure: insufficient data
  }

  // find growth centers:
  growth_primaries = create_list();
  growth_indices_x = create_list();
  growth_indices_y = create_list();
  growth_indices_z = create_list();
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        // TODO: Merge this with the loop below!
        cl = c_cell(c, idx);
        b = &(cl->primary);
        if (bi_grws(*b)) {
          update_growth(b);
          if (bi_gcore(*b)) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
            l_append_element(growth_primaries, (void*) 1); // primary
            l_append_element(growth_indices_x, (void*) idx.x);
            l_append_element(growth_indices_y, (void*) idx.y);
            l_append_element(growth_indices_z, (void*) idx.z);
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
          }
        }
        b = &(cl->secondary);
        if (bi_grws(*b)) {
          update_growth(b);
          if (bi_gcore(*b)) {
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
            l_append_element(growth_primaries, (void*) 0); // secondary
            l_append_element(growth_indices_x, (void*) idx.x);
            l_append_element(growth_indices_y, (void*) idx.y);
            l_append_element(growth_indices_z, (void*) idx.z);
#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
          }
        }
        if (bi_grws(cl->primary) || bi_grws(cl->secondary)) {
          // All growing things are subject to decay if not renewed:
          // Primary:
          // Secondary:
          // TODO: decay of dead parts...
          if (bi_gcore(cl->primary) || bi_gcore(cl->secondary)) {
          }
        }
      }
    }
  }

  // loop forward in time growing things as necessary:
  for (t = c->growth_counter; t < c->growth_counter + cycles; ++t) {
    // grow at each core (grow_from_core checks tick timing)
    for (i = 0; i < l_get_length(growth_primaries); ++i) {
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
      is_primary = (int) l_get_item(growth_primaries, i);
      idx.x = (ch_idx_t) l_get_item(growth_indices_x, i);
      idx.y = (ch_idx_t) l_get_item(growth_indices_y, i);
      idx.z = (ch_idx_t) l_get_item(growth_indices_z, i);
#pragma GCC diagnostic warning "-Wpointer-to-int-cast"
      grow_from_core(neighborhood, idx, is_primary, t);
    }
  }
  // Cleanup our growth_* lists before we're done:
  cleanup_list(growth_primaries);
  cleanup_list(growth_indices_x);
  cleanup_list(growth_indices_y);
  cleanup_list(growth_indices_z);
  return 1; // success
}
