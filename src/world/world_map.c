// world_map.c
// World map structure definition.

#ifdef DEBUG
  #include <stdio.h>
#endif

#include "noise/noise.h"

#include "datatypes/list.h"
#include "datatypes/queue.h"
#include "datatypes/map.h"

#include "gen/terrain.h"
#include "gen/geology.h"
#include "gen/climate.h"
#include "gen/biology.h"
#include "math/curve.h"

#include "world_map.h"

/***********
 * Globals *
 ***********/

// The globally-accessible world:
world_map* THE_WORLD;

/******************************
 * Constructors & Destructors *
 ******************************/

world_map *create_world_map(ptrdiff_t seed, wm_pos_t width, wm_pos_t height) {
  world_map *result = (world_map*) malloc(sizeof(world_map));

  result->seed = prng(prng(prng(seed)));
  result->width = width;
  result->height = height;
  result->regions = (world_region *) calloc(width*height, sizeof(world_region));
  result->tectonics = create_tectonic_sheet(
    prng(seed + 65442),
    (size_t) (2.0 * (width / TECTONIC_SHEET_SCALE)),
    (size_t) (height / TECTONIC_SHEET_SCALE)
  );

  result->air_elements = create_list();
  result->water_elements = create_list();
  result->life_elements = create_list();
  result->stone_elements = create_list();
  result->metal_elements = create_list();
  result->rare_elements = create_list();

  result->all_elements = create_list();
  result->all_nutrients = create_list();
  result->all_strata = create_list();
  result->all_water = create_list();
  result->all_rivers = create_list();
  result->all_biomes = create_list();
  result->all_civs = create_list();

  return result;
}

void cleanup_world_map(world_map *wm) {
  // most of these don't need special cleanup beyond a free()
  destroy_list(wm->all_strata);
  destroy_list(wm->all_water);
  destroy_list(wm->all_rivers);
  l_foreach(wm->all_biomes, &cleanup_v_biome);
  cleanup_list(wm->all_biomes);
  destroy_list(wm->all_civs);
  free(wm->regions);
  free(wm);
}

biome* create_biome(biome_category category) {
  biome *result = (biome*) malloc(sizeof(biome));
  result->category = category;

  result->niches = create_list();

  result->hanging_terrestrial_flora = create_list();
  result->ephemeral_terrestrial_flora = create_list();
  result->ubiquitous_terrestrial_flora = create_list();
  result->close_spaced_terrestrial_flora = create_list();
  result->medium_spaced_terrestrial_flora = create_list();
  result->wide_spaced_terrestrial_flora = create_list();

  result->hanging_subterranean_flora = create_list();
  result->ephemeral_subterranean_flora = create_list();
  result->ubiquitous_subterranean_flora = create_list();
  result->close_spaced_subterranean_flora = create_list();
  result->medium_spaced_subterranean_flora = create_list();
  result->wide_spaced_subterranean_flora = create_list();

  result->ephemeral_aquatic_flora = create_list();
  result->ubiquitous_aquatic_flora = create_list();
  result->close_spaced_aquatic_flora = create_list();
  result->medium_spaced_aquatic_flora = create_list();
  result->wide_spaced_aquatic_flora = create_list();


  // TODO: Fauna

  return result;
}

CLEANUP_IMPL(biome) {
  destroy_list(doomed->niches); // free() is enough for a niche

  cleanup_list(doomed->hanging_terrestrial_flora);
  cleanup_list(doomed->ephemeral_terrestrial_flora);
  cleanup_list(doomed->ubiquitous_terrestrial_flora);
  cleanup_list(doomed->close_spaced_terrestrial_flora);
  cleanup_list(doomed->medium_spaced_terrestrial_flora);
  cleanup_list(doomed->wide_spaced_terrestrial_flora);

  cleanup_list(doomed->hanging_subterranean_flora);
  cleanup_list(doomed->ephemeral_subterranean_flora);
  cleanup_list(doomed->ubiquitous_subterranean_flora);
  cleanup_list(doomed->close_spaced_subterranean_flora);
  cleanup_list(doomed->medium_spaced_subterranean_flora);
  cleanup_list(doomed->wide_spaced_subterranean_flora);

  cleanup_list(doomed->ephemeral_aquatic_flora);
  cleanup_list(doomed->ubiquitous_aquatic_flora);
  cleanup_list(doomed->close_spaced_aquatic_flora);
  cleanup_list(doomed->medium_spaced_aquatic_flora);
  cleanup_list(doomed->wide_spaced_aquatic_flora);

  free(doomed);
}

biome* create_merged_biome(
  world_region *wr1,
  world_region *wr2,
  float str1,
  float str2
) {
  biome *result = create_biome(WM_BC_UNK);
  
  if (str1 + str2 == 0) {
#ifdef DEBUG
    printf(
      "Warning: Region contender strengths are both zero: using 100%% wr1.\n"
    );
#endif
    str1 = 1.0;
  }

  if (wr1 == NULL) {
    return result; // return an empty biome if we're outside the world map
    // TODO: Something else here?
  }

  merge_all_biomes_into(wr1, str1 / (str1 + str2), result);
  if (wr2 != NULL) {
    merge_all_biomes_into(wr2, str2 / (str1 + str2), result);
  }
  return result;
}

void cleanup_biome(biome *b) {
  cleanup_list(b->hanging_terrestrial_flora);
  cleanup_list(b->ephemeral_terrestrial_flora);
  cleanup_list(b->ubiquitous_terrestrial_flora);
  cleanup_list(b->close_spaced_terrestrial_flora);
  cleanup_list(b->medium_spaced_terrestrial_flora);
  cleanup_list(b->wide_spaced_terrestrial_flora);

  cleanup_list(b->hanging_subterranean_flora);
  cleanup_list(b->ephemeral_subterranean_flora);
  cleanup_list(b->ubiquitous_subterranean_flora);
  cleanup_list(b->close_spaced_subterranean_flora);
  cleanup_list(b->medium_spaced_subterranean_flora);
  cleanup_list(b->wide_spaced_subterranean_flora);

  cleanup_list(b->ephemeral_aquatic_flora);
  cleanup_list(b->ubiquitous_aquatic_flora);
  cleanup_list(b->close_spaced_aquatic_flora);
  cleanup_list(b->medium_spaced_aquatic_flora);
  cleanup_list(b->wide_spaced_aquatic_flora);

  // TODO: Fauna

  free(b);
}

/*************
 * Functions *
 *************/

altitude_category classify_altitude(float altitude) {
  if (
    altitude
  < (
      TR_HEIGHT_OCEAN_DEPTHS
    + 0.9 * (TR_HEIGHT_CONTINENTAL_SHELF - TR_HEIGHT_OCEAN_DEPTHS)
    )
  ) {
    return WM_AC_OCEAN_DEPTHS;
  } else if (altitude < TR_HEIGHT_SEA_LEVEL) {
    return WM_AC_CONT_SHELF;
  } else if (
    altitude
  < (
      TR_HEIGHT_COASTAL_PLAINS
    + 0.3 * (TR_HEIGHT_HIGHLANDS - TR_HEIGHT_COASTAL_PLAINS)
    )
  ) {
    return WM_AC_COASTAL_PLAINS;
  } else if (
    altitude
  < (
      TR_HEIGHT_COASTAL_PLAINS
    + 0.9 * (TR_HEIGHT_HIGHLANDS - TR_HEIGHT_COASTAL_PLAINS)
    )
  ) {
    return WM_AC_INLAND_HILLS;
  } else if (
    altitude
  < (
      TR_HEIGHT_HIGHLANDS
    + 0.9 * (TR_HEIGHT_MOUNTAIN_BASES - TR_HEIGHT_HIGHLANDS)
    )
  ) {
    return WM_AC_HIGHLANDS;
  } else if (
    altitude
  < (
      TR_HEIGHT_MOUNTAIN_BASES
    + 0.4 * (TR_HEIGHT_MOUNTAIN_TOPS - TR_HEIGHT_MOUNTAIN_BASES)
    )
  ) {
    return WM_AC_MOUNTAIN_SLOPES;
  } else {
    return WM_AC_MOUNTAIN_PEAKS;
  }
}

precipitation_category classify_precipitation(float *precipitation) {
  size_t i;
  float p;
  float mean, min, max;

  // compute min/max and mean:
  mean = 0;
  min = precipitation[0];
  max = precipitation[0];
  for (i = 0; i < WM_N_SEASONS; ++i) {
    p = precipitation[i];
    if (p > max) { max = p; }
    if (p < min) { min = p; }
    mean += p;
  }
  mean /= (float) WM_N_SEASONS;

  // classify:
  if (mean < CL_PRECIPITATION_DESERT) {
    return WM_PC_DESERT;
  } else if (mean < CL_PRECIPITATION_ARID) {
    return WM_PC_ARID;
  } else if (
    mean
  < (
      CL_PRECIPITATION_ARID
    + 0.6 * (CL_PRECIPITATION_NORMAL - CL_PRECIPITATION_ARID)
    )
  ) {
    return WM_PC_DRY;
  } else if (
    mean
  < (
      CL_PRECIPITATION_ARID
    + 0.4 * (CL_PRECIPITATION_NORMAL - CL_PRECIPITATION_ARID)
    )
  ) {
    if (max - min > CL_PRECIPITATION_NORMAL - CL_PRECIPITATION_ARID) {
      return WM_PC_SEASONAL;
    } else {
      return WM_PC_NORMAL;
    }
  } else if (mean < CL_PRECIPITATION_LUSH) {
    return WM_PC_WET;
  } else if (mean < CL_PRECIPITATION_SOAKED) {
    return WM_PC_SOAKING;
  } else {
    return WM_PC_FLOODED;
  }
}

temperature_category classify_temperature(
  float *lows,
  float *means
) {
  size_t i;
  float low, mean, high;

  // compute min and mean:
  mean = 0; // mean throughout the whole year
  low = lows[0]; // winter low
  high = means[0]; // summer mean
  for (i = 0; i < WM_N_SEASONS; ++i) {
    if (lows[i] < low) { low = lows[i]; }
    if (means[i] > high) { high = means[i]; }
    mean += means[i];
  }
  mean /= (float) WM_N_SEASONS;

  // classify:
  if (
    high
  < (
      CL_TEMP_ANTARCTIC_YEAR_HIGH
    + 0.6 * (CL_TEMP_TUNDRA_YEAR_HIGH - CL_TEMP_ANTARCTIC_YEAR_HIGH)
    )
  ) {
    return WM_TC_ARCTIC;
  } else if (
    mean
  < (
      CL_TEMP_TUNDRA_YEAR_MEAN
    + 0.5 * (CL_TEMP_COLD_TEMPERATE_YEAR_MEAN - CL_TEMP_TUNDRA_YEAR_MEAN)
    )
  ) {
    return WM_TC_TUNDRA;
  } else if (
    mean
  < (
      CL_TEMP_COLD_TEMPERATE_YEAR_MEAN
    + 0.5 * (
        CL_TEMP_MODERATE_TEMPERATE_YEAR_MEAN
      - CL_TEMP_COLD_TEMPERATE_YEAR_MEAN
      )
    )
  ) {
    if (low < CL_TEMP_SEASON_LOW_REGULAR_FROST) {
      return WM_TC_COLD_FROST;
    } else {
      return WM_TC_COLD_RARE_FROST;
    }
  } else if (
    mean
  < (
      CL_TEMP_MODERATE_TEMPERATE_YEAR_MEAN
    + 0.5 * (
        CL_TEMP_WARM_TEMPERATE_YEAR_MEAN
      - CL_TEMP_MODERATE_TEMPERATE_YEAR_MEAN
      )
    )
  ) {
    if (low < CL_TEMP_SEASON_LOW_INTERMITTENT_FROST) {
      return WM_TC_MILD_FROST;
    } else {
      return WM_TC_MILD_RARE_FROST;
    }
  } else if (
    mean
  < (
      CL_TEMP_WARM_TEMPERATE_YEAR_MEAN
    + 0.5 * (CL_TEMP_SUBTROPICAL_YEAR_MEAN - CL_TEMP_WARM_TEMPERATE_YEAR_MEAN)
    )
  ) {
    if (low < CL_TEMP_SEASON_LOW_INTERMITTENT_FROST) {
      return WM_TC_WARM_FROST;
    } else {
      return WM_TC_WARM_NO_FROST;
    }
  } else if (
    mean
  < (
      CL_TEMP_SUBTROPICAL_YEAR_MEAN
    + 0.5 * (CL_TEMP_TROPICAL_YEAR_MEAN - CL_TEMP_SUBTROPICAL_YEAR_MEAN)
    )
  ) {
    return WM_TC_HOT;
  } else {
    return WM_TC_TROPICAL;
  }
}

void summarize_region(world_region *wr) {
  wr->s_altitude = classify_altitude(wr->topography.geologic_height);
  wr->s_precipitation = classify_precipitation(wr->climate.atmosphere.rainfall);
  wr->s_temperature = classify_temperature(
    wr->climate.atmosphere.temp_low,
    wr->climate.atmosphere.temp_mean
  );
}

void summarize_all_regions(world_map *wm) {
  world_map_pos iter;
  for (iter.x = 0; iter.x < wm->width; ++iter.x) {
    for (iter.y = 0; iter.y < wm->height; ++iter.y) {
      summarize_region(get_world_region(wm, &iter));
    }
  }
}

void get_world_neighborhood_small(
  world_map *wm,
  world_map_pos *wmpos,
  world_region* neighborhood[]
) {
  world_map_pos iter;
  size_t i = 0;
  for (iter.x = wmpos->x - 1; iter.x <= wmpos->x + 1; iter.x += 1) {
    for (iter.y = wmpos->y - 1; iter.y <= wmpos->y + 1; iter.y += 1) {
      neighborhood[i] = get_world_region(wm, &iter);
      i += 1;
    }
  }
}

void get_world_neighborhood(
  world_map *wm,
  world_map_pos *wmpos,
  world_region* neighborhood[]
) {
  world_map_pos iter;
  size_t i = 0;
  for (iter.x = wmpos->x - 2; iter.x <= wmpos->x + 2; iter.x += 1) {
    for (iter.y = wmpos->y - 2; iter.y <= wmpos->y + 2; iter.y += 1) {
      neighborhood[i] = get_world_region(wm, &iter);
      i += 1;
    }
  }
}

void compute_region_interpolation_values(
  world_map *wm,
  world_region* neighborhood[],
  global_pos *glpos,
  manifold_point result[]
) {
  world_region *wr;
  size_t i;
  vector v;
  global_pos anchor;
  world_map_pos wmpos, xy;
  float str, slope;

  glpos__wmpos(glpos, &wmpos);

  i = 0;
  for (xy.x = wmpos.x - 2; xy.x <= wmpos.x + 2; xy.x += 1) {
    for (xy.y = wmpos.y - 2; xy.y <= wmpos.y + 2; xy.y += 1) {
      wr = neighborhood[i];
      if (wr != NULL) {
        copy_glpos(&(wr->anchor), &anchor);
      } else {
        compute_region_anchor(wm, &xy, &anchor);
      }
      v.x = glpos->x - anchor.x;
      v.y = glpos->y - anchor.y;
      v.z = glpos->z - anchor.z;

      // Map the distance to this anchor to [0, 1]
      str = 1 - (sqrtf(v.x*v.x + v.y*v.y) / MAX_REGION_INFULENCE_DISTANCE);
      vnorm(&v); // we'll need this for dx/dy
      // if we're beyond the edge of influence, the result is 0
      if (str < 0) {
        result[i].z = 0;
        result[i].dx = 0;
        result[i].dy = 0;
      } else {
        // A biased sigmoid:
        slope = strict_sigmoid_slope(str, WORLD_REGION_INFLUENCE_SHAPE);
        str = strict_sigmoid(str, WORLD_REGION_INFLUENCE_SHAPE);
        result[i].z = str;
        result[i].dx = v.x * slope;
        result[i].dy = v.y * slope;
      }

      // Update i:
      i += 1;
    }
  }
}

void compute_region_contenders(
  world_map *wm,
  world_region* neighborhood[],
  global_pos *glpos,
  ptrdiff_t seed,
  world_region **r_best, world_region **r_secondbest,
  float *r_strbest, float *r_strsecond
) {
  world_region *wr; // best and second-best regions
  size_t i;
  global_pos anchor;
  vector v, vbest, vsecond;
  float str, noise;
  world_map_pos wmpos;
  ptrdiff_t local_seed;

  seed = prng(seed + 172841);

  glpos__wmpos(glpos, &wmpos);

  // Figure out the two nearest world regions:
  // Setup worst-case defaults:
  vbest.x = WORLD_REGION_BLOCKS;
  vbest.y = WORLD_REGION_BLOCKS;
  vbest.z = 0;
  *r_strbest = 0;
  vsecond.x = WORLD_REGION_BLOCKS;
  vsecond.y = WORLD_REGION_BLOCKS;
  vsecond.z = 0;
  *r_strsecond = 0;

  *r_secondbest = NULL;
  *r_best = NULL;

  // Figure out which of our neighbors are closest:
  wmpos.x -= 1;
  wmpos.y -= 1;
  for (i = 0; i < 9; i += 1) {
    wr = neighborhood[i];
    if (wr != NULL) {
      copy_glpos(&(wr->anchor), &anchor);
      local_seed = prng(wr->seed + seed);
    } else {
      compute_region_anchor(wm, &wmpos, &anchor);
      local_seed = prng(prng(prng(wmpos.x) + wmpos.y) + seed + 51923);
    }
    v.x = glpos->x - anchor.x;
    v.y = glpos->y - anchor.y;
    v.z = glpos->z - anchor.z;
    str = 1 - (vmag(&v) / MAX_REGION_ANCHOR_DISTANCE);
    // 3D base noise:
    noise = (
      sxnoise_3d(
        v.x * WM_REGION_CONTENTION_NOISE_SCALE,
        v.y * WM_REGION_CONTENTION_NOISE_SCALE,
        v.z * WM_REGION_CONTENTION_NOISE_SCALE,
        local_seed * 576 + 9123
      ) + 
      0.6 * sxnoise_3d(
        v.x * WM_REGION_CONTENTION_NOISE_SCALE * 2.1,
        v.y * WM_REGION_CONTENTION_NOISE_SCALE * 2.1,
        v.z * WM_REGION_CONTENTION_NOISE_SCALE * 2.1,
        local_seed * 577 + 9124
      )
    ) / 1.6;
    local_seed = prng(local_seed);
    noise = (1 + noise * WM_REGION_CONTENTION_NOISE_STRENGTH) / 2.0;
    // [
    //   0.5 - WM_REGION_CONTENTION_NOISE_STRENGTH/2,
    //   0.5 + WM_REGION_CONTENTION_NOISE_STRENGTH/2
    // ]
    str *= noise;
    // Polar noise:
    vxyz__polar(&v, &v);
    noise = tiled_func(
      &sxnoise_2d,
      v.y * WM_REGION_CONTENTION_POLAR_SCALE,
      v.z * WM_REGION_CONTENTION_POLAR_SCALE,
      2*M_PI*WM_REGION_CONTENTION_POLAR_SCALE,
      2*M_PI*WM_REGION_CONTENTION_POLAR_SCALE,
      local_seed
    );
    vpolar__xyz(&v, &v);
    noise = (1 + noise * WM_REGION_CONTENTION_POLAR_STRENGTH) / 2.0;
    // [
    //   0.5 - WM_REGION_CONTENTION_POLAR_STRENGTH/2,
    //   0.5 + WM_REGION_CONTENTION_POLAR_STRENGTH/2
    // ]
    str *= noise;
    if (str > *r_strbest) {
      vcopy_as(&vsecond, &vbest);
      *r_strsecond = *r_strbest;
      *r_secondbest = *r_best;

      vcopy_as(&vbest, &v);
      *r_strbest = str;
      *r_best = wr;
    } else if (str > *r_strsecond) {
      vcopy_as(&vsecond, &v);
      *r_strsecond = str;
      *r_secondbest = wr;
    }
    // Update wmpos based on i:
    if (i == 2 || i == 5) {
      wmpos.x += 1;
      wmpos.y -= 2;
    } else {
      wmpos.y += 1;
    }
  }
}

int find_geopt_in_wr(void *v_gpt, void *v_wr_ptr) {
  geopt pt = (geopt) v_gpt;
  world_region *wr = (world_region*) v_wr_ptr;
  world_map_pos wmpos;
  geopt__wmpos(wr->world, &pt, &wmpos);
  return (wmpos.x == wr->pos.x && wmpos.y == wr->pos.y);
}

void find_valley(world_map *wm, world_map_pos *pos) {
  world_region *wr;
  wr = get_world_region(wm, pos);
  if (wr == NULL) {
    return;
  }

  while (wr->topography.downhill != NULL) {
    wr = wr->topography.downhill;
  }
  copy_wmpos(&(wr->pos), pos);
}

int breadth_first_iter(
  world_map *wm,
  world_map_pos *origin,
  int min_size,
  int max_size,
  void *arg,
  step_result (*process)(search_step, world_region*, void*)
) {
  world_map_pos wmpos;
  queue *open = create_queue();
  map *visited = create_map(1, 2048);
  world_region *this, *next;

  int size = 0, sresult;

  this = get_world_region(wm, origin);
  if (this != NULL) {
    q_push_element(open, this);
    m_put_value(visited, (void*) 1, (map_key_t) this);
    process(SSTEP_INIT, this, arg);
  }

  while (!q_is_empty(open) && (max_size < 0 || size <= max_size)) {
    // Grab the next open region:
    this = (world_region*) q_pop_element(open);
    sresult = process(SSTEP_PROCESS, this, arg);
    if (sresult == SRESULT_ABORT) {
      cleanup_queue(open);
      cleanup_map(visited);
      process(SSTEP_CLEANUP, this, arg);
      return 0;
    } else if (sresult == SRESULT_FINISHED) {
      process(SSTEP_FINISH, this, arg);
      process(SSTEP_CLEANUP, this, arg);
      cleanup_queue(open);
      cleanup_map(visited);
      return 1;
    } else if (sresult == SRESULT_IGNORE) {
      continue;
    } else { // SRESULT_CONTINUE
      size += 1;
      // Add our neighbors to the open list:
      wmpos.x = this->pos.x + 1;
      wmpos.y = this->pos.y;
      next = get_world_region(wm, &wmpos);
      if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
        q_push_element(open, next);
        m_put_value(visited, (void*) 1, (map_key_t) next);
      }
      wmpos.x -= 2;
      next = get_world_region(wm, &wmpos);
      if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
        q_push_element(open, next);
        m_put_value(visited, (void*) 1, (map_key_t) next);
      }
      wmpos.x += 2;
      wmpos.y -= 1;
      next = get_world_region(wm, &wmpos);
      if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
        q_push_element(open, next);
        m_put_value(visited, (void*) 1, (map_key_t) next);
      }
      wmpos.y += 2;
      next = get_world_region(wm, &wmpos);
      if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
        q_push_element(open, next);
        m_put_value(visited, (void*) 1, (map_key_t) next);
      }
    }
  }
  if ((max_size >= 0 && size > max_size) || size < min_size) {
    // We hit one of the size limits: cleanup without finishing and fail.
    cleanup_queue(open);
    cleanup_map(visited);
    process(SSTEP_CLEANUP, NULL, arg);
    return 0;
  }
  // Otherwise we should finish, cleanup, and succeed.
  process(SSTEP_FINISH, NULL, arg);
  process(SSTEP_CLEANUP, NULL, arg);
  cleanup_queue(open);
  cleanup_map(visited);
  return 1;
}

int blob_first_iter(
  world_map *wm,
  world_map_pos *origin,
  int min_size,
  int max_size,
  int fill_edges,
  int smoothness,
  ptrdiff_t seed,
  void *arg,
  step_result (*process)(search_step, world_region*, void*)
) {
  world_map_pos wmpos;
  queue *open = create_queue();
  map *visited = create_map(1, 2048);
  world_region *this, *next;
  seed = prng(seed);

  int size = 0, sresult, stopping = 0;

  this = get_world_region(wm, origin);
  if (this != NULL) {
    q_push_element(open, this);
    m_put_value(visited, (void*) 1, (map_key_t) this);
    process(SSTEP_INIT, this, arg);
  }

  while (
    !q_is_empty(open)
 && (fill_edges || max_size < 0 || size <= max_size)
  ) {
    if (fill_edges && max_size >= 0 && size > max_size) {
      stopping = 1;
    }
    // Shuffle the queue regularly:
    if (size % smoothness == 1 || smoothness <= 1) {
      q_shuffle(open, seed);
      seed = prng(seed);
    }
    // Grab the next open region:
    this = (world_region*) q_pop_element(open);
    sresult = process(SSTEP_PROCESS, this, arg);
    if (sresult == SRESULT_ABORT) {
      process(SSTEP_CLEANUP, this, arg);
      cleanup_queue(open);
      cleanup_map(visited);
      return 0;
    } else if (sresult == SRESULT_FINISHED) {
      if (fill_edges) {
        size += 1;
        stopping = 1;
      } else {
        process(SSTEP_FINISH, this, arg);
        process(SSTEP_CLEANUP, this, arg);
        cleanup_queue(open);
        cleanup_map(visited);
        return 1;
      }
    } else if (sresult == SRESULT_IGNORE) {
      continue;
    } else { // SRESULT_CONTINUE
      size += 1;
      if (!stopping) {
        // Add our neighbors to the open list:
        wmpos.x = this->pos.x + 1;
        wmpos.y = this->pos.y;
        next = get_world_region(wm, &wmpos);
        if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
          q_push_element(open, next);
          m_put_value(visited, (void*) 1, (map_key_t) next);
        }
        wmpos.x -= 2;
        next = get_world_region(wm, &wmpos);
        if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
          q_push_element(open, next);
          m_put_value(visited, (void*) 1, (map_key_t) next);
        }
        wmpos.x += 2;
        wmpos.y -= 1;
        next = get_world_region(wm, &wmpos);
        if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
          q_push_element(open, next);
          m_put_value(visited, (void*) 1, (map_key_t) next);
        }
        wmpos.y += 2;
        next = get_world_region(wm, &wmpos);
        if (next != NULL && !m_contains_key(visited, (map_key_t) next)) {
          q_push_element(open, next);
          m_put_value(visited, (void*) 1, (map_key_t) next);
        }
      }
    }
  }
  if ((!fill_edges && max_size >= 0 && size > max_size) || size < min_size) {
    // We hit one of the size limits: cleanup without finishing and fail.
    cleanup_queue(open);
    cleanup_map(visited);
    process(SSTEP_CLEANUP, NULL, arg);
    return 0;
  }
  // Otherwise we should finish, cleanup, and succeed.
  process(SSTEP_FINISH, NULL, arg);
  process(SSTEP_CLEANUP, NULL, arg);
  cleanup_queue(open);
  cleanup_map(visited);
  return 1;
}

void fill_with_regions(
  world_map *wm,
  void *arg,
  int (*validate)(world_region*, void*, ptrdiff_t),
  int (*fill)(world_region*, void*, ptrdiff_t),
  ptrdiff_t seed
) {
  // TODO: Introduce noisy breadth-first search to avoid diamond-shaped regions
  // everywhere?
  bitmap *valid = create_bitmap(wm->width * wm->height);
  world_map_pos wmpos;
  world_region *wr;
  ptrdiff_t remaining, chosen, orig_seed;

  seed = prng(seed + 44655411);
  orig_seed = seed;

  while (1) { // break when no valid regions remain (see middle)
    // Reset our valid mask and create a new one:
    bm_clear_bits(valid, 0, wm->width * wm->height);
    for (wmpos.x = 0; wmpos.x < wm->width; ++wmpos.x) {
      for (wmpos.y = 0; wmpos.y < wm->height; ++wmpos.y) {
        wr = get_world_region(wm, &wmpos); // no need to worry about NULL
        if (validate(wr, arg, orig_seed)) {
          bm_set_bits(valid, wmpos.x + wm->width * wmpos.y, 1);
        }
      }
    }

    // Check that we've still got stuff to fill:
    remaining = bm_popcount(valid);
    if (remaining == 0) {
      break;
    }

    // Find a random valid spot to fill from:
    chosen = posmod(seed, remaining);
    seed = prng(seed);
    chosen = bm_select_closed(valid, chosen);
#ifdef DEBUG
    if (chosen < 0) {
      printf("Error: failed to properly select a valid world region!\n");
      exit(EXIT_FAILURE);
    }
#endif
    wmpos.x = chosen % wm->width;
    wmpos.y = chosen / wm->width;
    wr = get_world_region(wm, &wmpos); // no need to worry about NULL

    // Call our fill function:
    if (!fill(wr, arg, orig_seed)) {
      // if our fill function returns false we immediately do the same:
      return;
    }

    // Now we loop back and fill from a different spot.
  }
}

void add_biome(world_region *wr, biome *b) {
  if (wr->ecology.biome_count < WM_MAX_BIOME_OVERLAP) {
    wr->ecology.biomes[wr->ecology.biome_count] = b;
    wr->ecology.biome_count += 1;
#ifdef DEBUG
  } else {
    printf("Warning: Biome not added due to overlap limit.\n");
#endif
  }
}

void merge_all_biomes_into(world_region *wr, float strength, biome *target) {
  size_t i;
  for (i = 0; i < wr->ecology.biome_count; ++i) {
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->hanging_terrestrial_flora,
      strength,
      target->hanging_terrestrial_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->ephemeral_terrestrial_flora,
      strength,
      target->ephemeral_terrestrial_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->ubiquitous_terrestrial_flora,
      strength,
      target->ubiquitous_terrestrial_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->close_spaced_terrestrial_flora,
      strength,
      target->close_spaced_terrestrial_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->medium_spaced_terrestrial_flora,
      strength,
      target->medium_spaced_terrestrial_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->wide_spaced_terrestrial_flora,
      strength,
      target->wide_spaced_terrestrial_flora
    );

    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->hanging_subterranean_flora,
      strength,
      target->hanging_subterranean_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->ephemeral_subterranean_flora,
      strength,
      target->ephemeral_subterranean_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->ubiquitous_subterranean_flora,
      strength,
      target->ubiquitous_subterranean_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->close_spaced_subterranean_flora,
      strength,
      target->close_spaced_subterranean_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->medium_spaced_subterranean_flora,
      strength,
      target->medium_spaced_subterranean_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->wide_spaced_subterranean_flora,
      strength,
      target->wide_spaced_subterranean_flora
    );

    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->ephemeral_aquatic_flora,
      strength,
      target->ephemeral_aquatic_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->ubiquitous_aquatic_flora,
      strength,
      target->ubiquitous_aquatic_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->close_spaced_aquatic_flora,
      strength,
      target->close_spaced_aquatic_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->medium_spaced_aquatic_flora,
      strength,
      target->medium_spaced_aquatic_flora
    );
    merge_and_scale_frequent_species(
      wr->ecology.biomes[i]->wide_spaced_aquatic_flora,
      strength,
      target->wide_spaced_aquatic_flora
    );
  }
}


void merge_and_scale_frequent_species(
  list const * const from,
  float str,
  list *to
) {
  size_t i;
  frequent_species fqsp;
  for (i = 0; i < l_get_length(from); ++i) {
    fqsp = (frequent_species) l_get_item(from, i);
    frequent_species_set_frequency(
      &fqsp,
      frequent_species_frequency(fqsp) * str
    );
    l_append_element(to, (void*) fqsp);
  }
}

species pick_element(
  world_map *wm,
  element_categorization constraints,
  list *exclude,
  ptrdiff_t seed
) {
  size_t i;
  list *valid;
  species result = SP_INVALID;
  if (constraints == EL_CATEGORY_NONE) {
    return SP_INVALID;
  }
  valid = create_list();
  for (i = 0; i < l_get_length(wm->all_elements); ++i) {
    element_species *el = (element_species*) l_get_item(wm->all_elements, i);
    if (
        (
          constraints == EL_CATEGORY_ANY
       || el_is_member_of_any(el, constraints)
        )
     && (
          exclude == NULL
       || !l_contains(exclude, species__v(el->id))
        )
    ) {
      l_append_element(valid, species__v(el->id));
    }
  }
  if (!l_is_empty(valid)) {
//#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    result = v__species(l_pick_random(valid, seed));
//#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
  } // else result is still SP_INVALID
  cleanup_list(valid);
  return result;
}
