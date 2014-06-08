// worldgen.c
// World map generation.

#include "datatypes/bitmap.h"
#include "noise/noise.h"
#include "world/blocks.h"

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
    return;
  }
  strata_cell(wr, rpos, result);
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
  ptrdiff_t hash;
  world_region *wr;
  for (i = 0; i < MAX_STRATA_LAYERS * STRATA_COMPLEXITY; ++i) {
    // Create a stratum and append it to the list of all strata:
    hash = expanded_hash_1d(wm->seed + 567*i),
    s = create_stratum(
      hash,
      randf(0, wm->width), randf(0, wm->height), // TODO: Seed these properly
      avg_size * (0.6 + randf(0, 0.8)), // size
      avg_thickness * (0.4 + randf(0, 1.2)), // thickness
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
          wr = get_world_region(wm, &xy);
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
};
typedef struct jm_gencolumn_s jm_gencolumn;

void launch_job_gencolumn(world_map *world, region_chunk_pos *target_chunk) {
  jm_gencolumn *mem = (jm_gencolumn *) malloc(sizeof(jm_gencolumn));
  mem->world = world;
  mem->target_chunk.x = target_chunk->x;
  mem->target_chunk.y = target_chunk->y;
  mem->target_chunk.z = target_chunk->z;
  mem->current_region = NULL;
  start_job(&job_gencolumn, mem, NULL);
}

void (*job_gencolumn(void *jmem)) () {
  jm_gencolumn *mem = (jm_gencolumn *) jmem;
  world_map_pos wmpos;
  rcpos__wmpos(&(mem->target_chunk), &wmpos);
  mem->current_region = get_world_region(mem->world, &wmpos);
  mem->target_chunk.z = 0;
  return (void (*) ()) &job_gencolumn__chunk;
}

void (*job_gencolumn__chunk(void *jmem)) () {
  jm_gencolumn *mem = (jm_gencolumn *) jmem;
  chunk *c = create_chunk(&(mem->target_chunk));
  chunk_index idx;
  region_pos rpos;
  for (idx.x = 0; idx.x < CHUNK_SIZE; ++idx.x) {
    for (idx.y = 0; idx.y < CHUNK_SIZE; ++idx.y) {
      for (idx.z = 0; idx.z < CHUNK_SIZE; ++idx.z) {
        cidx__rpos(c, &idx, &rpos);
        world_cell(mem->world, &rpos, c_cell(c, idx));
      }
    }
  }
  // TODO: How to write the chunk to disk/mmap?
  cleanup_chunk(c);
  mem->target_chunk.z += 1;
  // TODO: dynamic limiting!
  if (mem->target_chunk.z > 100) {
    return NULL;
  }
  return (void (*) ()) &job_gencolumn__chunk;
}
