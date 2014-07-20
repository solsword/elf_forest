// worldgen.c
// World map generation.

#include "datatypes/bitmap.h"
#include "noise/noise.h"
#include "world/blocks.h"
#include "data/data.h"

#include "geology.h"

#include "worldgen.h"

/***********
 * Globals *
 ***********/

world_map* THE_WORLD = NULL;

/******************************
 * Constructors & Destructors *
 ******************************/

world_map *create_world_map(ptrdiff_t seed, wm_pos_t width, wm_pos_t height) {
  world_map *result = (world_map*) malloc(sizeof(world_map));
  world_map_pos xy;
  world_region* wr;
  ptrdiff_t hash = seed + 71;
  result->seed = seed;
  result->width = width;
  result->height = height;
  result->regions = (world_region *) calloc(width*height, sizeof(world_region));
  result->all_strata = create_list();
  result->all_biomes = create_list();
  result->all_civs = create_list();
  for (xy.x = 0; xy.x < result->width; ++xy.x) {
    for (xy.y = 0; xy.y < result->height; ++xy.y) {
      wr = get_world_region(wm, &xy); // no need to worry about NULL here
      wmpos__rpos(&xy, &(wr->anchor));
      wr->anchor.x += WORLD_REGION_SIZE * CHUNK_SIZE * float_hash_1d(hash); 
      wr->anchor.y += WORLD_REGION_SIZE * CHUNK_SIZE * float_hash_1d(hash); 
      wr->anchor.z = (
        BASE_STRATUM_THICKNESS * MAX_STRATA_LAYERS * float_hash_1d(hash)
      );
      hash += 1;
    }
  }
  return result;
}

void cleanup_world_map(world_map *wm) {
  destroy_list(wm->all_strata);
  destroy_list(wm->all_biomes);
  destroy_list(wm->all_civs);
  free(wm->regions);
  free(wm);
}

/*************
 * Functions *
 *************/

void setup_worldgen(ptrdiff_t seed) {
  THE_WORLD = create_world_map(seed, WORLD_WIDTH, WORLD_HEIGHT);
  generate_geology(THE_WORLD);
}

void cleanup_worldgen() {
  cleanup_world_map(THE_WORLD);
}

void world_cell(world_map *wm, region_pos *rpos, cell *result) {
  world_map_pos wmpos, iter;
  world_region *neighborhood[9];
  size_t i = 0;
  rpos__wmpos(rpos, &wmpos);
  // default values:
  result->primary = b_make_block(B_VOID);
  result->secondary = b_make_block(B_VOID);
  result->p_data = 0;
  result->s_data = 0;
  i = 0;
  for (iter.x = wmpos.x - 1; iter.x <= wmpos.x + 1; ++iter.x) {
    for (iter.y = wmpos.y - 1; iter.y <= wmpos.y + 1; ++iter.y) {
      neighborhood[i] = get_world_region(wm, &iter);
      i += 1;
    }
  }
  if (rpos->z < 0) {
    result->primary = b_make_block(B_BOUNDARY);
  } else if (neighborhood[4] == NULL) {
    // Outside the world:
    result->primary = b_make_block(B_AIR);
  } else {
    strata_cell(neighborhood, rpos, result);
  }
  if (b_is(result->primary, B_VOID)) {
    // TODO: Other things like plants here...
    result->primary = b_make_block(B_AIR);
  }
}

void generate_geology(world_map *wm) {
  size_t i, j;
  world_map_pos xy;
  region_pos corners[4];
  r_pos_t t[4];
  r_pos_t t_avg;
  stratum *s;

  float avg_size = sqrtf(wm->width*wm->height);
  avg_size *= STRATA_AVG_SIZE;
  avg_size *= WORLD_REGION_SIZE * CHUNK_SIZE;

  map_function profile = MFN_SPREAD_UP;
  geologic_source source = GEO_SEDIMENTAY;
  ptrdiff_t hash, h1, h2, h3, h4, h5;
  world_region *wr;
  for (i = 0; i < MAX_STRATA_LAYERS * STRATA_COMPLEXITY; ++i) {
    // Create a stratum and append it to the list of all strata:
    hash = expanded_hash_1d(wm->seed + 567*i);
    h1 = hash_1d(hash);
    h2 = hash_1d(h1);
    h3 = hash_1d(h2);
    h4 = hash_1d(h3);
    h5 = hash_1d(h4);
    switch (h4 % 3) {
      case 0:
        profile = MFN_SPREAD_UP;
        break;
      case 1:
        profile = MFN_TERRACE;
        break;
      case 2:
      default:
        profile = MFN_HILL;
        break;
    }
    switch (h5 % 3) {
      case 0:
        source = GEO_IGNEOUS;
        break;
      case 1:
        source = GEO_METAMORPHIC;
        break;
      case 2:
      default:
        source = GEO_SEDIMENTAY;
        break;
    }
    s = create_stratum(
      hash,
      float_hash_1d(hash)*wm->width, float_hash_1d(h1)*wm->height,
      avg_size * (0.6 + float_hash_1d(h2)*0.8), // size
      BASE_STRATUM_THICKNESS * exp(-0.5 + float_hash_1d(h3)*3.5), // thickness
      profile, // profile
      source
    );
    l_append_element(wm->all_strata, (void*) s);
    // Render the stratum into the various regions:
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = 0; xy.y < wm->height; ++xy.y) {
        // Compute thickness to determine if this region needs to be aware of
        // this stratum:
        // TODO: use four-corners method instead
        wmpos__rpos(&xy, &(corners[0]));
        copy_rpos(&(corners[0]), &(corners[1]));
        corners[1].y += WORLD_REGION_SIZE*CHUNK_SIZE;
        copy_rpos(&(corners[1]), &(corners[2]));
        corners[2].x += WORLD_REGION_SIZE*CHUNK_SIZE;
        copy_rpos(&(corners[0]), &(corners[3]));
        corners[3].x += WORLD_REGION_SIZE*CHUNK_SIZE;
        t[0] = compute_stratum_height(s, &(corners[0]));
        t[1] = compute_stratum_height(s, &(corners[1]));
        t[2] = compute_stratum_height(s, &(corners[2]));
        t[3] = compute_stratum_height(s, &(corners[3]));
        t_avg = (t[0] + t[1] + t[2] + t[3]) / 4.0;
        // If any corner has material, add this stratum to this region:
        if (t[0] + t[1] + t[2] + t[3] > 0) {
          //TODO: Real logging/debugging
          //printf("Adding stratum to region at %zu, %zu.\n", xy.x, xy.y);
          wr = get_world_region(wm, &xy); // no need to worry about NULL here
          if (wr->geology.stratum_count < MAX_STRATA_LAYERS) {
            // adjust existing strata:
            for (j = 0; j < wr->geology.stratum_count; ++j) {
              wr->geology.bottoms[j] *= (
                wr->geology.total_height + t_avg
              ) / wr->geology.total_height;
            }
            wr->geology.total_height += t_avg;
            wr->geology.bottoms[wr->geology.stratum_count] = 1 - (
              t_avg / fmax(BASE_STRATUM_THICKNESS*6, wr->geology.total_height)
              // the higher of the new total height or approximately 6 strata
              // of height
            );
            wr->geology.strata[wr->geology.stratum_count] = s;
            wr->geology.stratum_count += 1;
          } // it's okay if some strata are zoned out by the layers limit
        }
      }
    }
  }
}

void strata_cell(
  world_region* neighborhood[],
  region_pos *rpos,
  cell *result
) {
  static r_pos_t surface_height;
  float column_height;
  static region_pos pr_rpos = { .x = -1, .y = -1, .z = -1 };
  int i;
  r_pos_t h;
  stratum *st;
  region_pos trp;
  copy_rpos(rpos, &trp);
  trp.z = 0;

  // DEBUG: (to show the strata)
  /*
  if (abs(rpos->x - 32770) < CHUNK_SIZE) {
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
    return;
  }

  // DEBUG: Caves to show things off more:
  if (
    sxnoise_3d(rpos->x*1/12.0, rpos->y*1/12.0, rpos->z*1/12.0) >
      sxnoise_3d(rpos->x*1/52.0, rpos->y*1/52.0, rpos->z*1/52.0)
  ) {
    result->primary = b_make_block(B_AIR);
    result->secondary = b_make_block(B_VOID);
    return;
  }
  */

  if (pr_rpos.x != rpos->x || pr_rpos.y != rpos->y) {
    // No need to recompute the surface height if we're at the same x/y.
    surface_height = compute_surface_height(&(neighborhood[4]), &rpos);
  }

  // DEBUG:
  /*
  if (rpos->x == 3 && rpos->y == 3) {
    printf("rpos: %ld, %ld, %ld\n", rpos->x, rpos->y, rpos->z);
    printf("stratum: %ld\n", THE_WORLD->regions[0].geology.strata[4]);
    printf(
      "heights: %ld : %ld : %ld : %ld : %ld : %ld\n",
      heights[0],
      heights[1],
      heights[2],
      heights[3],
      heights[4],
      heights[5]
    );
    printf(
      "pointers: %ld : %ld : %ld : %ld : %ld : %ld\n",
      wr->geology.strata[0],
      wr->geology.strata[1],
      wr->geology.strata[2],
      wr->geology.strata[3],
      wr->geology.strata[4],
      wr->geology.strata[5]
    );
    printf(
      "seeds: %ld : %ld : %ld : %ld : %ld : %ld\n",
      wr->geology.strata[0]->seed,
      wr->geology.strata[1]->seed,
      wr->geology.strata[2]->seed,
      wr->geology.strata[3]->seed,
      wr->geology.strata[4]->seed,
      wr->geology.strata[5]->seed
    );
    printf(
      "cx/cy: %.2f/%.2f : %.2f/%.2f : %.2f/%.2f : %.2f/%.2f : %.2f/%.2f : %.2f/%.2f\n",
      wr->geology.strata[0]->cx, wr->geology.strata[0]->cy,
      wr->geology.strata[1]->cx, wr->geology.strata[1]->cy,
      wr->geology.strata[2]->cx, wr->geology.strata[2]->cy,
      wr->geology.strata[3]->cx, wr->geology.strata[3]->cy,
      wr->geology.strata[4]->cx, wr->geology.strata[4]->cy,
      wr->geology.strata[5]->cx, wr->geology.strata[5]->cy
    );
    printf(
      "size: %.2f : %.2f : %.2f : %.2f : %.2f : %.2f\n",
      wr->geology.strata[0]->size,
      wr->geology.strata[1]->size,
      wr->geology.strata[2]->size,
      wr->geology.strata[3]->size,
      wr->geology.strata[4]->size,
      wr->geology.strata[5]->size
    );
    printf(
      "thickness: %.2f : %.2f : %.2f : %.2f : %.2f : %.2f\n",
      wr->geology.strata[0]->thickness,
      wr->geology.strata[1]->thickness,
      wr->geology.strata[2]->thickness,
      wr->geology.strata[3]->thickness,
      wr->geology.strata[4]->thickness,
      wr->geology.strata[5]->thickness
    );
    if (pr_rpos.x != rpos->x || pr_rpos.y != rpos->y) {
      printf("(recomputed)\n");
      printf("(recomputed)\n");
      printf("(recomputed)\n");
    }
  }
  // */

  // Figure out elevations from the bottom up:
  h = 0;
  for (i = 0; i < wr->geology.stratum_count; ++i) {
    st = wr->geology.strata[i];
    //printf("Thickness: %d\n", sd[i].thickness);
    h += heights[i];
    if (h > rpos->z) { // might not happen at all
      result->primary = b_make_species(B_STONE, st->base_species);
      // DEBUG:
      /*
      result->primary = b_make_block(B_DIRT);
      if (h == rpos->z + 1) {
        result->secondary = b_make_block(B_GRASS_ROOTS);
      }
      */
      // TODO: A real material computation:
      /*
      result->primary = stratum_material(
        rpos,
        st,
        &(sd[i])
      );
      // */
      // TODO: block data
      // TODO: caching and/or batch processing?
      break;
    // DEBUG:
    /*
    } else if (h == rpos->z) {
      result->primary = b_make_block(B_GRASS);
    // */
    }
  }
  // Keep track of our previous position:
  copy_rpos(rpos, &pr_rpos);
}

/********
 * Jobs *
 ********/

struct jm_gencolumn_s {
  // External inputs:
  world_map *world;
  region_chunk_pos target_chunk;

  // Internal variables:
  world_region *current_region;
  chunk *current_chunk;
  region_pos origin;
  region_pos current_cell;
  chunk_index current_index;
  r_pos_t stratum_heights[CHUNK_SIZE*CHUNK_SIZE*MAX_STRATA_LAYERS];
    // this is roughly 256k at 4 bytes per r_pos_t
  stratum* strata[CHUNK_SIZE*CHUNK_SIZE];
    // this holds the current stratum at each x/y position
  size_t hindices[CHUNK_SIZE*CHUNK_SIZE];
    // holds our indices within each heights column
  r_cpos_t hsofar[CHUNK_SIZE*CHUNK_SIZE];
    // holds the height so far within each column
};
typedef struct jm_gencolumn_s jm_gencolumn;

void launch_job_gencolumn(world_map *world, region_chunk_pos *target_chunk) {
  jm_gencolumn *mem = (jm_gencolumn *) malloc(sizeof(jm_gencolumn));
  mem->world = world;
  mem->target_chunk.x = target_chunk->x;
  mem->target_chunk.y = target_chunk->y;
  mem->target_chunk.z = target_chunk->z;
  mem->current_region = NULL;
  mem->current_chunk = NULL;
  mem->current_cell.x = 0;
  mem->current_cell.y = 0;
  mem->current_cell.z = 0;
  rpos__cidx(&(mem->current_cell), &(mem->current_index));
  rcpos__rpos(&(mem->target_chunk), &(mem->origin));
  start_job(&job_gencolumn, mem, NULL);
}

void (*job_gencolumn(void *jmem)) () {
  jm_gencolumn *mem = (jm_gencolumn *) jmem;
  world_map_pos wmpos;
  rcpos__wmpos(&(mem->target_chunk), &wmpos);
  // Figure out the relevant region and make sure we're at the bottom of the
  // world:
  mem->current_region = get_world_region(mem->world, &wmpos);
  if (mem->current_region == NULL) {
    // TODO: Something more sophisticated here.
    return NULL;
  }
  mem->target_chunk.z = 0;
  // Start at the very beginning of the target chunk:
  rcpos__rpos(&(mem->target_chunk), &(mem->current_cell));
  rpos__cidx(&(mem->current_cell), &(mem->current_index));
  return (void (*) ()) &job_gencolumn__init_column;
}

void (*job_gencolumn__init_column(void *jmem)) () {
  jm_gencolumn *mem = (jm_gencolumn *) jmem;
  stratum *st;
  size_t i;
  // Compute stratum heights for the current column:
  for (i = 0; i < mem->current_region->geology.stratum_count; ++i) {
    st = mem->current_region->geology.strata[i];
    if (st != NULL) {
      mem->stratum_heights[
        mem->current_index.x +
        mem->current_index.y*CHUNK_SIZE +
        i*CHUNK_SIZE*CHUNK_SIZE
      ] = compute_stratum_height(st, &(mem->current_cell));
    } else {
      mem->stratum_heights[
        mem->current_index.x +
        mem->current_index.y*CHUNK_SIZE +
        i*CHUNK_SIZE*CHUNK_SIZE
      ] = 0;
    }
  }
  // Set up our iteration variables:
  mem->strata[
    mem->current_index.x +
    mem->current_index.y*CHUNK_SIZE
  ] = mem->current_region->geology.strata[0];
  mem->hindices[mem->current_index.x + mem->current_index.y*CHUNK_SIZE] = 0;
  mem->hsofar[
    mem->current_index.x +
    mem->current_index.y*CHUNK_SIZE
  ] = mem->stratum_heights[
        mem->current_index.x +
        mem->current_index.y*CHUNK_SIZE +
        0
      ];
  // Check whether to compute a new column or continue to the column fill
  // process:
  if (
    (mem->current_cell.x == mem->origin.x + CHUNK_SIZE)
  &&
    (mem->current_cell.y == mem->origin.y + CHUNK_SIZE)
  ) {
    // Start the column fill process:
    mem->current_cell.x -= CHUNK_SIZE;
    mem->current_cell.y -= CHUNK_SIZE;
    return (void (*) ()) &job_gencolumn__fill_column;
  } else if (mem->current_cell.x == mem->origin.x + CHUNK_SIZE) {
    mem->current_cell.y += 1;
    mem->current_cell.x -= CHUNK_SIZE;
  } else {
    mem->current_cell.x += 1;
  }
  // Compute the next strata stack:
  return (void (*) ()) &job_gencolumn__init_column;
}

void (*job_gencolumn__fill_column(void *jmem)) () {
  // Fills all of the cells in a single chunk
  jm_gencolumn *mem = (jm_gencolumn *) jmem;
  chunk *c = create_chunk(&(mem->target_chunk)); // chunk to fill in
  chunk_or_approx coa; // for passing the chunk tot he compile queue
  cell *cell; // cell to fill in
  r_pos_t absheight; // absolute height of a cell
  size_t xyidx; // cached x/y index value
  uint8_t finished = 1; // are we done with this column entirely?
  chunk_index idx = {.x = 0, .y = 0, .z = 0}; // position with this chunk
  // Compute the current chunk index and region chunk position:
  rcpos__rpos(&(mem->target_chunk), &(mem->current_cell));
  // Loop through the chunk filling in cells:
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      // Cache this computation:
      xyidx = idx.x + idx.y*CHUNK_SIZE;
      if (mem->strata[xyidx] == NULL) {
        // we're done with this column
        continue;
      }
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        // Compute absolute height:
        absheight = mem->current_cell.z + idx.z;
        // Iterate through strata in this column until either the total height
        // so far is above the current height, or there are no more strata
        // left:
        while (mem->hsofar[xyidx] <= absheight && mem->strata[xyidx] != NULL) {
          mem->hindices[xyidx] += 1;
          mem->strata[xyidx] = mem->current_region->geology.strata[
            mem->hindices[xyidx]
          ];
          mem->hsofar[xyidx] += mem->stratum_heights[
            xyidx + mem->hindices[xyidx]*CHUNK_SIZE*CHUNK_SIZE
          ];
        }
        // Set the value of the current cell:
        cell = c_cell(c, idx);
        if (mem->strata[xyidx] != NULL) {
          // TODO: Different stone types here
          cell->primary = b_make_block(B_STONE);
          cell->secondary = b_make_block(B_VOID);
        } else {
          cell->primary = b_make_block(B_AIR);
          cell->secondary = b_make_block(B_VOID);
        }
      }
      // If any column still has strata, the next chunk upwards needs to be
      // generated:
      if (mem->strata[xyidx] != NULL) {
        finished = 0;
      }
    }
  }
  // Yield the generated chunk to the data subsystem for compilation:
  c->chunk_flags |= CF_LOADED;
  ch__coa(c, &coa);
  mark_for_compilation(&coa);
  // Target the next-higher chunk:
  mem->target_chunk.z += 1;
  if (finished) {
    return (void (*) ()) NULL;
  } else {
    return (void (*) ()) &job_gencolumn__fill_column;
  }
}
