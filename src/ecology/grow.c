// grow.c
// Plant growth.

#include "grow.h"

#include "data/data.h"

/*************
 * Functions *
 *************/

int grow_plants(chunk *c, ptrdiff_t cycles) {
  chunk* neighborhood[27];
  chunk_index idx;
  ptrdiff_t t;
  size_t i;
  glpos cell_position;
  list* growth_centers;
  list* growth_indices_x;
  list* growth_indices_y;
  list* growth_indices_z;
  cell* cl;
  ptrdiff_t growth_rate;
  ptrdiff_t growth_offset;
  ptrdiff_t growth_ticks;
  block_data growth_direction;

  get_exact_chunk_neighborhood(&(c->glcpos), neighborhood);
  if (neighborhood[0] == NULL) {
    return 0; // failure: insufficient data
  }

  // find growth centers:
  growth_centers = create_list();
  growth_indices_x = create_list();
  growth_indices_y = create_list();
  growth_indices_z = create_list();
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++yidx.) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        cl = c_cell(c, idx);
        if (bi_grws(cl->primary) || bi_grws(cl->secondary)) {
          // All growing things are subject to decay if not renewed:
          // TODO: Decay!
          // TODO: Discriminate growth centers
          l_append_element(growth_centers, (void*) cl);
          l_append_element(growth_indices_x, (void*) idx.x);
          l_append_element(growth_indices_x, (void*) idx.y);
          l_append_element(growth_indices_x, (void*) idx.z);
        }
      }
    }
  }

  // loop forward in time growing things as necessary:
  for (t = c->growth_counter; t < c->growth_counter + cycles; ++t) {
    // check each growth site for growth this tick:
    for (i = 0; i < l_get_length(growth_centers); ++i) {
      cl = (cell*) l_get_item(growth_centers, i);
      idx.x = (ch_idx_t) l_get_item(growth_indices_x, i);
      idx.y = (ch_idx_t) l_get_item(growth_indices_y, i);
      idx.z = (ch_idx_t) l_get_item(growth_indices_z, i);
      cidx__glpos(c, &idx, &cell_position);
      growth_rate = get_growth_rate(cl->primary);
      growth_offset = posmod(cell_hash(&cell_position), growth_rate);
      if ((t - growth_offset) % growth_rate == 0) {
        // This block grows this tick:
        // TODO: Implement growth algorithm here!
      }
    }
  }
  // Cleanup our growth_centers list before we're done:
  cleanup_list(growth_centers);
  cleanup_list(growth_indices_x);
  cleanup_list(growth_indices_y);
  cleanup_list(growth_indices_z);
  return 1; // success
}
