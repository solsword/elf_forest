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
  list* growth_sites;
  list* growth_indices_x;
  list* growth_indices_y;
  list* growth_indices_z;
  list* new_growth_sites;
  list* new_growth_indices_x;
  list* new_growth_indices_y;
  list* new_growth_indices_z;
  void* ferry;
  cell* cl;
  ptrdiff_t growth_rate;
  ptrdiff_t growth_offset;
  ptrdiff_t growth_ticks;
  block_data growth_direction;

  get_exact_chunk_neighborhood(&(c->glcpos), neighborhood);
  if (neighborhood[0] == NULL) {
    return 0; // failure: insufficient data
  }

  // find growth sites:
  growth_sites = create_list();
  growth_indices_x = create_list();
  growth_indices_y = create_list();
  growth_indices_z = create_list();
  new_growth_sites = create_list();
  new_growth_indices_x = create_list();
  new_growth_indices_y = create_list();
  new_growth_indices_z = create_list();
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++yidx.) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        cl = c_cell(c, idx);
        if (bi_grws(cl->primary) || bi_grws(cl->secondary)) {
          l_append_element(growth_sites, (void*) cl);
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
    for (i = 0; i < l_get_length(growth_sites); ++i) {
      cl = (cell*) l_get_item(growth_sites, i);
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
    // Add any new growth sites to our main lists for the next tick:
    ferry = l_pop_element(new_growth_sites);
    while (ferry != NULL) {
      l_append_element(growth_sites, ferry);
      ferry = l_pop_element(new_growth_sites);
    }
    ferry = l_pop_element(new_growth_indices_x);
    while (ferry != NULL) {
      l_append_element(growth_indices_x, ferry);
      ferry = l_pop_element(new_growth_sites);
    }
    ferry = l_pop_element(new_growth_indices_y);
    while (ferry != NULL) {
      l_append_element(growth_indices_y, ferry);
      ferry = l_pop_element(new_growth_sites);
    }
    ferry = l_pop_element(new_growth_indices_z);
    while (ferry != NULL) {
      l_append_element(growth_indices_z, ferry);
      ferry = l_pop_element(new_growth_sites);
    }
  }
  // Cleanup our growth_sites list before we're done:
  cleanup_list(growth_sites);
  cleanup_list(growth_indices_x);
  cleanup_list(growth_indices_y);
  cleanup_list(growth_indices_z);
  cleanup_list(new_growth_sites);
  cleanup_list(new_growth_indices_x);
  cleanup_list(new_growth_indices_y);
  cleanup_list(new_growth_indices_z);
  return 1; // success
}
