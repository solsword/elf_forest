// worldgen.c
// World map generation.

#include "worldgen.h"

/******************************
 * Constructors & Destructors *
 ******************************/

world_map *create_world_map(ptrdiff_t seed, size_t width, size_t height) {
  world_map *result = (world_map*) malloc(sizeof(world_map));
  result->seed = seed;
  result->width = width;
  result->height = height;
  result->regions = (world_region *) calloc(width*height, sizeof(world_region));
  result->all_strata = create_list();
  result->all_biomes = create_list();
  result->all_civs = create_list();
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

void generate_geology(world_map *wm) {
  int i;
  world_map_pos xy;
  region_pos rpos;
  size_t stc;
  float t;
  stratum *s;
  float avg_size = sqrtf(wm->width*wm_>height) / ((float) STRATA_COMPLEXITY);
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
      for (xy.y = 0; xy.y < wm->width; ++xy.y) {
        // Compute thickness to determine if this region needs to be aware of
        // this stratum:
        // TODO: use four-corners method instead
        wmpos__rpos(&xy, &rpos);
        t = stratum_core(rpos.x, rpos.y, s) + stratum_detail(rpos.x, rpos.y, s);
        if (t > 0) { // In this case, add this stratum to this region:
          wr = &(wm->regions[xy.x+xy.y*wm->width]);
          stc = wr->geology.stratum_count;
          if (stc < MAX_STRATA_LAYERS) {
            wr->geology.strata[stc] = s;
            wr->geology.stratum_count += 1;
          } else {
            printf("Warning: strata stacked too high at %d, %d!\n", xy.x, xy.y);
          }
        }
      }
    }
  }
}
