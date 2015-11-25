// world_map.c
// World map structure definition.

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
  // none of these need special cleanup beyond a free()
  destroy_list(wm->all_strata);
  destroy_list(wm->all_water);
  destroy_list(wm->all_rivers);
  destroy_list(wm->all_biomes);
  destroy_list(wm->all_civs);
  free(wm->regions);
  free(wm);
}

void create_biome(biome_category category) {
  biome *result = (biome*) malloc(sizeof(biome));

  result->category = category;

  result->all_plants = create_list();
  result->mushrooms = create_list();
  result->giant_mushrooms = create_list();
  result->mosses = create_list();
  result->grasses = create_list();
  result->vines = create_list();
  result->herbs = create_list();
  result->bushes = create_list();
  result->shrubs = create_list();
  result->trees = create_list();
  result->aquatic_grasses = create_list();
  result->aquatic_plants = create_list();
  result->corals = create_list();

  // TODO: Animals as well!

  return result;
}

void cleanup_biome(biome *b) {
  cleanup_list(b->all_plant_species);
  cleanup_list(b->mushrooms);
  cleanup_list(b->giant_mushrooms);
  cleanup_list(b->mosses);
  cleanup_list(b->grasses);
  cleanup_list(b->vines);
  cleanup_list(b->herbs);
  cleanup_list(b->bushes);
  cleanup_list(b->shrubs);
  cleanup_list(b->trees);
  cleanup_list(b->aquatic_grasses);
  cleanup_list(b->aquatic_plants);
  cleanup_list(b->corals);
  free(b);
}

/*************
 * Functions *
 *************/

altitude_category classify_altitude(float altitude) {
  if (
    altitude
  < (
      WM_TR_HEIGHT_OCEAN_DEPTHS
    + 0.9 * (WM_TR_HEIGHT_CONTINENTAL_SHELF - WM_TR_HEIGHT_OCEAN_DEPTHS)
    )
  ) {
    return WM_AC_OCEAN_DEPTHS;
  } else if (altitude < WM_TR_HEIGHT_SEA_LEVEL) {
    return WM_AC_CONTINENTAL_SHELF;
  } else if (
    altitude
  < (
      WM_TR_HEIGHT_COASTAL_PLAINS
    + 0.3 * (WM_TR_HEIGHT_HIGHLANDS - WM_TR_HEIGHT_COASTAL_PLAINS)
    )
  ) {
    return WM_AC_COASTAL_PLAINS;
  } else if (
    altitude
  < (
      WM_TR_HEIGHT_COASTAL_PLAINS
    + 0.9 * (WM_TR_HEIGHT_HIGHLANDS - WM_TR_HEIGHT_COASTAL_PLAINS)
    )
  ) {
    return WM_AC_INLAND_HILLS;
  } else if (
    altitude
  < (
      WM_TR_HEIGHT_HIGHLANDS
    + 0.9 * (WM_TR_HEIGHT_MOUNTAIN_BASES - WM_TR_HEIGHT_HIGHLANDS)
    )
  ) {
    return WM_AC_HIGHLANDS;
  } else if (
    altitude
  < (
      WM_TR_HEIGHT_MOUNTAIN_BASES
    + 0.4 * (WM_TR_HEIGHT_MOUNTAIN_TOPS - WM_TR_HEIGHT_MOUNTAIN_BASES)
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
    return WM_TC_BAKING;
  }
}

void summarize_region(world_region *wr) {
  wr->s_altitude = classify_altitude(wr->topology.geologic_height);
  wr->precipitation = classify_precipitation(wr->climate.atmosphere.rainfall);
  wr->temperature = classify_temperature(
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
  world_region **best, world_region **secondbest,
  float *strbest, float *strsecond
) {
  world_region *wr; // best and second-best regions
  size_t i;
  global_pos anchor;
  vector v, vbest, vsecond;
  float str, noise;
  ptrdiff_t seed;
  world_map_pos wmpos;

  glpos__wmpos(glpos, &wmpos);

  // Figure out the two nearest world regions:
  // Setup worst-case defaults:
  vbest.x = WORLD_REGION_BLOCKS;
  vbest.y = WORLD_REGION_BLOCKS;
  vbest.z = 0;
  *strbest = 0;
  vsecond.x = WORLD_REGION_BLOCKS;
  vsecond.y = WORLD_REGION_BLOCKS;
  vsecond.z = 0;
  *strsecond = 0;

  *secondbest = NULL;
  *best = NULL;

  // Figure out which of our neighbors are closest:
  wmpos.x -= 1;
  wmpos.y -= 1;
  for (i = 0; i < 9; i += 1) {
    wr = neighborhood[i];
    if (wr != NULL) {
      copy_glpos(&(wr->anchor), &anchor);
      seed = prng(wr->seed + 172841);
    } else {
      compute_region_anchor(wm, &wmpos, &anchor);
      seed = prng(prng(prng(wmpos.x) + wmpos.y) + 51923);
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
        seed * 576 + 9123
      ) + 
      0.6 * sxnoise_3d(
        v.x * WM_REGION_CONTENTION_NOISE_SCALE * 2.1,
        v.y * WM_REGION_CONTENTION_NOISE_SCALE * 2.1,
        v.z * WM_REGION_CONTENTION_NOISE_SCALE * 2.1,
        seed * 577 + 9124
      )
    ) / 1.6;
    seed = prng(seed);
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
      seed
    );
    vpolar__xyz(&v, &v);
    noise = (1 + noise * WM_REGION_CONTENTION_POLAR_STRENGTH) / 2.0;
    // [
    //   0.5 - WM_REGION_CONTENTION_POLAR_STRENGTH/2,
    //   0.5 + WM_REGION_CONTENTION_POLAR_STRENGTH/2
    // ]
    str *= noise;
    if (str > *strbest) {
      vcopy_as(&vsecond, &vbest);
      *strsecond = *strbest;
      *secondbest = *best;

      vcopy_as(&vbest, &v);
      *strbest = str;
      *best = wr;
    } else if (str > *strsecond) {
      vcopy_as(&vsecond, &v);
      *strsecond = str;
      *secondbest = wr;
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

  while (!q_is_empty(open) && (max_size < 0 || size <= max_size) ) {
    // Grab the next open region:
    this = (world_region*) q_pop_element(open);
    sresult = process(SSTEP_PROCESS, this, arg);
    if (sresult == SRESULT_ABORT) {
      cleanup_queue(open);
      cleanup_map(visited);
      process(SSTEP_CLEANUP, this, arg);
      return 0;
    } else if (sresult == SRESULT_FINISHED) {
      cleanup_queue(open);
      cleanup_map(visited);
      process(SSTEP_FINISH, this, arg);
      process(SSTEP_CLEANUP, this, arg);
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
    // we hit one of the size limits: cleanup and return 0
    cleanup_queue(open);
    cleanup_map(visited);
    process(SSTEP_CLEANUP, NULL, arg);
    return 0;
  }
  // Otherwise we should flag each region as belonging to this body of water
  // and return 1.
  process(SSTEP_FINISH, NULL, arg);
  process(SSTEP_CLEANUP, NULL, arg);
  return 1;
}

void add_biome(world_region *wr, biome_category category) {
  if (wr->ecology.biome_count < WM_MAX_BIOME_OVERLAP) {
    wr->ecology.biomes[wr->ecology.biome_count] = create_biome(category);
    wr->ecology.biome_count += 1;
#ifdef DEBUG
  } else {
    printf("Warning: Biome (%d) not added due to overlap limit.\n", category);
#endif
  }
}

element_species* pick_element(
  world_map *wm,
  element_categorization constraints,
  list *exclude,
  ptrdiff_t seed
) {
  size_t i;
  list *valid = create_list();
  element_species* result = NULL;
  for (i = 0; i < l_get_length(wm->all_elements); ++i) {
    element_species *el = (element_species*) l_get_item(wm->all_elements, i);
    if (
        (
          constraints == 0
       || el_is_member_of_any(el, constraints)
        )
     && (
          exclude == NULL
       || !l_contains(exclude, (void*) el)
        )
    ) {
      l_append_element(valid, (void*) el);
    }
  }
  if (!l_is_empty(valid)) {
//#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    result = (element_species*) l_pick_random(valid, seed);
//#pragma GCC diagnostic warning "-Wint-to-pointer-cast"
  } // else result is still 0
  cleanup_list(valid);
  return result;
}
