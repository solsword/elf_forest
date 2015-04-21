#ifndef TERRAIN_H
#define TERRAIN_H

// terrain.h
// Code for determining terrain height.

#include <math.h>

#include "noise/noise.h"
#include "world/world.h"
#include "world/world_map.h"
#include "math/manifold.h"
#include "util.h"

/**************
 * Parameters *
 **************/

// Terrain building parameters:

#define TR_TECT_CROP 0.95

#define TR_PARTICLE_START_TRIES 3

#define TR_BUILD_STARTING_HEIGHT 0.92
#define TR_BUILD_HEIGHT_FALLOFF = 0.83
#define TR_BUILD_MAX_WANDER 100000
#define TR_BUILD_SLIP = 3

#define TR_BUILD_WAVE_COUNT 27
#define TR_BUILD_WAVE_SIZE 90
#define TR_BUILD_WAVE_GROWTH 20
#define TR_BUILD_WAVE_COMPOUND 4

// TODO: Fiddle with this?
#define TR_BUILD_SLUMP_MAX_SLOPE 0.01
#define TR_BUILD_SLUMP_RATE 0.2
#define TR_BUILD_SLUMP_SAVE_MIX 8.0

#define TR_BUILD_SLUMP_STEPS 5
#define TR_BUILD_FINAL_SLUMPS 1

// modulation noise scales are in noise cells/heightmap width
#define TR_BUILD_MODULATION_LARGE_SCALE 8.9
#define TR_BUILD_MODULATION_SMALL_SCALE 14.2
#define TR_BUILD_MODULATION_SMALL_STR 0.65

#define TR_BUILD_EROSION_STEPS 3
#define TR_BUILD_EROSION_FLOW_STEPS 6
#define TR_BUILD_EROSION_FLOW_SLUMP_STEPS 6
#define TR_BUILD_EROSION_FLOW_MAXSLOPE 0.2
#define TR_BUILD_EROSION_FLOW_SLUMP_RATE 0.3
#define TR_BUILD_EROSION_STR 0.2
#define TR_BUILD_EROSION_MODULATION 0.85


// Geoform parameters:
// (geoform mapping maps [0, 1] to real heights; see compute_geoforms)

#define       TR_GEOMAP_SHELF 0.38
#define       TR_GEOMAP_SHORE 0.46
#define      TR_GEOMAP_PLAINS 0.55
#define       TR_GEOMAP_HILLS 0.68
#define   TR_GEOMAP_MOUNTAINS 0.75

#define TR_GMS_DEPTHS (-4.1)
#define TR_GMC_DEPTHS 0.4
#define TR_GMS_SHELF (-1.8)
#define TR_GMC_SHELF 0.4
#define TR_GMS_PLAINS (-1.4)
#define TR_GMC_PLAINS 0.5
#define TR_GMS_HILLS 1.9
#define TR_GMC_HILLS 0.35
#define TR_GMS_HIGHLANDS (-1.3)
#define TR_GMC_HIGHLANDS 0.7
#define TR_GMS_MOUNTAINS 1.2
#define TR_GMC_MOUNTAINS 1.0

// Geoform heights:
#define        TR_HEIGHT_OCEAN_DEPTHS 1500
#define   TR_HEIGHT_CONTINENTAL_SHELF 14750
#define           TR_HEIGHT_SEA_LEVEL 15000
#define      TR_HEIGHT_COASTAL_PLAINS 15300
#define           TR_HEIGHT_HIGHLANDS 16500
#define      TR_HEIGHT_MOUNTAIN_BASES 18500
#define       TR_HEIGHT_MOUNTAIN_TOPS 27000

// Min/max height:
// TODO: Redo this...
#define TR_MIN_HEIGHT 0
#define TR_MAX_HEIGHT (\
  TR_HEIGHT_MOUNTAIN_TOPS +\
  TR_SCALE_HILLS +\
  TR_SCALE_RIDGES +\
  TR_SCALE_MOUNDS +\
  TR_SCALE_DETAILS +\
  TR_SCALE_BUMPS \
)

// Base soil parameters
// TODO: Revamp dirt distribution
#define    TR_DIRT_NOISE_SCALE 0.0004
#define  TR_DIRT_EROSION_SCALE 0.003

#define TR_DIRT_MOUNTAIN_EROSION 11
#define TR_DIRT_HILL_EROSION 5

#define TR_BASE_SOIL_DEPTH 8

#define TR_ALTITUDE_EROSION_STRENGTH 6

#define TR_SLOPE_EROSION_STRENGTH 12

// Beach height above sea level:
#define TR_BEACH_BASE_HEIGHT 7
#define TR_BEACH_HEIGHT_VAR 6
#define TR_BEACH_HEIGHT_NOISE_SCALE (1.0 / 70.0)

// Soil alt base scale
#define TR_SOIL_ALT_NOISE_SCALE (1.0 / 120.0)

// Soil alternate threshold:
#define TR_SOIL_ALT_THRESHOLD 0.5

// The strength and base scale of the noise that distorts strata boundaries:
#define TR_STRATA_FRACTION_NOISE_STRENGTH 16
#define TR_STRATA_FRACTION_NOISE_SCALE (1.0 / 40.0)

/*********
 * Enums *
 *********/

enum terrain_region_e {
  TR_REGION_DEPTHS = 0,
  TR_REGION_SHELF = 1,
  TR_REGION_PLAINS = 2,
  TR_REGION_HILLS = 3,
  TR_REGION_HIGHLANDS = 4,
  TR_REGION_MOUNTAINS = 5,
};
typedef enum terrain_region_e terrain_region;

/***********
 * Globals *
 ***********/

// An array of names for each terrain region:
extern char const * const TR_REGION_NAMES[];

/********************
 * Inline Functions *
 ********************/

static inline void trig_component(
  manifold_point *result,
  float x, float y,
  float dxx, float dxy,
  float dyx, float dyy,
  float frequency,
  ptrdiff_t *salt
) {
  manifold_point cos_part, sin_part;
  float phase;

  // cos part:
  phase = 2 * M_PI * ptrf(*salt);
  *salt = prng(*salt);
  cos_part.z = cosf(phase + x * frequency);
  cos_part.dx = (1 + dxx) * frequency * (-sinf(phase + x * frequency));
  cos_part.dy = dxy * frequency * (-sinf(phase + x * frequency));

  // sin part:
  phase = 2 * M_PI * ptrf(*salt);
  *salt = prng(*salt);
  sin_part.z = sinf(phase + y * frequency);
  sin_part.dx = dyx * frequency * cosf(phase + y * frequency);
  sin_part.dy = (1 + dyy) * frequency * cosf(phase + y * frequency);

  // result:
  mani_copy(result, &cos_part);
  mani_multiply(result, &sin_part);
}

static inline void simplex_component(
  manifold_point *result,
  float x, float y,
  float dxx, float dxy,
  float dyx, float dyy,
  float frequency,
  ptrdiff_t *salt
) {
  mani_get_sxnoise_2d(
    result,
    x * frequency,
    y * frequency,
    *salt
  ); *salt = prng(*salt);
  // DEBUG:
  // printf("simp-comp-base x y freq: %.3f %.3f %.8f\n", x, y, frequency);
  // printf("simp-comp-base salt: %td\n", *salt);
  // printf(
  //   "simp-comp-base: %.3f :: %.4f, %.4f\n",
  //   result->z,
  //   result->dx,
  //   result->dy
  // );
  mani_compose_double_simple(
    result,
    dxx * frequency,
    dxy * frequency,
    dyx * frequency,
    dyy * frequency
  );
}

static inline void worley_component(
  manifold_point *result,
  float x, float y,
  float dxx, float dxy,
  float dyx, float dyy,
  float frequency,
  ptrdiff_t *salt,
  uint32_t flags
) {
  mani_get_wrnoise_2d(
    result,
    x * frequency,
    y * frequency,
    *salt,
    0, 0,
    flags
  ); *salt = prng(*salt);
  mani_compose_double_simple(
    result,
    dxx * frequency, dxy * frequency,
    dyx * frequency, dyy * frequency
  );
}

static inline void get_standard_distortion(
  gl_pos_t x, gl_pos_t y, ptrdiff_t *salt,
  float frequency,
  float scale,
  manifold_point *result_x,
  manifold_point *result_y
) {
  // x distortion
  simplex_component(
    result_x,
    x, y,
    1, 0,
    0, 1,
    frequency,
    salt
  );
  mani_scale_const(result_x, scale);
  // y distortion
  simplex_component(
    result_y,
    x, y,
    1, 0,
    0, 1,
    frequency,
    salt
  );
  mani_scale_const(result_y, scale);
}

static inline void geomap_segment(
  manifold_point *result, manifold_point *base, manifold_point *interp,
  float sm_str, float sm_ctr,
  float lower, float upper,
  float lower_height, float upper_height
) {
  mani_copy(interp, base);
  // DEBUG:
  // printf("Geomap segment :: base %.2f\n", base->z);
  // printf("Geomap segment :: interp %.2f\n", interp->z);
  mani_offset_const(interp, -lower);
  // printf("Geomap segment :: offset :: interp %.2f\n", interp->z);
  mani_scale_const(interp, 1.0/(upper - lower));
  // printf("Geomap segment :: scale :: interp %.2f\n", interp->z);
  mani_smooth(interp, sm_str, sm_ctr);
  // printf("Geomap segment :: smooth :: interp %.2f\n", interp->z);
  mani_copy(result, interp);
  // printf("Geomap segment :: copy :: result %.2f\n", result->z);
  mani_scale_const(
    result,
    (upper_height - lower_height)
  );
  // printf("Geomap segment :: scale :: result %.2f\n", result->z);
  mani_offset_const(result, lower_height);
  // printf("Geomap segment :: offset :: result %.2f\n", result->z);
  // printf("\n\n\n");
}

// Remaps the given value (on [0, 1]) to account for the shape and height of
// the overall height profile.
static inline void geomap(
  manifold_point *result,
  manifold_point *base,
  terrain_region* region,
  manifold_point* interp
) {
  if (base->z < TR_GEOMAP_SHELF) { // deep ocean
    *region = TR_REGION_DEPTHS;
    geomap_segment(
      result, base, interp,
      TR_GMS_DEPTHS, TR_GMC_DEPTHS,
      0, TR_GEOMAP_SHELF,
      TR_HEIGHT_OCEAN_DEPTHS, TR_HEIGHT_CONTINENTAL_SHELF
    );
  } else if (base->z < TR_GEOMAP_SHORE) { // continental shelf
    *region = TR_REGION_SHELF;
    geomap_segment(
      result, base, interp,
      TR_GMS_SHELF, TR_GMC_SHELF,
      TR_GEOMAP_SHELF, TR_GEOMAP_SHORE,
      TR_HEIGHT_CONTINENTAL_SHELF, TR_HEIGHT_SEA_LEVEL
    );
    // TODO: Sea cliffs?
  } else if (base->z < TR_GEOMAP_PLAINS) { // coastal plain
    *region = TR_REGION_PLAINS;
    geomap_segment(
      result, base, interp,
      TR_GMS_PLAINS, TR_GMC_PLAINS,
      TR_GEOMAP_SHORE, TR_GEOMAP_PLAINS,
      TR_HEIGHT_SEA_LEVEL, TR_HEIGHT_COASTAL_PLAINS
    );
  } else if (base->z < TR_GEOMAP_HILLS) { // hills
    *region = TR_REGION_HILLS;
    geomap_segment(
      result, base, interp,
      TR_GMS_HILLS, TR_GMC_HILLS,
      TR_GEOMAP_PLAINS, TR_GEOMAP_HILLS,
      TR_HEIGHT_COASTAL_PLAINS, TR_HEIGHT_HIGHLANDS
    );
  } else if (base->z < TR_GEOMAP_MOUNTAINS) { // highlands
    *region = TR_REGION_HIGHLANDS;
    geomap_segment(
      result, base, interp,
      TR_GMS_HIGHLANDS, TR_GMC_HIGHLANDS,
      TR_GEOMAP_HILLS, TR_GEOMAP_MOUNTAINS,
      TR_HEIGHT_HIGHLANDS, TR_HEIGHT_MOUNTAIN_BASES
    );
  } else { // mountains
    *region = TR_REGION_MOUNTAINS;
    geomap_segment(
      result, base, interp,
      TR_GMS_MOUNTAINS, TR_GMC_MOUNTAINS,
      TR_GEOMAP_MOUNTAINS, 1.0,
      TR_HEIGHT_MOUNTAIN_BASES, TR_HEIGHT_MOUNTAIN_TOPS
    );
  }
}

/*************
 * Functions *
 *************/

// Performs initial setup for terrain generation.
void setup_terrain_gen();

// Adds a particle at the given height to the world map, letting it wander
// randomly until it stops next to a higher region (or until it has taken
// max_steps steps). The first slip times it would settle, it keeps going.
void run_particle(
  heightmap *hm,
  float height,
  size_t slip,
  size_t max_steps,
  ptrdiff_t seed
);

// Generates topology for a world using the world's tectonics as a base.
void generate_topology(world_map *wm);

// Computes the terrain height at the given region position in blocks, and
// writes out the gross, rock and dirt heights at that location.
void compute_terrain_height(
  global_pos *pos,
  manifold_point *r_gross,
  manifold_point *r_rocks,
  manifold_point *r_dirt
);

// Figures out the dirt height at the given location and writes it into the
// result parameter.
// TODO: Something about this
void compute_dirt_height(
  global_pos *pos, ptrdiff_t *salt,
  manifold_point *rocks_height,
  manifold_point *mountains, manifold_point *hills,
  manifold_point *details, manifold_point *bumps,
  manifold_point *result
);

// Computes the terrain region and interpolation values at the given position.
void geoform_info(global_pos *pos, terrain_region* region, float* tr_interp);

// Computes the cell contents at the given position based on the terrain.
void terrain_cell(
  world_map *wm,
  world_region* neighborhood[],
  global_pos* glpos,
  cell* result
);

// Computes a stone cell from within the base strata layers.
void stone_cell(
  world_map *wm, global_pos *glpos,
  float h, float ceiling,
  world_region *best, world_region *secondbest, float strbest, float strsecond,
  cell *result
);

// Computes a dirt cell from the dirt layer.
void dirt_cell(
  world_map *wm, global_pos *glpos,
  float h, float elev,
  world_region *wr,
  cell *result
);

#endif // ifndef TERRAIN_H
