// worldgen.c
// World map generation.

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

void setup_world_map() {
  THE_WORLD = create_world_map(WORLD_SEED, WORLD_WIDTH, WORLD_HEIGHT);
  generate_geology(THE_WORLD);
}

void world_cell(region_pos *rpos, cell *result) {
  world_map_pos wmpos;
  world_region *wr;
  rpos__wmpos(rpos, &wmpos);
  // default values:
  result->primary = b_make_block(B_VOID);
  result->secondary = b_make_block(B_VOID);
  result->p_data = 0;
  result->s_data = 0;
  wr = get_world_region(THE_WORLD, &wmpos);
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
  float t;
  stratum *s;

  float avg_size = sqrtf(wm->width*wm->height);
  avg_size /= fmax(1.0, ((float) STRATA_COMPLEXITY));
  avg_size *= WORLD_REGION_SIZE;

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
        //t = stratum_core(rpos.x, rpos.y, s) + stratum_detail(rpos.x, rpos.y, s);
        t = stratum_core(rpos.x, rpos.y, s);
        if (t > 0) { // In this case, add this stratum to this region:
          //TODO: Real logging/debugging
          //printf("Adding stratum to region at %zu, %zu.\n", xy.x, xy.y);
          wr = get_world_region(wm, &xy);
          stc = wr->geology.stratum_count;
          if (stc < MAX_STRATA_LAYERS - 1) {
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
  static stratum_dynamics sd[MAX_STRATA_LAYERS];
  size_t i;
  r_pos_t h;
  stratum *st;
  stratum *below;
  column_dynamics cd = { .pressure=0, .erosion=0 };
  // Compute strata heights from the top down:
  for (i = wr->geology.stratum_count - 1; i > -1; --i) {
    st = wr->geology.strata[i];
    if (i > 0) {
      below = wr->geology.strata[i-1];
    } else {
      below = NULL;
    }
    compute_stratum_dynamics(
      rpos->x, rpos->y,
      st, below,
      &cd, &(sd[i])
    );
  }
  // Figure out elevations from the bottom up:
  result->primary = b_make_block(B_VOID);
  result->secondary = b_make_block(B_VOID);
  result->p_data = 0;
  result->s_data = 0;
  h = 0;
  for (i = 0; i < wr->geology.stratum_count; ++i) {
    st = wr->geology.strata[i];
    sd[i].elevation = h;
    h += sd[i].thickness;
    if (h > rpos->z) { // might not happen at all
      result->primary = stratum_material(
        rpos,
        st,
        &(sd[i])
      );
      result->secondary = b_make_block(B_VOID);
      // TODO: block data
      result->p_data = 0;
      result->s_data = 0;
      // TODO: caching and/or batch processing?
      break;
    }
  }
}
