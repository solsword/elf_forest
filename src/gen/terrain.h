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

// Basic frequencies:
#define   TR_FREQUENCY_CONTINENTS 0.000001 // 15625-chunk ~= 330 km features
#define    TR_FREQUENCY_PGEOFORMS 0.000002 // ???
#define    TR_FREQUENCY_SGEOFORMS 0.000003 // 7812.5-chunk ~= 170 km features
#define    TR_FREQUENCY_GEODETAIL 0.000015 // 1562.5-chunk ~= 33 km features
#define    TR_FREQUENCY_MOUNTAINS 0.0001 // 312.5-chunk ~= 6.7 km features
#define        TR_FREQUENCY_HILLS 0.0005 // 62.5-chunk ~= 1.3 km features
#define       TR_FREQUENCY_RIDGES 0.003 // ~10-chunk ~= 220 m features
#define       TR_FREQUENCY_MOUNDS 0.01 // ~6-chunk ~= 64 m features
#define      TR_FREQUENCY_DETAILS 0.05 // ~20-cell ~= 13 m features
#define        TR_FREQUENCY_BUMPS 0.25 // ~4-cell ~= 2.5 m variance

// Distortion frequencies:
#define   TR_DFQ_CONTINENTS 0.0000005 // 1/4 of base
#define    TR_DFQ_PGEOFORMS 0.000000625 // 1/4 of base
#define    TR_DFQ_SGEOFORMS 0.0000008 // 1/5 of base
#define    TR_DFQ_GEODETAIL 0.000004 // 1/5 of base
#define    TR_DFQ_MOUNTAINS 0.00002 // 1/5 of base
#define        TR_DFQ_HILLS 0.0002 // 2/5 of base
#define       TR_DFQ_RIDGES 0.0006 // 1/5 of base
#define       TR_DFQ_MOUNDS 0.004 // 2/5 of base
#define      TR_DFQ_DETAILS 0.02 // 2/5 of base
#define        TR_DFQ_BUMPS 0.05 // 1/5 of base

// Distortion strengths:
#define   TR_DS_CONTINENTS 1500000.0 // ~1.5 periods
#define    TR_DS_PGEOFORMS 800000.0 // ???
#define    TR_DS_SGEOFORMS 400000.0 // 1.6 periods
#define    TR_DS_GEODETAIL 37500.0 // ~3/4 period
#define    TR_DS_MOUNTAINS 5000.0 // ~1/2 period
#define        TR_DS_HILLS 2000.0 // ~1 period
#define       TR_DS_RIDGES 50.0 // ~1/4 period
#define       TR_DS_MOUNDS 30.0 // ~1/2 period
#define      TR_DS_DETAILS 10.0 // ~1/2 period
#define        TR_DS_BUMPS 1.5 // ~3/8 period


// Geoform parameters:

// Noise->geoform mapping (see compute_geoforms):
/*
#define       TR_GEOMAP_SHELF 0.38
#define       TR_GEOMAP_SHORE 0.47
#define      TR_GEOMAP_PLAINS 0.58
#define       TR_GEOMAP_HILLS 0.73
#define   TR_GEOMAP_MOUNTAINS 0.80
*/
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

// Low-frequency terrain noise contributions:
#define   TR_SHARE_CONTINENTS 2.8
#define    TR_SHARE_PGEOFORMS 1.7
#define    TR_SHARE_SGEOFORMS 1.6
#define   TR_SHARE_GEODETAILS 1.2

#define TR_TOTAL_SHARES (\
  TR_SHARE_CONTINENTS +\
  TR_SHARE_PGEOFORMS +\
  TR_SHARE_SGEOFORMS +\
  TR_SHARE_GEODETAILS\
)

// High-frequency terrain noise contributions:
#define   TR_SCALE_MOUNTAINS 6000
#define       TR_SCALE_HILLS 900
#define      TR_SCALE_RIDGES 60
#define      TR_SCALE_MOUNDS 15
#define     TR_SCALE_DETAILS 6
#define       TR_SCALE_BUMPS 0.7

// Min/max height:
// TODO: is this correct?
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

// A base salt for terrain noise:
extern ptrdiff_t TR_NOISE_SALT;

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

static inline void get_noise(
  gl_pos_t x, gl_pos_t y, ptrdiff_t *salt,
  manifold_point *continents,
  manifold_point *primary_geoforms, manifold_point *secondary_geoforms,
  manifold_point *geodetails, manifold_point *mountains,
  manifold_point *hills, manifold_point *ridges,
  manifold_point *mounds, manifold_point *details, manifold_point *bumps
) {
  manifold_point temp; // temporary manifold point for intermediates
  manifold_point dst_x, dst_y; //distortion values

  manifold_point scaleinterp;
  manifold_point geodetailed, mountainous, hilly, ridged;
  manifold_point mounded, detailed, bumpy;

  // pre-salt our x/y values just a bit to avoid the noise=0 at origin problem.
  *salt = prng(*salt);
  x += *salt % 439;
  y += *salt % 752;
  *salt = prng(*salt);

  // continents
  // ----------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_CONTINENTS,
    TR_DS_CONTINENTS,
    &dst_x, &dst_y
  );
    // interpolant between larger and smaller scales:
  simplex_component(
    &scaleinterp,
    x, y,
    1, 0,
    0, 1,
    TR_DFQ_CONTINENTS,
    salt
  );
  // DEBUG:
  // printf("cont-scint-simplex: %.3f\n", scaleinterp.z);
  mani_scale_const(&scaleinterp, 0.3);
  mani_offset_const(&scaleinterp, 0.7);
  // DEBUG:
  // printf("cont-scint: %.3f\n", scaleinterp.z);

    // large-scale part of continents
  trig_component(
    &temp,
    x + dst_x.z, y + dst_y.z,
    1 + dst_x.dx, dst_x.dy,
    dst_y.dx, 1 + dst_y.dy,
    TR_FREQUENCY_CONTINENTS,
    salt
  );
  /*
  printf(
    "conts-trig: %td, %td :: %.3f, %.3f\n",
    x, y,
    dst_x.z, dst_y.z
  );
  */

  mani_copy(continents, &scaleinterp);
  mani_multiply(continents, &temp);

  // DEBUG:
  // printf("cont-base: %.3f\n", continents->z);

    // small-scale part of continents
  trig_component(
    &temp,
    x + 0.8 * dst_y.z, y - 0.8 * dst_x.z,
    1 + 0.8 * dst_y.dx, 0.8 * dst_y.dy,
    0.8 * dst_x.dx, 1 - 0.8 * dst_x.dy,
    TR_FREQUENCY_CONTINENTS * 1.6,
    salt
  );

  // we'll use the small cos part to accumulate the other half of the
  // continents manifold and then add it back in:
  mani_scale_const(&scaleinterp, -1);
  mani_offset_const(&scaleinterp, 1);
  mani_multiply(&temp, &scaleinterp);

  mani_add(continents, &temp);

  // DEBUG:
  // printf("cont-full: %.3f\n", continents->z);

  // a bit of smoothing at the end:
  mani_offset_const(continents, 1);
  mani_scale_const(continents, 0.5);

  mani_smooth(continents, -0.8, 0.35);

  mani_scale_const(continents, 2.0);
  mani_offset_const(continents, -1);
    // continents should be in [-1, 1].

  // DEBUG:
  // printf("cont-smooth: %.3f\n", continents->z);

  // geodetails fading
  // -----------------
  simplex_component(
    &geodetailed,
    x - dst_y.z,
    y - dst_x.z,
    1 - dst_y.dx, -dst_y.dy,
    -dst_x.dx, 1 - dst_x.dy,
    TR_FREQUENCY_CONTINENTS * 1.7,
    salt
  );
  mani_offset_const(&geodetailed, 1);
  mani_scale_const(&geodetailed, 0.5);
    // geodetailed should be in [0, 1].

  // primary geoforms (mountain bones)
  // ---------------------------------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_PGEOFORMS,
    TR_DS_PGEOFORMS,
    &dst_x, &dst_y
  );

    // trig component
  trig_component(
    primary_geoforms,
    x + 0.7 * dst_y.z, y - 0.7 * dst_x.z,
    1 + 0.7 * dst_y.dx, 0.7 * dst_y.dy,
    0.7 * dst_x.dx, 1 - 0.7 * dst_x.dy,
    TR_FREQUENCY_PGEOFORMS * 1.4,
    salt
  );

    // simplex noise component
  simplex_component(
    &temp,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx, dst_x.dy,
    dst_y.dx, 1 + dst_y.dy,
    TR_FREQUENCY_PGEOFORMS,
    salt
  );
  mani_add(primary_geoforms, &temp);

    // worley noise component
  worley_component(
    &temp,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx, -dst_y.dy,
    dst_x.dx, 1 + dst_x.dy,
    TR_FREQUENCY_PGEOFORMS * 0.8,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  mani_add(primary_geoforms, &temp);

  mani_scale_const(primary_geoforms, (1.0/3.0));
  mani_offset_const(primary_geoforms, 1);
  mani_scale_const(primary_geoforms, 0.5);
  mani_smooth(primary_geoforms, 3.1, 0.35);
  mani_multiply(primary_geoforms, primary_geoforms);
  mani_scale_const(primary_geoforms, 2.0);
  mani_offset_const(primary_geoforms, -1);
    // primary_geoforms should be in [-1, 1].

  // mountains fading
  // ----------------
  if (primary_geoforms->z < 0) {
    mountainous.z = 0;
    mountainous.dx = 0;
    mountainous.dy = 0;
  } else {
    mani_copy(&mountainous, primary_geoforms);
  }
  mani_smooth(&mountainous, 3.6, 0.7);
  simplex_component(
    &temp,
    x + dst_y.z,
    y - dst_x.z,
    1 + dst_y.dx, dst_y.dy,
    -dst_x.dx, 1 - dst_x.dy,
    TR_FREQUENCY_SGEOFORMS * 0.7,
    salt
  );
  mani_scale_const(&temp, 0.2);
  mani_add(&mountainous, &temp);

  mani_scale_const(&mountainous, 1.0/1.2);
  if (mountainous.z < 0) {
    mountainous.z = 0;
    mountainous.dx = 0;
    mountainous.dy = 0;
  }
    // mountainous should be in [0, 1].

  // hills fading
  // ------------
  if (primary_geoforms->z < 0) {
    hilly.z = 0;
    hilly.dx = 0;
    hilly.dy = 0;
  } else {
    mani_copy(&hilly, primary_geoforms);
  }
  mani_smooth(&hilly, 2.4, 0.1);
  simplex_component(
    &temp,
    x + dst_x.z * 0.6,
    y - dst_y.z * 0.6,
    1 + dst_x.dx * 0.6, dst_x.dy * 0.6,
    -dst_y.dx * 0.6, 1 - dst_y.dy * 0.6,
    TR_FREQUENCY_SGEOFORMS * 0.8,
    salt
  );
  mani_add(&hilly, &temp);
  simplex_component(
    &temp,
    x - dst_y.z * 0.4,
    y + dst_x.z * 0.4,
    1 - dst_y.dx * 0.4, -dst_y.dy * 0.4,
    dst_x.dx * 0.4, 1 + dst_x.dy * 0.4,
    TR_FREQUENCY_GEODETAIL * 0.3,
    salt
  );
  mani_scale_const(&temp, 0.5);
  mani_add(&hilly, &temp);
  mani_scale_const(&hilly, 1.0/2.5);
  if (hilly.z < 0) {
    hilly.z = 0;
    hilly.dx = 0;
    hilly.dy = 0;
  }
    // hilly should be in [0, 1].

  // ridges fading
  // -------------
  if (primary_geoforms->z < -0.05) {
    ridged.z = 0;
    ridged.dx = 0;
    ridged.dy = 0;
  } else {
    mani_copy(&ridged, primary_geoforms);
    mani_offset_const(&ridged, 0.05);
  }
  mani_smooth(&ridged, 2.9, 0.1);
  simplex_component(
    &temp,
    x + dst_x.z*0.4,
    y - dst_y.z*0.4,
    1 + dst_x.dx*0.4, dst_x.dy * 0.4,
    -dst_y.dx * 0.4, 1 - dst_y.dy * 0.4,
    TR_FREQUENCY_SGEOFORMS * 0.9,
    salt
  );
  mani_add(&ridged, &temp);

  simplex_component(
    &temp,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx, -dst_y.dy,
    dst_x.dx, 1 + dst_x.dy,
    TR_FREQUENCY_GEODETAIL * 0.4,
    salt
  );
  mani_scale_const(&ridged, 0.5);
  mani_add(&ridged, &temp);

  mani_scale_const(&ridged, 0.9/2.5);
  mani_offset_const(&ridged, 0.1);
    // ridged should be in [0.1, 1].

  // secondary geoforms (not used for roughness vaules)
  // --------------------------------------------------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_SGEOFORMS,
    TR_DS_SGEOFORMS,
    &dst_x, &dst_y
  );

    // simplex component
  simplex_component(
    secondary_geoforms,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx, dst_x.dy,
    dst_y.dx, 1 + dst_y.dy,
    TR_FREQUENCY_SGEOFORMS,
    salt
  );

    // worley component
  worley_component(
    &temp,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx, -dst_y.dy,
    dst_x.dx, 1 + dst_x.dy,
    TR_FREQUENCY_SGEOFORMS,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  mani_add(secondary_geoforms, &temp);
  mani_scale_const(secondary_geoforms, 0.5);
    // secondary_geoforms should be in [-1, 1].

  // geodetails
  // ----------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_GEODETAIL,
    TR_DS_GEODETAIL,
    &dst_x, &dst_y
  );

    // simplex component
  simplex_component(
    geodetails,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx, dst_x.dy,
    dst_y.dx, 1 + dst_y.dy,
    TR_FREQUENCY_GEODETAIL,
    salt
  );
  mani_scale_const(geodetails, 2.0);

    // worley component
  worley_component(
    &temp,
    x + dst_y.z,
    y - dst_x.z,
    1 + dst_y.dx, dst_y.dy,
    -dst_x.dx, 1 - dst_x.dy,
    TR_FREQUENCY_GEODETAIL,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  mani_add(geodetails, &temp);

  mani_scale_const(geodetails, 1.0/3.0);
  mani_multiply(geodetails, &geodetailed);
    // geodetails should be in [-1, 1].

  // mountains
  // ---------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_MOUNTAINS,
    TR_DS_MOUNTAINS,
    &dst_x, &dst_y
  );

    //simplex component
  simplex_component(
    mountains,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx, dst_x.dy,
    dst_y.dx, 1 + dst_y.dy,
    TR_FREQUENCY_MOUNTAINS,
    salt
  );

  mani_offset_const(mountains, 1);
  mani_scale_const(mountains, 0.5);

    // first worley component
  worley_component(
    &temp,
    x + dst_y.z,
    y + dst_x.z,
    1 + dst_y.dx, dst_y.dy,
    dst_x.dx, 1 + dst_x.dy,
    TR_FREQUENCY_MOUNTAINS,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  mani_smooth(&temp, 2.4, 0.5);
  mani_scale_const(&temp, 2.0);
  mani_add(mountains, &temp);

    // second worley component
  worley_component(
    &temp,
    x - dst_x.z,
    y - dst_y.z,
    1 - dst_x.dx, -dst_x.dy,
    -dst_y.dx, 1 - dst_y.dy,
    TR_FREQUENCY_MOUNTAINS,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST | WORLEY_FLAG_SMOOTH_SIDES
  );
  mani_add(mountains, &temp);

  mani_scale_const(mountains, 0.25);
  mani_multiply(mountains, &mountainous);
    // mountains should be in [0, 1].

  // mounds fading
  // -------------
  simplex_component(
    &mounded,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx, -dst_y.dy,
    dst_x.dx, 1 + dst_x.dy,
    TR_FREQUENCY_MOUNTAINS * 0.6,
    salt
  );
  mani_offset_const(&mounded, 1);
  mani_scale_const(&mounded, 0.25);
  mani_offset_const(&mounded, 0.5);
    // mounded should be in [0.5, 1].

  // hills
  // -----
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_HILLS,
    TR_DS_HILLS,
    &dst_x, &dst_y
  );

  simplex_component(
    hills,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx, dst_x.dy,
    dst_y.dx, 1 + dst_y.dy,
    TR_FREQUENCY_HILLS,
    salt
  );
  mani_multiply(hills, &hilly);
    // hills should be in [-1, 1].

  // detail fading
  // -------------
  simplex_component(
    &detailed,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx, -dst_y.dy,
    dst_x.dx, 1 + dst_x.dy,
    TR_FREQUENCY_HILLS * 0.5,
    salt
  );
  mani_offset_const(&detailed, 1);
  mani_scale_const(&detailed, 0.5 * 0.8);
  mani_offset_const(&detailed, 0.2);
    // detailed should be in [0.2, 1].

  // ridges
  // ------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_RIDGES,
    TR_DS_RIDGES,
    &dst_x, &dst_y
  );

  worley_component(
    ridges,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx, dst_x.dy,
    dst_y.dx, 1 + dst_y.dy,
    TR_FREQUENCY_RIDGES,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST
  );

  worley_component(
    &temp,
    x + dst_y.z,
    y - dst_x.z,
    1 + dst_y.dx, dst_y.dy,
    -dst_x.dx, 1 - dst_x.dy,
    TR_FREQUENCY_RIDGES * 1.8,
    salt,
    WORLEY_FLAG_INCLUDE_NEXTBEST
  );
  mani_scale_const(&temp, 0.5);
  mani_add(ridges, &temp);
  mani_scale_const(ridges, 1.0/1.5);
  mani_multiply(ridges, &ridged);
    // ridges should be in [0, 1].

  // bumps fading
  // ------------
  simplex_component(
    &bumpy,
    x - dst_y.z,
    y + dst_x.z,
    1 - dst_y.dx, -dst_y.dy,
    dst_x.dx, 1 + dst_x.dy,
    TR_FREQUENCY_RIDGES * 0.2,
    salt
  );
  mani_offset_const(&bumpy, 1);
  mani_scale_const(&bumpy, 0.5 * 0.6);
  mani_offset_const(&bumpy, 0.4);
    // bumpy should be in [0.4, 1].

  // mounds
  // ------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_MOUNDS,
    TR_DS_MOUNDS,
    &dst_x, &dst_y
  );

  simplex_component(
    mounds,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx, dst_x.dy,
    dst_y.dx, 1 + dst_y.dy,
    TR_FREQUENCY_MOUNDS,
    salt
  );
  mani_multiply(mounds, &mounded);
    // mounds should be in [-1, 1].

  // details
  // -------
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_DETAILS,
    TR_DS_DETAILS,
    &dst_x, &dst_y
  );

  simplex_component(
    details,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx, dst_x.dy,
    dst_y.dx, 1 + dst_y.dy,
    TR_FREQUENCY_DETAILS,
    salt
  );
  mani_multiply(details, &detailed);
    // details should be in [-1, 1].

  // bumps
  get_standard_distortion(
    x, y, salt,
    TR_DFQ_BUMPS,
    TR_DS_BUMPS,
    &dst_x, &dst_y
  );

  simplex_component(
    bumps,
    x + dst_x.z,
    y + dst_y.z,
    1 + dst_x.dx, dst_x.dy,
    dst_y.dx, 1 + dst_y.dy,
    TR_FREQUENCY_BUMPS,
    salt
  );
  mani_multiply(bumps, &bumpy);
    // bumps should be in [-1, 1].
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

// Computes basic geoform information to be used by compute_terrain_height.
static inline void compute_base_geoforms(
  global_pos* pos, ptrdiff_t *salt,
  manifold_point *continents,
  manifold_point *primary_geoforms, manifold_point *secondary_geoforms,
  manifold_point *geodetails, manifold_point *mountains,
  manifold_point *hills, manifold_point *ridges,
  manifold_point *mounds, manifold_point *details, manifold_point *bumps,
  manifold_point* base,
  terrain_region* region,
  manifold_point* tr_interp,
  manifold_point* height
) {
  manifold_point temp;

  // make some noise:
  get_noise(
    pos->x, pos->y, salt,
    continents, primary_geoforms, secondary_geoforms,
    geodetails, mountains,
    hills, ridges, mounds, details, bumps
  );
  *salt = prng(*salt);

  // DEBUG: print noise
  /*
  printf("Base geoforms noise!\n");
  printf(
    "continents: %.3f :: %.2f, %.2f\n",
    continents->z,
    continents->dx,
    continents->dy
  );
  printf(
    "primary_geoforms: %.3f :: %.2f, %.2f\n",
    primary_geoforms->z,
    primary_geoforms->dx,
    primary_geoforms->dy
  );
  printf(
    "secondary_geoforms: %.3f :: %.2f, %.2f\n",
    secondary_geoforms->z,
    secondary_geoforms->dx,
    secondary_geoforms->dy
  );
  printf(
    "geodetails: %.3f :: %.2f, %.2f\n",
    geodetails->z,
    geodetails->dx,
    geodetails->dy
  );
  printf(
    "mountains: %.3f :: %.2f, %.2f\n",
    mountains->z,
    mountains->dx,
    mountains->dy
  );
  printf(
    "hills: %.3f :: %.2f, %.2f\n",
    hills->z,
    hills->dx,
    hills->dy
  );
  printf(
    "ridges: %.3f :: %.2f, %.2f\n",
    ridges->z,
    ridges->dx,
    ridges->dy
  );
  printf(
    "mounds: %.3f :: %.2f, %.2f\n",
    mounds->z,
    mounds->dx,
    mounds->dy
  );
  printf(
    "details: %.3f :: %.2f, %.2f\n",
    details->z,
    details->dx,
    details->dy
  );
  printf(
    "bumps: %.3f :: %.2f, %.2f\n",
    bumps->z,
    bumps->dx,
    bumps->dy
  );
  printf("\n\n\n");
  // */

  base->z = 0;
  base->dx = 0;
  base->dy = 0;

  mani_copy(&temp, continents);
  mani_scale_const(&temp, TR_SHARE_CONTINENTS);
  mani_add(base, &temp);

  // DEBUG:
  // printf("base-c: %.8f :: %.8f %.8f\n", base->z, base->dx, base->dy);

  //* DEBUG:
  mani_copy(&temp, primary_geoforms);
  mani_scale_const(&temp, TR_SHARE_PGEOFORMS);
  mani_add(base, &temp);

  mani_copy(&temp, secondary_geoforms);
  mani_scale_const(&temp, TR_SHARE_SGEOFORMS);
  mani_add(base, &temp);

  mani_copy(&temp, geodetails);
  mani_scale_const(&temp, TR_SHARE_GEODETAILS);
  mani_add(base, &temp);

  mani_scale_const(base, 1.0/(float) (TR_TOTAL_SHARES));
  // */

  mani_offset_const(base, 1);
  mani_scale_const(base, 0.4999999); // squash into [0, 1]

  // spread things out:
  mani_smooth(base, 0.8, TR_GEOMAP_MOUNTAINS);
  mani_smooth(base, 1.2, TR_GEOMAP_SHORE);
  //mani_smooth(base, -1.4, TR_GEOMAP_SHELF);

  // DEBUG:
  // printf("base: %.8f :: %.8f %.8f\n", base->z, base->dx, base->dy);

  // remap everything:
  geomap(height, base, region, tr_interp);
}

/*************
 * Functions *
 *************/

// Performs initial setup for terrain generation.
void setup_terrain_gen(ptrdiff_t seed);

// Computes the terrain height at the given region position in blocks, and
// writes out the gross (large-scale), rock and dirt heights at that location.
void compute_terrain_height(
  global_pos *pos,
  manifold_point *r_gross,
  manifold_point *r_rocks,
  manifold_point *r_dirt
);

// Alters the various detail values according to the terrain region
// classification.
void alter_terrain_values(
  global_pos *pos, ptrdiff_t *salt,
  terrain_region region, manifold_point *tr_interp,
  manifold_point *mountains, manifold_point *hills, manifold_point *ridges,
  manifold_point *mounds, manifold_point *details, manifold_point *bumps
);

// Figures out the dirt height at the given location and writes it into the
// result parameter.
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
