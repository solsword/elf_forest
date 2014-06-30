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

ptrdiff_t const WORLD_SEED = 7184921;

/******************************
 * Constructors & Destructors *
 ******************************/

world_map *create_world_map(ptrdiff_t seed, wm_pos_t width, wm_pos_t height) {
  world_map *result = (world_map*) malloc(sizeof(world_map));
  result->seed = seed;
  result->width = width;
  result->height = height;
  result->regions = (world_region *) calloc(width*height, sizeof(world_region));
  result->all_strata = create_list();
  result->all_biomes = create_list();
  result->all_civs = create_list();
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

void setup_worldgen() {
  THE_WORLD = create_world_map(WORLD_SEED, WORLD_WIDTH, WORLD_HEIGHT);
  generate_geology(THE_WORLD);
}

void cleanup_worldgen() {
  cleanup_world_map(THE_WORLD);
}

void world_cell(world_map *wm, region_pos *rpos, cell *result) {
  world_map_pos wmpos;
  world_region *wr;
  rpos__wmpos(rpos, &wmpos);
  // default values:
  result->primary = b_make_block(B_VOID);
  result->secondary = b_make_block(B_VOID);
  result->p_data = 0;
  result->s_data = 0;
  wr = get_world_region(wm, &wmpos);
  if (rpos->z < 0) {
    result->primary = b_make_block(B_BOUNDARY);
  } else if (wr == NULL) {
    // Outside the world:
    result->primary = b_make_block(B_AIR);
  } else {
    strata_cell(wr, rpos, result);
  }
  if (b_is(result->primary, B_VOID)) {
    // TODO: Other things like plants here...
    result->primary = b_make_block(B_AIR);
  }
}

void generate_geology(world_map *wm) {
  int i;
  world_map_pos xy;
  region_pos rpos;
  size_t stc;
  r_pos_t t;
  stratum *s;

  float avg_size = sqrtf(wm->width*wm->height);
  avg_size /= fmax(1.0, ((float) STRATA_COMPLEXITY));
  avg_size *= WORLD_REGION_SIZE * CHUNK_SIZE;

  float avg_thickness = 10.0; // TODO: Something else here
  map_function profile = MFN_SPREAD_UP;// TODO: Something else here
  ptrdiff_t hash, h1, h2, h3;
  world_region *wr;
  for (i = 0; i < MAX_STRATA_LAYERS * STRATA_COMPLEXITY; ++i) {
    // Create a stratum and append it to the list of all strata:
    hash = expanded_hash_1d(wm->seed + 567*i);
    h1 = hash_1d(hash);
    h2 = hash_1d(h1);
    h3 = hash_1d(h2);
    s = create_stratum(
      hash,
      float_hash_1d(hash)*wm->width, float_hash_1d(h1)*wm->height,
      avg_size * (0.6 + float_hash_1d(h2)*0.8), // size
      avg_thickness * (0.4 + float_hash_1d(h3)*1.2), // thickness
      profile, // profile
      GEO_IGNEOUS // TODO: Something else here
    );
    l_append_element(wm->all_strata, (void*) s);
    // Render the stratum into the various regions:
    for (xy.x = 0; xy.x < wm->width; ++xy.x) {
      for (xy.y = 0; xy.y < wm->height; ++xy.y) {
        // Compute thickness to determine if this region needs to be aware of
        // this stratum:
        // TODO: use four-corners method instead
        wmpos__rpos(&xy, &rpos);
        t = compute_stratum_height(s, &rpos);
        if (t > 0) { // In this case, add this stratum to this region:
          //TODO: Real logging/debugging
          //printf("Adding stratum to region at %zu, %zu.\n", xy.x, xy.y);
          wr = get_world_region(wm, &xy); // no need to worry about NULL here
          stc = wr->geology.stratum_count;
          if (stc < MAX_STRATA_LAYERS) {
            wr->geology.strata[stc] = s;
            wr->geology.stratum_count += 1;
          } else {
            printf(
              "Warning: strata stacked too high at %zu, %zu!\n",
              xy.x,
              xy.y
            );
          }
        }
      }
    }
  }
}

void strata_cell(
  world_region *wr,
  region_pos *rpos,
  cell *result
) {
  static r_pos_t heights[MAX_STRATA_LAYERS];
  static region_pos pr_rpos = { .x = -1, .y = -1, .z = -1 };
  int i;
  r_pos_t h;
  stratum *st;

  if (pr_rpos.x != rpos->x || pr_rpos.y != rpos->y) {
    // No need to recompute the strata column if we're at the same x/y.
    // Compute strata heights from the bottom up:
    for (i = 0; i < wr->geology.stratum_count; ++i) {
      st = wr->geology.strata[i];
      heights[i] = compute_stratum_height(st, rpos);
    }
  }
  // Figure out elevations from the bottom up:
  h = 0;
  for (i = 0; i < wr->geology.stratum_count; ++i) {
    st = wr->geology.strata[i];
    //printf("Thickness: %d\n", sd[i].thickness);
    h += heights[i];
    if (h > rpos->z) { // might not happen at all
      result->primary = b_make_block(B_STONE);
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
        mem->current_cell.x +
        mem->current_cell.y*CHUNK_SIZE +
        i*CHUNK_SIZE*CHUNK_SIZE
      ] = compute_stratum_height(st, &(mem->current_cell));
    } else {
      mem->stratum_heights[
        mem->current_cell.x +
        mem->current_cell.y*CHUNK_SIZE +
        i*CHUNK_SIZE*CHUNK_SIZE
      ] = 0;
    }
  }
  // Set up our iteration variables:
  mem->strata[
    mem->current_cell.x +
    mem->current_cell.y*CHUNK_SIZE
  ] = mem->current_region->geology.strata[0];
  mem->hindices[mem->current_cell.x + mem->current_cell.y*CHUNK_SIZE] = 0;
  mem->hsofar[
    mem->current_cell.x +
    mem->current_cell.y*CHUNK_SIZE
  ] = mem->stratum_heights[
        mem->current_cell.x +
        mem->current_cell.y*CHUNK_SIZE +
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
