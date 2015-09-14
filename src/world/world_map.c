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
  result->n_elements = 0;
  result->elements = NULL;
  result->tectonics = create_tectonic_sheet(
    prng(seed + 65442),
    (size_t) (2.0 * (width / TECTONIC_SHEET_SCALE)),
    (size_t) (height / TECTONIC_SHEET_SCALE)
  );
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

/*************
 * Functions *
 *************/

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
